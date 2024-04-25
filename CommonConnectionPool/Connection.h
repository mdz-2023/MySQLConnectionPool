#pragma once
#include <mysql.h>
#include <string>
#include <ctime>
#include <chrono>
using namespace std;
/*
* �������ݿ����ӣ���װ��������
* ���ݿ���ɾ�Ĳ���صĹ���
*/
class Connection
{
public:
	//��ʼ�����ݿ�����
	Connection();
	//�ͷ����ݿ�������Դ
	~Connection();

	// �������ݿ�
	bool connect(string ip, unsigned short port,
		string user, string password, string dbname);

	// ���²��� insrt delete update
	bool update(string sql);

	// ��ѯ���� select
	MYSQL_RES* query(string sql);

	// ˢ�����ӵ���ʼ����ʱ��
	void refreshAliveTime() { _aliveTime = std::chrono::high_resolution_clock::now(); }
	// ���ر����ӵĿ���ʱ��
	std::chrono::duration<double> getAliveTime() { return std::chrono::high_resolution_clock::now() - _aliveTime; }

private:
	MYSQL* _conn; // ��ʾ��MySQL Server��һ������
	chrono::high_resolution_clock::time_point _aliveTime; // ��¼�������״̬�����ʼʱ��
};

