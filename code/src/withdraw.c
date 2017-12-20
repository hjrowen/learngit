#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <pthread.h>
#include "../inc/bank.h"
#include "../inc/dao.h"
#include "../inc/network.h"

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static size_t g_count = 0;

void sigint(int signum)
{
	pthread_mutex_lock(&g_mutex);
	printf("Withdarwal service：will have to stop.\n");
	pthread_mutex_unlock(&g_mutex);
	for (;;)
	{	
		if (0 == g_count)
			exit(0);
		sleep(10);
	}
}

void* proc_withdraw (void* agr)
{
	WITHDRAW_REQUEST req;
	memset(&req, 0, sizeof(req));
	WITHDRAW_RESPOND res;
	memset(&res, 0, sizeof(res));
	ACCOUNT acc;
	memset(&acc, 0, sizeof(acc));
	// Mask SIGINT
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	int err = pthread_sigmask(SIG_SETMASK, &set, NULL);
	if (0 != err)
	{
		sprintf(res.error, "connect failure!");
		goto send_respond;
	}
	if (-1 == rcvreq((int)agr, (WITHDRAW_REQUEST*)&req, sizeof(req)))
	{
		return NULL;
	}
	if (-1 == get(req.id, &acc))
	{
		sprintf(res.error, "无效账户！");
		goto send_respond;
	}
	if (0 != strcmp(req.name, acc.name))
	{
		sprintf(res.error, "无效户名！");
		goto send_respond;
	}
	if (0 != strcmp(req.passwd, acc.passwd))
	{
		sprintf(res.error, "密码错误！");
		goto send_respond;
	}
	if (req.money > acc.balance)
	{
		sprintf(res.error, "无效金额！");
		goto send_respond;
	}
	acc.balance -= req.money;
	res.balance -= req.money;
	if (-1 == update(&acc))
	{
		sprintf(res.error, "取款失败！");
		goto send_respond;
	}
send_respond:
	sndres((int)agr, &res, sizeof(res));

	close((int)agr);
	pthread_mutex_lock(&g_mutex);
	--g_count;
	pthread_mutex_unlock(&g_mutex);
	return NULL;
}

int main(void)
{
	if (SIG_ERR == signal(SIGINT, sigint))
	{
		perror("signal");
		return -1;
	}
	int sockfd = init(TYPE_WITHDRAW);
	if (-1 == sockfd)
	{
		printf("withdraw server：starup failure!\n");
		return -1;
	}
	printf("withdraw server：starup ready!\n");
	
	for (;;)
	{
		int connfd;
		if (-1== (connfd = my_accept(sockfd)))
		{
			continue;
		}
		pthread_t tid;
		if (0 != pthread_create(&tid, NULL, proc_withdraw, (void*)connfd))
		{
			close(connfd);
			continue;
		}
		pthread_detach(tid);
		pthread_mutex_lock(&g_mutex);
		++g_count;
		pthread_mutex_unlock(&g_mutex);
	}

	return 0;
}

