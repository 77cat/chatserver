// muduo网络库给用户提供了两个主要的类
//  TcpServer:用于编写服务器程序的
//  TcpClient:用于编写客户端程序的

// epoll + 线程池
// 好处：能够把网络I/O的代码和业务代码区分开  （I/O的代码直接封装好，只需要写好业务代码即可）
// 业务代码主要关注：用户的连接和断开，用户的可读写事件
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include<string>
// 里面包含绑定器<functional> 提供了如 std::bind 和 std::placeholders 这样的工具，它们用于绑定成员函数到回调函数。
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders; // namespace placeholders;

// 基于事件驱动/IO复用的muduo网络库开发服务器程序
// step1:组合TcpServer对象
// step2:创建EventLoop事件循环对象的指针
// step3:明确TcpServer构造函数需要什么参数，输出ChatServer的构造函数
// step4:在当前服务器类的构造函数当中，注册处理连接的回调函数和处理读写事件的回调函数
// step5:设置合适的服务端线程数量，muduo库会自己分配I/O线程和worker线程
class ChatServer
{
public:
    ChatServer(EventLoop *loop,               // 事件循环
               const InetAddress &listenAddr, // IP+Port
               const string &nameArg)         // 服务器的名字
        : _server(loop, listenAddr, nameArg), _loop(loop)
    // 因为封装函数没有写好默认值，所以在这里需要单独给各个参数赋值
    // 初始化列表中的 _server 初始化了一个 TcpServer 实例，_loop 则保存了传入的 EventLoop 指针。
    {
        // 给服务器注册用户连接和创建回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
        // 因为下面的函数只有一个参数，我们自己写的涉及两个参数，所以要用绑定器进行绑定，绑定this到onconnect方法当中
        // 其中_1是参数占位符
        // 构造函数体内，我们设置了当有新的连接建立或者已有连接断开时要调用的回调函数。
        // 这里使用了 std::bind 来将 ChatServer 类的成员函数 onConnection 绑定为 TcpServer 的连接回调。

        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
        // 表示给3个参数进行占位

        // 设置服务器端的线程数量  1个I/O线程，3个worker线程
        _server.setThreadNum(4);
    }

    // 开启事件循环
    void start()
    {
        _server.start();
    }

private:
    // 专门处理用户的连接创建和断开  epoll listenfd  accept   它将在每次有新的连接建立或断开时被调用。
    void onConnection(const TcpConnectionPtr &conn)
    {
        if(conn->connected())
        {
            cout<<conn->peerAddress().toIpPort()<<" -> "<<conn->localAddress().toIpPort()<<" state:online"<<endl;
        }
        else
        {
            cout<<conn->peerAddress().toIpPort()<<" -> "<<conn->localAddress().toIpPort()<<" state:offline"<<endl;
            conn->shutdown();   //相当于close(fd)  也就是再linux上，把资源释放掉
            // _loop->quit();  //相当于退出epoll
        }

    }

    // 专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr &conn, // 表示连接
                   Buffer *buffer,                  // 表示缓冲区
                   Timestamp time)               // 接收数据的时间信息
    {
        //将发送的数据再原封不动发回去
        string buf = buffer->retrieveAllAsString();  //可以将接收的数据全都放到字符串当中
        cout<<"recv data:"<<buf<<"time:"<<time.toString()<<endl;//时间信息也可以转化为字符串
        conn->send(buf);
    }
    TcpServer _server; // step1  定义tcpserver类型的指针
    EventLoop *_loop;  // step2  相当于是我们自己的epoll事件循环
};

int main()
{
    EventLoop loop;   //epoll
    InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"ChatServer");

    server.start();  //listenfd epoll_ctl=>epoll   将听的epoll连接到epoll上
    loop.loop(); //epoll_wait以阻塞方式等待新用户连接，已连接用户的读写事件等
    return 0;
}


/*
绑定器的使用放在有道笔记里面

*/