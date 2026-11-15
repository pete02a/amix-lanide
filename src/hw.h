/* Persistent polled LANIDE hardware contract. All calls serialized at IPL4.
 * There is no controller IRQ and no wait-for-clock loop at elevated IPL. */
extern int lanide_hw_attach(),lanide_hw_start(),lanide_hw_stop(),lanide_hw_tick(),lanide_hw_send(),lanide_hw_recv();
extern void lanide_hw_detach(),lanide_hw_halt();
extern unsigned char lanide_hw_mac[6];
extern int lanide_hw_running,lanide_hw_failed,lanide_hw_txbusy,lanide_hw_sim;
extern unsigned long lanide_hw_txok,lanide_hw_txerr,lanide_hw_rxok,lanide_hw_rxerr,lanide_hw_timeouts;

/* One invalid frame was consumed; count against the caller RX budget. */
#define LANIDE_RX_DROPPED (-2)
