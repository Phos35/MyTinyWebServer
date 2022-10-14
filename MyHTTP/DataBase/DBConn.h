#ifndef DB_CONN_H
#define DB_CONN_H
#include <mysql/mysql.h>

// 前向声明
class DBConnPool;

// 数据库连接的RAII封装
class DBConn{
public:
    DBConn(MYSQL *conn = nullptr, DBConnPool *pool = nullptr);

    ~DBConn();

    /**
     * @brief: 获取管理该连接的连接池
     * @return {*}
     */    
    DBConnPool *getPool();

    /**
     * @brief: 设定管理该连接的连接池
     * @return {*}
     * @param {DBConnPool} *pool
     */    
    void setPool(DBConnPool *pool);

    /**
     * @brief: 获取mysql数据库连接
     * @return {*}
     */
    MYSQL *getRaw();

    /**
     * @brief: 设定数据连接
     * @return {*}
     * @param {MYSQL} *raw
     */    
    void setRaw(MYSQL *raw);

private:
    MYSQL *conn;            // mysql 连接
    DBConnPool *ownerPool;  // 连接所属的连接池
};

#endif