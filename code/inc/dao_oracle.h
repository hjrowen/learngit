#ifndef _DAT_H
#define _DAT_H
#include "../inc/bank.h"

//连接数据库
int connect_db(void);

//判断数据库操作是否成功
int is_success(const char* info);

//生成帐号
int genid(void);

// 保存账户信息
int save(const ACCOUNT* acc);

// 获取账户信息
int get(int id, ACCOUNT* acc);

//转账业务
int transfer(int destination_id, int source_id, double money); 

// 更新账户信息
int update(const ACCOUNT* acc);

// 删除账户信息
int delete(int id);

#endif	// _DAO_H

