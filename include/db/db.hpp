#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>
using namespace std;

class MySQL{
public:
    MySQL();
    
    // 释放开辟的空间
    ~MySQL();

    bool connect();

    // 更新操做
    bool update(string sql);
    // 查询操作
    MYSQL_RES* query(string sql);
    //获取链接
    MYSQL * getConnection();

private:
    MYSQL *_conn;
};

#endif