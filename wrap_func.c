#include "wrap_func.h"
int Socket(int domain, int type, int protocol){
    int fd;
    if((fd = socket(domain, type, protocol)) == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    return fd;
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    if(bind(sockfd, addr, addrlen) == -1){
        perror("bind");
        exit(EXIT_FAILURE);
    }
}

void Listen(int sockfd, int backlog){
    if(listen(sockfd, backlog) == -1){
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

int Accept(int sockfd, struct sockaddr_in *address, socklen_t *address_len){
    int fd;
    if((fd = accept(sockfd, (struct sockaddr *)address, address_len)) == -1){
        perror("accept::");
        exit(EXIT_FAILURE);
    }
    return fd;
}

void Close(int fd){
    if(close(fd) == -1){
        perror("close");
        exit(EXIT_FAILURE);
    }
}

void Inet_pton(int af, char *src, void *dst){
    int i;
    if((i=inet_pton(af, src, dst)) != 1){
        fprintf(stdout, "inet_pton return %d\n", i);
        exit(EXIT_FAILURE);
    }
}

int Read(int fd, void *buf, size_t count){
    int n;
    if((n = read(fd, buf, count)) == -1){
        perror("read");
        exit(EXIT_FAILURE);
    }
    return n;
}
int Write(int fd, const void *buf, size_t count){
    int n;
    if((n=write(fd, buf, count))==-1){
        perror("write");
        exit(EXIT_FAILURE);
    }
    return n;
}
/*
DIR* Opendir(const char *name){
    DIR *dir;
    if((dir = opendir(name))==NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    return dir;
}*/

