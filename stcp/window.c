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
window_node * wn_con( const struct packet * pt_packet, int datalen, window_node ** pt_next_wn ){
    window_node * pt_wn = malloc(sizeof(window_node));
    pt_wn->wn_packet = pt_packet;
    pt_wn->wn_next = (*pt_next_wn);
    pt_wn->wn_datalen = datalen;
    (*pt_next_wn) = pt_wn;
    return pt_wn;
}

/* remove packets from buf.
   send ack at the same time if it's a recv window. */
void win_dequeue( window * pt_win ){
fprintf(stderr,">>>win_dequeue: inside win_dequeue\n");
    window_node * pt_wn = pt_win->win_buf;
    window_node * pt_wn_tmp;
    while( pt_wn != NULL && 
       win_get_first_num(pt_win) > get_seq_number( pt_wn->wn_packet)){

       /* if this is a recv window, send data to app and ack */
       if( pt_win->win_type == WIN_RECV){
           /* send ack for this packet */
           /* TODO need to change this for part b */
           struct packet pa;
           int ackloc =  get_seq_number(pt_wn->wn_packet)+pt_wn->wn_datalen;
           fill_header(pa.pa_header, 0, ackloc, TH_ACK, 0); 
           send_packet(pt_win->win_ctx, &pa, 0, 1);
           /* change context */
           pt_win->win_ctx->ack_passive = ackloc; 
           /* send data to app */
           send_packet(pt_win->win_ctx, pt_wn->wn_packet,pt_wn->wn_datalen ,0);
       }


       /* if this is a send window, just dequeue the acked packets from the front of the window */
       pt_wn_tmp = pt_wn;
       pt_wn = pt_wn->wn_next;
       free( pt_wn_tmp );
    }

fprintf(stderr,">>>win_dequeue: about to return\n");
}

/* for send window, send out the packet and enqueue.
   for recv window, just enqueue.
 */
int win_enqueue( window * pt_win, const struct packet * pt_packet, int datalen ){
fprintf(stderr,"<<<win_enqueue: inside win_enqueue\n");
   int seqNum;
   window_node ** pt_next_wn;

   seqNum = get_seq_number( pt_packet );
   
   if ( seqNum < win_get_first_num( pt_win ) ){
       fprintf(stderr,"[window] rejecting a packet -- seq # too low\n");
       return -1;
   } 
   if ( seqNum+datalen-1 > win_get_last_num( pt_win ) ){
       fprintf(stderr,"[window] rejecting a packet -- seq # too high\n");
       return -1;
   }

   /* iterate through packets in the buf list.  
      insert the new packet in the list so that the seq numbers of packets 
      in the list are kept in increasing order. */
   for (pt_next_wn = &(pt_win->win_buf) ;
       (*pt_next_wn) != NULL && seqNum > get_seq_number( (*pt_next_wn)->wn_packet ) ;
       pt_next_wn = &((*pt_next_wn)->wn_next)  )
       ;
   wn_con( pt_packet, datalen, pt_next_wn );

   /* if this is a send window, send the packet to newtork layer. 
      increment seq_active.
   */  
   if (pt_win->win_type == WIN_SEND){
       send_packet(pt_win->win_ctx, pt_packet, datalen,1); 
       pt_win->win_ctx->seq_active += datalen;
   }
 
fprintf(stderr,"<<<win_enqueue: about to return\n");
   return 1;
}

int wn_get_packet_size( window_node * p_wn ){
    return sizeof(struct tcphdr) + p_wn->wn_datalen;
}
