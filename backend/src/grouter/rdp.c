#include "rdp.h"
#include "time.h"
#include <stdio.h>
#include <stdlib.h>

const struct timespec sleeptime = {0, 1000000L}; // 1000000L = 1 millisecond, sleep otherwise crash
const int timeout = 10; //in milliseconds
int connection_count = 0;
/*
 * RDP packets sent from user
 */
err_t
rdp_send(struct udp_pcb *pcb, struct pbuf *p)
{
    /* send to the packet using remote ip and port stored in the pcb */
    debug_print("CONNECTIONS TOTAL: %d\n", connection_count);
    debug_print("PCB BLOCK USED TO SEND:\n");
    printPCB(pcb);

    // Retrieve state associated with this pcb, if all is good it
    // should be in a linked list containing one node.
    int* state = retrieve_local_state(pcb, pcb->remote_ip, pcb->remote_port);

    err_t udp_error = rdp_timed_send(pcb, p, state);
    return udp_error;
}

err_t
rdp_timed_send(struct udp_pcb *pcb, struct pbuf *p, int* state){
    int countdown = timeout;
    pcb->rdp_ack_received = false;
    err_t udp_err;
    char* payload = p->payload;


    //Initial send
    udp_err = rdp_sendto(pcb, p, pcb->remote_ip, pcb->remote_port, state);
    int count = 1;
    return udp_err;
    while(!(pcb->rdp_ack_received)){
        if(countdown < 0){
            debug_print("\n\nTIMEOUT! RESENDING\n\n");

            // This line fixes a memory leak or something, somewhere... in udp_send's call stack.
            p = pbuf_alloc(PBUF_TRANSPORT, strlen(payload), PBUF_RAM);

            udp_err = rdp_sendto(pcb, p, pcb->remote_ip, pcb->remote_port, state);

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
rdp_sendto(struct udp_pcb *pcb, struct pbuf *p, uchar *dst_ip, uint16_t dst_port, int* state){
    udp_sendto(pcb, p, dst_ip, dst_port + RDP_FLAG + get_state_flag(state));
}

int get_state_flag(int* state){
    int flag = 0;
    int state_val = *state;

    flag += (state_val / 2) * SEQ_BIT_1;
    int remainder = state_val % 2;

    flag += remainder * SEQ_BIT_0;

    debug_print("STATE_FLAG: SEQ %d\tFLAG %d\n", state_val, flag);
    return flag;
}

/*
 * callback function for RDP packets received
 */
void rdp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, uchar *addr, uint16_t port, bool is_rdp, bool is_ack, int seq) {
    debug_print("RDP Receive has been called: RDP %d ACK %d SEQ %d\n", is_rdp, is_ack, seq);


    debug_print("COMPARE BELOW\n\n\n");
    //printPCB((struct udp_pcb*) arg);
    printPCB(pcb);
    debug_print("COMPARE ABOVE\n\n\n");
    // First retrieve the state associated with that sender
    int* state = retrieve_local_state(pcb, addr, port);

    if(is_ack){
        // Stop the rdp_timed_send loop
        rdp_recv_ack(pcb, state, seq);
    }else{
        // Display & ACK the data
        if(rdp_same_state(*state, seq)){
            handle_in_seq_recv(pcb, p, seq, addr, port, state);
        }else{
            handle_out_of_seq_recv(pcb, seq, addr, port);
        }
        
    }
}

int* retrieve_local_state(struct udp_pcb *pcb, uchar *addr, uint16_t port){

    struct rdp_connection_node** node = &(pcb->rdp_connection);

    // HEAD unitialized case
    if((*node) == NULL){
        debug_print("Connection Not Found, Creating Connection\n");
        (*node) = rdp_create_connection(node, addr, port);
        print_ip_and_port((*node)->sender_ip, (*node)->sender_port);
        return (*node)->local_state;
    }

    while((*node)->next != NULL){
        if(ip_addr_cmp((*node)->sender_ip, addr) && (*node)->sender_port == port){

            debug_print("Connection Found!\n");
            debug_print("Connection State: SEQ %d\n", *((*node)->local_state));
            print_ip_and_port((*node)->sender_ip, (*node)->sender_port);
            print_ip_and_port((*node)->sender_ip, (*node)->sender_port);
            return (*node)->local_state;
        }
        node = &((*node)->next);
    }

    // TAIL match
    if(ip_addr_cmp((*node)->sender_ip, addr) && (*node)->sender_port == port){
        debug_print("Connection Found!\n");
        debug_print("Connection State: SEQ %d\n", *((*node)->local_state));
        return (*node)->local_state;
    }

    debug_print("Connection Not Found, Creating Connection\n");
    (*node)->next = rdp_create_connection(node, addr, port);
    print_ip_and_port((*node)->sender_ip, (*node)->sender_port);

    return (*node)->next->local_state;

}


struct rdp_connection_node* rdp_create_connection(struct rdp_connection_node* connection, uchar *addr, uint16_t port){
    connection_count += 1;
    connection = (struct rdp_connection_node*) malloc(sizeof(struct rdp_connection_node));

    // TODO: not sure I have to copy this...
    connection->sender_ip = addr;
    connection->sender_port = port;
    connection->local_state = (int*) malloc(sizeof(int));
    *(connection->local_state) = 0;

    return connection;
}


void handle_in_seq_recv(struct udp_pcb *pcb, struct pbuf *p, int seq, uchar *addr, uint16_t port, int* state){
    printf("%s", (char*)p->payload);
    rdp_send_ack(pcb, seq, addr, port, &seq);
    next_state(state);
}

void handle_out_of_seq_recv(struct udp_pcb *pcb, int seq, uchar *addr, uint16_t port){
    rdp_send_ack(pcb, seq, addr, port, &seq);
}


bool rdp_same_state(int state, int seq){
    return state == seq;
}

void rdp_recv_ack(struct udp_pcb *pcb, int* state, int seq){
    if(rdp_same_state(*state, seq)){
        pcb->rdp_ack_received = true;
        next_state(state);
    }
}

void next_state(int* state){
    int current_state = *state;
    current_state = (current_state + 1) % 4;
    *state = current_state;
    debug_print("PCB NEW STATE: SEQ %d\n", *state);
}

err_t
rdp_send_ack(struct udp_pcb *pcb, int seq, uchar *addr, uint16_t port, int* state){
    struct pbuf *p = create_ack_pbuf();

    debug_print("PCB BLOCK USED TO SEND ACK:\n");
    printPCB(pcb);
    print_ip_and_port(addr, port);

    err_t udp_err = rdp_sendto(pcb, p, pcb->remote_ip, pcb->remote_port + ACK_FLAG, state);

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

