#这里的用途是由所有源码生成可执行文件

#定义了一个SRC_LIST变量，包含了该目录下所有的源文件
aux_source_directory(. SRC_LIST)  #这个指令只能调当前目录下的，不能搜索当前目录的子目录，例如db下的源文件不能包含进来
aux_source_directory(./db DB_LIST)   #补充子目录的源文件
aux_source_directory(./model MODEL_LIST)   #补充子目录的源文件
aux_source_directory(./redis REDIS_LIST)   #补充子目录的源文件

#指定生成可执行文件
# add_executable(ChatServer ${SRC_LIST})
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})
#指定可执行文件链接时，需要依赖的库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)