#ifndef __TRANS
#define __TRANS
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

#define BACKLOG 34
#define BUFMAX  1024
#define PATHMAX 128
#define PATHTOSTRORE "."
///////////////////////////////////////

struct sendinfo{
    int filefd;
    long filesize;
    int sockfd;
    int conn_check;
};
struct recvinfo{
    int filefd;
    int sockfd;
    char path[PATHMAX];
};
//static char send_buf[BUFMAX], recv_buf[BUFMAX];
//static int  recvfd;//sock_create 
//static int  maxfd;
//static int  fileid = 0;



int sock_create(int port);
        
int sock_close(int fd);

struct sendinfo* file_send_init(char *ip, int port, char *path);

struct recvinfo* file_recv_init(char *path_to_store, int fd);

int file_send_for_select(struct sendinfo* send_info);

int file_recv_for_select(struct recvinfo* recv_info);

#endif
