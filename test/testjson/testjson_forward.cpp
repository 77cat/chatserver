//这里是json序列化
#include "json.hpp"
using json = nlohmann::json;

#include<iostream>
#include<vector>
#include<map>
#include<string>

using namespace std;

//json序列化示例1
void func1()
{
    json js;
    js["msg_type"] =2;  //功能2 表示发送信息
    js["from"] = "zhang san";
    js["to"] = "li si";
    js["msg"] = "hello,what are you doing now?";

    string sendBuf = js.dump();//表示将js里面的数据序列化后给sendBuf遍历

    cout<<js<<endl;
    cout<<sendBuf.c_str()<<endl;  //通过网络传送的时候输出该元素转为char* 才能输出，因此要加后缀才能正常输出
    //转成sendBuf的形式就可以通过网络发送数据
}

//json序列化示例2
void func2()
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
    //此时msg后面放的键值对，在大括号中，本身也是字符串，所以说，json里面放置的数据类型也可以是键值对
    //同样里面包含的类型可以为数组，字符串，等
    cout << js << endl;

// 对于 std::map 或 std::unordered_map，元素的插入顺序不一定会被保留，
// 因为 std::unordered_map 是基于哈希表实现的，
// 而 std::map 是基于红黑树实现的，它们的内部存储结构不保证元素的插入顺序与遍历顺序相同。
}

//json序列化数组示例3
void func3()
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
    cout<<js<<endl;
}

int main()
{
    func1();
    func2();
    func3();
    return 0;
}

