#pragma once
#include <string>
#include "Connection.h"
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
using namespace std;
/*
* 连接池相关的功能模块
* 实现线程安全的单例模式
*/
class CommonConnectionPool
{
public:
	// 获取连接池唯一实例
	static CommonConnectionPool* getConnectionPool();

	// 给外部接口，从连接池中获取一个可用的空闲连接
	// 返回智能指针，使得用户使用完对象后不用手动将指针返回队列
	// 通过重定义智能指针的删除函数，完成指针的回收
	shared_ptr<Connection> getConnection();

private:
	CommonConnectionPool(); // 单例 构造函数私有化

	// 从配置文件中加载配置项
	bool loadConfigFile();
	// 运行在独立的线程中，专门负责生产新连接
	void produceConnectionTask();
	// 运行在独立线程中，专门回收空闲时间超过maxIdleTime的连接
	void scannerConnectionTask(); 

	// mysql相关参数
	string _ip;
	unsigned short _port;
	string _username;
	string _password;
	string _dbname;

	// 连接池相关参数
	int _initSize; // 初始连接量
	int _maxSize; // 最大连接量
	int _maxIdleTime; // 连接池最大等待时间 
	int _connectionTimeout; // 连接池获取连接的超时时间

	queue<Connection*> _connectionQue; // 存储mysql空闲连接的队列
	mutex _queueMutex; // 维护连接队列的线程安全互斥锁

	//atomic是原子操作，即 ++_connectionCnt 是线程安全的
	atomic_int _connectionCnt; // 记录所创建的connection连接的总数量

	// 用于进程间通信
	condition_variable cv; // 设置条件变量，用于连接生产线程和链接消费线程的通信
};

