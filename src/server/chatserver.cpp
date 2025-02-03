#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include "chatserver.hpp"
#include<string>
#include <functional>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop,
                        const InetAddress &listenAddr,
                        const string &nameArg)
        :_server(loop,listenAddr,nameArg),_loop(loop)
{
    //注册链接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));

    //注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));

    //设置线程数量
    _server.setThreadNum(4);
}
//其中，四个线程主要包括，一个主reactor，三个子reactor()
//主reactor主要负责新用户的链接，子reactor主要负责链接用户的读写事件的处理



//启动服务
void ChatServer::start()
{
    _server.start();
}

//链接监听与读写事件监听

//上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    //客户端断开连接
    if(!conn->connected())
    {
        //这里专门写一个方法，是客户端断开的时候可以自动更改用户的状态，如果下线了还在online,会导致后面登陆不上
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }

}

//上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                            Buffer *buffer,
                            Timestamp time)
{
    //收到消息后
    string buf = buffer->retrieveAllAsString();  //相当于把缓存区接收器里的字符给buf
    json js = json::parse(buf);
    //达到的目的：完全解耦网络模块和业务模块的代码

    //通过js["msgid"]获取 =》 业务handle =》 conn js time
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    //回调消息对应绑定好的事件处理器，来执行相应的业务处理，具体的事件绑定过程在chatservice里面
    msgHandler(conn,js,time);
}