#ifndef FRIENDMESSAGE_H
#define FRIENDMESSAGE_H

#include "user.hpp"
#include <vector>

using namespace std;

class FriendModel{
public:
    //添加好友关系
    void insert(int userid,int friendid);

    //返回用户好友列表 多表查询，查找好友的id对应的User表中的名称
    vector<User> query(int userid);
private:
    
};

#endif