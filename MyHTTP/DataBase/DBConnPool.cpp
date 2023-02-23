#include "DBConnPool.h"

DBConnPool* DBConnPool::getInstance()
{
    static DBConnPool pool;
    return &pool;
}

void DBConnPool::init(std::string host, std::string user, 
                      std::string pwd, std::string dbName,
                      int size)
{
    // 初始化数据
    this->host = host;
    this->user = user;
    this->pwd = pwd;
    this->dbName = dbName;
    connQue.reserve(size);

    // 创建size个连接
    int actulSize = 0;
    for (int i = 0; i < size; i++)
    {
        // 创建连接
        MYSQL *conn = nullptr;
        conn = mysql_init(conn);
        if(conn == nullptr)
        {
            LOG_WARN << "MYSQL Connection Init Failed: " << mysql_error(conn);
            continue;
        }

        // 连接到数据库
        conn = mysql_real_connect(conn, host.c_str(), user.c_str(), pwd.c_str(), dbName.c_str(), 0, nullptr, 0);
        if(conn == nullptr)
        {
            LOG_WARN << "MYSQL Connection Connect Failed: " << mysql_error(conn);
            continue;
        }

        // 添加到连接队列中
        actulSize++;
        connQue.push(conn);
    }
    LOG_INFO << "DBConnPool Actual Size: " << actulSize;
    inited = true;
}

void DBConnPool::getConn(DBConn& conn)
{
    if(inited == false)
    {
        LOG_ERROR << "GetConn Before DBConnPool Inited";
        exit(-1);
    }
    MYSQL *rawConn = connQue.pop();
    conn.setPool(this);
    conn.setRaw(rawConn);
}

void DBConnPool::releaseConn(MYSQL* conn)
{
    connQue.push(conn);
}

DBConnPool::DBConnPool()
{
}

DBConnPool::~DBConnPool()
{
    // 释放连接所用的资源
    while(connQue.empty() == false)
    {
        MYSQL *conn = connQue.pop();
        mysql_close(conn);
    }
}
