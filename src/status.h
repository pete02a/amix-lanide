#define LAN_STATUS (('L'<<8)|1)
struct lan_status {
 unsigned long version,sim,running,error,opens;
 unsigned long tx,rx,txerr,rxerr,timeout,dropped,delivered;
 unsigned char mac[6];
};
