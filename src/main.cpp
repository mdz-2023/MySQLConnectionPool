#include <iostream>
#include <memory>
#include <chrono>
#include "public.h"
#include "Connection.h"
#include "CommonConnectionPool.h"

using namespace std;

int main() {
	/*
	Connection conn;
	conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
	conn.update(string("insert into user(name,age,sex) value('zhang san','20','male')"));
	*/
	auto begin = std::chrono::high_resolution_clock::now();

	//for (int i = 0; i < 1000; ++i) {
	//	Connection conn;
	//	if (conn.connect("127.0.0.1", 3306, "root", "123456", "chat") == true) {
	//		conn.update(string("insert into user(name,age,sex) value('zhang san','20','male')"));
	//	}
	//}
	for (int i = 0; i < 1; ++i) {
		CommonConnectionPool* cp = CommonConnectionPool::getConnectionPool();
		shared_ptr<Connection> sp = cp->getConnection();
		// sp->update(string("insert into user(name,password) value('zhang san0','23446')"));
		sp->update(string("update password='12222' from user where name='zhang san'"));
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end - begin;
	cout << diff.count() << endl;
	return 0;
}