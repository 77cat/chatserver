#include "usermodel.hpp"
#include "db.h"
#include <iostream>
using namespace std;

//User表的增删查改查
bool UserModel::insert(User &user)
{
    //1.组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into user(name, password, state) values('%s', '%s', '%s')",
    user.getName().c_str(),user.getPwd().c_str(),user.getState().c_str());

    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            //获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

//根据用户号码查询用户信息
User UserModel::query(int id)
{
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"select * from user where id = %d", id);

    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);     //这里用res保存注册的信息
        if(res != nullptr)  //只要有接收到的注册信息
        {
            MYSQL_ROW row = mysql_fetch_row(res);   //提取行信息（按行提取信息
            if(row != nullptr)  //行内信息不为空，也进行一个输出的工作
            {
                User user;
                user.setId(atoi(row[0]));  //atoi表示自动转化为整型int
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);   //这里一定要及时清除内存，否则会使内存不断泄露
                return user;
            }
        }
    }
    return User();//如果没有找到，就默认返回User  这里的默认id = -1   
}


//更新用户的状态信息
bool UserModel::updateState(User user)
{
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"update user set state = '%s' where id = %d ",user.getState().c_str(),user.getId());

    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}


//重置用户的状态信息
void UserModel::resetState()
{
    //1.组装sql语句
    char sql[1024] = "update user set state = 'offline' where state = 'online'";
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }

}