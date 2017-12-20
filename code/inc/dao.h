#ifndef _DAT_H
#define _DAT_H
#include "../inc/bank.h"

// 文件锁
int file_lock(int fd, short l_type);

// 生成唯一的帐号
int genid(void);

// 保存账户信息
int save(const ACCOUNT* acc);

// 获取账户信息
int get(int id, ACCOUNT* acc);

// 更新账户信息
int update(const ACCOUNT* acc);

// 删除账户信息
int delete(int id);

// 记录错误信息
int infoer(char* info, size_t info_len);

#endif	// _DAO_H

