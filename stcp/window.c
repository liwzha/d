#include"window.h"
/*
void win_init( window * pt_win ){
pt_win->win_type = WIN_UNDEF;
win_firstNum = -1;
win_buf = NULL;
}
*/
void win_init( window * pt_win, enum window_type wt, context_t* ctx){
    pt_win->win_type = wt;
    pt_win->win_ctx = ctx;
    pt_win->win_buf = NULL;
}

int get_seq_number( const struct packet * pt_packet ){
    return pt_packet->pa_header.th_seq;
}

int get_ack_number( const struct packet * pt_packet ){
    return pt_packet->pa_header.th_ack;
}

/* return number of packets in the buf */
int win_getsize( window * pt_win ){
    window_node * p_wn = pt_win->win_buf;
    int counter = 0;
    while( p_wn != NULL ){
        p_wn = p_wn->wn_next;
        counter ++;
    }
    return counter;
}

/* return the first seq number of packet in the window or, seq number of next packet to receive */
int win_get_first_num( window * pt_win ){
    if (pt_win->win_type == WIN_RECV)
        return pt_win->win_ctx->ack_passive;
    if (pt_win->win_type == WIN_SEND){
        return pt_win->win_ctx->ack_active;
    }
}

/* return the largest possible seq number of packet in the window / of packet to receive */
int win_get_last_num( window * pt_win ){
    return win_get_first_num(pt_win) + 3072 - 1; /* TODO save window size--3072 as variable*/
}

/* construct a window_node for the packet and insert it between the owner of (pt_next_wn) and the node pointed by *pt_next_wn */
void wn_con( const struct packet * pt_packet, int datalen, window_node ** pt_next_wn ){
fprintf(stderr,">>>window_node:inside window_node \n");
    int n, ret;
    unsigned int mask=0;
    window_node * pt_wn = malloc(sizeof(window_node));
    
    pt_wn->wn_packet = pt_packet;
    pt_wn->wn_next = (*pt_next_wn);
    pt_wn->wn_datalen = datalen;
    gettimeofday(&(pt_wn->wn_sendtime) ,NULL);
    (*pt_next_wn) = pt_wn;
    pt_wn->wn_retransmitcnt = 0;
fprintf(stderr,"<<<<window_node:exiting window_node %d %d\n",
              pt_wn->wn_sendtime.tv_sec,
              pt_wn->wn_sendtime.tv_usec);
}

/* remove packets from buf.
send ack at the same time if it's a recv window. */
void win_dequeue( window * pt_win ){
    if(pt_win->win_buf == NULL) return;
fprintf(stderr,">>> [%s window] win_dequeue: inside win_dequeue, num of packets in buf: %d, first num: %d\n", pt_win->win_type==WIN_RECV?"recv":"send", win_getsize(pt_win), win_get_first_num(pt_win));
    window_node ** p_pt_wn = &(pt_win->win_buf);
    window_node * pt_wn_tmp;

    int ackloc=-1;

    while( (*p_pt_wn) != NULL && win_dequeue_helper(pt_win, get_seq_number( (*p_pt_wn)->wn_packet))){
       /* if this is a recv window, cumulative ack , send data to app */
       if (pt_win->win_type == WIN_RECV){
          ackloc = get_seq_number((*p_pt_wn)->wn_packet)+(*p_pt_wn)->wn_datalen;
          /* send data to app */
          send_packet((pt_win)->win_ctx, (*p_pt_wn)->wn_packet,(*p_pt_wn)->wn_datalen ,0);
       }
           /* if this is a send window, just dequeue the acked packets from the front of the window*/
       pt_wn_tmp = (*p_pt_wn);
       (*p_pt_wn) = pt_wn_tmp->wn_next;
       free( pt_wn_tmp );
    }

   if (pt_win->win_type == WIN_RECV && ackloc!=-1){
           /* send ack for this packet */
           struct packet pa;
           fill_header(&(pa.pa_header), 0, ackloc, TH_ACK, 0);
           send_packet(pt_win->win_ctx, &pa, 0, 1);
fprintf(stderr,"ack sent, ack #: %d\n", ackloc);
           /* change context */
           (pt_win)->win_ctx->ack_passive = ackloc;
       }


fprintf(stderr,">>>win_dequeue: about to return, packets in the buf %d, win lower bound: %d\n",win_getsize(pt_win), win_get_first_num(pt_win));
}

/* for send window, send out the packet and enqueue.
for recv window, just enqueue.
*/
int win_enqueue( window * pt_win, const struct packet * pt_packet, int datalen ){
fprintf(stderr,"<<<[%s window] win_enqueue: inside win_enqueue\n",pt_win->win_type==WIN_RECV?"recv":"send");
   int seqNum;
   window_node ** pt_next_wn;
   seqNum = get_seq_number( pt_packet );
   
   if ( seqNum < win_get_first_num( pt_win ) ){
       fprintf(stderr,"[%s window] rejecting a packet -- seq # too low\n",pt_win->win_type==WIN_RECV?"recv":"send") ;
       return -1;
   }
/* TODO: for sending window, should not just reject the packet */
   if ( seqNum+datalen-1 > win_get_last_num( pt_win ) ){
       fprintf(stderr,"[%s window] rejecting a packet -- seq # too high\n",pt_win->win_type==WIN_RECV?"recv":"send");
       if (pt_win->win_type == WIN_RECV)
           return -1;
   }

   /* iterate through packets in the buf list.
insert the new packet in the list so that the seq numbers of packets
in the list are kept in increasing order. */
   for (pt_next_wn = &(pt_win->win_buf) ;
       (*pt_next_wn) != NULL && seqNum > get_seq_number( (*pt_next_wn)->wn_packet ) ;
       pt_next_wn = &((*pt_next_wn)->wn_next) ) ;
   wn_con( pt_packet, datalen, pt_next_wn );

   /* if this is a send window, send the packet to newtork layer.
increment seq_active.
*/
   if (pt_win->win_type == WIN_SEND){
fprintf(stderr,"this is a send window, about to call send_packet\n");
       send_packet(pt_win->win_ctx, pt_packet, datalen,1);
       pt_win->win_ctx->seq_active += datalen;
   }
 
fprintf(stderr,"<<<win_enqueue: about to return, packets in the buf: %d\n",win_getsize(pt_win));
   return 1;
}

int wn_get_packet_size( window_node * p_wn ){
    return sizeof(struct tcphdr) + p_wn->wn_datalen;
}

window_node * win_get_last_node( window * pt_win ){
   window_node * pt_wn = pt_win->win_buf;
   while (pt_wn != NULL && pt_wn->wn_next != NULL){
       pt_wn = pt_wn->wn_next;  
   }
   return pt_wn;
}

int win_isfull( window * pt_win ){
    window_node * pt_wn = win_get_last_node( pt_win );
    if (pt_wn == NULL)
        return 0;
    if (pt_wn->wn_datalen + get_seq_number(pt_wn->wn_packet) > win_get_last_num( pt_win))
        return 1;

    return 0;
}

int win_dequeue_helper( window* pt_win, int seq_num ){
    if (pt_win->win_type == WIN_RECV && 
        win_get_first_num(pt_win) >= seq_num)
        return 1;
    if (pt_win->win_type == WIN_SEND && 
        win_get_first_num(pt_win) > seq_num)
        return 1;
    return 0;

}
