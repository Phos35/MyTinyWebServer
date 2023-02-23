#ifndef DB_CONN_POOL_H
#define DB_CONN_POOL_H
#include "BlockQue.hpp"
#include "DBConn.h"
#include "AsyncLog.h"
#include <string>
#include <mysql/mysql.h>

// 数据库连接池，采用单例模式
class DBConnPool{
public:
    /**
     * @brief: 获取数据库连接池实例
     * @return {*}
     * @param {DBConnPool*} 数据库连接池指针
     */
    static DBConnPool* getInstance();

    /**
     * @brief: 初始化数据库连接池
     * @return {*}
     * @param {string} host 数据库运行主机名
     * @param {string} user 数据库用户名
     * @param {string} pwd 数据库密码
     * @param {string} dbName 数据库名称
     * @param {int} size 连接池大小
     */    
    void init(std::string host, std::string user, 
              std::string pwd, std::string dbName,
              int size);

    /**
     * @brief: 获取一个数据库连接
     * @return {*}
     */    
    void getConn(DBConn& conn);

    /**
     * @brief: 释放一个数据库连接，即放入连接队列中
     * @return {*}
     * @param {MYSQL*} conn
     */        
    void releaseConn(MYSQL* conn);

private:
    BlockQueue<MYSQL *> connQue;    // 数据库连接队列

    bool inited;                    // 初始化标志

    std::string host;
    std::string user;
    std::string pwd;
    std::string dbName;

    DBConnPool();
    ~DBConnPool();
};
#endif