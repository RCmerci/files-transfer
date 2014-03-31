#ifndef __MULTHREAD_TRANS
#define __MULTHREAD_TRANS

#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>
#include <strings.h>
#include "trans.h"
#define  THREADNUM 20
#define  IPLEN 20
#define  INPUTBUFMAX 128
#define  SENDINFOPSET 10
#define  RECVINFOPSET 10

#define MAX(a,b)     ((a)>(b)?(a):(b))
//#define  MAXFDS 30
//unp 26章
struct thread_arg{
    char ip[IPLEN];
    char filepath[PATHMAX];
    int accfd;
    int case_num;
};

struct thread_id_and_ip{
    char ip[IPLEN];
    pthread_t threadid;
};
struct thread_id_and_path{
    pthread_t threadid;
    char filepath[PATHMAX];
    int sockfd;
};
struct thread_id_and_path communicate_with_threads[THREADNUM];

int port;
/*
int fds_num_for_select;
int fds_for_select[MAXFDS];
*/

//一个线程结束时候，处理
struct thread_id_and_ip ips[THREADNUM];
//
int init();

void loop();
//shunt :分流器
//与同一ip传输用同一线程，即1ip1线程
int shunt(int accfd, struct sockaddr_in* sockaddr, char *input);


#endif
