#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../inc/bank.h"
#include "../inc/network.h"

static char* g_IP = NULL;

//菜单循环
void menu_loop(int (*menu)(void), int (*on_menu[])(void), size_t size)
{
	while (1)
	{
		int id = menu();
		if (id < 0 || size <= id)
		{
			printf("无效选择！\n");
		}
		else if (-1 == on_menu[id]())
		{
			break;
		}
	}
}

//主菜单
int main_menu(void)
{
	printf("----------------\n");
	printf("my_bank\n");
	printf("----------------\n");
	printf("1.open ACCOUNT\n");
	printf("2.destroy ACCOUNT\n");
	printf("3.SAVE\n");
	printf("4.WITHDRAW\n");
	printf("5.QUERY\n");
	printf("6.TRANSFER\n");
	printf("0.EXIT\n");
	printf("----------------\n");
	printf("select:");
	int id = -1;
	scanf("%d", &id);
	scanf("%*[^\n]");
	scanf("%*c");
	return id;
}

//退出
int on_quit(void)
{
	printf("Bye\n");
	return -1;
}

//开户
int on_open(void)
{
	OPEN_REQUEST req;
	memset(&req, 0, sizeof(req));
	printf("NAME:");
	scanf("%s", req.name);
	printf("PASSWD:");
	scanf("%s", req.passwd);
	printf("BALANCE:");
	scanf("%lf", &req.balance);
	int sockfd;
	if (-1 == (sockfd = sndreq(g_IP, TYPE_OPEN, (OPEN_REQUEST*)&req, sizeof(req))))
	{
		return 0;
	}
	OPEN_RESPOND res;
	if (-1 == rcvres(sockfd, (OPEN_RESPOND*)&res, sizeof(res)))
	{
		return 0;
	}
	if (0 != strlen(res.error))
	{
		printf("%s\n", res.error);
		return 0;
	}
	printf("ACOUNT_ID:%d\n", res.id);

	return 0;
}

//清户
int on_close(void)
{
	CLOSE_REQUEST req = {};
	printf("ACOUNT_ID:");
	scanf("%d", &req.id);
	printf("NAME:");
	scanf("%s", req.name);
	printf("PASSWORD:");
	scanf("%s", req.passwd);
	int sockfd;
	if (-1 == (sockfd = sndreq(g_IP, TYPE_CLOSE, (CLOSE_REQUEST*)&req, sizeof(req))))
	{
		return 0;
	}
	CLOSE_RESPOND res;
	if (-1 == rcvres(sockfd, (CLOSE_RESPOND*)&res, sizeof(res)))
	{
		return 0;
	}
	if (0 != strlen(res.error))
	{
		printf("%s\n", res.error);
		return 0;
	}
	printf("Delete success!\n");

	return 0;
}

//存款
int on_save(void)
{
	SAVE_REQUEST req = {};
	printf("ACOUNT_ID");
	scanf("%d", &req.id);
	printf("NAME:");
	scanf("%s", req.name);
	printf("Enter the deposit amount：");
	scanf("%lf", &req.money);
	int sockfd;
	if (-1 == (sockfd = sndreq(g_IP, TYPE_SAVE, (SAVE_REQUEST*)&req, sizeof(req))))
	{
		return 0;
	}
	SAVE_RESPOND res;
	if (-1 == rcvres(sockfd, (SAVE_RESPOND*)&res, sizeof(res)))
	{
		return 0;
	}
	if (0 != strlen(res.error))
	{
		printf("%s\n", res.error);
		return 0;
	}
	printf("Deposit success！\n");
	
	return 0;
	
}

//取款
int on_withdraw(void)
{
	WITHDRAW_REQUEST req = {};
	memset(&req, 0, sizeof(req));
	printf("ACOUNT_ID:");
	scanf("%d", &req.id);
	printf("NAME:");
	scanf("%s", req.name);
	printf("PASSWORD:");
	scanf("%s", req.passwd);
	printf("Enter a withdrawal amount：");
	scanf("%lf", &req.money);
	int sockfd;
	if (-1 == (sockfd = sndreq(g_IP, TYPE_WITHDRAW, (WITHDRAW_REQUEST*)&req, sizeof(req))))
	{
		return 0;
	}
	WITHDRAW_RESPOND res;
	if (-1 == rcvres(sockfd, (WITHDRAW_RESPOND*)&res, sizeof(res)))
	{
		return 0;
	}
	if (0 != strlen(res.error))
	{
		printf("%s\n", res.error);
		return 0;
	}
	printf("Withdraw money！\n");

	return 0;
}

//查询
int on_query(void)
{
	QUERY_REQUEST req;
	printf("ACOUNT_ID:");
	scanf("%d", &req.id);
	printf("NAME:");
	scanf("%s", req.name);
	printf("PASSWORD:");
	scanf("%s", req.passwd);
	int sockfd;
	if (-1 == (sockfd = sndreq(g_IP, TYPE_QUERY, (void*)&req, sizeof(req))))
	{
		return 0;
	}
	QUERY_RESPOND res;
	memset(&res, 0, sizeof(res));
	if (-1 == rcvres(sockfd, (void*)&res, sizeof(res)))
	{
		printf("rcvres failure!\n");
		return 0;
	}
	if (0 != strlen(res.error))
	{
		printf("%s\n", res.error);
		return 0;
	}
	printf("Balance：%.2lf\n", res.balance);

	return 0;
}

//转账
int on_transfer(void)
{
	TRANSFER_REQUEST req;
	memset(&req, 0, sizeof(req));
	printf("ACOUNT_ID:");
	scanf("%d", &req.id[0]);
	printf("Target ACOUNT_ID");
	scanf("%d", &req.id[1]);
	printf("NAME:");
	scanf("%s", req.name[0]);
	printf("Target's name:");
	scanf("%s", req.name[1]);
	printf("PASSWORD:");
	scanf("%s", req.passwd);
	printf("BALANCE：");
	scanf("%lf", &req.money);
	int sockfd;
	if (-1 == (sockfd = sndreq(g_IP, TYPE_TRANSFER, (TRANSFER_REQUEST*)&req ,sizeof(req))))
	{
		return 0;
	}
	TRANSFER_RESPOND res;
	memset(&res, 0, sizeof(res));
	if (-1 == rcvres(sockfd, (TRANSFER_RESPOND*)&res, sizeof(res)))
	{
		return 0;
	}
	if (0 != strlen(res.error))
	{
		printf("%s\n", res.error);
		return 0;
	}
	printf("Transfer success！\n");

	return 0;
}

int main(int agrc, char* agrv[])
{
	if (2 > agrc)
	{
		printf("用法：%s <IP地址>\n", agrv[0]);
		return -1;
	}
	g_IP = agrv[1];
	//菜单选项
	int (*on_menu[])(void) = {
		on_quit,
		on_open,
		on_close,
		on_save,
		on_withdraw,
		on_query,
		on_transfer
	};
	menu_loop(main_menu, on_menu, sizeof(on_menu) / sizeof (on_menu[0]));

	return 0;
}

