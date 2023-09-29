#ifndef PUBLIC_H
#define PUBLIC_H

/*
    server和client公共文件
*/
enum EnMsgType{
    LOGIN_MSG = 1, //登录消息
    REG_MSG,     //注册消息
    REG_MSG_ACK, //注册响应消息
    LOGIN_MSG_ACK, //登录响应
    ONE_CHAT_MSG,    //一对一聊天
    Add_Friend, //添加好友消息

    CREATE_GROUP_MSG,//创建群组
    ADD_GROUP_MSG,//加入群组
    GROUP_CHAT_MSG,//群聊天

    LOGINOUT_MSG,//注销
};


#endif