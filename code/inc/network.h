#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <sys/types.h>

int init(unsigned short type);

void deinit(int socket);

int my_accept(int socket);

int rcvreq(int connfd, void* req, size_t len);

int sndres(int connfd, const void* res, size_t len);

int sndreq(const char* server, unsigned short type, const void* req, size_t len);

int rcvres(int sockfd, void* res, size_t len);

#endif	//__NETWORK_H__
