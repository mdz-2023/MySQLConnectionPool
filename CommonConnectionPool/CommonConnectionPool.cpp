#include "CommonConnectionPool.h"
#include "public.h"
#include <fstream>
#include <functional>
#include <thread>

//线程安全的懒汉式单例 函数接口
CommonConnectionPool* CommonConnectionPool::getConnectionPool()
{
	static CommonConnectionPool connPool; // 系统为静态变量自动加线程锁 lock unlock
	return &connPool;
}

shared_ptr<Connection> CommonConnectionPool::getConnection()
{
	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty()) {
		// 队列为空就等待一段时间，这段时间内一直等待生产者唤醒
		// 循环是为了再醒来的时候有其他消费者抢占了资源，那我就继续等待
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout))){
			// 如果在超时后醒来
			if (_connectionQue.empty()) {
				LOG("获取空闲连接超时...获取连接失败！");
				return nullptr;
			}
		}
		
	}
	/*
	shared_ptr智能指针析构时，会把Connection资源直接delete掉，
	相当于调用Connection的析构函数，connection就被close掉了
	这里需要自定义shared_ptr的资源释放方式，把connection直接归还到queue中
	构造函数的第二个参数传入一个自定义的指针回收函数，这里直接写lambda表达式
	*/
	shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection* pcon) {
		//这里是在服务器应用线程中调用的，所以要考虑队列的线程安全
		// 此函数在sp离开作用于的时候自动调用
		unique_lock<mutex> lock(_queueMutex);
		_connectionQue.push(pcon);
		pcon->refreshAliveTime();
		}
	);
	_connectionQue.pop();
	cv.notify_all();// 消费完链接之后，通知生产者线程检查一下，如果队列为空，生产新连接

	return sp;
}

CommonConnectionPool::CommonConnectionPool()
{
	if (!loadConfigFile()) {
		LOG("配置文件有误");
	}

	// 创建初始数量的连接
	for (auto i = 0; i < _initSize; ++i) {
		Connection* p = new Connection();
		//cout << p << endl;
		p->connect(_ip, _port, _username, _password, _dbname);
		_connectionQue.push(p);
		p->refreshAliveTime();
		_connectionCnt++;
	}

	// 启动一个新的线程作为连接的生产者，生产多于初始数量的连接
	// 创建 std::thread 对象时，可以传递给它一个可调用对象（例如函数、函数对象、lambda表达式等）
	// 这个可调用对象将在新线程中执行。
	// 
	// std::bind 是一个函数模板，用于生成一个可调用对象
	// 它可以将一个成员函数与一个对象实例绑定在一起，生成一个可以像普通函数一样调用的对象
	thread produce(std::bind(&CommonConnectionPool::produceConnectionTask, this));// 生产者线程对象
	produce.detach();// 守护线程
	// 另一种调用方法，lambda表达式通过 & 捕获了当前对象的引用
	//thread produce([&]() {this->produceConnectionTask(); }); 


	//启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行回收
	thread scanner([&]() {this->scannerConnectionTask(); });
	scanner.detach();
}

bool CommonConnectionPool::loadConfigFile()
{
	ifstream infile; // 只读文件对象
	infile.open("../mysql.conf");
	if (infile.is_open() == false) {
		LOG("mysql.conf file is not exist!");
		return false;
	}
	string line;
	while (getline(infile, line)) {
		int idx = line.find('=');
		if (idx == string::npos) {
			continue;
		}
		string key = line.substr(0, idx);
		string value = line.substr(idx + 1, line.size() - idx - 1);
		if (key == "ip") {
			_ip = value;
		}
		else if (key == "port") {
			_port = stoi(value);
		}
		else if (key == "username") {
			_username = value;
		}
		else if (key == "password") {
			_password = value;
		}
		else if (key == "dbname") {
			_dbname = value;
		}
		else if (key == "initSize") {
			_initSize = stoi(value);
		}
		else if (key == "maxSize") {
			_maxSize = stoi(value);
		}
		else if (key == "maxIdleTime") {
			_maxIdleTime = stoi(value);
		}
		else if (key == "connectionTime") {
			_connectionTimeout = stoi(value);
		}
	}
	return true;
}

void CommonConnectionPool::produceConnectionTask()
{
	// 互斥锁确保了对队列的线程安全访问
	// 条件变量用于在队列为空时使生产者线程等待，以及在队列中有新连接时通知消费者线程
	while (1) {
		unique_lock<mutex> lock(_queueMutex); // 上锁，线程安全
		// 条件变量通常与互斥锁一起使用，以便在多线程环境中安全地等待某个条件成立
		while (!_connectionQue.empty()) {
			cv.wait(lock); // 使当前线程等待,自动管理锁的状态,不需要手动解锁和重新锁定互斥锁
			//队列不空，此处生产线程进入等待状态，同时释放 _queueMutex 锁
			// 直到消费者消费完了，此处接到通知才能 wait 调用返回
			// 并且 lock 会自动重新获取互斥锁
		}

		// 连接数量没有达到上限，继续创建新的连接
		if (_connectionCnt < _maxSize) {
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			_connectionQue.push(p);
			p->refreshAliveTime();
			_connectionCnt++;
		}

		// 通知消费者线程，可以消费连接了
		cv.notify_all();
	}
}

void CommonConnectionPool::scannerConnectionTask()
{
	while (1) {
		this_thread::sleep_for(chrono::seconds(_maxIdleTime)); // 睡眠等待连接空闲超时

		//扫描整个队列，释放多余连接
		unique_lock<mutex> lock(_queueMutex); // 自动加锁解锁
		while (_connectionCnt > _initSize) {
			Connection* p = _connectionQue.front();
			if (p->getAliveTime() >= chrono::duration<double>(_maxIdleTime)) {
				_connectionQue.pop();
				_connectionCnt--;
				delete p;// 调用~Connection() 释放连接
			}
			else {
				break; // 队头的未超时，其他的也不
			}
		}
	}
}
