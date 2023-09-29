#include "db.hpp"
#include <muduo/base/Logging.h>


using namespace muduo;

static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";
MySQL::MySQL(){
    //开辟链接资源的空间
    _conn = mysql_init(nullptr);
}

// 释放开辟的空间
MySQL::~MySQL(){
    if(_conn != nullptr){
        mysql_close(_conn);
    }
}

bool MySQL::connect(){
    // 链接数据库
    MYSQL *p = mysql_real_connect(_conn,server.c_str(),user.c_str(),
                password.c_str(),dbname.c_str(),3306,nullptr,0);
    if(p!=nullptr){
        LOG_INFO<<"connect success";
        // 支持中文
        mysql_query(_conn,"set names gbk");
    }else{
        LOG_INFO<<"connect failed";
    }
    return p;
}

// 更新操做
bool MySQL::update(string sql){
    if(mysql_query(_conn,sql.c_str())){
        LOG_INFO<<__FILE__<<":"<<__LINE__<<":"
        <<sql<<"update failed";
        return false;
    }
    return true;
}

// 查询操作
MYSQL_RES* MySQL::query(string sql){
    if(mysql_query(_conn, sql.c_str())){
        LOG_INFO<<__FILE__<<":"<<__LINE__<<":"
        <<sql<<"query failed";     
        return nullptr;      
    }
    return mysql_use_result(_conn);
}

MYSQL* MySQL::getConnection(){
    return _conn;
}