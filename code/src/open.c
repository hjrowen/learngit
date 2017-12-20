#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include "../inc/bank.h"
#include "../inc/dao.h"
#include "../inc/network.h"
#include <pthread.h>

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static size_t g_count = 0;

void sigint(int signum)
{
	pthread_mutex_lock(&g_mutex);
	printf("Account service：will have to stop.\n");
	pthread_mutex_unlock(&g_mutex);
	for (;;)
	{	
		if (0 == g_count)
			exit(0);
		sleep(10);
	}
}

void* proc_open(void* agr)
{
	OPEN_REQUEST req;
	memset(&req, 0, sizeof(req));
	OPEN_RESPOND res;
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
	int rcv_res = rcvreq((int)agr, (OPEN_REQUEST*)&req, sizeof(req));
	if (0 >= rcv_res) 
	{
		if (-2 == rcv_res) // -2 meaning wait timeout
			sprintf(res.error, "Time out!");
		else
			sprintf(res.error, "Unknow err!");
		goto send_respond;
	}
	strcpy(acc.name, req.name);
	strcpy(acc.passwd, req.passwd);
	acc.balance = req.balance;
	pthread_mutex_lock(&g_mutex);
	if (-1 == (acc.id = genid()))
	{
		sprintf(res.error, "Generate account failure！");
		pthread_mutex_unlock(&g_mutex);
		goto send_respond;
	}
	pthread_mutex_unlock(&g_mutex);
	if (-1 == save(&acc))
	{
		sprintf(res.error, "Save account failure！");
		goto send_respond;
	}
	res.id = acc.id;
send_respond:
	sndres((int)agr,  &res, sizeof(res));

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
	int sockfd;
	if (-1 == (sockfd = init(TYPE_OPEN)))
	{
		printf("Account service startup failed！\n");
		return -1;
	}
	printf("Account service：startup ready.\n");
	for (;;)
	{
		int connfd = -1;
		if (-1 == (connfd = my_accept(sockfd)))
			continue;
		
		pthread_t tid;
		if (0 != pthread_create(&tid, NULL, proc_open, (void*)connfd))
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

