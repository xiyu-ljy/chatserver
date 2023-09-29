#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"
#include <string>
#include <vector>
using namespace std;

class GroupModel
{
public:
    // 群组创建
    bool createGroup(Group &group);
    // 群组加入
    void addGroup(int userid,int groupid, string role);
    // 查询用户所在的群组
    vector<Group> queryGroups(int userid);
    // 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其他成员群发消息
    vector<int> queryGroupUsers(int userid, int groupid);
};

#endif