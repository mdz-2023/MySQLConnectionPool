cmake_minimum_required(VERSION 3.0)
project(myConnPool)

# 设置调试信息 以及 启动C++11语言标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++14 -fPIC")

include_directories("/usr/include/mysql")
link_libraries()

# 定义参与编译的源代码文件 
aux_source_directory(. SRC_LIST)

# 编译生成动态库
add_library(myConnPool SHARED ${SRC_LIST})

add_executable(testConnPool ${SRC_LIST})

set(MYSQL_LIBRARIES /usr/lib/x86_64-linux-gnu/libmysqlclient.so)

target_link_libraries(myConnPool pthread ${MYSQL_LIBRARIES} )
target_link_libraries(testConnPool pthread ${MYSQL_LIBRARIES})