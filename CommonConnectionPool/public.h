#pragma once
/*
* ����ͷ�ļ�
* ��Ź������ݽṹ��
*/
#include <iostream>
using namespace std;

/*
* ����һ����ΪLOG�ĺ꣬�򻯴�ӡ���
* \�ű�ʾ�꽫����һ�м���
*/ 
#define LOG(str)  \
	cout << __FILE__ << ":" << __LINE__ << " "  \
		__TIMESTAMP__ << ":" << str << endl;