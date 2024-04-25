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
* ���ӳ���صĹ���ģ��
* ʵ���̰߳�ȫ�ĵ���ģʽ
*/
class CommonConnectionPool
{
public:
	// ��ȡ���ӳ�Ψһʵ��
	static CommonConnectionPool* getConnectionPool();

	// ���ⲿ�ӿڣ������ӳ��л�ȡһ�����õĿ�������
	// ��������ָ�룬ʹ���û�ʹ�����������ֶ���ָ�뷵�ض���
	// ͨ���ض�������ָ���ɾ�����������ָ��Ļ���
	shared_ptr<Connection> getConnection();

private:
	CommonConnectionPool(); // ���� ���캯��˽�л�

	// �������ļ��м���������
	bool loadConfigFile();
	// �����ڶ������߳��У�ר�Ÿ�������������
	void produceConnectionTask();
	// �����ڶ����߳��У�ר�Ż��տ���ʱ�䳬��maxIdleTime������
	void scannerConnectionTask(); 

	// mysql��ز���
	string _ip;
	unsigned short _port;
	string _username;
	string _password;
	string _dbname;

	// ���ӳ���ز���
	int _initSize; // ��ʼ������
	int _maxSize; // ���������
	int _maxIdleTime; // ���ӳ����ȴ�ʱ�� 
	int _connectionTimeout; // ���ӳػ�ȡ���ӵĳ�ʱʱ��

	queue<Connection*> _connectionQue; // �洢mysql�������ӵĶ���
	mutex _queueMutex; // ά�����Ӷ��е��̰߳�ȫ������

	//atomic��ԭ�Ӳ������� ++_connectionCnt ���̰߳�ȫ��
	atomic_int _connectionCnt; // ��¼��������connection���ӵ�������

	// ���ڽ��̼�ͨ��
	condition_variable cv; // ���������������������������̺߳����������̵߳�ͨ��
};

