#ifndef WINDOW_H
#define WINDOW_H

#include"transport.h"

typedef struct window_node window_node;
struct window_node {
    struct packet * wn_packet;
    int wn_datalen;
    window_node * wn_next;
    struct timeval wn_sendtime; /* Time stamp when the data packet is sent */
    short int wn_retransmitcnt; /* number of times this packte is transmitted */
};

enum window_type {WIN_RECV, WIN_SEND, WIN_UNDEF};

typedef struct window window;
struct window {
    context_t* win_ctx;
#define WIN_SIZE 3072
    enum window_type win_type;
    window_node * win_buf;
}; 

/* initialize a window without */
/* void win_init( window *pt_win ); */

int win_getsize( window * pt_win);

/* initialize a window by specifying the member values */
void win_init( window * pt_win, enum window_type wt, context_t* ctx);

/* get the seq number of a packet */
int get_seq_number( const struct packet * pt_packet );

/* get the ack number of a packet */
int get_ack_number( const struct packet * pt_packet );

/* return the first seq number of packet in the window or, seq number of next packet to receive */
int win_get_first_num( window * pt_win );

/* return the largest possible seq number of packet in the window / of packet to receive */
int win_get_last_num( window * pt_win );

/* construct a window_node for the packet and insert it between the owner of (pt_next_wn) and the node pointed by *pt_next_wn */
void wn_con( const struct packet * pt_packet, int datalen, window_node ** pt_next_wn );


/* remove packets from buf.
   send ack at the same time if it's a recv window. */
void win_dequeue( window * pt_win );

/* add a packet into the window */
int win_enqueue( window * pt_win, const struct packet * pt_packet, int datalen );

int win_isfull( window * pt_win );

window_node * win_get_last_node( window * pt_win );

int wn_get_packet_size( window_node * p_wn );

bool_t wn_is_packet_delayed( window_node * pt_wn );

window_node * wn_find_packet (window_node ** pt_next_wn, int seq, bool_t ack);


#endif

