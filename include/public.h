#pragma once
/*
* 公共头文件
* 存放共用数据结构等
*/
#include <iostream>
using namespace std;

/*
* 定义一个名为LOG的宏，简化打印输出
* \号表示宏将在下一行继续
*/ 
#define LOG(str)  \
	cout << __FILE__ << ":" << __LINE__ << " "  \
		__TIMESTAMP__ << ":" << str << endl;