#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "../inc/bank.h"
#include "../inc/dao.h"
#include "../inc/network.h"

void sigint(int signum)
{
	printf("清户服务：即将停止。\n");
	exit(0);
}

void* proc_close(void* agr)
{
	CLOSE_REQUEST req;
	if (-1 == rcvreq((int)agr, (CLOSE_REQUEST*)&req, sizeof(req)))
	{
		return NULL;
	}
	CLOSE_RESPOND res = {};
	ACCOUNT acc;
	if (-1 == get(req.id, &acc))
	{
		sprintf(res.error, "无效帐号！");
		goto send_respond;
	}
	if (0 != strcmp(acc.name, req.name))
	{
		sprintf(res.error, "无效户名！");
		goto send_respond;
	}
	if ( 0 != strcmp(acc.passwd, req.passwd))
	{
		sprintf(res.error, "密码错误！");
		goto send_respond;
	}
	if (-1 == delete(req.id))
	{
		sprintf(res.error, "清户失败！");
		goto send_respond;
	}
send_respond:
	if (-1 == sndres((int)agr, (CLOSE_RESPOND*)&res, sizeof(res)))
	{
		close((int)agr);
		return NULL;
	}
	close((int)agr);

	return NULL;
}

int main(void)
{
	if (SIG_ERR == signal(SIGINT, sigint))
	{
		perror("signal");
		return -1;
	}
	int sockfd;
	if (-1 == (sockfd = init(TYPE_CLOSE)))
	{
		printf("Account cancellation server: starup failuer!\n");
		return -1;
	}
	printf("Account cancellation server: starup ready\n");
	while (1)
	{
		int connfd = -1;
		if (-1 == (connfd = my_accept(sockfd)))
		{
			continue;
		}
		pthread_t tid;
		pthread_create(&tid, NULL, proc_close, (void*)connfd);
		pthread_detach(tid);
	}

	return 0;
}

