#include "DBConn.h"
#include "DBConnPool.h"

DBConn::DBConn(MYSQL *conn, DBConnPool *pool)
:conn(conn), ownerPool(pool)
{

}

DBConn::~DBConn()
{
    ownerPool->releaseConn(conn);
}

/**
 * @brief: 获取管理该连接的连接池
 * @return {*}
 */    
DBConnPool* DBConn::getPool()
{
    return ownerPool;
}

/**
 * @brief: 设定管理该连接的连接池
 * @return {*}
 * @param {DBConnPool} *pool
 */    
void DBConn::setPool(DBConnPool *pool)
{
    ownerPool = pool;
}

/**
 * @brief: 获取mysql数据库连接
 * @return {*}
 */
MYSQL* DBConn::getRaw()
{
    return conn;
}

/**
 * @brief: 设定数据连接
 * @return {*}
 * @param {MYSQL} *raw
 */    
void DBConn::setRaw(MYSQL *raw)
{
    conn = raw;
}