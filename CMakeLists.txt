cmake_minimum_required(VERSION 3.0)
project(chat)

#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

#指定搜索目录
# add_subdirectory(testmuduo)
#执行chat这个project的时候，先是从chat这个文件夹下的cmakelist.txt开始执行
#进入当前的cmakelist文件之后，执行到上面搜索指定目录的操作之后，进入到对应的文件夹testmuduo
#于此同时，开始执行testmuduo文件夹里的cmakelist文件

#配置最终的可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#配置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
#加载子目录
add_subdirectory(src)
