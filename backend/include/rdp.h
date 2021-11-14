#include "udp.h"
#include <stdbool.h>

#define RDP_FLAG 32768 //last bit
#define ACK_FLAG 16384 //second to last bit

// Comment the line below to not see any debug prints
#define RDP_DEBUG

#ifdef RDP_DEBUG
#define debug_print(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define debug_print(fmt, ...) do {} while (0)
#endif


// Is called during gnc while loop when waiting for user input
err_t
rdp_send(struct udp_pcb *pcb, struct pbuf *p);

// Calls udp_send with RDP_FLAG turned on
err_t
rdp_sendto(struct udp_pcb *pcb, struct pbuf *p, uchar *dst_ip, uint16_t dst_port);

// Uses a timeout mechanism to repetitively call rdp_send_to upon timeout
err_t
rdp_timed_send(struct udp_pcb *pcb, struct pbuf *p);

// Sends a packat with the ACK_FLAG turned on
err_t
rdp_send_ack(struct udp_pcb *pcb, uchar *addr, uint16_t port);
struct pbuf* create_ack_pbuf();

// Is called by udp_recv_callback whenever the RDP_FLAG is set
void rdp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, uchar *addr, uint16_t port, bool is_rdp, bool is_ack);

/*
    DEBUG METHODS
*/
void printPCB(struct udp_pcb *pcb);
void print_ip_and_port(uchar *addr, uint16_t port);