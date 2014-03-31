#include <stdio.h>
#include "multhread_trans.h"
extern int recvfd;

//格式：file-path 192.168.1.1
static int analyze_input(char *input, char* file_path, char *ip){
    char *p, *pre;
/*
    while(*p != '\0'){
        pre = p;
        strtok_r(input, " ", &p);
    }
*/
    p = strchr(input, ' ');
    memcpy(file_path, input, p-input);
    file_path[p-input] = '\0';
    memcpy(ip, p+1, strlen(p+1)-1);
    ip[strlen(p+1)-1] = '\0';

    return 0;
}
static int add_event_to_thread(pthread_t threadid, char * filepath, int accfd){
    int order = 0;
    while(threadid != communicate_with_threads[order].threadid){
        ++order;
        if(order >= THREADNUM){
            fprintf(stderr, "add_event_to_thread:order >= THREADNUM\n");
        }
    }
    if(filepath != NULL)
        memcpy(communicate_with_threads[order].filepath, filepath, strlen(filepath)+1);
    if(accfd != 0)
        communicate_with_threads[order].sockfd = accfd;
    
    return 0;
///////////////////////////////////////////////////    
}
static char * judge_has_content(pthread_t threadid){
    int i;
    for(i=0;i<THREADNUM;++i){
        if(communicate_with_threads[i].threadid == threadid){
            if(communicate_with_threads[i].filepath[0] != '\0'){
                //*flag = 1;
                return communicate_with_threads[i].filepath;
            }
            return NULL;
        }
    }
    return NULL;
}
static int *judge_has_sockfd(pthread_t threadid){
    int i;
    for(i=0;i<THREADNUM;++i){
        if(communicate_with_threads[i].threadid == threadid){
            if(communicate_with_threads[i].sockfd != 0){
                return &communicate_with_threads[i].sockfd;
            }
        }
        return NULL;
    }
    return NULL;
}
static void add_threadid_to_comm(pthread_t threadid){
    int k = 0;
    while(1){
        if(communicate_with_threads[k].threadid == 0){
            communicate_with_threads[k++].threadid = threadid;
            break;
            }
        if(k == THREADNUM){
            fprintf(stderr, "too many thread.\n");
            exit(1);
        }
    }
    
}
static int connect_err(struct sendinfo *send_info_p){
    int error = 0, len;
    len = sizeof(error);
    if(getsockopt(send_info_p->sockfd, SOL_SOCKET, SO_ERROR, &error, &len)<0){
        perror("getsockopt");
        pthread_exit(NULL);
    }
    if(error){
        fprintf(stderr, "error in connect.\n");
        Close(send_info_p->sockfd);
        Close(send_info_p->filefd);
        free(send_info_p);
        return -1;
    }
    return 0;
}
//type: send=1,recv=2
static int add_send_recv_info_to_set(void *p, void **dst, int type){ 
    int i = 0;
    struct sendinfo *p1;
    struct sendinfo **dst1;
    switch(type){
        case 1:
            p1 = (struct sendinfo*)p;
            dst1 = (struct sendinfo **)dst;

            while(dst1[i] != NULL) ++i;
            dst1[i] = p1;    

            break;
        case 2:
            p1 = (struct recvinfo *)p;
            dst1 = (struct recvinfo **)dst;
            while(dst1[i] != NULL) ++i;
            dst1[i] = p1;

            break;
        default:
            return -1;
    }
    return 0;
}
static void *case_func(void *temp_arg){
    struct thread_arg arg = *(struct thread_arg*)temp_arg;
    struct sendinfo *send_info_p;
    struct recvinfo *recv_info_p;
    struct sendinfo *send_info_p_set[SENDINFOPSET];
    struct recvinfo *recv_info_p_set[RECVINFOPSET];
    struct timeval timev = {2, 0};
    fd_set wset, rset;
    free(temp_arg);


    int i, n, maxfd = 0;
    for(i=0;i<SENDINFOPSET;++i){
        send_info_p_set[i] = NULL;
        recv_info_p_set[i] = NULL;
    }
    if(arg.case_num == 1){
        recv_info_p = file_recv_init(PATHTOSTRORE, arg.accfd);
        recv_info_p_set[0] = recv_info_p;
    }
    else if(arg.case_num == 2){
        printf("%s\n",arg.ip);/////////////////***********************
        send_info_p = file_send_init(arg.ip, port, arg.filepath);
        send_info_p_set[0] = send_info_p;
    }

    while(1){
        //int judge_flag = 0;
        char *filepath;
        int *sockfd;
        FD_ZERO(&wset);
        FD_ZERO(&rset);
        for(i=0; i<SENDINFOPSET;++i){
            if(send_info_p_set[i] != NULL){
                FD_SET(send_info_p_set[i]->sockfd, &wset);
                maxfd = MAX(maxfd, send_info_p_set[i]->sockfd);
            }
            if(recv_info_p_set[i] != NULL){
                FD_SET(recv_info_p_set[i]->sockfd, &rset);
                maxfd = MAX(maxfd, recv_info_p_set[i]->sockfd);
            }
        }
        if((filepath = judge_has_content(pthread_self()))||(sockfd = judge_has_sockfd(pthread_self()))||(n = select(maxfd+1, &rset, &wset, NULL, &timev))){

            if(filepath != NULL){
                send_info_p = file_send_init(arg.ip, port, filepath);
                add_send_recv_info_to_set((void *)send_info_p, (void **)send_info_p_set, 1);
                filepath[0] = '\0';
            } 
            else if(sockfd != NULL){
                recv_info_p = file_recv_init(PATHTOSTRORE, *sockfd);
                add_send_recv_info_to_set((void *)recv_info_p, (void **)recv_info_p_set, 2);
                *sockfd = 0;
            }
            else{
                int temp, n1, n2;
                for(temp=0;temp<SENDINFOPSET;++temp){
                    if(send_info_p_set[temp] != NULL){
                        if(FD_ISSET(send_info_p_set[temp]->sockfd, &wset)){
                            if(send_info_p_set[temp]->conn_check == 0){
                                if(connect_err(send_info_p_set[temp])==-1){
                                    send_info_p_set[temp] = NULL;
                                    send_info_p_set[temp]->conn_check = 1;
                                    goto __RECV;
                                }
                                send_info_p_set[temp]->conn_check = 1;
                            }
                            n1 = file_send_for_select(send_info_p_set[temp]);
                            if(n1 == 0){
                                send_info_p_set[temp] = NULL;
                            }
                        }
                    }
                __RECV:
                    if(recv_info_p_set[temp] != NULL){
                        if(FD_ISSET(recv_info_p_set[temp]->sockfd, &rset)){
                            n2 = file_recv_for_select(recv_info_p_set[temp]);
                            if(n2 == 0){
                                recv_info_p_set[temp] = NULL;
                            }
                        }
                    }
                }
            }
        }
    }
/////////////////////////////////////////////////

}
int init(){
    int i;
    /*
    fds_num_for_select = 0;
    for(i=0;i<MAXFDS;++i){
        fds_for_select[i] = 0;
    }
    */
    for(i=0;i<THREADNUM;++i){
        ips[i].ip[0] = '\0';
        communicate_with_threads[i].threadid = 0;
        communicate_with_threads[i].filepath[0] = '\0';
        communicate_with_threads[i].sockfd = 0;
    }
    sock_create(port);
    
    return 0;
}
void loop(){
    fd_set rset,wset;
    int n,acfd,stdin_no, sockaddr_len;
    char stdinbuf[INPUTBUFMAX];
    struct sockaddr_in sockaddr;

    stdin_no = fileno(stdin);
    while(1){
        FD_ZERO(&rset);
        FD_ZERO(&wset);
        FD_SET(stdin_no, &rset);
        FD_SET(recvfd, &rset);
        if((n = select(recvfd>stdin_no?(recvfd+1):(stdin_no+1), &rset, &wset, NULL,NULL))==-1){
            perror("select");
            exit(1);
        }
        if(FD_ISSET(stdin_no, &rset)){
            fgets(stdinbuf, INPUTBUFMAX, stdin);
            //printf("stdinbuf :%p\n",stdinbuf);////*************////////////
            shunt(0, NULL, stdinbuf);
        }
        if(FD_ISSET(recvfd, &rset)){
            sockaddr_len = sizeof(struct sockaddr_in);
            //printf("im here\n");/*************/
            acfd = Accept(recvfd, (struct sockaddr*)&sockaddr, &sockaddr_len);
            shunt(acfd, &sockaddr, NULL);
        }
        /*
        for(i=0;i<fds_num_for_select;++i){
            if(FD_ISSET(fds_for_select[i],))
        }
        */
    }

}

int shunt(int accfd, struct sockaddr_in* sockaddr, char *input){
    int __case;
    char file_path[PATHMAX], ip[IPLEN];

    if(accfd != 0) __case = 1;
    if(input != NULL) __case = 2;
    printf("__case:%d\n", __case);////***************************///

    if(__case != 1 && __case != 2){
        fprintf(stderr, "error in shunt.\n");
        exit(1);
    }
    
    if(__case == 1){
        pthread_t threadid;
        int ips_order = 0, zero_order = -1;
        //char ip[IPLEN];
        
        if(inet_ntop(AF_INET, &sockaddr->sin_addr, ip, IPLEN) == NULL){
            perror("inet_ntop");
            exit(1);
        }

        while(1){
            if(strlen(ips[ips_order].ip) == strlen(ip) && memcmp(ip, ips[ips_order].ip, strlen(ip)) == 0){
                
                add_event_to_thread(ips[ips_order].threadid, NULL, accfd);
                break;
            }
            if(zero_order == -1 && ips[ips_order].ip[0] == '\0')
                zero_order = ips_order;
            if(ips_order+1 == THREADNUM){
                //int k = 0;
                struct thread_arg *arg = malloc(sizeof(struct thread_arg));
                memcpy(ips[zero_order].ip, ip, strlen(ip)+1);
                memcpy(arg->ip, ip, strlen(ip)+1);
                arg->accfd = accfd;
                arg->case_num = 1;
                
                pthread_create(&threadid, NULL, case_func, arg);
                ips[zero_order].threadid = threadid;

                add_threadid_to_comm(threadid);
                break;
            }
            ++ips_order;
        }
        /////////////////////////////////////////////////////////
    }

    if(__case == 2){
        int ips_order=0, zero_order = -1;
        pthread_t threadid;
        analyze_input(input, file_path, ip);
        printf("analyze input:%s,%s\n", file_path, ip);/**********/
        while(1){
            if(strlen(ips[ips_order].ip) == strlen(ip) && memcmp(ip, ips[ips_order].ip, strlen(ip)) == 0){

                add_event_to_thread(ips[ips_order].threadid, file_path, 0);

                break;
            }
            if(zero_order == -1 && ips[ips_order].ip[0] == '\0')
                zero_order = ips_order;
            if(ips_order+1 == THREADNUM){

                //int k = 0;
                struct thread_arg *arg = malloc(sizeof(struct thread_arg));
                memcpy(ips[zero_order].ip, ip, strlen(ip)+1);
                memcpy(arg->ip, ip, strlen(ip)+1);
                memcpy(arg->filepath, file_path, strlen(file_path)+1);
                arg->case_num = 2;

                pthread_create(&threadid, NULL, case_func, arg);

                ips[zero_order].threadid = threadid;

                add_threadid_to_comm(threadid);
                break;
            }
            ++ips_order;
        }
    }

    return 0;
}   
