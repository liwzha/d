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
    gettimeofday(&pt_wn->wn_sendtime ,NULL);
    (*pt_next_wn) = pt_wn;
    pt_wn->wn_retransmitcnt = 0;
fprintf(stderr,"<<<<window_node:exiting window_node %d %d\n",
              pt_wn->wn_sendtime.tv_sec,
              pt_wn->wn_sendtime.tv_usec);
}

/* remove packets from buf.
   send ack at the same time if it's a recv window. */
void win_dequeue( window * pt_win ){
    if(pt_win->win_buf == NULL)  return;
fprintf(stderr,">>>win_dequeue: inside win_dequeue, num of packets in buf: %d\n",win_getsize(pt_win));
    window_node ** p_pt_wn = (&pt_win->win_buf);
    window_node * pt_wn_tmp;
fprintf(stderr,"...before whilei, win seq num:%d, packet's seq num:%d\n",win_get_first_num(pt_win),get_seq_number((*p_pt_wn)->wn_packet));
    while( (*p_pt_wn) != NULL && 
       win_get_first_num(pt_win) >= get_seq_number( (*p_pt_wn)->wn_packet)){
fprintf(stderr,"...inside while\n");
       /* if this is a recv window, send data to app and ack */
       if (pt_win->win_type == WIN_RECV){
           /* send ack for this packet */
           /* TODO need to change this for part b */
           struct packet pa;
           int ackloc =  get_seq_number((*p_pt_wn)->wn_packet)+(*p_pt_wn)->wn_datalen;
fprintf(stderr,"before fill header\n");
           fill_header(&(pa.pa_header), 0, ackloc, TH_ACK, 0); 
fprintf(stderr,"after fill header\n");
           send_packet(pt_win->win_ctx, &pa, 0, 1);
fprintf(stderr,"ack sent\n");
           /* change context */
           (pt_win)->win_ctx->ack_passive = ackloc; 
           /* send data to app */
           send_packet((pt_win)->win_ctx, (*p_pt_wn)->wn_packet,(*p_pt_wn)->wn_datalen ,0);
       }


       /* if this is a send window, just dequeue the acked packets from the front of the window*/ 
	pt_wn_tmp = (*p_pt_wn);
       (*p_pt_wn) = pt_wn_tmp->wn_next;
       free( pt_wn_tmp );
    }

fprintf(stderr,">>>win_dequeue: about to return, packets in the buf %d\n",win_getsize(pt_win));
}

/* for send window, send out the packet and enqueue.
   for recv window, just enqueue.
 */
int win_enqueue( window * pt_win, const struct packet * pt_packet, int datalen ){
fprintf(stderr,"<<<win_enqueue: inside win_enqueue\n");
   int seqNum;
   window_node ** pt_next_wn;
fprintf(stderr,"before get seq number\n");
   seqNum = get_seq_number( pt_packet );
fprintf(stderr,"after get seq number\n");
   
   if ( seqNum < win_get_first_num( pt_win ) ){
       fprintf(stderr,"[window] rejecting a packet -- seq # too low\n");
       return -1;
   } 
   if ( seqNum+datalen-1 > win_get_last_num( pt_win ) ){
       fprintf(stderr,"[window] rejecting a packet -- seq # too high\n");
       return -1;
   }
   if ( wn_is_packet_delayed( pt_win )){
       fprintf(stderr,"[window] rejecting a packet -- sequence timed out\n");
       return -1;
   }

fprintf(stderr,"before for loop\n");
   /* iterate through packets in the buf list.  
      insert the new packet in the list so that the seq numbers of packets 
      in the list are kept in increasing order. */
   for (pt_next_wn = &(pt_win->win_buf) ;
       (*pt_next_wn) != NULL && seqNum > get_seq_number( (*pt_next_wn)->wn_packet ) ;
       pt_next_wn = &((*pt_next_wn)->wn_next)  )       ;
fprintf(stderr,"before wn_con\n");
   wn_con( pt_packet, datalen, pt_next_wn );
fprintf(stderr,"after wn_con\n");

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

/*TODO*/
bool_t wn_is_packet_delayed( window_node * pt_wn ){
    return FALSE;
}

/* TODO*/
window_node * wn_find_packet (window_node ** pt_next_wn, int seq, bool_t ack){
    if(ack == 0){
        window_node *p_pt_wn = *pt_next_wn;
        while(p_pt_wn) {
                if( get_seq_number(p_pt_wn->wn_packet) == seq) {
                        return p_pt_wn;
                }
                
                p_pt_wn = p_pt_wn->wn_next;
        }

        return NULL;
    }
    else if(ack == 1){
        window_node *p_pt_wn = *pt_next_wn;

        while(p_pt_wn) {
                if(get_seq_number(p_pt_wn->wn_packet) +  p_pt_wn->wn_datalen == seq) { /* plus header?*/
                        return p_pt_wn;
                } else if ( get_seq_number(p_pt_wn->wn_packet) >= seq) {
                        return NULL;
                }                
                p_pt_wn = p_pt_wn->wn_next;
        }

        return NULL;
    }
    return NULL;
}

