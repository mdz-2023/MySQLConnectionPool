#pragma once
#include <mysql.h>
#include <string>
#include <ctime>
#include <chrono>
using namespace std;
/*
* 建立数据库连接，封装操作函数
* 数据库增删改查相关的功能
*/
class Connection
{
public:
	//初始化数据库连接
	Connection();
	//释放数据库连接资源
	~Connection();

	// 连接数据库
	bool connect(string ip, unsigned short port,
		string user, string password, string dbname);

	// 更新操作 insrt delete update
	bool update(string sql);

	// 查询操作 select
	MYSQL_RES* query(string sql);

	// 刷新连接的起始空闲时间
	void refreshAliveTime() { _aliveTime = std::chrono::high_resolution_clock::now(); }
	// 返回本连接的空闲时间
	std::chrono::duration<double> getAliveTime() { return std::chrono::high_resolution_clock::now() - _aliveTime; }

private:
	MYSQL* _conn; // 表示和MySQL Server的一条连接
	chrono::high_resolution_clock::time_point _aliveTime; // 记录进入空闲状态后的起始时间
};

