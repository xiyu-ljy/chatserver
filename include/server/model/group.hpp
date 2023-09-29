#ifndef GROUP_H
#define GROUP_H

#include "groupuser.hpp"
#include <string>
#include <vector>
using namespace std;

class Group{
public:
    Group(int id=-1,string name="",string desc=""){
        this->id = id;
        this->name = name;
        this->desc = desc;
    }

    void setId(int id){this->id = id;}
    void setName(string id){this->name = name;}
    void setDesc(string id){this->desc = desc;}

    int getId(){return id;}
    string getName(){return name;}
    string getDesc(){return desc;}
    vector<GroupUser>& getUsers(){return users;}

private:
    int id;
    string name;
    string desc;
    vector<GroupUser> users;
};

#endif