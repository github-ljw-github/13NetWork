#ifndef __WRAP_H
#define __WRAP_H

#include <poll.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <linux/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>


ssize_t Write(int fildes, const void *buf, size_t nbyte);
int Socket(int domain, int type, int protocol);
int Setsockopt(int sockfd, int level, int optname,
	       const void *optval, socklen_t optlen);

ssize_t Read(int fildes, void *buf, size_t nbyte);
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
int Poll(struct pollfd *fds, nfds_t nfds, int timeout);

#endif
