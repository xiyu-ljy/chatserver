#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

// user表的数据操作类
class UserModel{
public:
    bool insert(User &user);
    User query(int id);
    //更新在线状态
    bool updateState(User& user);
    //重置方法
    void resetModel();
};

#endif