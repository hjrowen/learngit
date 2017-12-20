#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <pthread.h>
#include "../inc/bank.h"
#include "../inc/dao.h"
#include "../inc/network.h"

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static size_t g_count = 0;

void sigint(int signum)
{
	pthread_mutex_lock(&g_mutex);
	printf("Deposit service：will have to stop.\n");
	pthread_mutex_unlock(&g_mutex);
	for (;;)
	{	
		if (0 == g_count)
			exit(0);
		sleep(10);
	}
}

void* proc_transfer(void* agr)
{
	TRANSFER_REQUEST req;
	memset(&req, 0, sizeof(req));
	TRANSFER_RESPOND res;
	memset(&res, 0, sizeof(res));
	ACCOUNT trans;
	memset(&trans, 0, sizeof(ACCOUNT));
	ACCOUNT by_trans;
	memset(&by_trans, 0, sizeof(ACCOUNT));
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
	int rcv_res = rcvreq((int)agr, (SAVE_REQUEST*)&req, sizeof(req));
	if (0 >= rcv_res) 
	{
		if (-2 == rcv_res) // -2 meaning wait timeout
			sprintf(res.error, "Time out!");
		else
			sprintf(res.error, "Unknow err!");
		goto send_respond;
	}
	if (-1 == get(req.id[0], &trans))
	{
		sprintf(res.error, "Invalid ACCOUNT_ID");
		goto send_respond;	
	}
	if (-1 == get(req.id[1], &by_trans))
	{
		sprintf(res.error, "Invalid target ACCOUNT_ID");
		goto send_respond;	
	}
	if (0 != strcmp(req.name[0], trans.name))
	{
		sprintf(res.error, "Invalid name！");
		goto send_respond;
	}
	if (0 != strcmp(req.name[1], by_trans.name))
	{
		sprintf(res.error, "Invalid taqrget name！");
		goto send_respond;
	}
	if (0 != strcmp(req.passwd, trans.passwd))
	{
		sprintf(res.error, "Invalid password！");
		goto send_respond;
	}
	if (req.money < 0 || req.money > trans.balance)
	{
		sprintf(res.error, "Invalid balance！");
		goto send_respond;
	}
	trans.balance -= req.money;
	by_trans.balance += req.money;
	if (-1 == update(&trans))
	{
		sprintf(res.error, "Transfer Failure!");
		goto send_respond;
	}
	if (-1 == update(&by_trans))
	{
		sprintf(res.error, "Transfer Failure!");
		char err_buf[1024];
		memset(err_buf, 0, sizeof(err_buf));
		time_t time_val = time(0);
		sprintf(err_buf, "transfer err : %s %s -> %s %lf￥\n", ctime(&time_val), trans.id, by_trans.id, req.money);
		infoerr(err_buf, sizeof(err_buf));
	}
	res.balance = trans.balance;
send_respond:
	sndres(*(int*)agr, &res, sizeof(res));

	close(*(int*)agr);
	pthread_mutex_lock(&g_mutex);
	--g_count;
	pthread_mutex_unlock(&g_mutex);
	return NULL;
}

int main(void)
{
	if (SIG_ERR == signal(SIGINT, sigint))
		perror("signal"), exit(-1);
	int sockfd = init(TYPE_TRANSFER);
	if (-1 == sockfd)
	{
		printf("Transfer service：startup failure.\n");
		return -1;
	}
	printf("Transfer service：startup ready.\n");
	for (;;)
	{
		int connfd;
		if (-1== (connfd = my_accept(sockfd)))
		{
			continue;
		}
		pthread_t tid;
		if (0 != pthread_create(&tid, NULL, proc_transfer, (void*)connfd))
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

