#include"window.h"

void win_init( window * pt_win ){
    pt_win->win_type = WIN_UNDEF;
    win_firstNum = -1;
    win_buf = NULL;
} 

void win_init( window * pt_win, window_type wt, int first_seq_num){
    pt_win->win_type = wt;
    pt_firstNum = first_seq_num;
    win_buf = NULL;
}

int get_seq_number( const packet * pt_packet ){
    return pt_packet->pa_header.th_seq;
}

int get_ack_number( const packet * pt_packet ){
    return pt_packet->pa_header.th_ack;
}

int get_data_length( const packet * pt_packet ){
    return pt_packet->pa_header.
}

/* return the first seq number of packet in the window / of packet to receive */
int win_get_first_num( window * pt_win ){
    return pt_win->win_lowNum;
}

/* return the largest possible seq number of packet in the window / of packet to receive */
int win_get_last_num( window * pt_win ){
    return pt_win->win_lowNum + pt_win->win_size;
}

/* construct a window_node for the packet and insert it between the owner of (pt_next_wn) and the node pointed by *pt_next_wn */
window_node * wn_con( const packet * pt_packet, window_node ** pt_next_wn ){
    window_node * pt_wn = malloc(sizeof(window_node));
    pt_wn->wn_packet = pt_packet;
    pt_wn->wn_next = (*pt_next_wn)->wn_next;
    (*pt_next_wn) = pt_wn;
    return pt_wn;
}


/* remove packets from buf.
   send ack at the same time if it's a recv window. */
void win_dequeue( window * pt_win ){
    window_node * pt_wn = pt_win->win_buf;
    window_node * pt_wn_tmp;
    while( pt_wn != NULL || 
       win_get_first_num(pt_win) == get_seq_num( pt_wn->wn_packet)){
                     
       if( pt_win->win_type == WIN_SEND ){
           //TODO send ack for this packet
           //TODO send data to app
       }
       pt_wn_tmp = pt_wn;
       pt_wn = pt_wn->next;
       free( pt_wn_tmp );
    }
}

int win_enqueue( window * pt_win, const packet * pt_packet ){
   int seqNum, dataLen;
   window_node ** pt_next_wn;

   seqNum = get_seq_number( pt_packet );
   dataLen = get_data_length( pt_packet );
   
   if ( seqNum < win_get_first_num( pt_win ) ){
       fprintf(stderr,"[window] rejecting a packet -- seq # too low\n");
       return -1;
   } 
   if ( seqNum+dataLen-1 > win_get_last_num( pt_win ) ){
       fprintf(stderr,"[window] rejecting a packet -- seq # too high\n");
       return -1;
   }

   /* iterate through packets in the buf list.  
      insert the new packet in the list so that the seq numbers of packets 
      in the list are kept in increasing order. */
   for (pt_next_wn = &(pt_win->win_buf) ;
       pt_next_wn != NULL && seqNum > get_seq_num( (*pt_next_wn)->wn_packet ) ;
       pt_next_wn = &((*pt_next_wn)->wn_next)  )
       ;

   wn_con( pt_packet, pt_next_wn );
   
   return 1;
}


