#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>


using namespace std;
using namespace muduo;
// 获取单例对象的接口函数
ChatService* ChatService::instance(){
    static ChatService service;
    return &service;
}

// 注册消息以及对应的回调操作
ChatService::ChatService()
{
    _msghandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::login,this,_1,_2,_3)});
    _msghandlerMap.insert({REG_MSG,std::bind(&ChatService::reg,this,_1,_2,_3)});
    _msghandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::oneChat,this,_1,_2,_3)});
    _msghandlerMap.insert({Add_Friend,std::bind(&ChatService::addFriend,this,_1,_2,_3)});
    _msghandlerMap.insert({CREATE_GROUP_MSG,std::bind(&ChatService::createGroup,this,_1,_2,_3)});
    _msghandlerMap.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup,this,_1,_2,_3)});
    _msghandlerMap.insert({GROUP_CHAT_MSG,std::bind(&ChatService::groupChat,this,_1,_2,_3)});
    _msghandlerMap.insert({LOGINOUT_MSG,std::bind(&ChatService::loginout,this,_1,_2,_3)});

    //连接redis服务器
    if(_redis.connect()){
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this,_1,_2));
    }

}

// 注册和登录业务
void ChatService::login(const TcpConnectionPtr & conn, json &js ,Timestamp _time)
{
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _uModel.query(id);
    if(user.getId()!=-1 && user.getPwd() == pwd){
        if(user.getState() == "online"){
            json response;
            // 用户已经登录不可重复登录
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 1;//为1表示响应失败
            response["errmsg"] = "该用户已经登录";
            conn->send(response.dump());
        }else{
            {
            lock_guard<mutex> loc(_mloc);
            _userConnectionMap.insert({id,conn});
            }
            //id用户登录成功后,向redis订阅id
            _redis.subscribe(id);

            //登录成功 ,查询是否有离线消息，消息打包带走
            user.setState("online");

            _uModel.updateState(user);
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;//为0表示登录成功
            response["id"] = user.getId();
            response["name"] = user.getName();
            //查询离线消息，打包带走
            vector<string> msg = _offModel.query(id);
            if(!msg.empty()){
                response["offlineMsg"] = msg;
                //删除取走的离线消息 
                _offModel.remove_(id);
            }
            //查询用户的好友信息并返回
            vector<User> userVec = _fModel.query(id);
            if(!userVec.empty()){
                vector<string> vec2;
                for(User& u:userVec){
                    json js;
                    js["id"] = u.getId();
                    js["name"] = u.getName();
                    js["state"] = u.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            conn->send(response.dump());

            

        }
    }else{
        //登录失败
         json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;//为1表示响应失败
        response["errmsg"] = "用户名或者密码错误";
        conn->send(response.dump());
    }
}
//登出业务
void ChatService::loginout(const TcpConnectionPtr &conn,json &js,Timestamp _time){
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> loc(_mloc);

        //释放map中的记录
        auto it=_userConnectionMap.find(userid);
        if(it != _userConnectionMap.end()){
            _userConnectionMap.erase(it);
        }
    }
    //取消再redis中订阅的通道
    _redis.unsubscribe(userid);

    //更新用户状态信息
    User user(userid);
    user.setState("offline");
    _uModel.updateState(user);

}
void ChatService::reg(const TcpConnectionPtr & conn, json &js ,Timestamp _time)
{
    LOG_INFO<<"do reg service";
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);

    bool state = _uModel.insert(user);
    if(state)//注册成功
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;//为0表示响应成功
        response["id"] = user.getId();

        conn->send(response.dump());

    }else //注册失败
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;//为1表示响应失败
        conn->send(response.dump());

    }
}

//一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js,Timestamp _time)
{
    int toid = js["toid"].get<int>();

    {
        lock_guard<mutex> lock(_mloc);
        auto it = _userConnectionMap.find(toid);
        if(it!=_userConnectionMap.end()){
           //在线了，转发消息
            it->second->send(js.dump());
            return;
        }
    }

    //其余主机登录
    User user = _uModel.query(toid);
    if(user.getState() == "online"){
        _redis.publish(toid,js.dump());
    }

    // toid不在线，存储离线消息
    _offModel.insert(toid,js.dump());

} 

// 添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn,json &js,Timestamp _time){
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    //添加好友信息
    _fModel.insert(userid,friendid);

}

//创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn,json &js,Timestamp _time){
     int userid = js["id"].get<int>();
     string name = js["groupname"];
     string desc = js["groupdesc"];

     Group group(-1,name,desc);
     if(_gModel.createGroup(group)){
        // 存储群组创建人信息
        _gModel.addGroup(userid,group.getId(),"creator");
     }
}

//加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn,json &js,Timestamp _time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _gModel.addGroup(userid,groupid,"normal");
}



//群聊业务
void ChatService::groupChat(const TcpConnectionPtr &conn,json &js,Timestamp _time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _gModel.queryGroupUsers(userid,groupid);
    lock_guard<mutex> lock(_mloc);
    
    for(int id:useridVec){
        auto it = _userConnectionMap.find(id);
        if(it != _userConnectionMap.end()){
            it->second->send(js.dump());
        }
         else
        {   
            User user = _uModel.query(id);
            if(user.getState()=="online"){
                _redis.publish(id,js.dump());
            }
            else{
                _offModel.insert(id,js.dump());
            }
        }
    }
}

MsgHandler ChatService::getMsgHandle(int msgid){
    auto it = _msghandlerMap.find(msgid);
    if(it==_msghandlerMap.end()){
        
        //返回一个默认的处理器，什么也不做
        return [msgid](const TcpConnectionPtr & conn, json &js ,Timestamp time){
            LOG_ERROR<<"msgid:"<<msgid<<"cannot find hanler";
        };
    }
    else {
        return _msghandlerMap[msgid];
    }
}

void ChatService::clientCloseException(const TcpConnectionPtr& conn){
    User user;
    {
        lock_guard<mutex> loc(_mloc);
        //更新用户状态信息

        //释放map中的记录
        for(auto it=_userConnectionMap.begin();it!=_userConnectionMap.end();it++){
            if(it->second==conn){
                _userConnectionMap.erase(it);
                user.setId(it->first);
                break;
            }
        }
    }
    //注销
    _redis.unsubscribe(user.getId());

    //更新用户状态
    if(user.getId()!=-1){
        user.setState("offline");
        _uModel.updateState(user);
    }
    
}

// 把所有状态重置成offline
void ChatService::reset(){
    _uModel.resetModel();
}


void ChatService::handleRedisSubscribeMessage(int userid,string msg){
    lock_guard<mutex> lock(_mloc);
    auto it = _userConnectionMap.find(userid);
    if(it!=_userConnectionMap.end()){
        it->second->send(msg);
        return;
    }
    _offModel.insert(userid,msg);
}