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
/*
struct packet
{
    struct tcphdr pa_header; 
#define MAX_PACKET_SIZE 536
    char data[MAX_PACKET_SIZE];
};
*/


/* this structure is global to a mysocket descriptor */
typedef struct
{
    bool_t done;    /* TRUE once connection is closed */

    int connection_state;   /* state of the connection (established, etc.) */
    tcp_seq initial_sequence_num;

    /* any other connection-wide global variables go here */
    int seq_active;
    int ack_active;
    int seq_passive;
    int ack_passive;
    mysocket_t sockfd;
    
} context_t;


static void generate_initial_seq_num(context_t *ctx);
static void control_loop(mysocket_t sd, context_t *ctx);

int fill_header(STCPHeader *pheader, int seqNum, int ackNum, int flag, int dataLen){

    pheader->th_seq = seqNum;
    pheader->th_ack = ackNum;
    pheader->th_flags = flag;
    pheader->th_off = sizeof( STCPHeader );

    return sizeof( STCPHeader ) + dataLen;
}


/* only a dummy function for now. TODO complete the function, modify the interface if necessary. */
/*
int wait_recv(mysocket_t sd, struct packet *pt_pa, int flags, int abstime) {
    unsigned int mask=0;
    int recv_len;

    mask = stcp_wait_for_event(fd, expected, 100);
    if( mask == 0 )
        // TODO report time out

    if( mask & APP_DATA )
        recv_len = stcp_app_recv(sd, pt_pa->pa_buf, 536);
    else if( mask & NETWORK_DATA )
        recv_len = stcp_network_recv(sd, pt_pa, 536);

    return 0;
}
*/


int wait_recv(mysocket_t sd, struct packet *p, int expected, int msec, int realrecv)
{
fprintf(stderr,"[wait_recv] inside wait_recv, msec = %d\n",msec);
    struct timespec *ts = (struct timespec *)malloc(sizeof(struct timespec));
    int n, ret;
    unsigned int mask=0;
    if(msec == 0) ts = NULL;
    else {
        /* (*ts) = time(NULL); */
        time(ts);
        ts->tv_sec += msec/1000.0;
    }

fprintf(stderr,"[wait_recv] about to call stcp_wait_for_event\n");
    mask = stcp_wait_for_event(sd, APP_DATA | NETWORK_DATA,ts);
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

fprintf(stderr,"[wait_recv] about to return\n");
    return ret;
}

/* dummy function for compilation.  TODO delete this function?  */
void dump_packet(struct packet *p){
    return;
}

/* dummy function for compilation.  TODO modify / delete this function? */
static void exit_control_loop(int reason)
{
    /* ctx->control_loop_not_running = reason; */
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
                packetlen = fill_header(hdr,ctx->seq_active,0,TH_SYN, 0);
                packet->pa_header = *hdr;
                /* dump_packet(packet) */
                sent = stcp_network_send(sd,(char *)packet, packetlen,NULL);
                if(sent == -1) perror("network_send");
                printf("network_send: %d bytes\n", sent);
                dprintf("active child: SYN_SENT\n");

                /* recv SYNACK */
                if(wait_recv(sd, packet, TH_SYNACK, 100, 1) == 0)
                {
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
                /*debug("network_send: %d bytes\n", sent);*/
                if(sent == -1) perror("network_send");
                /* if recv SYNACK again  */
                if(wait_recv(sd, packet, TH_SYNACK, 500, 1) == 0) {
                    ctx->connection_state = CSTATE_SYNACK_RCVD;
                }
                else 
		{
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
                    printf("network_send:%d bytes\n", sent);
                    if(sent == -1) perror("network_send");
                    /* recv ACK */
                    if(wait_recv(sd, packet, TH_ACK, 100, 1) == 0)
                    {
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
                dprintf("active child: ESTABLISHED\n");
                /* send FIN */
                packetlen = fill_header(hdr, ctx->seq_active, 0, TH_FIN, 0);
                packet->pa_header = *hdr;
                dump_packet(packet);
                if(stcp_network_send(ctx->sockfd, (char *)packet, packetlen, NULL) == -1)
                    perror("network_send");
                /* recv FINACK */
                if(wait_recv(ctx->sockfd, packet, TH_FIN | TH_ACK, 100, 1) == 0) {
                    ctx->ack_active = packet->pa_header.th_ack;
                    ctx->ack_passive = packet->pa_header.th_seq + 1;
                    ctx->connection_state = CSTATE_TIME_WAIT;
                }
                else {
                    ctx->connection_state = CSTATE_ESTABLISHED;
                }
                break;
            case CSTATE_TIME_WAIT:
                dprintf("active child: TIME_WAIT\n");
                /* send ACK */
                packetlen = fill_header(hdr, 0, ctx->ack_passive, TH_ACK, 0);
                packet->pa_header = *hdr;
                dump_packet(packet);
                if(stcp_network_send(ctx->sockfd, (char *)packet, packetlen, NULL) == -1)
                    perror("network_send");
                /* if recv SYNACK again */
                if(wait_recv(ctx->sockfd, packet, TH_SYN | TH_ACK, 500, 1) == 0) {
                    ctx->connection_state = CSTATE_ESTABLISHED;
                }
                else {
                    /* terminate connection */
                    close(ctx->sockfd);
                    ctx->connection_state = CSTATE_CLOSED;
                    exit_control_loop(1);
                }
                break;
            default:
                break;
        }
    
    
    }
    
    printf("FIN handshaking passed\n");
    printf("!! seq / ack active : %d %d\n", ctx->seq_active, ctx->ack_active);
    printf("!! ack passive: %d\n", ctx->ack_passive);
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
                if(wait_recv(ctx->sockfd, packet, TH_ACK, 100, 1) == 0) {
                    ctx->ack_active = packet->pa_header.th_ack;
                    /* terminate connection */
                    close(ctx->sockfd);
                    ctx->connection_state = CSTATE_CLOSED;
                    exit_control_loop(1);
                }
                else ctx->connection_state = CSTATE_CLOSE_WAIT;
                break;
            default:
                break;
        }
    }
    
    printf("FIN handshaking passed\n");
    printf("!! seq / ack active : %d %d\n", ctx->seq_active, ctx->ack_active);
    printf("!! ack passive: %d\n", ctx->ack_passive);
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
    assert(ctx);

    while (!ctx->done)
    {
        unsigned int event;

        /* see stcp_api.h or stcp_api.c for details of this function */
        /* XXX: you will need to change some of these arguments! */
        event = stcp_wait_for_event(sd, 0, NULL);

        /* check whether it was the network, app, or a close request */
        if (event & APP_DATA)
        {
            /* the application has requested that data be sent */
            /* see stcp_app_recv() */
        }

        /* etc. */
    }
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



