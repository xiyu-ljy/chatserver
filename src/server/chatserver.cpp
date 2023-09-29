#include "chatserver.hpp"
#include "chatservice.hpp"
#include "json.hpp"

#include <functional>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;
// 初始化muduo库中的Server
ChatServer::ChatServer(EventLoop* loop,
        const InetAddress& listenAddr,
        const string& nameArg):_server(loop,listenAddr,nameArg),_loop(loop)
{
    //回调函数注册
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));

    // 设置线程数量
    _server.setThreadNum(4);

}

 // 启动服务
void ChatServer::start()
{
    _server.start();
}


// 上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    // 客户端断开连接，释放资源
    if(!conn->connected()){
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }

}
// 上报读写事件相关的回调函数
void ChatServer::onMessage(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    // 数据解码
    json js = json::parse(buf);
    //业务模块代码和网络模块代码松耦合
    //通过js的msgid获取业务id--》调用对应的处理函数
    MsgHandler handler = ChatService::instance()->getMsgHandle(js["msgid"].get<int>());
    handler(conn,js,time);
}

