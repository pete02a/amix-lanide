/* Native AMIX DLPI/status acceptance tool. -t REQUIRES the explicit no-MMIO
 * simulation mode; production use only opens and reads interface counters. */
#include <sys/types.h>
#include <sys/stropts.h>
#include <sys/dlpi.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "status.h"
static union {long aligned;char bytes[2048];} ctl;
static unsigned char data[2048];
static int clen,dlen,checks;
static void die(s) char *s;{perror(s);exit(1);}
static void check(v,s) int v;char *s;
{checks++;if(!v){fprintf(stderr,"FAIL: %s\n",s);exit(1);}}
static void expired(sig) int sig;{write(2,"DLPI timeout\n",13);_exit(3);}
static void sendctl(fd,p,n,b,nb) int fd,n,nb;char *p,*b;
{struct strbuf c,d;c.maxlen=0;c.len=n;c.buf=p;d.maxlen=0;d.len=nb;d.buf=b;if(putmsg(fd,&c,nb?&d:0,0)<0)die("putmsg");}
static long receive(fd) int fd;
{
 struct strbuf c,d;int flags=0,rc;
 c.maxlen=sizeof(ctl.bytes);c.len=0;c.buf=ctl.bytes;
 d.maxlen=sizeof(data);d.len=0;d.buf=(char *)data;
 alarm(5);rc=getmsg(fd,&c,&d,&flags);alarm(0);
 if(rc<0)die("getmsg");check(rc==0,"no truncated STREAMS message");
 clen=c.len;dlen=d.len;check(clen>=sizeof(long),"control primitive present");return *(long *)ctl.bytes;
}
static void statfd(fd,s) int fd;struct lan_status *s;
{struct strioctl x;x.ic_cmd=LAN_STATUS;x.ic_timout=5;x.ic_len=sizeof(*s);x.ic_dp=(char *)s;if(ioctl(fd,I_STR,&x)<0)die("LAN_STATUS");check(x.ic_len==sizeof(*s)&&s->version==1,"status ABI");}
static void info(fd,state) int fd,state;
{
 dl_info_req_t req;dl_info_ack_t *p;
 req.dl_primitive=DL_INFO_REQ;sendctl(fd,(char *)&req,sizeof(req),0,0);
 check(receive(fd)==DL_INFO_ACK&&clen>=sizeof(*p),"INFO_ACK");p=(dl_info_ack_t *)ctl.bytes;
 check(p->dl_provider_style==DL_STYLE1&&p->dl_max_sdu==1500&&p->dl_current_state==state,"style1/state/MTU");
}
static void bindfd(fd,sap) int fd;unsigned long sap;
{
 dl_bind_req_t r;dl_bind_ack_t *p;
 memset((char *)&r,0,sizeof(r));r.dl_primitive=DL_BIND_REQ;r.dl_sap=sap;r.dl_service_mode=DL_CLDLS;
 sendctl(fd,(char *)&r,sizeof(r),0,0);check(receive(fd)==DL_BIND_ACK&&clen>=sizeof(*p)+6,"BIND_ACK MAC");
 p=(dl_bind_ack_t *)ctl.bytes;check(p->dl_sap==sap&&p->dl_addr_length==6&&p->dl_addr_offset==sizeof(*p),"SAP/address shape");
}
static void err(fd,prim,why) int fd;long prim,why;
{dl_error_ack_t *p;check(receive(fd)==DL_ERROR_ACK&&clen>=sizeof(*p),"ERROR_ACK");p=(dl_error_ack_t *)ctl.bytes;check(p->dl_error_primitive==prim&&p->dl_errno==why,"error primitive/reason");}
int main(argc,argv) int argc;char **argv;
{
 int fd,fd2,f[6],i,ninth,test=argc>1&&!strcmp(argv[1],"-t");
 char *node=test?(argc>2?argv[2]:"/dev/lan0"):(argc>1?argv[1]:"/dev/lan0");
 struct lan_status st;unsigned char own[6];long unknown=0x12345678L;
 struct {dl_unitdata_req_t r;unsigned char dest[6];} u;
 static char payload[]="LANIDE-STREAMS-SELFTEST";
 dl_unbind_req_t unbind;
 signal(SIGALRM,expired);
 fd=open(node,O_RDWR);if(fd<0)die(node);statfd(fd,&st);
 printf("lan0: SIM=%lu RUNNING=%lu ERROR=%lu OPENS=%lu MAC=%02x:%02x:%02x:%02x:%02x:%02x\n",st.sim,st.running,st.error,st.opens,st.mac[0],st.mac[1],st.mac[2],st.mac[3],st.mac[4],st.mac[5]);
 printf("TX=%lu RX=%lu TXERR=%lu RXERR=%lu TIMEOUT=%lu DROPPED=%lu DELIVERED=%lu\n",st.tx,st.rx,st.txerr,st.rxerr,st.timeout,st.dropped,st.delivered);
 if(!test){close(fd);return st.error?1:0;}
 check(st.sim==1,"selftest refuses real NIC traffic");memcpy(own,st.mac,6);
 info(fd,DL_UNBOUND);bindfd(fd,0x88b5);info(fd,DL_IDLE);
 check(!memcmp(ctl.bytes+((dl_info_ack_t *)ctl.bytes)->dl_addr_offset,own,6),"INFO station MAC");
 fd2=open(node,O_RDWR);if(fd2<0)die("second stream");bindfd(fd2,0x88b5);
 for(i=0;i<6;i++){f[i]=open(node,O_RDWR);if(f[i]<0)die("eight stream capacity");}
 ninth=open(node,O_RDWR);check(ninth<0&&errno==ENOSPC,"ninth open rejected");
 for(i=0;i<6;i++)close(f[i]);
 statfd(fd,&st);check(st.opens==2,"closing extra streams keeps two");
 sendctl(fd,(char *)&unknown,sizeof(unknown),0,0);err(fd,unknown,DL_BADPRIM);
 memset((char *)&u,0,sizeof(u));u.r.dl_primitive=DL_UNITDATA_REQ;u.r.dl_dest_addr_length=6;u.r.dl_dest_addr_offset=0;
 sendctl(fd,(char *)&u,sizeof(u),payload,sizeof(payload)-1);err(fd,DL_UNITDATA_REQ,DL_BADADDR);
 u.r.dl_dest_addr_offset=0xffffffffUL;
 sendctl(fd,(char *)&u,sizeof(u),payload,sizeof(payload)-1);err(fd,DL_UNITDATA_REQ,DL_BADADDR);
 u.r.dl_dest_addr_offset=sizeof(u.r);u.r.dl_dest_addr_length=5;
 sendctl(fd,(char *)&u,sizeof(u),payload,sizeof(payload)-1);err(fd,DL_UNITDATA_REQ,DL_BADADDR);
 u.r.dl_dest_addr_length=6;for(i=0;i<6;i++)u.dest[i]=255;
 sendctl(fd,(char *)&u,sizeof(u),payload,sizeof(payload)-1);
 check(receive(fd)==DL_UNITDATA_IND,"first subscriber RX");check(dlen==46&&!memcmp(data,payload,sizeof(payload)-1),"frame payload and min padding");
 for(i=sizeof(payload)-1;i<dlen;i++)check(!data[i],"zero Ethernet padding");
 check(receive(fd2)==DL_UNITDATA_IND,"second same-SAP subscriber RX");check(dlen==46&&!memcmp(data,payload,sizeof(payload)-1),"second payload");
 statfd(fd,&st);check(st.tx==1&&st.rx==1&&st.delivered==2&&!st.error,"async TX RX demux counters");
 close(fd2);statfd(fd,&st);check(st.opens==1&&st.running==1,"one close leaves other running");
 unbind.dl_primitive=DL_UNBIND_REQ;sendctl(fd,(char *)&unbind,sizeof(unbind),0,0);
 check(receive(fd)==DL_OK_ACK,"UNBIND_ACK");info(fd,DL_UNBOUND);close(fd);
 fd=open(node,O_RDWR);if(fd<0)die("reopen after last close");statfd(fd,&st);check(st.opens==1&&st.running==1&&!st.error,"last close quiesce/reopen");close(fd);
 printf("PASS: native no-MMIO DLPI/8streams/multibind/unitdata/padding/errors/close/reopen (%d checks)\n",checks);return 0;
}
