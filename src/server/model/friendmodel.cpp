#include "db.hpp"
#include "friendmodel.hpp"
#include <iostream>

using namespace std;

//添加好友关系
void FriendModel::insert(int userid,int friendid){
    char sql[1024]={0};
    // sql语句查询
    sprintf(sql,"insert into friend values(%d,%d)",
            userid,friendid);

    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}

//返回用户好友列表 多表查询，查找好友的id对应的User表中的名称
vector<User> FriendModel::query(int userid){
 char sql[1024]={0};
    // sql语句
    sprintf(sql,"select u.id,u.name,u.state  from user u inner join friend f on f.friendid = u.id where f.userid = %d",userid);

    vector<User> vec;
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr){
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res))!=nullptr) {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}