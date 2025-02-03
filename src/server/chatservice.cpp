#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <string>
#include<vector>

using namespace muduo;
using namespace std;
//获取单例对象的接口函数
ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}


//注册消息以及对应的Handler回调操作
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    //将消息id与对应的处理器绑定起来
    _msgHandlerMap.insert({GREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
     _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
    
    //连接redis服务器
    if(_redis.connect())
    {
        //设置上报消息的回调(连接成功以后必须预制一个回调)
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this,_1, _2));
        //这里需要上报的两个参数分别是，通道号和对应消息
    }


}

//服务器异常，业务重置方法
void ChatService::reset()
{
    //把online状态的用户，设置成offline
    _userModel.resetState();
}


//获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    //记录错误日志，msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end())
    {
        //LOG_ERROR<<"msgid"<<msgid<<"can not find handler";  //一直到最后都没找到，就输出这个
        //可以返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR<<"msgid"<<msgid<<"can not find handler";
        };

    }
    else
    {
        return _msgHandlerMap[msgid];  //如果找到了，就返回对应的处理器
    }
    

}
//处理登陆业务   ORM  对象关系映射：帮助oop解决了业务层操作的对象   和数据层数据库  之间的拆分问题
//业务代码中，尽量不要出现数据层的增删改查
//处理登陆业务的时候，三个要素，主要是  id pwd 
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    //LOG_INFO<<"do login service!!";   //info默认就是通知的意思
    int id = js["id"].get<int>();
    string pwd = js["password"];
    User user = _userModel.query(id);
    if(user.getId() == id && user.getPwd() == pwd)  //表示该用户是存在的
    {
        if(user.getState() == "online")
        {
            //该用户已经登陆，不允许重复登陆
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;  //错误类型是2
            response["errmsg"] = "this account is using, input another!";
            conn->send(response.dump());
        }

        else
        {
            //登陆成功，记录用户连接信息
            {
            lock_guard<mutex> lock(_connMutex);   //函数里面是通过引用接收   大括号内加锁，出括号解锁，刚好保证临界字段的线程安全
            _userConnMap.insert({id,conn});  //后续这个map会被多线程调用
            }

            //id用户登陆成功后，向redis订阅channel(id)
            _redis.subscribe(id);   //xxxxxxx
            
            //登陆成功，更新用户状态信息， state offline 更新成 online
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            //登陆成功后，要查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty())  //表示离线消息不为空
            {
                response["offlinemsg"] = vec;
                //读取该用户的离线消息后，把该用户的所有离线消息删除掉
                _offlineMsgModel.remove(id);
            }

            //查询该用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty())
            {
                vector<string> vec2;
                for(User &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 查询用户的群组信息
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }

            conn->send(response.dump());
        }

    }

    else
    {
        //该用户不存在，或者，用户存在但是密码错误，所以登陆失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "id or password is invalid!";
        conn->send(response.dump());
    }
}

//处理注册业务  需要填写的信息 name password   要返回的信息 id
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // LOG_INFO<<"do reg service!!";   //info默认就是通知的意思
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);   //从user文件中把后面的插入信息提前过来
    if(state)
    {
        //注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;  //0表示没有错误
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["id"] = 1;
        conn->send(response.dump());
        //这段代码的主要功能是将 response 对象的内容发送出去。
        //conn 是一个连接对象，它具有 send 方法用于发送数据。
        //而 response 是一个对象，response.dump() 方法的作用是将该对象的数据转换为某种可以发送的格式(字符串，字节流)

    }
}

//处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);  //做线程加速操作
        auto it = _userConnMap.find(userid);
        if(it != _userConnMap.end())  //一直遍历到结尾都没找到，就要报错
        {
            _userConnMap.erase(it);
        }
    }

    //用户注销，相当于下线，在redis中取消订阅通道
    _redis.unsubscribe(userid);

    //更新用户状态
    User user(userid,"","","offline");
    _userModel.updateState(user);

}


//c处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        //加到括号里面，进行线程安全保护
        lock_guard<mutex> lock(_connMutex);
        for(auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if(it->second == conn)
            {
                //从map表中删除用户的链接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    //用户注销，相当于下线，在redis中取消订阅通道
    _redis.unsubscribe(user.getId());

    //更新用户的状态信息
    if(user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

//一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    //先获取接收方的id 
    int toid = js["toid"].get<int>();

    //标识用户是否在线
    // bool userState = false;
    {
        lock_guard<mutex> lock(_connMutex);  //互斥锁
        auto it = _userConnMap.find(toid);    //在map里面寻找id
        if(it != _userConnMap.end())  //it不等于最后的值，表示此时找到了
        {
            //toid在线，转发消息  服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }

    // 查询toid是否在线 (当前主机找不到，但是显示在线，表明在其他的主机上应该是登陆的，此时也可以发送消息)
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }

    //不满足if条件，toid不在线，存储离线消息（当前主机找不到，且状态也是不在线的）
    _offlineMsgModel.insert(toid,js.dump());  //发消息给toid，发送的消息是js
}


//添加好友业务 msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    //存储好友信息
    _friendModel.insert(userid,friendid);
}

//创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    //存储新创建的群组消息
    Group group(-1, name, desc);
    if(_groupModel.createGroup(group))  //表示如果有新创建的群组信息
    {
        //如果群创建成功，就首先存储群组创建人信息
        _groupModel.addGroup(userid,group.getId(),"creator");

    }
}

void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid,groupid,"normal");  //一般成员加入进来的角色就是normal

}

//群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid,groupid);
    lock_guard<mutex> lock(_connMutex);   //在外面统一加锁，保证map表线程安全
    for(int id : useridVec)
    {
       
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end())
        {
            //转发群消息
            it->second->send(js.dump());
        }
        else
        {
            //存储离线群消息
            _offlineMsgModel.insert(id,js.dump());
        }
    }
}

//从redis消息队列中获取订阅的消息

void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end())
    {
        it->second->send(msg);   //不必进行json的反序列化，直接把msg发送过去就可以
        return;
    }

    //接收到了该用户的消息后，却发现该用户下线了，此时需要存储离线消息
    //存储该用户的离线消息
    _offlineMsgModel.insert(userid,msg);

}