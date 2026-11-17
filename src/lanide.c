/* LANIDE STREAMS/DLPI style1 provider, polled, one physical unit/eight SAPs.
 * Interface shape follows the AMIX C3-6 network contract. No Z3660 MMIO.
 * Persistent driver: unload never; close/halt explicitly quiesce engines. */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/queue.h"
#include "sys/sysmacros.h"
#include "sys/cred.h"
#include "sys/dlpi.h"
#include "sys/socket.h"
#include "sys/sockio.h"
#include "sys/conf.h"
#include "sys/moddefs.h"
#include "sys/inline.h"
#include "sys/amixddi.h"
/* Stock _KERNEL net/if.h defines private COMMON copies of kernel globals.
 * Only the public ifstats layout is needed here. */
#undef _KERNEL
#include "net/if.h"
#define _KERNEL 1
#include "hw.h"
#include "status.h"
extern int timeout(),strcmp();
extern void untimeout(),bcopy(),bzero();
#define NS 8
#define CTLEN(m) ((int)((m)->b_wptr-(m)->b_rptr))
static struct endpoint {queue_t *q;unsigned long sap;int state;} ep[NS];
static struct ifstats stats;
static int opens,poll_running,poll_id,in_poll,closing,stop_age;
static unsigned long dropped,rx_delivered;
static unsigned char txbuf[1514],rxbuf[1514];
static int lopen(),lclose(),wput(),wsrv(),rsrv();
static void poller(),proto(),ioc();
static struct module_info mi={0x6c61,"lan",0,1500,16384,4096};
static struct qinit ri={0,rsrv,lopen,lclose,0,&mi,0};
static struct qinit wi={wput,wsrv,0,0,0,&mi,0};
struct streamtab lanideinfo={&ri,&wi,0,0};
static void cancelpoll()
{int armed=poll_running;poll_running=0;if(armed&&!in_poll)untimeout(poll_id);}
static void setstats()
{
 stats.ifs_ipackets=lanide_hw_rxok;stats.ifs_opackets=lanide_hw_txok;
 stats.ifs_ierrors=lanide_hw_rxerr;stats.ifs_oerrors=lanide_hw_txerr;
}
static int lopen(q,devp,flag,sflag,cred) queue_t *q;dev_t *devp;int flag,sflag;cred_t *cred;
{
 unsigned int n=getminor(*devp);int i,s,e;
 if(sflag==MODOPEN||sflag==CLONEOPEN||(n&15))return EINVAL;
 s=spl6();
 if(lanide_hw_failed||closing){splx(s);return EIO;}
 if(q->q_ptr){splx(s);return 0;}
 i=(n>>4)-1;
 if(n==0){for(i=0;i<NS&&ep[i].q;i++){} }
 if(i<0||i>=NS){splx(s);return ENOSPC;}
 if(ep[i].q){splx(s);return EBUSY;}
 if(!opens){e=lanide_hw_start();if(e){splx(s);return e;}}
 ep[i].sap=0;ep[i].state=DL_UNBOUND;
 q->q_ptr=WR(q)->q_ptr=(char *)&ep[i];ep[i].q=q;opens++;
 *devp=makedevice(getmajor(*devp),(i+1)<<4);
 stats.ifs_active=1;
 if(!poll_running){poll_running=1;poll_id=timeout(poller,(caddr_t)0,1);}
 splx(s);return 0;
}
static int lclose(q) queue_t *q;
{
 struct endpoint *slt=(struct endpoint *)q->q_ptr;int s,e;
 s=spl6();
 if(slt){slt->q=0;slt->sap=0;slt->state=DL_UNBOUND;q->q_ptr=WR(q)->q_ptr=0;opens--;}
 flushq(WR(q),FLUSHALL);
 if(!opens){
  stats.ifs_active=0;e=lanide_hw_stop();
  if(e){closing=1;stop_age=0;if(!poll_running){poll_running=1;poll_id=timeout(poller,(caddr_t)0,1);}}
  else{closing=0;cancelpoll();}
 }
 splx(s);return 0;
}
static void errorack(q,m,prim,dlerr,err) queue_t *q;mblk_t *m;unsigned long prim;int dlerr,err;
{
 dl_error_ack_t *p;freemsg(m);m=allocb(sizeof(*p),BPRI_MED);if(!m)return;
 bzero((char *)m->b_wptr,sizeof(*p));p=(dl_error_ack_t *)m->b_wptr;
 p->dl_primitive=DL_ERROR_ACK;p->dl_error_primitive=prim;p->dl_errno=dlerr;p->dl_unix_errno=err;
 m->b_wptr+=sizeof(*p);m->b_datap->db_type=M_PCPROTO;qreply(q,m);
}
static void okack(q,m,prim) queue_t *q;mblk_t *m;unsigned long prim;
{
 dl_ok_ack_t *p;freemsg(m);m=allocb(sizeof(*p),BPRI_MED);if(!m)return;
 p=(dl_ok_ack_t *)m->b_wptr;p->dl_primitive=DL_OK_ACK;p->dl_correct_primitive=prim;
 m->b_wptr+=sizeof(*p);m->b_datap->db_type=M_PCPROTO;qreply(q,m);
}
static void proto(q,m) queue_t *q;mblk_t *m;
{
 struct endpoint *slt=(struct endpoint *)q->q_ptr;unsigned long prim;int n=CTLEN(m),s;
 if(!slt||n<sizeof(long)){errorack(q,m,0,DL_BADPRIM,0);return;}
 prim=*(unsigned long *)m->b_rptr;
 switch(prim){
 case DL_INFO_REQ:{
  dl_info_ack_t *p;freemsg(m);m=allocb(sizeof(*p)+6,BPRI_MED);if(!m)return;
  p=(dl_info_ack_t *)m->b_wptr;bzero((char *)p,sizeof(*p));
  p->dl_primitive=DL_INFO_ACK;p->dl_max_sdu=1500;p->dl_min_sdu=1;p->dl_mac_type=DL_ETHER;
  p->dl_current_state=slt->state;p->dl_max_idu=1500;p->dl_service_mode=DL_CLDLS;p->dl_provider_style=DL_STYLE1;
  m->b_wptr+=sizeof(*p);
  if(slt->state==DL_IDLE){p->dl_addr_length=6;p->dl_addr_offset=sizeof(*p);bcopy((char *)lanide_hw_mac,(char *)m->b_wptr,6);m->b_wptr+=6;}
  m->b_datap->db_type=M_PCPROTO;qreply(q,m);return;
 }
 case DL_ATTACH_REQ:
  if(n<sizeof(dl_attach_req_t)||((dl_attach_req_t *)m->b_rptr)->dl_ppa!=0){errorack(q,m,prim,DL_BADPPA,0);return;}
  if(slt->state!=DL_UNBOUND){errorack(q,m,prim,DL_OUTSTATE,0);return;}okack(q,m,prim);return;
 case DL_DETACH_REQ:
  if(slt->state!=DL_UNBOUND){errorack(q,m,prim,DL_OUTSTATE,0);return;}okack(q,m,prim);return;
 case DL_BIND_REQ:{
  dl_bind_req_t *req;dl_bind_ack_t *p;unsigned long sap;
  if(n<sizeof(*req)){errorack(q,m,prim,DL_BADPRIM,0);return;}req=(dl_bind_req_t *)m->b_rptr;sap=req->dl_sap;
  if(slt->state!=DL_UNBOUND){errorack(q,m,prim,DL_OUTSTATE,0);return;}
  if(sap>65535){errorack(q,m,prim,DL_BADSAP,0);return;}
  if(req->dl_service_mode&&req->dl_service_mode!=DL_CLDLS){errorack(q,m,prim,DL_NOTSUPPORTED,0);return;}
  freemsg(m);m=allocb(sizeof(*p)+6,BPRI_MED);if(!m)return;
  p=(dl_bind_ack_t *)m->b_wptr;bzero((char *)p,sizeof(*p));p->dl_primitive=DL_BIND_ACK;p->dl_sap=sap;p->dl_addr_length=6;p->dl_addr_offset=sizeof(*p);
  m->b_wptr+=sizeof(*p);bcopy((char *)lanide_hw_mac,(char *)m->b_wptr,6);m->b_wptr+=6;
  s=spl6();slt->sap=sap;slt->state=DL_IDLE;splx(s);
  m->b_datap->db_type=M_PCPROTO;qreply(q,m);return;
 }
 case DL_UNBIND_REQ:
  if(slt->state!=DL_IDLE){errorack(q,m,prim,DL_OUTSTATE,0);return;}
  s=spl6();slt->sap=0;slt->state=DL_UNBOUND;flushq(q,FLUSHALL);splx(s);okack(q,m,prim);return;
 case DL_UNITDATA_REQ:{
  dl_unitdata_req_t *p=(dl_unitdata_req_t *)m->b_rptr;
  if(slt->state!=DL_IDLE){errorack(q,m,prim,DL_OUTSTATE,0);return;}
  if(n<sizeof(*p)||p->dl_dest_addr_length!=6||p->dl_dest_addr_offset<sizeof(*p)||p->dl_dest_addr_offset>(unsigned int)n-6){errorack(q,m,prim,DL_BADADDR,0);return;}
  putq(q,m);(void)wsrv(q);return;
 }
 default:errorack(q,m,prim,DL_BADPRIM,0);return;
 }
}
static int wsrv(q) queue_t *q;
{
 mblk_t *m,*part;dl_unitdata_req_t *u;struct endpoint *slt=(struct endpoint *)q->q_ptr;
 int e,n,total,s;
 if(!slt)return 0;
 while((m=getq(q))!=0){
  s=spl6();
  if(slt->state!=DL_IDLE){splx(s);freemsg(m);continue;}
  if(lanide_hw_txbusy){putbq(q,m);splx(s);break;}
  u=(dl_unitdata_req_t *)m->b_rptr;
  bcopy((char *)m->b_rptr+u->dl_dest_addr_offset,(char *)txbuf,6);bcopy((char *)lanide_hw_mac,(char *)txbuf+6,6);
  txbuf[12]=slt->sap>>8;txbuf[13]=slt->sap&255;total=14;e=0;
  for(part=m->b_cont;part;part=part->b_cont){
   n=CTLEN(part);if(n<0||n>1514-total){e=EMSGSIZE;break;}
   bcopy((char *)part->b_rptr,(char *)txbuf+total,n);total+=n;
  }
  if(!e)e=lanide_hw_send(txbuf,total);
  if(e==EAGAIN){putbq(q,m);splx(s);break;}
  if(e){lanide_hw_txerr++;dropped++;}
  splx(s);freemsg(m);
 }
 return 0;
}
static int wput(q,m) queue_t *q;mblk_t *m;
{
 switch(m->b_datap->db_type){
 case M_PROTO:case M_PCPROTO:proto(q,m);break;
 case M_IOCTL:ioc(q,m);break;
 case M_FLUSH:
  if(CTLEN(m)<1){freemsg(m);break;}
  if(*m->b_rptr&FLUSHW){flushq(q,FLUSHALL);*m->b_rptr&=~FLUSHW;}
  if(*m->b_rptr&FLUSHR)qreply(q,m);else freemsg(m);break;
 default:freemsg(m);break;
 }return 0;
}
static int rsrv(q) queue_t *q;{return 0;}
static void deliver(b,n) unsigned char *b;int n;
{
 unsigned int sap=((unsigned int)b[12]<<8)|b[13];int i,ours=1,bcast=1;
 for(i=0;i<6;i++){if(b[i]!=lanide_hw_mac[i])ours=0;if(b[i]!=255)bcast=0;}
 if(!ours&&!bcast)return;
 for(i=0;i<NS;i++)if(ep[i].q&&ep[i].state==DL_IDLE&&ep[i].sap==sap){
  mblk_t *m;dl_unitdata_ind_t *u;
  if(!canput(ep[i].q->q_next)){dropped++;continue;}
  m=allocb(sizeof(*u)+12,BPRI_MED);if(!m){dropped++;continue;}
  m->b_cont=allocb(n-14,BPRI_MED);if(!m->b_cont){freemsg(m);dropped++;continue;}
  u=(dl_unitdata_ind_t *)m->b_wptr;bzero((char *)u,sizeof(*u));
  u->dl_primitive=DL_UNITDATA_IND;u->dl_dest_addr_length=6;u->dl_dest_addr_offset=sizeof(*u);
  u->dl_src_addr_length=6;u->dl_src_addr_offset=sizeof(*u)+6;
  m->b_wptr+=sizeof(*u);bcopy((char *)b,(char *)m->b_wptr,12);m->b_wptr+=12;
  bcopy((char *)b+14,(char *)m->b_cont->b_wptr,n-14);m->b_cont->b_wptr+=n-14;
  m->b_datap->db_type=M_PROTO;m->b_cont->b_datap->db_type=M_DATA;
  putnext(ep[i].q,m);rx_delivered++;
 }
}
static void poller(arg) caddr_t arg;
{
 int s=spl6(),i,n,e;in_poll=1;
 if(closing){
  e=lanide_hw_stop();if(!e){closing=0;poll_running=0;}
  else if(++stop_age>=10){lanide_hw_failed=EIO;lanide_hw_halt();closing=0;poll_running=0;}
 }else if(poll_running&&opens){
  if(lanide_hw_tick()){lanide_hw_halt();poll_running=0;}
  else{
   for(i=0;i<4;i++){n=lanide_hw_recv(rxbuf);if(n==LANIDE_RX_DROPPED){dropped++;continue;}if(n<=0)break;deliver(rxbuf,n);if(!opens)break;}
   /* Like the existing polled AMIX provider: resume our own write service.
    * qenable is not an exported k46 DDI entry; no kernel-private call. */
   for(i=0;i<NS;i++)if(ep[i].q)(void)wsrv(WR(ep[i].q));
  }
 }
 setstats();in_poll=0;
 if(poll_running)poll_id=timeout(poller,(caddr_t)0,1);
 splx(s);
}
static void ioc(q,m) queue_t *q;mblk_t *m;
{
 struct iocblk *p=(struct iocblk *)m->b_rptr;int s;
 if(CTLEN(m)<sizeof(*p)){freemsg(m);return;}
 if(p->ioc_cmd==SIOCSIFFLAGS||p->ioc_cmd==SIOCGIFFLAGS){p->ioc_error=0;p->ioc_rval=0;m->b_datap->db_type=M_IOCACK;qreply(q,m);return;}
 if(p->ioc_cmd==LAN_STATUS&&p->ioc_count!=TRANSPARENT){
  struct lan_status *st;
  if(m->b_cont){freemsg(m->b_cont);m->b_cont=0;}
  m->b_cont=allocb(sizeof(*st),BPRI_MED);
  if(!m->b_cont){p->ioc_error=ENOMEM;goto nak;}
  st=(struct lan_status *)m->b_cont->b_wptr;bzero((char *)st,sizeof(*st));
  s=spl6();st->version=1;st->sim=lanide_hw_sim;st->running=lanide_hw_running;st->error=lanide_hw_failed;st->opens=opens;
  st->tx=lanide_hw_txok;st->rx=lanide_hw_rxok;st->txerr=lanide_hw_txerr;st->rxerr=lanide_hw_rxerr;st->timeout=lanide_hw_timeouts;st->dropped=dropped;st->delivered=rx_delivered;
  bcopy((char *)lanide_hw_mac,(char *)st->mac,6);splx(s);
  m->b_cont->b_wptr+=sizeof(*st);p->ioc_count=sizeof(*st);p->ioc_error=0;p->ioc_rval=0;
  m->b_datap->db_type=M_IOCACK;qreply(q,m);return;
 }
 p->ioc_error=EINVAL;
nak:p->ioc_rval=-1;p->ioc_count=0;m->b_datap->db_type=M_IOCNAK;qreply(q,m);
}
static int lanide_load(params) char *params;
{
 int sim=0,e;
#ifdef LANIDE_NATIVE_TEST
 /* A separate validation binary, never the exported production image's file. */
 sim=1;
 if(params&&*params&&strcmp(params,"sim=0")&&strcmp(params,"sim=1"))return EINVAL;
#else
 /* The production binary cannot be turned into a fake NIC by a boot record. */
 if(params&&*params&&strcmp(params,"sim=0"))return EINVAL;
#endif
 e=lanide_hw_attach(sim);if(e)return e;
 bzero((char *)&stats,sizeof(stats));stats.ifs_name="lan";stats.ifs_unit=0;stats.ifs_mtu=1500;
 e=if_attach(&stats);if(e){lanide_hw_detach();return e;}
 return 0;
}
static int lanide_unload()
{
 if(opens||poll_running)return EBUSY;
 if(lanide_hw_stop())return EBUSY;
 (void)if_detach(&stats);lanide_hw_detach();return 0;
}
static void lanide_halt()
{int s=spl6();cancelpoll();lanide_hw_halt();splx(s);}
struct mod_drv_data lanide_drvdata[]={
 {{0},0,0,{0,0,0,0,0,0,0,0,0,0,(struct tty *)0,&lanideinfo,(int *)0},57,1}
};
MOD_DRV_WRAPPER(lanide,lanide_load,lanide_unload,lanide_halt,"LANIDE polled STREAMS DLPI lan0");
