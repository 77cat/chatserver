cmake_minimum_required(VERSION 3.0)
project(main)

#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

#设置可执行文件最终存储路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#配置头文件搜索路径
#include_directories()
#配置库文件搜索路径
#link_directories()

#设置需要编译的源文件列表
set(SRC_LIST ./muduo_server.cpp)   #但是这行代码相当于只把一个文件放到列表路径里面
# ./ 表示当前路径下的文件名字

#如何放置多个要编译的文件？ 把 .指定路径下 所有源文件名字放入变量名SRC_LIST里面
#aux_source_directory(. SRC_LIST)

#表示生成可执行文件server,由SRC_LIST变量所定义的源文件编译而来
add_executable(server ${SRC_LIST})
#表示server这个目标程序，需要连接muduo_net muduo_base pthread 这三个库文件
target_link_libraries(server muduo_net muduo_base pthread)  #这里需要放置要连接的库