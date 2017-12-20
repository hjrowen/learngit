#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include "../inc/bank.h"
#include "../inc/dao.h"
#include "../inc/network.h"

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static size_t g_count = 0;

void sigint(int signum)
{
	pthread_mutex_lock(&g_mutex);
	printf("Query service：will have to stop.\n");
	pthread_mutex_unlock(&g_mutex);
	for (;;)
	{	
		if (0 == g_count)
			exit(0);
		sleep(10);
	}
}

void* proc_query(void* agr)
{
	QUERY_REQUEST req;
	memset(&req, 0, sizeof(req));
	QUERY_RESPOND res;
	memset(&res, 0, sizeof(res));
	ACCOUNT acc;
	memset(&acc, 0, sizeof(acc));
	Mask SIGINT
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	int err = pthread_sigmask(SIG_SETMASK, &set, NULL);
	if (0 != err)
	{
		sprintf(res.error, "connect failure!");
		goto send_respond;
	}
	int rcv_res = rcvreq((int)agr, (QUERY_REQUEST*)&req, sizeof(req));
	if (0 >= rcv_res) 
	{
		if (-2 == rcv_res) // -2 meaning wait timeout
			sprintf(res.error, "Time out!");
		else
			sprintf(res.error, "Unknow err!");
		goto send_respond;
	}
	if (-1 == get(req.id, &acc))
	{
		printf("id == %d, acc == %d\n", req.id, acc.id);
		sprintf(res.error, "Invalid ACCOUNT_ID！");
		goto send_respond;
	}
	if (0 != strcmp(req.name, acc.name))
	{
		sprintf(res.error, "Invalid name！");
		goto send_respond;
	}
	if (0 != strcmp(req.passwd, acc.passwd))
	{
		sprintf(res.error, "Invalid password！");
		goto send_respond;
	}
	res.balance = acc.balance;
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
	printf("Query service：startup ready.\n");
	int sockfd;
	if (-1 == (sockfd = init(TYPE_QUERY)))
	{
		printf("Query service startup failed！\n");
		return -1;
	}
	for (;;)
	{
		int connfd;
		if (-1== (connfd = my_accept(sockfd)))
		{
			continue;
		}
		pthread_t tid;
		if (0 != pthread_create(&tid, NULL, proc_query, (void*)connfd))
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

