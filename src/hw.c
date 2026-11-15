/* LANIDE persistent polling backend. NIC SRAM belongs to the interface while
 * attached; close stops engines, not a diagnostic snapshot/restore operation.
 * Stage5 proved the mapping/metadata/ARP byte contracts on real hardware.
 * No reset, PHY, clock, EIE or board-IRQ writes. */
#ifdef LANIDE6_HOST
#include <errno.h>
#include <stdio.h>
extern unsigned short test_read16(char *);
extern void test_write16(char *,unsigned short);
#define R16(p) test_read16(p)
#define W16(p,v) test_write16(p,(unsigned short)(v))
#define DDI_CPUTYPE test_cpu
extern long test_cpu;
#else
#include "sys/types.h"
#include "sys/errno.h"
#include "sys/amixddi.h"
extern int printf();
#define R16(p) (*(volatile unsigned short *)(p))
#define W16(p,v) do {unsigned short w_=(unsigned short)(v); \
 __asm__ __volatile__("mov.w %1,(%0)" : : "a"(p),"d"(w_):"memory");}while(0)
#endif
#include "hw.h"
#define BASE 0x40000000UL
#define ESTAT 0x7e1a
#define EIR 0x7e1c
#define ECON1 0x7e1e
#define ECON2 0x7e6e
#define EIE 0x7e72
#define RXST 0x1000U
#define RXEND 0x6000U
extern int autocon(),bprobe();
extern char *dev_kvmap();
extern void dev_kvunmap();
unsigned char lanide_hw_mac[6];
int lanide_hw_running,lanide_hw_failed,lanide_hw_txbusy,lanide_hw_sim;
unsigned long lanide_hw_txok,lanide_hw_txerr,lanide_hw_rxok,lanide_hw_rxerr,lanide_hw_timeouts;
static char *regs,*ram;
static unsigned int cursor,txage;
static unsigned short duplex;
static unsigned char simframe[1514];
static unsigned int simlen;
static unsigned short reg(o) unsigned int o;{return R16(regs+o-0x7000);}
static void put(o,v) unsigned int o,v;{W16(regs+o-0x7000,v);}
static unsigned int next(p,n) unsigned int p,n;
{p+=n;return p>=RXEND?p-(RXEND-RXST):p;}
static unsigned int le(v) unsigned int v;{return((v&255)<<8)|(v>>8);}
static void duplex_set(st) unsigned int st;
{
 unsigned int v=reg(0x7e42);
 put(0x7e42,(v&~0xe9U)|0xb0U|((st&0x400)?1:0));
 put(0x7e44,(st&0x400)?0x15:0x12);duplex=st&0x400;
}
int lanide_hw_attach(sim) int sim;
{
 long b=0,z=0;unsigned short w;unsigned int i;int any=0;
 if(regs||ram||lanide_hw_running)return EBUSY;
 lanide_hw_sim=sim;lanide_hw_failed=lanide_hw_txbusy=0;simlen=0;
 lanide_hw_txok=lanide_hw_txerr=lanide_hw_rxok=lanide_hw_rxerr=lanide_hw_timeouts=0;
 if(sim){
  static unsigned char m[6]={2,0x4c,0x41,0x4e,0,1};
  for(i=0;i<6;i++)lanide_hw_mac[i]=m[i];
  printf("lanide: SIMULATION ONLY, no MMIO; not hardware acceptance\n");return 0;
 }
 if(DDI_CPUTYPE!=40&&DDI_CPUTYPE!=60)return ENODEV;
 if(!autocon(0x0a1c007bL,0,&b,&z)){printf("lanide: absent; NO MMIO\n");return ENXIO;}
 if((unsigned long)b!=BASE||(unsigned long)z!=0x10000UL)return EINVAL;
 regs=dev_kvmap(BASE+0x7000,0x1000UL,0x40,1);if(!regs)return ENOMEM;
 /* Process-context preflight, not a full word bus-fault exception wrapper. */
 {
  static unsigned short ro[]={0x7e00,0x7e02,0x7e04,0x7e06,0x7e08,0x7e12,
   ESTAT,EIR,ECON1,0x7e34,0x7e40,0x7e42,0x7e44,0x7e4a,
   0x7e60,0x7e62,0x7e64,ECON2,EIE};
  for(i=0;i<sizeof(ro)/sizeof(ro[0]);i++)
   if(bprobe(regs+ro[i]-0x7000,-1)||bprobe(regs+ro[i]-0x7000+1,-1)){
    lanide_hw_detach();return EIO;
   }
 }
 ram=dev_kvmap(BASE,0x6000UL,0x40,1);if(!ram){lanide_hw_detach();return ENOMEM;}
 for(i=0;i<0x6000;i++)if(bprobe(ram+i,-1)){lanide_hw_detach();return EIO;}
 if((reg(EIE)&0x7fef)||(reg(ECON1)&0xc8e3)||(reg(ESTAT)&0x20ff)||(reg(ECON2)&0xf0)||(reg(0x7e40)&0x10)){
  lanide_hw_detach();return EBUSY;
 }
 for(i=0;i<3;i++){w=reg(0x7e64-2*i);lanide_hw_mac[2*i]=w&255;lanide_hw_mac[2*i+1]=w>>8;any|=w;}
 if(!any||(lanide_hw_mac[0]&1)){lanide_hw_detach();return EINVAL;}
 return 0;
}
void lanide_hw_detach()
{
 if(ram)dev_kvunmap(ram,0x6000UL);
 if(regs)dev_kvunmap(regs,0x1000UL);
 regs=ram=0;lanide_hw_sim=0;
}
int lanide_hw_start()
{
 unsigned int st,i;
 if(lanide_hw_failed||lanide_hw_running)return lanide_hw_running?0:EIO;
 if(lanide_hw_sim){simlen=0;lanide_hw_running=1;return 0;}
 if(!regs||!ram)return ENXIO;
 st=reg(ESTAT);
 if((st&0x1a00)!=0x1a00||!(reg(ECON2)&0x8000))return ENODEV;
 if((reg(EIE)&0x7fef)||(reg(ECON1)&0xc8e3)||(st&0x20ff)||(reg(ECON2)&0xf0)||(reg(0x7e40)&0x10))return EBUSY;
 /* No live engine; clear only our RX/TX status before taking ownership. */
 put(EIR+0x180,15);
 cursor=RXST;put(0x7e04,RXST);put(0x7e06,RXEND-2);
 put(0x7e34,0x59);put(0x7e4a,1518);put(0x7e00,0);
 duplex_set(st);
 if(reg(0x7e08)!=RXST||reg(0x7e04)!=RXST||reg(0x7e34)!=0x59)return EIO;
 for(i=0;i<30;i++)W16(ram+2*i,0);
 put(0x7f16,0);lanide_hw_txbusy=0;txage=0;
 put(ECON1+0x100,1);
 if(!(reg(ECON1)&1)){lanide_hw_failed=EIO;lanide_hw_halt();return EIO;}
 lanide_hw_running=1;return 0;
}
/* Nonblocking quiesce. Caller may retry on later ticks; never spin for HZ. */
int lanide_hw_stop()
{
 unsigned int i;
 lanide_hw_running=0;
 if(lanide_hw_sim){lanide_hw_txbusy=0;simlen=0;return 0;}
 if(!regs)return 0;
 put(ECON1+0x180,3);
 if((reg(ECON1)&3)||(reg(ESTAT)&0x2000))return EBUSY;
 for(i=0;i<255&&(reg(ESTAT)&255);i++)put(ECON1+0x100,0x100);
 if(reg(ESTAT)&255)return EBUSY;
 put(EIR+0x180,15);lanide_hw_txbusy=0;return 0;
}
void lanide_hw_halt(){lanide_hw_running=0;if(regs)put(ECON1+0x180,3);}
/* One scheduler tick: supervise completion/abort, no waits inside callout. */
int lanide_hw_tick()
{
 unsigned int st,c,f;
 if(!lanide_hw_running)return EIO;
 if(lanide_hw_sim){if(lanide_hw_txbusy){lanide_hw_txbusy=0;lanide_hw_txok++;}return 0;}
 st=reg(ESTAT);c=reg(ECON1);
 if((st&0x1a00)!=0x1a00||!(c&1)||(c&0xc8e0)||(reg(EIE)&0x7fef)||(reg(ECON2)&0xf0)||!(reg(ECON2)&0x8000)||(reg(0x7e40)&0x10)){
  lanide_hw_failed=EIO;lanide_hw_halt();return EIO;
 }
 if(lanide_hw_txbusy){
  if(c&2){
   if(++txage>=10){put(ECON1+0x180,2);lanide_hw_timeouts++;lanide_hw_txerr++;txage=100;}
   if(txage>=100&&(reg(ECON1)&2)){lanide_hw_failed=EIO;lanide_hw_halt();return EIO;}
  }else{
   f=reg(EIR);
   if(txage<100){if((f&12)==8&&!(reg(0x7e12)&0x710))lanide_hw_txok++;else lanide_hw_txerr++;}
   put(EIR+0x180,12);lanide_hw_txbusy=0;txage=0;
  }
 }
 if(!lanide_hw_txbusy&&(st&0x100)&&duplex!=(st&0x400))duplex_set(st);
 if(reg(EIR)&3){lanide_hw_rxerr++;put(EIR+0x180,3);}
 return 0;
}
int lanide_hw_send(b,n) unsigned char *b;unsigned int n;
{
 unsigned int i,len;unsigned short v;
 if(!lanide_hw_running||lanide_hw_failed)return EIO;
 if(n<14||n>1514)return EMSGSIZE;
 if(lanide_hw_txbusy)return EAGAIN;
 len=n<60?60:n;
 if(lanide_hw_sim){if(simlen)return EAGAIN;for(i=0;i<len;i++)simframe[i]=i<n?b[i]:0;simlen=len;lanide_hw_txbusy=1;return 0;}
 if(!(reg(ESTAT)&0x100))return EAGAIN;
 if(reg(ECON1)&2){lanide_hw_failed=EIO;return EIO;}
 for(i=0;i<len;i+=2){v=(unsigned short)(i<n?b[i]:0)<<8;if(i+1<n)v|=b[i+1];W16(ram+i,v);}
 put(0x7f16,0);put(0x7e00,0);put(0x7e02,len);
 lanide_hw_txbusy=1;txage=0;put(ECON1+0x100,2);return 0;
}
/* Return one complete frame (without CRC), zero when empty, LANIDE_RX_DROPPED on discard, -errno on fault. */
int lanide_hw_recv(out) unsigned char *out;
{
 unsigned int n,x,st,p,i,len;unsigned short w;
 if(!lanide_hw_running||lanide_hw_failed)return -EIO;
 if(lanide_hw_sim){if(!simlen)return 0;n=simlen;for(i=0;i<n;i++)out[i]=simframe[i];simlen=0;lanide_hw_rxok++;return n;}
 st=reg(ESTAT);if(!(st&255))return 0;
 p=cursor;x=le(R16(ram+p));n=le(R16(ram+next(p,2)));w=le(R16(ram+next(p,4)));
 if(x<RXST||x>=RXEND||(x&1)||n>RXEND-RXST-10||x!=next(p,(8+n+1)&~1U)){
  printf("lanide: bad RX header cursor=%x next=%x bytes=%x status=%x; stopped\n",p,x,n,(unsigned int)w);
  lanide_hw_rxerr++;lanide_hw_failed=EIO;lanide_hw_halt();return -EIO;
 }
 /* Safe ring allocation but unusable packet: discard just this frame.
  * DS39935B table9-1: RXOK bit7, length check bit5, CRC error bit4.
  * Bit6 is an EtherType indication, NOT an error. Never copy invalid data. */
 if(n<64||n>1518||!(w&0x80)||(w&0x30)){
  put(0x7e06,x==RXST?RXEND-2:x-2);put(ECON1+0x100,0x100);
  cursor=x;lanide_hw_rxerr++;return LANIDE_RX_DROPPED;
 }
 len=n-4;
 for(i=0;i<len;i+=2){w=R16(ram+next(p,8+i));out[i]=w>>8;if(i+1<len)out[i+1]=w&255;}
 put(0x7e06,x==RXST?RXEND-2:x-2);put(ECON1+0x100,0x100);cursor=x;lanide_hw_rxok++;return len;
}
