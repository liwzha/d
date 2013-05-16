#ifndef WINDOW_H
#define WINDOW_H

#include"transport.h"

typedef struct window_node window_node;
struct window_node {
    struct packet * wn_packet;
    window_node * wn_next;
};

enum window_type {WIN_RECV, WIN_SEND, WIN_UNDEF};

typedef struct window window;

struct window {
#define win_size 3072
    enum window_type win_type;
    int win_firstNum;
    window_node * win_buf;
}; 





#endif

