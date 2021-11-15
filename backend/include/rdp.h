#include "udp.h"
#include <stdbool.h>

#define RDP_FLAG 32768 //last bit
#define ACK_FLAG 16384 //second to last bit
#define SEQ_BIT_1 8192 //second to last bit
#define SEQ_BIT_0 4096 //second to last bit

// Comment the line below to not see any debug prints
//#define RDP_DEBUG

#ifdef RDP_DEBUG
#define debug_print(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define debug_print(fmt, ...) do {} while (0)
#endif


/*
 * RDP packets sent from user
 */
err_t
rdp_send(struct udp_pcb *pcb, struct pbuf *p);

struct rdp_connection_node* rdp_create_connection(struct rdp_connection_node* connection, uchar *addr, uint16_t port);

int* retrieve_local_state(struct udp_pcb *pcb, uchar *addr, uint16_t port);

void handle_in_seq_recv(struct udp_pcb *pcb, struct pbuf *p, int seq, int* state);

void handle_out_of_seq_recv(struct udp_pcb *pcb, int seq);

void rdp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, uchar *addr, uint16_t port, bool is_rdp, bool is_ack, int seq);

bool rdp_same_state(int state, int seq);

void rdp_recv_ack(struct udp_pcb *pcb, int* state, int seq);

err_t
rdp_timed_send(struct udp_pcb *pcb, struct pbuf *p, int* state);

err_t
rdp_sendto(struct udp_pcb *pcb, struct pbuf *p, uchar *dst_ip, uint16_t dst_port, int* state);

int get_state_flag(int* state);

void next_state(int* state);

err_t
rdp_send_ack(struct udp_pcb *pcb, int seq, int* state);

struct pbuf* create_ack_pbuf();

void printPCB(struct udp_pcb *pcb);

void print_ip_and_port(uchar *addr, uint16_t port);