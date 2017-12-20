#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "../inc/network.h"

//初始化
int init(unsigned short type)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
	{
		perror("socket");
		return -1;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(type);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (-1 == bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)))
	{
		perror("bind");
		return -1;
	}
	if (-1 == listen(sockfd, 1024))
	{
		perror("listen");
		return -1;
	}
	return sockfd;
}

//终结化
void deinit(int socket)
{
	close(socket);
}

int rcvreq(int socket, void* req, size_t len)
{
	int connfd = -1;
	while (1)
	{
		struct sockaddr_in cli_addr;
		socklen_t len = sizeof(cli_addr);
		connfd = accept(socket, (struct sockaddr*)&cli_addr, &len);
		if (-1 == connfd)
		{
			perror("connfd");
			continue;
		}
		printf("来自的%s请求\n", inet_ntoa(cli_addr.sin_addr));
		pid_t pid = fork();
		if (-1 == pid)
		{
			perror("fork");
			close(connfd);
			continue;
		}
		if (0 == pid)
		{
			break;
		}
		close(connfd);
	}
	ssize_t rb = recv(connfd, req, len, 0);
	if (-1 == rb)
	{
		perror("recv");
		close(connfd);
		return -1;
	}
	if (0 == rb)
	{
		printf("客户机已关闭！\n");
		close(connfd);
		return -1;
	}
	return connfd;
}

//发送响应
int sndres(int connfd, const void* res, size_t len)
{
	if (-1 == send(connfd, res, len, 0))
	{
		perror("send");
		close(connfd);
		return -1;
	}
	return 0;
}

//发送请求
int sndreq(const char* server, unsigned short type, const void* req, size_t len)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
	{
		perror("socket");
		return -1;
	}
	struct sockaddr_in ser_addr;
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(type);
	ser_addr.sin_addr.s_addr = inet_addr(server);
	if (-1 == connect(sockfd, (struct sockaddr*)&ser_addr, sizeof(ser_addr)))
	{
		perror("connect");
		return -1;
	}
	size_t res = send(sockfd, req, len, 0);
	if (-1 == res)
	{
		perror("send");
		return -1;
	}
	if (0 == res)
	{
		printf("服务器已关闭！\n");
		return -1;
	}
	return sockfd;
}

//接收响应
int rcvres(int sockfd, void* res, size_t len)
{
	size_t rb = recv(sockfd, res, len, 0);
	if (-1 == rb)
	{
		perror("recv");
		close(sockfd);
		return -1;
	}
	if (0 == rb)
	{
		printf("服务器已关闭！\n");
		close(sockfd);
		return -1;
	}
	close(sockfd);
	return 0;
}

