#define LANIDE6_HOST
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "hw.c"
long test_cpu=40;
static char regbuf[4096],membuf[24576];
static unsigned short rg[2048],mem[12288];
static int cases,live_r,live_m,maps,unmaps,failmap,present,badbase,badsize;
static int pending,complete,stucktx,stuckrx,stuckcount,txabort;
static unsigned long probes,faultat,writes,requests;
static unsigned int ix(unsigned int o){return(o-0x7000)/2;}
static int isreg(char *p){return(uintptr_t)p>=(uintptr_t)regbuf&&(uintptr_t)p<(uintptr_t)regbuf+4096;}
int autocon(long pc,int n,long *b,long *z){assert(pc==0x0a1c007bL&&!n);if(!present)return 0;*b=badbase?0x50000000UL:BASE;*z=badsize?0x8000UL:0x10000UL;return 1;}
char *dev_kvmap(unsigned long a,unsigned long n,int cm,int f){assert(cm==0x40&&f==1);maps++;if(maps==failmap)return 0;if(a==BASE+0x7000){assert(n==4096&&!live_r);live_r=1;return regbuf;}assert(a==BASE&&n==24576&&!live_m);live_m=1;return membuf;}
void dev_kvunmap(char *p,unsigned long n){if(p==regbuf){assert(live_r&&n==4096);live_r=0;}else{assert(p==membuf&&live_m&&n==24576);live_m=0;}unmaps++;}
int bprobe(char *p,int v){assert(v==-1);if(isreg(p)){assert(live_r);assert((unsigned int)(p-regbuf)<0xf00);/* No SET/CLR or data-port sweep. */}else assert(live_m&&(uintptr_t)p>=(uintptr_t)membuf&&(uintptr_t)p<(uintptr_t)membuf+24576);return ++probes==faultat;}
unsigned short test_read16(char *p){unsigned int o;if(!isreg(p)){assert(live_m);return mem[(p-membuf)/2];}assert(live_r);o=(unsigned int)(p-regbuf)+0x7000;
 if(o==ECON1&&complete&&(rg[ix(ECON1)]&2)){rg[ix(ECON1)]&=~2;rg[ix(EIR)]|=txabort?4:8;rg[ix(0x7e12)]=txabort?0x10:0x40;complete=0;}
 if(o==ESTAT)return(rg[ix(ESTAT)]&~255U)|pending|(stuckrx?0x2000:0);
 if(o==EIR)return rg[ix(EIR)]|(pending?0x40:0);
 return rg[ix(o)];
}
void test_write16(char *p,unsigned short v){unsigned int o;writes++;if(!isreg(p)){assert(live_m&&!(rg[ix(ECON1)]&2));assert((uintptr_t)p>=(uintptr_t)membuf&&(uintptr_t)p<(uintptr_t)membuf+0x1000);mem[(p-membuf)/2]=v;return;}
 assert(live_r);o=(unsigned int)(p-regbuf)+0x7000;
 if(o==ECON1+0x100){assert(v==1||v==2||v==0x100);if(v==0x100){assert(pending);if(!stuckcount)pending--;return;}if(v==2){assert(!(rg[ix(ECON1)]&2));requests++;}rg[ix(ECON1)]|=v;return;}
 if(o==ECON1+0x180){assert(v==3||v==2);rg[ix(ECON1)]&=~(stucktx?(v&~2):v);return;}
 if(o==EIR+0x180){assert((v&~15U)==0);rg[ix(EIR)]&=~v;return;}
 if(o==0x7f16){assert(!v);return;}
 assert(o==0x7e00||o==0x7e02||o==0x7e04||o==0x7e06||o==0x7e34||o==0x7e42||o==0x7e44||o==0x7e4a);
 assert(o==0x7e06||o==0x7e00||o==0x7e02||o==0x7e42||o==0x7e44||!(rg[ix(ECON1)]&3));
 if(o==0x7e04)rg[ix(0x7e08)]=v;
 rg[ix(o)]=v;
}
static void reset(void){cases++;assert(!live_r&&!live_m);memset(rg,0,sizeof(rg));memset(mem,0,sizeof(mem));rg[ix(ESTAT)]=0x5f00;rg[ix(EIE)]=0x8010;rg[ix(ECON2)]=0xc200;rg[ix(0x7e64)]=0x8440;rg[ix(0x7e62)]=0x1c32;rg[ix(0x7e60)]=0x76f2;test_cpu=40;present=1;badbase=badsize=maps=unmaps=failmap=pending=complete=stucktx=stuckrx=stuckcount=txabort=0;probes=faultat=writes=requests=0;lanide_hw_running=lanide_hw_failed=lanide_hw_txbusy=lanide_hw_sim=0;}
static void finish(void){stucktx=stuckrx=stuckcount=0;assert(!lanide_hw_stop());lanide_hw_detach();assert(!live_r&&!live_m);assert(unmaps==maps-(failmap?1:0));}
static void inject(unsigned int p,unsigned int bytes,unsigned int status){unsigned int i,x=next(p,(8+bytes+1)&~1U);cursor=p;mem[p/2]=le(x);mem[next(p,2)/2]=le(bytes);mem[next(p,4)/2]=le(status);for(i=0;i<bytes-4;i+=2)mem[next(p,8+i)/2]=((i&255)<<8)|((i+1)&255);pending=1;}

static void rawhead(unsigned int p,unsigned int n,unsigned int status){
 cursor=p;mem[p/2]=le(next(p,(8+n+1)&~1U));mem[next(p,2)/2]=le(n);mem[next(p,4)/2]=le(status);pending=1;
}
static void recovery_tests(void){
 unsigned char out[1514],tx[60]={0};unsigned int i,j,p,g;
 unsigned int lengths[]={0,1,3,4,17,18,63,64,1518,1519,1522,20470};
 unsigned int positions[]={RXST,RXEND-8,RXEND-2,0x247a};
 unsigned int corrupt[]={RXST-2,RXEND,RXST+1,RXST,RXST+74};
 /* Every low status bit combination, including ordinary EtherType bit6. */
 for(i=0;i<256;i++){
  int good=(i&0x80)&&!(i&0x30),n;
  reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());
  memset(out,0xaa,sizeof(out));rawhead(0x247a,64,0x300|i);
  n=lanide_hw_recv(out);assert(n==(good?60:LANIDE_RX_DROPPED));
  assert(!pending&&lanide_hw_running&&!lanide_hw_failed&&(rg[ix(ECON1)]&1));
  assert(lanide_hw_rxok==(unsigned long)good&&lanide_hw_rxerr==(unsigned long)!good);
  if(!good)for(j=0;j<sizeof(out);j++)assert(out[j]==0xaa);
  finish();
 }
 for(i=0;i<sizeof(lengths)/sizeof(lengths[0]);i++){
  int good=lengths[i]>=64&&lengths[i]<=1518;
  reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());rawhead(RXEND-2,lengths[i],0x80);
  memset(out,0xaa,sizeof(out));assert(lanide_hw_recv(out)==(good?(int)lengths[i]-4:LANIDE_RX_DROPPED));
  assert(lanide_hw_running&&!lanide_hw_failed&&!pending);
  if(!good)for(j=0;j<sizeof(out);j++)assert(out[j]==0xaa);
  finish();
 }
 for(i=0;i<sizeof(positions)/sizeof(positions[0]);i++){
  reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());p=positions[i];g=next(p,72);
  inject(p,64,0x3a8);inject(g,64,0xc0);cursor=p;pending=2;
  assert(lanide_hw_recv(out)==LANIDE_RX_DROPPED&&cursor==g&&pending==1);
  assert(lanide_hw_running&&!lanide_hw_failed);
  assert(lanide_hw_recv(out)==60&&!pending&&lanide_hw_rxerr==1&&lanide_hw_rxok==1);finish();
 }
 /* Four invalid records consume exactly four receive calls, leaving a good
  * record for the next budget; the backend never loops internally. */
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());p=RXEND-8;
 for(i=0;i<5;i++){inject(p,64,i<4?0x3a8:0x80);p=next(p,72);}
 cursor=RXEND-8;pending=5;
 for(i=0;i<4;i++)assert(lanide_hw_recv(out)==LANIDE_RX_DROPPED);
 assert(pending==1&&lanide_hw_running&&lanide_hw_rxerr==4);
 assert(lanide_hw_recv(out)==60&&!pending&&lanide_hw_rxok==1);
 assert(!lanide_hw_send(tx,60));complete=1;assert(!lanide_hw_tick()&&lanide_hw_txok==1);finish();
 /* Bad packet flags never hide truly unsafe ring metadata. */
 for(i=0;i<sizeof(corrupt)/sizeof(corrupt[0]);i++){
  reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());rawhead(RXST,64,0x3a8);mem[RXST/2]=le(corrupt[i]);
  assert(lanide_hw_recv(out)==-EIO&&lanide_hw_failed&&!lanide_hw_running&&pending==1);finish();
 }
 for(i=0;i<2;i++){
  reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());rawhead(RXST,i?65535:20471,0x3a8);
  assert(lanide_hw_recv(out)==-EIO&&lanide_hw_failed&&pending==1);finish();
 }
}

static void earlytx_tests(void){
 unsigned char b[60]={0};unsigned int i;unsigned long before;
 reset();assert(lanide_hw_txpoll()==EIO);finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());
 assert(!lanide_hw_send(b,60));before=writes;
 for(i=0;i<1000;i++)assert(!lanide_hw_txpoll()&&lanide_hw_txbusy);
 assert(writes==before&&txage==0&&!lanide_hw_timeouts);
 complete=1;assert(!lanide_hw_txpoll()&&!lanide_hw_txbusy&&lanide_hw_txok==1);
 assert(!lanide_hw_txpoll()&&lanide_hw_txok==1);
 assert(!lanide_hw_send(b,60));txabort=complete=1;
 assert(!lanide_hw_txpoll()&&!lanide_hw_txbusy&&lanide_hw_txerr==1&&lanide_hw_txok==1);finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());assert(!lanide_hw_send(b,60));
 for(i=0;i<9;i++){assert(!lanide_hw_tick());assert(!lanide_hw_txpoll());}
 assert(txage==9&&!lanide_hw_timeouts);assert(!lanide_hw_tick()&&lanide_hw_timeouts==1);
 assert(!lanide_hw_txpoll()&&!lanide_hw_txbusy&&lanide_hw_txerr==1);finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());assert(!lanide_hw_send(b,60));rg[ix(EIE)]=1;
 assert(lanide_hw_txpoll()==EIO&&lanide_hw_failed==EIO&&!lanide_hw_running);rg[ix(EIE)]=0;finish();
 reset();assert(!lanide_hw_attach(1)&&!lanide_hw_start());assert(!lanide_hw_send(b,60));
 assert(!lanide_hw_txpoll()&&lanide_hw_txbusy&&!lanide_hw_txok);assert(!lanide_hw_tick()&&!lanide_hw_txbusy&&lanide_hw_txok==1);finish();
}
int main(void){earlytx_tests();unsigned int i,p,n;unsigned long k;unsigned char b[1514],out[1514];
 for(i=0;i<1514;i++)b[i]=i;
 reset();present=0;assert(lanide_hw_attach(0)==ENXIO&&!maps&&!writes);finish();
 reset();test_cpu=30;assert(lanide_hw_attach(0)==ENODEV&&!maps);finish();
 reset();badbase=1;assert(lanide_hw_attach(0)==EINVAL&&!maps);finish();
 reset();badsize=1;assert(lanide_hw_attach(0)==EINVAL&&!maps);finish();
 for(i=1;i<=2;i++){reset();failmap=i;assert(lanide_hw_attach(0)==ENOMEM);finish();}
 for(k=1;k<=24614;k+=257){reset();faultat=k;assert(lanide_hw_attach(0)==EIO&&!writes);finish();}
 for(i=0;i<16;i++)if(0x7fef&(1U<<i)){reset();rg[ix(EIE)]|=1U<<i;assert(lanide_hw_attach(0)==EBUSY&&!writes);finish();}
 reset();pending=1;assert(lanide_hw_attach(0)==EBUSY&&!writes);finish();
 reset();rg[ix(0x7e40)]=16;assert(lanide_hw_attach(0)==EBUSY&&!writes);finish();
 reset();assert(!lanide_hw_attach(0));assert(lanide_hw_mac[0]==0x40&&lanide_hw_mac[5]==0x76);assert(!lanide_hw_start());assert(lanide_hw_running);
 assert(lanide_hw_send(b,13)==EMSGSIZE&&lanide_hw_send(b,1515)==EMSGSIZE);
 assert(!lanide_hw_send(b,41)&&lanide_hw_txbusy&&requests==1&&rg[ix(0x7e02)]==60);
 assert(mem[0]==1&&mem[20]==(40<<8)&&mem[21]==0);assert(lanide_hw_send(b,60)==EAGAIN&&requests==1);
 complete=1;assert(!lanide_hw_tick()&&!lanide_hw_txbusy&&lanide_hw_txok==1);assert(!lanide_hw_send(b,1514));complete=1;assert(!lanide_hw_tick());assert(lanide_hw_txok==2);
 for(p=RXEND-14;p<RXEND;p+=2)for(n=64;n<=1518;n+=(n==64?1:1453)){
  inject(p,n,0x80);assert(lanide_hw_recv(out)==(int)n-4&&!pending);for(i=0;i<n-4;i++)assert(out[i]==(i&255));
 }
 assert(lanide_hw_recv(out)==0);assert(!lanide_hw_stop()&&!lanide_hw_running);assert(!lanide_hw_start());finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());assert(!lanide_hw_send(b,60));txabort=1;complete=1;assert(!lanide_hw_tick()&&lanide_hw_txerr==1&&!lanide_hw_txbusy);finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());assert(!lanide_hw_send(b,60));for(i=0;i<11;i++)assert(!lanide_hw_tick());assert(!lanide_hw_txbusy&&lanide_hw_timeouts==1);finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());assert(!lanide_hw_send(b,60));stucktx=1;for(i=0;i<9;i++)assert(!lanide_hw_tick());assert(lanide_hw_tick()==EIO&&lanide_hw_failed&&!lanide_hw_running);finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());rg[ix(EIE)]|=8;assert(lanide_hw_tick()==EIO&&!lanide_hw_running);finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());rg[ix(0x7e40)]|=16;assert(lanide_hw_tick()==EIO&&!lanide_hw_running);finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());rg[ix(ECON2)]&=~0x8000;assert(lanide_hw_tick()==EIO);finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());rg[ix(ESTAT)]&=~0x100;assert(lanide_hw_send(b,60)==EAGAIN&&!requests);assert(!lanide_hw_tick());finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());inject(RXST,64,0x90);assert(lanide_hw_recv(out)==LANIDE_RX_DROPPED&&lanide_hw_running&&!lanide_hw_failed&&lanide_hw_rxerr==1);finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());inject(RXST,64,0x80);mem[RXST/2]|=0x100;assert(lanide_hw_recv(out)==-EIO);finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());pending=255;assert(!lanide_hw_stop()&&!pending);finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());stuckrx=1;assert(lanide_hw_stop()==EBUSY&&!lanide_hw_running);stuckrx=0;assert(!lanide_hw_stop());finish();
 reset();assert(!lanide_hw_attach(0)&&!lanide_hw_start());pending=2;stuckcount=1;assert(lanide_hw_stop()==EBUSY);stuckcount=0;assert(!lanide_hw_stop());finish();
 reset();assert(!lanide_hw_attach(1)&&!maps);assert(!lanide_hw_start()&&!lanide_hw_send(b,14));assert(lanide_hw_txbusy);assert(!lanide_hw_tick());assert(lanide_hw_recv(out)==60&&!memcmp(out,b,14));for(i=14;i<60;i++)assert(!out[i]);finish();
 recovery_tests();
 printf("PASS: %d lifecycle cases; asynchronous TX, RX wrap, quiesce/reopen, IRQ/PHY/reset-write allowlist\n",cases);return 0;
}
