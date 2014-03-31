#ifndef WRAP_FUNC
#define WRAP_SUNC
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
int Socket(int domain, int type, int protocol);

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

void Listen(int sockfd, int backlog);

int Accept(int sockfd, struct sockaddr_in *addr, socklen_t *addrlen);

void Close(int fd);

void Inet_pton(int af, char *src, void *dst);

int Read(int fd, void *buf, size_t count);

int Write(int fd, const void *buf, size_t count);

DIR *Opendir(const char *name);
#endif
