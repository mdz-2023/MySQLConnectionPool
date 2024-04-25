#include "CommonConnectionPool.h"
#include "public.h"
#include <fstream>
#include <functional>
#include <thread>

//�̰߳�ȫ������ʽ���� �����ӿ�
CommonConnectionPool* CommonConnectionPool::getConnectionPool()
{
	static CommonConnectionPool connPool; // ϵͳΪ��̬�����Զ����߳��� lock unlock
	return &connPool;
}

shared_ptr<Connection> CommonConnectionPool::getConnection()
{
	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty()) {
		// ����Ϊ�վ͵ȴ�һ��ʱ�䣬���ʱ����һֱ�ȴ������߻���
		// ѭ����Ϊ����������ʱ����������������ռ����Դ�����Ҿͼ����ȴ�
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout))){
			// ����ڳ�ʱ������
			if (_connectionQue.empty()) {
				LOG("��ȡ�������ӳ�ʱ...��ȡ����ʧ�ܣ�");
				return nullptr;
			}
		}
		
	}
	/*
	shared_ptr����ָ������ʱ�����Connection��Դֱ��delete����
	�൱�ڵ���Connection������������connection�ͱ�close����
	������Ҫ�Զ���shared_ptr����Դ�ͷŷ�ʽ����connectionֱ�ӹ黹��queue��
	���캯���ĵڶ�����������һ���Զ����ָ����պ���������ֱ��дlambda���ʽ
	*/
	shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection* pcon) {
		//�������ڷ�����Ӧ���߳��е��õģ�����Ҫ���Ƕ��е��̰߳�ȫ
		// �˺�����sp�뿪�����ڵ�ʱ���Զ�����
		unique_lock<mutex> lock(_queueMutex);
		_connectionQue.push(pcon);
		pcon->refreshAliveTime();
		}
	);
	_connectionQue.pop();
	cv.notify_all();// ����������֮��֪ͨ�������̼߳��һ�£��������Ϊ�գ�����������

	return sp;
}

CommonConnectionPool::CommonConnectionPool()
{
	if (!loadConfigFile()) {
		LOG("�����ļ�����");
	}

	// ������ʼ����������
	for (auto i = 0; i < _initSize; ++i) {
		Connection* p = new Connection();
		//cout << p << endl;
		p->connect(_ip, _port, _username, _password, _dbname);
		_connectionQue.push(p);
		p->refreshAliveTime();
		_connectionCnt++;
	}

	// ����һ���µ��߳���Ϊ���ӵ������ߣ��������ڳ�ʼ����������
	// ���� std::thread ����ʱ�����Դ��ݸ���һ���ɵ��ö������纯������������lambda���ʽ�ȣ�
	// ����ɵ��ö��������߳���ִ�С�
	// 
	// std::bind ��һ������ģ�壬��������һ���ɵ��ö���
	// �����Խ�һ����Ա������һ������ʵ������һ������һ����������ͨ����һ�����õĶ���
	thread produce(std::bind(&CommonConnectionPool::produceConnectionTask, this));// �������̶߳���
	produce.detach();// �ػ��߳�
	// ��һ�ֵ��÷�����lambda���ʽͨ�� & �����˵�ǰ���������
	//thread produce([&]() {this->produceConnectionTask(); }); 


	//����һ���µĶ�ʱ�̣߳�ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����л���
	thread scanner([&]() {this->scannerConnectionTask(); });
	scanner.detach();
}

bool CommonConnectionPool::loadConfigFile()
{
	ifstream infile; // ֻ���ļ�����
	infile.open("mysql.ini");
	if (infile.is_open() == false) {
		LOG("mysql.ini file is not exist!");
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
	// ������ȷ���˶Զ��е��̰߳�ȫ����
	// �������������ڶ���Ϊ��ʱʹ�������̵߳ȴ����Լ��ڶ�������������ʱ֪ͨ�������߳�
	while (1) {
		unique_lock<mutex> lock(_queueMutex); // �������̰߳�ȫ
		// ��������ͨ���뻥����һ��ʹ�ã��Ա��ڶ��̻߳����а�ȫ�صȴ�ĳ����������
		while (!_connectionQue.empty()) {
			cv.wait(lock); // ʹ��ǰ�̵߳ȴ�,�Զ���������״̬,����Ҫ�ֶ���������������������
			//���в��գ��˴������߳̽���ȴ�״̬��ͬʱ�ͷ� _queueMutex ��
			// ֱ���������������ˣ��˴��ӵ�֪ͨ���� wait ���÷���
			// ���� lock ���Զ����»�ȡ������
		}

		// ��������û�дﵽ���ޣ����������µ�����
		if (_connectionCnt < _maxSize) {
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			_connectionQue.push(p);
			p->refreshAliveTime();
			_connectionCnt++;
		}

		// ֪ͨ�������̣߳���������������
		cv.notify_all();
	}
}

void CommonConnectionPool::scannerConnectionTask()
{
	while (1) {
		this_thread::sleep_for(chrono::seconds(_maxIdleTime)); // ˯�ߵȴ����ӿ��г�ʱ

		//ɨ���������У��ͷŶ�������
		unique_lock<mutex> lock(_queueMutex); // �Զ���������
		while (_connectionCnt > _initSize) {
			Connection* p = _connectionQue.front();
			if (p->getAliveTime() >= chrono::duration<double>(_maxIdleTime)) {
				_connectionQue.pop();
				_connectionCnt--;
				delete p;// ����~Connection() �ͷ�����
			}
			else {
				break; // ��ͷ��δ��ʱ��������Ҳ��
			}
		}
	}
}
