#include <stdio.h>
#include "trans.h"
#include "wrap_func.h"
#define PORT_DEBUG 9999

int recvfd;

int sock_create(int port){
    struct sockaddr_in sockaddr;
    int flags;
    recvfd = Socket(AF_INET, SOCK_STREAM, 0);

    flags = fcntl(recvfd, F_GETFL, 0);
    fcntl(recvfd, F_SETFL, flags|O_NONBLOCK);
    
    bzero(&sockaddr, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    //Inet_pton(AF_INET, ip, &sockaddr.sin_addr);
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(recvfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
    Listen(recvfd, BACKLOG);
    //maxfd = recvfd+1;
    return 0;
}

int sock_close(int fd){
    Close(fd);
}
struct sendinfo* file_send_init(char *ip, int port, char *path){
    int filefd, fd, flags;
    long size;
    //char buf[BUFMAX];
    struct sockaddr_in sockaddr;
    struct sendinfo* send_info;

    send_info = malloc(sizeof(struct sendinfo));

    if((filefd = open(path, O_RDONLY))==-1){
        if(errno == EACCES){
            perror("open");
        }
        return NULL;
    }
    size = lseek(filefd, 0, SEEK_END);
    if(size == -1){
        perror("lseek");
    }
    fprintf(stdout, "%ld bytes to be sent.\n",size);
    lseek(filefd, 0, SEEK_SET);
    
    fd = Socket(AF_INET, SOCK_STREAM, 0);

    flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags|O_NONBLOCK);

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(9999);//PORT_DEBUG?PORT_DEBUG:port);
    Inet_pton(AF_INET, ip, &sockaddr.sin_addr);
    connect(fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));

    //if(fd >= maxfd)
    //    maxfd = fd+1;

    send_info->filefd = filefd;
    send_info->filesize = size;
    send_info->sockfd = fd;
    send_info->conn_check = 0;
    return send_info;
}
int file_send_for_select(struct sendinfo* send_info){
    int n;
    char send_buf[BUFMAX];
    if((n = read(send_info->filefd, send_buf, BUFMAX)) > 0){
        send(send_info->sockfd, send_buf, n, 0);
    }
    if(n==0){
        Close(send_info->filefd);
        Close(send_info->sockfd);
        free(send_info);
        fprintf(stdout, "one file sent.\n");
    }
    return n;
}

struct recvinfo* file_recv_init(char *path_to_store, int fd){
    struct recvinfo * recv_info;
    struct sockaddr_in sockaddr;
    int len, filefd, fileid = 0;
    char path[PATHMAX];
    sprintf(path, "%s/%d",path_to_store, fileid);
    len = sizeof(struct sockaddr_in);
    //fd = Accept(recvfd, (struct sockaddr*)&sockaddr, &len);
    while(1){
        if((filefd = open(path, O_CREAT|O_EXCL|O_WRONLY, 0666))==-1){
            ++fileid;
            sprintf(path, "%s/%d", path_to_store, fileid);
        }
        else
            break;
    }
    //if(fd >= maxfd)
    //    maxfd = fd+1;

    recv_info = malloc(sizeof(struct recvinfo));
    recv_info->filefd = filefd;
    recv_info->sockfd = fd;
    memcpy(recv_info->path, path, strlen(path));

    return recv_info;
}

int file_recv_for_select(struct recvinfo* recv_info){
    int n;
    char recv_buf[BUFMAX];
    if((n = recv(recv_info->sockfd, recv_buf, BUFMAX, 0)) != -1){
        Write(recv_info->filefd, recv_buf, n);
    }
    if(n == 0){
        fprintf(stdout, "recv a file.\n");
        Close(recv_info->filefd);
        Close(recv_info->sockfd);
        free(recv_info);
    }
    return n;
}
