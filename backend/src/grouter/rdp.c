#include "rdp.h"
#include "time.h"
#include <stdio.h>


unsigned static long seq = 0 ;

bool responseFlag = false;
const struct timespec sleeptime = {{0, 1000000L}}; // 1000000L = 1 millisecond, sleep otherwise crash
const int timeout = 100; //in milliseconds

/*
 * RDP packets sent from user
 */
err_t
rdp_send(struct udp_pcb *pcb, struct pbuf *p){
    /* send to the packet using remote ip and port stored in the pcb */
    debug_print("PCB BLOCK USED TO SEND:\n");
    printPCB(pcb);
    return rdp_timed_send(pcb, p);
}

/*
 * callback function for RDP packets received
 */
void rdp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, uchar *addr, uint16_t port, bool is_rdp, bool is_ack) {
    debug_print("RDP Receive has been called: RDP %d ACK %d\n", is_rdp, is_ack);
    if(is_ack){
        // Stop the rdp_timed_send loop
        debug_print("ACK RECEIVED!\n");
        responseFlag = true;
    }else{
        if(p->seq < seq)return;
        // Display & ACK the data
        printf("%s", (char*)p->payload);
        rdp_send_ack(arg, addr, port);
    }


}

err_t
rdp_timed_send(struct udp_pcb *pcb, struct pbuf *p){
    unsigned long mySeq = getSeqNum();
    p->seq = mySeq;
    int countdown = timeout;
    responseFlag = false;
    err_t udp_err;

    //Initial send
    udp_err = rdp_sendto(pcb, p, pcb->remote_ip, pcb->remote_port);
    int count = 1;

    while(!responseFlag){
        if(countdown < 0){
            debug_print("\n\nTIMEOUT! RESENDING\n\n");
            udp_err = rdp_sendto(pcb, p, pcb->remote_ip, pcb->remote_port);
            countdown = timeout;
            count += 1;
        }

        nanosleep(&sleeptime, NULL);
        countdown -= 1; //decrement by the amount of millis you slept for
    };

    printf("It took about: %d (ms) to get an ACK\n", count*timeout - countdown);
    return udp_err;
}

err_t
rdp_sendto(struct udp_pcb *pcb, struct pbuf *p, uchar *dst_ip, uint16_t dst_port){
    udp_sendto(pcb, p, dst_ip, dst_port + RDP_FLAG);
}

err_t
rdp_send_ack(struct udp_pcb *pcb, uchar *addr, uint16_t port){
    struct pbuf *p = create_ack_pbuf();

    debug_print("PCB BLOCK USED TO SEND ACK:\n");
    printPCB(pcb);
    print_ip_and_port(addr, port);

    
    err_t udp_err = rdp_sendto(pcb, p, pcb->remote_ip, pcb->remote_port + ACK_FLAG);
    return udp_err;
}

struct pbuf* create_ack_pbuf(){
    char* payload = "ACK";
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, strlen(payload), PBUF_RAM);
    return p;
}

void printPCB(struct udp_pcb *pcb){
    print_ip_and_port(pcb->local_ip, pcb->local_port);
    print_ip_and_port(pcb->remote_ip, pcb->remote_port);
}

void print_ip_and_port(uchar *addr, uint16_t port){
    debug_print("IP: %d.%d.%d.%d PORT: %d\n", addr[0], addr[1], addr[2], addr[3], port);
}

unsigned long getSeqNum(){
    return seq++;
}

