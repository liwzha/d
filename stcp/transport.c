/*
 * transport.c 
 *
 * CS244a HW#3 (Reliable Transport)
 *
 * This file implements the STCP layer that sits between the
 * mysocket and network layers. You are required to fill in the STCP
 * functionality in this file. 
 *
 */


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "mysock.h"
#include "stcp_api.h"
#include "transport.h"
#include "window.h"

enum {CSTATE_ESTABLISHED,
    CSTATE_CLOSED,
    CSTATE_SYN_RCVD,
    CSTATE_SYNACK_RCVD,
    CSTATE_CLOSE_WAIT,
    CSTATE_LAST_ACK,
    CSTATE_TIME_WAIT
    };    /* obviously you should have more states */

static void generate_initial_seq_num(context_t *ctx);
static void control_loop(mysocket_t sd, context_t *ctx);

int fill_header(STCPHeader *pheader, int seqNum, int ackNum, int flag, int dataLen){

    pheader->th_seq = seqNum;
    pheader->th_ack = ackNum;
    pheader->th_flags = flag;
    pheader->th_off = sizeof( struct tcphdr );

    return sizeof( struct tcphdr ) + dataLen;
}


int wait_recv(mysocket_t sd, struct packet *p, int expected, int msec, int realrecv)
{
fprintf(stderr,"[wait_recv] inside wait_recv, msec = %d\n",msec);
    struct timespec *ts = (struct timespec *)malloc(sizeof(struct timespec));
    int n, ret;
    unsigned int mask=0;
    if(msec == 0) ts = NULL;
    else {
        /* (*ts) = time(NULL); */
        time(&(ts->tv_sec));
        ts->tv_sec += msec/1000;
        ts->tv_nsec = 5000;
    }

fprintf(stderr,"[wait_recv] about to call stcp_wait_for_event\n");
    mask = stcp_wait_for_event(sd, APP_DATA | NETWORK_DATA, ts);/* debugging */
fprintf(stderr,"[wait_recv] returned from stcp_wait_for_event\n");
    if (mask == TIMEOUT) {
          ret = -1;
          printf("wait_recv: timeout\n");
    }
    else if (expected >= 0 && (mask & NETWORK_DATA)) { /*receive from network*/
fprintf(stderr,"[wait_recv] receive from network\n");
        n = stcp_network_recv(sd, (char *)p, sizeof(struct packet));
        printf("wait_recv got %d bytes from network, flag: %d / %d\n", n, p->pa_header.th_flags, expected);
        if(p->pa_header.th_flags == expected)
            ret = 0;
        else
            ret = -1;
    }
    else if (expected < 0 && (mask & APP_DATA)) { /*receive from app*/
fprintf(stderr,"[wait_recv] receive from app\n");
        n = stcp_app_recv(sd, p->data, 536);
        printf("wait_recv got %d bytes from app \n", n);
        if (n>0)
            ret = 0;
        else
            ret = -1; 
    }
   
    free(ts);

fprintf(stderr,"[wait_recv] about to return, return %d, expected:%d \n", ret,expected);
    return ret;
}

/* dummy function for compilation.  TODO delete this function?  */
void dump_packet(struct packet *p){
    return;
}

/* dummy function for compilation.  TODO modify / delete this function? */
static void exit_control_loop(context_t *ctx, int reason)
{
    /* ctx->control_loop_not_running = reason; */
    /* TODO */
    transport_passive_close(ctx);
}


/* initialise the transport layer, and start the main loop, handling
 * any data from the peer or the application.  this function should not
 * return until the connection is closed.
 */
void transport_init(mysocket_t sd, bool_t is_active)
{
fprintf(stderr, "[transport_init] inside transport_init\n");
    context_t *ctx;
    struct packet *packet = malloc(sizeof(struct packet));
    struct tcphdr *hdr = malloc(sizeof(struct tcphdr));
    int packetlen, sent, peerlen;

    ctx = (context_t *) calloc(1, sizeof(context_t));
    assert(ctx);
    ctx->connection_state = CSTATE_CLOSED;
    ctx->sockfd = sd;
    ctx->cong_estrtt = 100000;

    srand(time(NULL));
    generate_initial_seq_num(ctx);

    /* XXX: you should send a SYN packet here if is_active, or wait for one
     * to arrive if !is_active.  after the handshake completes, unblock the
     * application with stcp_unblock_application(sd).  you may also use
     * this to communicate an error condition back to the application, e.g.
     * if connection fails; to do so, just set errno appropriately (e.g. to
     * ECONNREFUSED, etc.) before calling the function.
     */
    ctx->seq_active = ctx->initial_sequence_num;
    if(is_active == 1) /* active mode */
    {
        while(ctx->connection_state != CSTATE_ESTABLISHED)
     {
        fprintf(stderr, "[transport_init] is_active == True \n");
        memset((void *)hdr, 0, sizeof(struct tcphdr));
        memset((void *)packet, 0, sizeof(struct packet));
        switch (ctx->connection_state)
         {
            case CSTATE_CLOSED:
                printf("active client:CLOSED\n");
                /* send SYN */
                packetlen = fill_header(&(packet->pa_header),ctx->seq_active,0,TH_SYN, 0);
                /* packet->pa_header = *hdr; */
                /* dump_packet(packet) */
                sent = stcp_network_send(sd,(char *)packet, packetlen,NULL);
                if(sent == -1) perror("network_send");
                struct timeval sendtime;
		gettimeofday(&sendtime, NULL);
		printf("network_send: %d bytes\n", sent);
                printf("active child: SYN_SENT\n");

                /* recv SYNACK */
                if(wait_recv(sd, packet, TH_SYN | TH_ACK, 0, 1) == 0)
                {
		    estimate_rtt(ctx,sendtime);
                    ctx->ack_active = packet->pa_header.th_ack;
                    ctx->seq_passive = packet->pa_header.th_seq;
                    ctx->ack_passive = packet->pa_header.th_seq + 1;
                    ctx->connection_state = CSTATE_SYNACK_RCVD;
                }
                else
                {
                    ctx->connection_state = CSTATE_CLOSED;
                }
                break;

            case CSTATE_SYNACK_RCVD:
                dprintf("active child: SYNACK_RCVD\n");
/* send ACK */
                packetlen = fill_header(hdr, 0, ctx->ack_passive, TH_ACK, 0);
                packet->pa_header = *hdr;
                /*dump_packet(packet);*/
                sent = stcp_network_send(sd, (char *)packet, packetlen, NULL);
        fprintf(stderr,"ACK sent\n");
                /*debug("network_send: %d bytes\n", sent);*/
                if(sent == -1) perror("network_send");
                /* if recv SYNACK again  */
                if(wait_recv(sd, packet, TH_SYNACK, 1000, 1) == 0) {
                    ctx->connection_state = CSTATE_SYNACK_RCVD;
                }
                else 
		{
		estimate_rtt(ctx, packet->pa_header.th_seq);
		ctx->connection_state = CSTATE_ESTABLISHED;
		printf("Established!\n");
		}
                break;

            default:
                break;
        }
     }
    }

    else
    {/* Passive mode */
        while(ctx->connection_state != CSTATE_ESTABLISHED)
        {
            memset((void *)hdr, 0, sizeof(struct tcphdr));
            memset((void *)packet, 0, sizeof(struct packet));
            switch(ctx->connection_state)
           {
                case CSTATE_CLOSED:
                    printf("passive child: CLOSED\n");
                    /* wait for SYN */
                    if(wait_recv(sd, packet, TH_SYN, 0, 1) == 0)
                    {
                        ctx->seq_passive = packet->pa_header.th_seq;
                        ctx->ack_passive = packet->pa_header.th_seq + 1;
                        ctx->connection_state = CSTATE_SYN_RCVD;
                    }
                    else continue;
                    break;
                case CSTATE_SYN_RCVD:
                    printf("passive child: SYN RCVD\n");
                    /* send SYNACK */
                    packetlen = fill_header(hdr, ctx->seq_active, ctx->ack_passive, TH_SYN | TH_ACK, 0);
                    packet->pa_header = *hdr;
                    /* dump_packet(packet); */
                    sent = stcp_network_send(sd, (char *)packet, packetlen, NULL);
		    fprintf(stderr,"SYNACK sent\n");
	            struct timeval sendtime;
		    gettimeofday(&sendtime,NULL);
                    printf("network_send:%d bytes\n", sent);
                    if(sent == -1) perror("network_send");
                    /* recv ACK */
                    if(wait_recv(sd, packet, TH_ACK, 0, 1) == 0)
                    {
			estimate_rtt(ctx, sendtime);
                        ctx->ack_active = packet->pa_header.th_ack;
                        ctx->connection_state = CSTATE_ESTABLISHED;
                        printf("Established!!!\n");
                        break;
                    }

                    else
                        ctx->connection_state=CSTATE_SYN_RCVD;
                    break;
                default:
                    break;
            }
        }
    }

    ctx->seq_active++;
    ctx->seq_passive++;
    printf("SYN handshaking passed\n");

    printf("@@@ seq/ack active: %d %d \n", ctx->seq_active, ctx->ack_active);
    printf("@@@ seq/ack passive: %d %d \n", ctx->seq_passive, ctx->ack_passive);

    free(hdr);
    free(packet);




    ctx->connection_state = CSTATE_ESTABLISHED;
    stcp_unblock_application(sd);

    control_loop(sd, ctx);

    /* do any cleanup here */
    free(ctx);
}

/**********************************************************************/
/* transport_active_close
 *
 * Called from child process at transport_appl_io()
 */
int transport_active_close(context_t *ctx)
{
    int packetlen;
    struct tcphdr *hdr = (struct tcphdr *)malloc(sizeof(struct tcphdr));
    struct packet *packet = (struct packet *)malloc(sizeof(struct packet));

    /* close(ctx->local_data_sd); */
    close(ctx->sockfd);
    printf("transport_active_close()\n");
    while(ctx->connection_state!=CSTATE_CLOSED)
    {



        memset((void *)hdr, 0, sizeof(struct tcphdr));
        memset((void *)packet, 0, sizeof(struct packet));

        switch(ctx->connection_state) {
            case CSTATE_ESTABLISHED:
                printf("active child: ESTABLISHED\n");
                /* send FIN */
                packetlen = fill_header(hdr, ctx->seq_active, 0, TH_FIN, 0);
                packet->pa_header = *hdr;
                dump_packet(packet);
                if(stcp_network_send(ctx->sockfd, (char *)packet, packetlen, NULL) == -1)
                    perror("network_send");
                /* recv FINACK */
                if(wait_recv(ctx->sockfd, packet, TH_FIN | TH_ACK, 0, 1) == 0) {
                    ctx->ack_active = packet->pa_header.th_ack;
                    ctx->ack_passive = packet->pa_header.th_seq + 1;
                    ctx->connection_state = CSTATE_TIME_WAIT;
                }
                else {
                    ctx->connection_state = CSTATE_ESTABLISHED;
                }
                break;
            case CSTATE_TIME_WAIT:
                printf("active child: TIME_WAIT\n");
                /* send ACK */
                packetlen = fill_header(hdr, 0, ctx->ack_passive, TH_ACK, 0);
                packet->pa_header = *hdr;
                dump_packet(packet);
                if(stcp_network_send(ctx->sockfd, (char *)packet, packetlen, NULL) == -1)
                    perror("network_send");
                printf("Send ACK Finished\n");
               /* if recv SYNACK again */
                /*
                if(wait_recv(ctx->sockfd, packet, TH_SYN | TH_ACK, 500, 1) == 0) {
                    ctx->connection_state = CSTATE_ESTABLISHED;
                }
                else {*/
                    /* terminate connection */
                /*else {*/
                    /* terminate connection */
                    close(ctx->sockfd);
                    ctx->connection_state = CSTATE_CLOSED;
/*                    exit_control_loop(1);*/

               /* }*/
                break;
            default:
                break;
        }


    }

    printf("FIN handshaking passed\n");
    printf("!! seq / ack active : %d %d\n", ctx->seq_active, ctx->ack_active);
    printf("!! ack passive: %d\n", ctx->ack_passive);


    ctx -> done = 1;
    free(hdr);
    free(packet);
    return 0;
}


/**********************************************************************/
/* transport_passive_close
 *
 * Called from child process at transport_sock_io()
 */

int transport_passive_close(context_t *ctx) {
    int packetlen;
    struct tcphdr *hdr = (struct tcphdr *)malloc(sizeof(struct tcphdr));
    struct packet *packet = (struct packet *)malloc(sizeof(struct packet));

    printf(">>transport_passive_close()\n");
    while(ctx->connection_state != CSTATE_CLOSED) {
        memset((void *)hdr, 0, sizeof(struct tcphdr));
        memset((void *)packet, 0, sizeof(struct packet));
        switch(ctx->connection_state) {
            case CSTATE_CLOSE_WAIT:
                printf("passive child: CLOSE_WAIT\n");
                /* send FINACK */
                packetlen = fill_header(hdr, ctx->seq_active, ctx->ack_passive, TH_FIN | TH_ACK, 0);
                packet->pa_header = *hdr;
                dump_packet(packet);
                if(stcp_network_send(ctx->sockfd, (char *)packet, packetlen, NULL) == -1)
                    perror("network_send");
                ctx->connection_state = CSTATE_LAST_ACK;
                break;
            case CSTATE_LAST_ACK:
                printf("passive child: LAST_ACK\n");
                /* recv ACK */
                if(wait_recv(ctx->sockfd, packet, TH_ACK, 0, 1) == 0) {

                printf("CLOSED!!!!\n");
                     ctx->ack_active = packet->pa_header.th_ack;
                    /* terminate connection */
               /*     close(ctx->sockfd);  */
printf("dddd\n");
                    ctx->connection_state = CSTATE_CLOSED;

    printf("state: %d\n", ctx->connection_state);
    /*exit_control_loop(1);*/
                }
                else ctx->connection_state = CSTATE_CLOSE_WAIT;
                break;
            default:
                break;
        }
        printf("state: %d\n", ctx->connection_state);
    }

    printf("FIN handshaking passed\n");
    printf("!! seq / ack active : %d %d\n", ctx->seq_active, ctx->ack_active);
    printf("!! ack passive: %d\n", ctx->ack_passive);
    ctx->done = 1;
    close(ctx->sockfd);
    free(hdr);
    free(packet);
    return 0;
}



/* generate random initial sequence number for an STCP connection */
static void generate_initial_seq_num(context_t *ctx)
{
    assert(ctx);

#ifdef FIXED_INITNUM
    /* please don't change this! */
    ctx->initial_sequence_num = 1;
#else
    /* you have to fill this up */
    ctx->initial_sequence_num =rand()%256;
#endif
}


/* control_loop() is the main STCP loop; it repeatedly waits for one of the
 * following to happen:
 *   - incoming data from the peer
 *   - new data from the application (via mywrite())
 *   - the socket to be closed (via myclose())
 *   - a timeout
 */
static void control_loop(mysocket_t sd, context_t *ctx)
{
/* tag:control_loop */
fprintf(stderr,"[control_loop] inside control_loop\n");
    struct packet *p_packet;
    struct packet *p_packet2;
    
    int datalen;
    assert(ctx);

    window recv_window;
    window send_window;

    win_init( &recv_window, WIN_RECV, ctx );
    win_init( &send_window, WIN_SEND, ctx );
    struct timespec *ts_timeout = NULL;

    unsigned int event_towait = ANY_EVENT;
    while (!ctx->done)
    {
        unsigned int event;
        if (win_isfull(&send_window))
            event_towait = TIMEOUT | NETWORK_DATA | APP_CLOSE_REQUESTED;
        else
            event_towait = ANY_EVENT;

fprintf(stderr,"[control loop] loop all over again\n");
fprintf(stderr,"[control loop] time out in %ld seconds \n", ts_timeout==NULL?-1:ts_timeout->tv_sec);
fprintf(stderr,"waiting for event...%s...",event_towait==ANY_EVENT?"ANY_EVENT":"ignoring app event");        
        /*
        ts_timeout = (struct timespec*) malloc(sizeof(struct timespec));
        time(&(ts_timeout->tv_sec));
        ts_timeout->tv_sec += 1;
        ts_timeout->tv_nsec = 5000;
        */   
        update_timeout_window_node(ctx, &ts_timeout, send_window.win_buf);

        event = stcp_wait_for_event(sd, event_towait, ts_timeout);
        /* event = stcp_wait_for_event(sd, ANY_EVENT, NULL); */
fprintf(stderr,"...got event!\n");
        /* check whether it was the network, app, or a close request */
        if (event & APP_DATA)  /* recv from app */
        {
fprintf(stderr,"[control_loop] event: APP_DATA\n");
            datalen = get_data_app(sd, ctx, &p_packet); 
            fill_header(&(p_packet->pa_header), ctx->seq_active,0,0,datalen);
            win_enqueue(&send_window, p_packet, datalen);
            update_timeout_window_node(ctx, &ts_timeout, send_window.win_buf);
/*  update_timeout(ctx, &ts_timeout, &send_window); */
        }
        if (event & NETWORK_DATA) /* recv from network */
        {
fprintf(stderr,"[control_loop] event: NETWORK_DATA\n");
            p_packet = malloc(sizeof(struct packet));
            datalen = stcp_network_recv(sd, (void*)(p_packet), sizeof(struct packet));
/* fprintf(stderr,"received %d bytes from network, headersize: %d bytes\n",datalen, sizeof(struct tcphdr)); */
            datalen -= sizeof(struct tcphdr);
            uint8_t flag = 0;
            flag = p_packet->pa_header.th_flags;
             
            /* if recv ack, change context and dequeue send window */
            if (flag & TH_ACK){
                fprintf(stderr,"Got ACK %d\n", p_packet->pa_header.th_ack);
                if (ctx->ack_active < p_packet->pa_header.th_ack){
                    ctx->ack_active = p_packet->pa_header.th_ack;
                    win_dequeue( &send_window );
                    /* update_timeout(ctx, &ts_timeout, &send_window);*/
                    update_timeout_window_node(ctx, &ts_timeout, send_window.win_buf);
                }
            }

            /* if recv data, add to recv window */
            if (datalen > 0){
               /* How about writing to the application layer??*/
                fprintf(stderr,"Got Data %d\n", p_packet->pa_header.th_seq);
                win_enqueue( &recv_window, p_packet, datalen );
                win_dequeue( &recv_window );
            }
            
            /* if recv FIN, passive close */
            if (flag & TH_FIN){
                    /* TODO */
                fprintf(stderr, "Got Fin\n");
                ctx->ack_passive = p_packet->pa_header.th_seq;
                ctx->connection_state = CSTATE_CLOSE_WAIT;
                free(p_packet);
                transport_passive_close(ctx);
                return;
            }
        }
        if (event == TIMEOUT){
            fprintf(stderr,"********************Got Timeout*************************\n");
        	window_node ** pt_next_wn;
        	pt_next_wn = &( send_window.win_buf);
        	while((*pt_next_wn) != NULL){        		
        		retransmit ( ctx, *pt_next_wn );
        		pt_next_wn = &((*pt_next_wn)->wn_next) ;
        	}
        }
        if (event & APP_CLOSE_REQUESTED){ /* active close */
            /* TODO  */         
	    	printf("Got closing request from the application\n");
            if(ctx->seq_active == ctx->ack_active){
            	/* Closing Control */
            	transport_active_close(ctx);
            	return;
            }
        }    
    }
}

/* get data from app and encapsulate it into a packet, filling header with 0. */
int get_data_app(mysocket_t sd, context_t *ctx, struct packet ** p_packet)
{        
        int datalen;
        *p_packet = (struct packet *)malloc(sizeof(struct packet));
        memset((void *)(*p_packet), 0, sizeof(struct packet));  
        datalen = stcp_app_recv(ctx->sockfd, (char *)(*p_packet)->data, 536);
        return datalen;
}

int send_packet (context_t *ctx, struct packet *pckt, int datalen, int flag)
{
        struct tcphdr *hdr;
        int packetlen;
        unsigned int sent;
 	if(flag==1){
fprintf(stderr,"*** [send_packet]: about to send data to network\n");
        	sent = stcp_network_send (ctx->sockfd, (char *)pckt, sizeof(struct tcphdr) + datalen, NULL);
fprintf(stderr,"*** [send_packet]: data sent to network, sent %d bytes\n",sent);
                return sent;
        }
        else{
fprintf(stderr,"*** [send_packet]: about to send data to app\n");
		stcp_app_send (ctx->sockfd, (char *)pckt->data, datalen);
fprintf(stderr,"*** [send_packet]: data sent to app\n");
                return 0;
        }
        free(hdr);
        free(pckt);
}


/**********************************************************************/
/* our_dprintf
 *
 * Send a formatted message to stdout.
 * 
 * format               A printf-style format string.
 *
 * This function is equivalent to a printf, but may be
 * changed to log errors to a file if desired.
 *
 * Calls to this function are generated by the dprintf amd
 * dperror macros in transport.h
 */
void our_dprintf(const char *format,...)
{
    va_list argptr;
    char buffer[1024];

    assert(format);
    va_start(argptr, format);
    vsnprintf(buffer, sizeof(buffer), format, argptr);
    va_end(argptr);
    fputs(buffer, stdout);
    fflush(stdout);
}

void estimate_rtt (context_t *ctx, struct timeval sendtime)
{
	struct timeval t;
    gettimeofday (&t, NULL);   
    long int sample_rtt;
    sample_rtt = 1000000 * (t.tv_sec - sendtime.tv_sec) + t.tv_usec - sendtime.tv_usec;
    ctx->cong_estrtt = (long int)(0.875 * ctx->cong_estrtt + 0.125 * sample_rtt);
}


void retransmit (context_t *ctx,struct timespec **t, window_node *seg )
{
fprintf(stderr, "************retransmiting***********\n");
    int rv = -1;
    if(seg==NULL)
	return;
    while (seg->wn_retransmitcnt < 6 && rv<0) {
            /* retransmit using Go-Back-N */
    	if (ctx->connection_state == CSTATE_LAST_ACK) {
        	ctx->connection_state = CSTATE_CLOSED;
        	ctx->done = TRUE;
    	}	
	              
    	seg->wn_retransmitcnt++;
fprintf(stderr,"Size of packet %d %d %d\n",wn_get_packet_size(seg),seg->wn_datalen,sizeof(struct packet));
	  /*  rv = stcp_network_send(ctx->sockfd,(char *)seg->wn_packet, wn_get_packet_size(seg),NULL);
    	    */
        rv = send_packet(ctx, (char *)seg->wn_packet,seg->wn_datalen, 1);
        gettimeofday (&seg->wn_sendtime, NULL);
        update_timeout_window_node(ctx, t, seg);
    	if(rv < 0)
		fprintf(stderr,"Packet sending failed!!");  
        if (seg->wn_retransmitcnt > 5){
     	    fprintf(stderr,"Network down!!");
            ctx->done = TRUE;
        }             
    }
    
fprintf(stderr, "************exiting: retransmiting***********\n"); 
}

/*
void update_timeout(context_t *ctx, struct timespec **t, window *pt_win){
    struct timeval rtt_timeout;
 TODO: change the next line: need null check 

    struct timeval packet_time;
    if (pt_win->win_buf == NULL){
        if ((*t)!=NULL)
            free(*t);
        (*t) = NULL;
fprintf(stderr,"------update timeout----->>no timeout now\n");
    }
    else{
        if ((*t)==NULL)
            (*t) = (struct timespec *) malloc(sizeof(struct timespec));
        packet_time = (pt_win->win_buf)->wn_sendtime;
        rtt_timeout.tv_sec = (int)(ctx->cong_estrtt/1000000);
        rtt_timeout.tv_usec = ctx->cong_estrtt - rtt_timeout.tv_sec * 1000000;
         convert to timespec 
        (*t)->tv_nsec = (rtt_timeout.tv_usec + packet_time.tv_usec )* 1000;
        (*t)->tv_sec = (rtt_timeout.tv_sec + packet_time.tv_sec + 1); 

fprintf(stderr,"------update timeout----->>timeout is set to %ld sec\n",(*t)->tv_sec);
    }
}
*/
void update_timeout_window_node(context_t *ctx, struct timespec **t, window_node *pt_win_node){
    struct timeval rtt_timeout;

    struct timeval packet_time;
    if (pt_win_node == NULL){
        if ((*t)!=NULL)
            free(*t);
        (*t) = NULL;
fprintf(stderr,"------update timeout----->>no timeout now\n");
    }
    else{
        if ((*t)==NULL)
            (*t) = (struct timespec *) malloc(sizeof(struct timespec));
        packet_time = pt_win_node->wn_sendtime;
        rtt_timeout.tv_sec = (long int)(ctx->cong_estrtt/1000000);
        rtt_timeout.tv_usec = ctx->cong_estrtt - rtt_timeout.tv_sec * 1000000;
        /* convert to timespec */
/*
 (*t)->tv_nsec = (rtt_timeout.tv_usec + packet_time.tv_usec )* (long int)1000; 
 (*t)->tv_sec = (rtt_timeout.tv_sec + packet_time.tv_sec + 1); 
long int sec_loc = (*t)->tv_nsec / (long int) 1000000;
 (*t)->tv_nsec %= 1000000;
 (*t)->tv_sec += sec_loc; 
*/


        (*t)->tv_nsec = ( packet_time.tv_usec )*1000;
        (*t)->tv_sec = (1 + packet_time.tv_sec);  
fprintf(stderr,"tv %ld:%ld \n",rtt_timeout.tv_sec, rtt_timeout.tv_usec);
fprintf(stderr,"------update timeout----->>timeout is set to %ld:%d sec and estimated_rtt=%d\n",(*t)->tv_sec,(*t)->tv_nsec,ctx->cong_estrtt);
    }
}

