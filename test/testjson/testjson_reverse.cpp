//这里是json反序列化
#include "json.hpp"
using json = nlohmann::json;

#include<iostream>
#include<vector>
#include<map>
#include<string>

using namespace std;

//json序列化示例1
string func1()
{
    json js;
    js["msg_type"] =2;  //功能2 表示发送信息
    js["from"] = "zhang san";
    js["to"] = "liu shuo";
    js["msg"] = "hello,what are you doing now?";

    string sendBuf = js.dump();//表示将js里面的数据序列化后给sendBuf遍历
    return sendBuf;

}

//json序列化示例2
string func2()
{
    json js;
   // 添加数组
    js["id"] = {1,2,3,4,5}; 
    // 添加key-value
    js["name"] = "zhang san"; 
    // 添加对象
    js["msg"]["zhang san"] = "hello world";
    //这种写法表示“zhang san”是msg内部的键
    js["msg"]["liu shuo"] = "hello china"; 
    cout << js << endl;

    // 上面等同于下面这句一次性添加数组对象
    js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello china"}};
    return js.dump();

}

//json序列化数组示例3
string func3()
{
    json js;
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);
    js["list"] = vec;   //相当于json数组里面套用数组

    map<int, string> m;
    m.insert({1,"黄山"});   //数组里面套用数组，将容器进行序列化
    m.insert({2,"华山"});
    m.insert({3,"泰山"});

    js["path"] = m ;      
    //可以看出，这些都是js已经封装好的结构 
    string sendBuf = js.dump();  //json数组对象，序列化json字符串，转化为可以发送的数据
    return sendBuf;
}

int main()
{
    

    string recvBuf =  func1();  //假设recvBuf就是通过网络发送过来的,经过了数据序列化

    //数据反序列化  json字符串反数据化  产生数据对象（看作容器，方便访问）
    json jsbuf = json::parse(recvBuf);   //这样就可以单独打印jsbuf的单个键值对

    cout<<"----------func1--------------------"<<endl;
    cout<<jsbuf["msg_type"]<<endl;
    cout<<jsbuf["from"]<<endl;
    cout<<jsbuf["to"]<<endl;
    cout<<jsbuf["msg"]<<endl;    //这样可以更详细的了解序列化和反序列化的工作原理
    //序列化就相当于个人档案  反序列化就相当于档案中的每一栏信息


    cout<<"----------func2--------------------"<<endl;
    string recvBuf2 =  func2();
    json jsbuf2 = json::parse(recvBuf2);
    cout<<jsbuf2["id"]<<endl;
    cout<<jsbuf2["name"]<<endl;
    cout<<jsbuf2["msg"]<<endl;

    //其他的打印功能
    cout<<jsbuf2["id"]<<endl;
    auto arr = jsbuf2["id"];   //自定义返回类型
    cout<<arr[2]<<endl;   //如果知道类型是string，可以直接打印想要的对应下标的元素

    //也可以自定义json反序列化后的数据类型
    auto msgjs = jsbuf2["msg"];
    cout<<msgjs["zhang san"]<<endl;    //这样打印的是zhang san 对应的msg元素
    cout<<msgjs["liu shuo"]<<endl;  


    cout<<"--------------func3--------------------"<<endl;
    string recvBuf3 =  func3();
    json jsbuf3 = json::parse(recvBuf3);   //这样就可以单独打印jsbuf的单个键值对

    vector<int> vec3 = jsbuf3["list"];
    for(int &v : vec3)
    {
        cout<<v<<" ";
    }
    cout<<endl;

    map<int, string> mymap = jsbuf3["path"];
    for(auto &p : mymap)
    {
        cout<<p.first<<" "<<p.second<<endl;
    }
    cout<<endl;
    return 0;
}

