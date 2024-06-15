#include "Connection.h"
#include "public.h"

Connection::Connection()
{
	_conn = mysql_init(nullptr);
}

Connection::~Connection()
{
	if (_conn != nullptr) {
		mysql_close(_conn);
	}
}

bool Connection::connect(string ip, unsigned short port, string user, string password, string dbname)
{
	// std::cout << ip << std::endl;
	// std::cout <<"ip="<< ip << "1" << port << " "  << user << " "  << password << " "  << dbname << std::endl;
	MYSQL* p = mysql_real_connect(_conn, ip.c_str(), user.c_str(), password.c_str(),
								    dbname.c_str(), port, nullptr, 0);
	// MYSQL* p = mysql_real_connect(_conn, "127.0.0.1", "root", "123456", "chat", 3306, nullptr, 0);
	if (p == nullptr) {
		cout << _conn  << endl;
		LOG("Conncet error: " + string(mysql_error(_conn)));
		return false;
	}
	return true;
}

bool Connection::update(string sql)
{
	if (mysql_query(_conn, sql.c_str())) {
		LOG("更新失败：" + string(mysql_error(_conn)));
		return false;
	}
	return true;
}

MYSQL_RES* Connection::query(string sql)
{
	if (mysql_query(_conn, sql.c_str())) {
		LOG("查询失败：" + sql);
		return nullptr;
	}
	return mysql_use_result(_conn);
}
