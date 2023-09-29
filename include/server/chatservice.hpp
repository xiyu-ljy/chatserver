#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <unordered_map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <mutex>
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

using json = nlohmann::json;
using namespace muduo;
using namespace muduo::net;
using namespace std;

// 处理消息回调函数类型
using MsgHandler = std::function<void(const TcpConnectionPtr & conn, json &js ,Timestamp time)>;

class ChatService{
public:
    static ChatService* instance();
    // 注册和登录业务
    void login(const TcpConnectionPtr & conn, json &js ,Timestamp _time);
    void reg(const TcpConnectionPtr & conn, json &js ,Timestamp _time);
     //一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn,json &js,Timestamp _time);
    //添加好友业务
    void addFriend(const TcpConnectionPtr &conn,json &js,Timestamp _time);
    //创建群组
    void createGroup(const TcpConnectionPtr &conn,json &js,Timestamp _time);
    //加入群组业务
    void addGroup(const TcpConnectionPtr &conn,json &js,Timestamp _time);
    // 群聊业务
    void groupChat(const TcpConnectionPtr &conn,json &js,Timestamp _time);
    // 登出业务
    void loginout(const TcpConnectionPtr &conn,json &js,Timestamp _time);

    // 获取对应id的处理器
    MsgHandler getMsgHandle(int msgid);

    //用户异常退出
    void clientCloseException(const TcpConnectionPtr & conn);

    void handleRedisSubscribeMessage(int id,string msg);

    //服务器异常，业务重置方法
    void reset();
    

private:
    ChatService();
    // 消息表，id对应的处理
    unordered_map<int, MsgHandler> _msghandlerMap;

    //保障_userConnectionMap线程安全
    std::mutex _mloc;
 
    //存储在线用户的通信连接
    unordered_map<int,TcpConnectionPtr> _userConnectionMap;

    //数据操作类对象
    UserModel _uModel;
    //离线消息处理对象
    OfflineMsgModel _offModel;
    //好友列表
    FriendModel _fModel;
    //群组相关
    GroupModel _gModel;

    Redis _redis;
};

#endif