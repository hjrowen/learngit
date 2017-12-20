#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include "../inc/bank.h"

//服务进程结构
typedef struct tag_Service
{
	char srv_path[PATH_MAX + 1];	//服务路径
	pid_t srv_pid;					//服务PID
} SERVICE;

//服务进程数组
static SERVICE g_srv[] = {
	{"./open",     -1},
	{"./query",    -1},
	{"./close",    -1},
	{"./save",     -1},
	{"./withdraw", -1},
	{"./transfer", -1}
};

//启动业务服务
void start(void)
{
	printf("business start up success!...\n");
	size_t i;
	for (i = 0; i < sizeof(g_srv) / sizeof(g_srv[0]); i++)
	{
		g_srv[i].srv_pid = vfork();
		if (-1 == g_srv[i].srv_pid)
		{
			perror("vfork");
			continue;
		}
		if (0 == g_srv[i].srv_pid)
		{
			if (-1 == execl(g_srv[i].srv_path, g_srv[i].srv_path, NULL))
			{
				perror("execl failed");
				_exit(EXIT_FAILURE);
			}
		}
	}
}

//停止业务服务
void stop(void)
{
	printf("business stop !...\n");
	size_t i;
	for (i = 0; i < sizeof(g_srv) / sizeof(g_srv[0]); i++)
	{
		if (-1 == g_srv[i].srv_pid)
		{
			continue;
		}
		if (-1 == kill(g_srv[i].srv_pid, SIGINT))
		{
			perror("kill failed");
		}
	}
}

int main(void)
{
	start();
	sleep(1);
	printf("close server with anykey...\n");
	getchar();
	stop();

	return 0;
}

