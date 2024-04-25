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
	MYSQL* p = mysql_real_connect(_conn, ip.c_str(), user.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
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
		LOG("¸üÐÂÊ§°Ü£º" + string(mysql_error(_conn)));
		return false;
	}
	return true;
}

MYSQL_RES* Connection::query(string sql)
{
	if (mysql_query(_conn, sql.c_str())) {
		LOG("²éÑ¯Ê§°Ü£º" + sql);
		return nullptr;
	}
	return mysql_use_result(_conn);
}
