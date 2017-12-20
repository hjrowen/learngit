#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include "../inc/dao.h"

const char* ID_FILE = "id.dat";

// 文件锁
int file_lock(int fd, short l_type)
{
	struct flock lock;
	lock.l_type=l_type;
	lock.l_pid=getpid();
	lock.l_whence=SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	return (fcntl(fd, F_SETLKW, &lock));
}

// 生成唯一的帐号
int genid(void)
{
	int id = 1000;
	int fd = open(ID_FILE, O_RDWR | O_CREAT, 0644);
	if (-1 == fd)
	{
		perror("open failed");
		return -1;
	}
	if (-1 == read(fd, &id, sizeof(id)))
	{
		perror("read failed");
		return -1;
	}
	++id;
	if (-1 == lseek(fd, 0, SEEK_SET))
	{
		perror("lseek failed");
		return -1;
	}
	if (-1 == write(fd, &id, sizeof(id)))
	{
		perror("write");
		return -1;
	}
	close(fd);
	return id;
}

// 保存账户信息
int save(const ACCOUNT* acc)
{
	char path_name[PATH_MAX + 1] = {0};
	sprintf(path_name, "%d.acc", acc->id);
	int fd = creat(path_name, 0644);
	if (-1 == fd)
	{
		perror("creat failed");
		return -1;
	}
	if (-1 == write(fd, acc, sizeof(*acc)))
	{
		perror("write failed");
		return -1;
	}
	close(fd);
	return 0;
}

// 获取账户信息
int get(int id, ACCOUNT* acc)
{
	char path_name[PATH_MAX + 1] = {0};
	sprintf(path_name, "%d.acc", id);
	int fd = open(path_name, O_RDONLY);
	if (-1 == fd)
	{
		perror("open failed");
		return -1;
	}
	while (-1 == file_lock(fd, F_RDLCK))
	{
		if (EAGAIN == errno)
			continue;
	}
		
	if (-1 == read(fd, acc, sizeof(*acc)))
	{
		perror("read failed");
		return -1;
	}
	while (-1 == file_lock(fd, F_UNLCK))
	{
		if (EAGAIN == errno)
			continue;
	}
	close(fd);
	return 0;
}

// 更新账户信息
int update(const ACCOUNT* acc)
{
	char path_name[PATH_MAX + 1] = {0};
	sprintf(path_name, "%d.acc", acc->id);
	int fd = open(path_name, O_WRONLY);
	if (-1 == fd)
	{
		perror("open");
		return -1;
	}
	while (-1 == file_lock(fd, F_WRLCK))
	{
		if (EAGAIN == errno)
			continue;
	}
	if (-1 == write(fd, acc, sizeof(*acc)))
	{
		perror("write failed");
		return -1;
	}
	while (-1 == file_lock(fd, F_UNLCK))
	{
		if (EAGAIN == errno)
			continue;
	}
	close(fd);
	return 0;
}

// 转帐
int infoerr(char*buf, size_t len)
{
	int fd = open("../doc/err.txt", O_CREAT | O_RDWR);
	if (-1 == fd)
	{
		perror("open");
		return -1;
	}
	if (-1 == write(fd, buf, len))
	{
		perror("write");
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

// 删除账户信息
int delete(int id)
{
	char path_name[PATH_MAX + 1];
	sprintf(path_name, "%d.acc", id);
	if (-1 == unlink(path_name))
	{
		perror("unlink failed");
		return -1;
	}
	return 0;
}

