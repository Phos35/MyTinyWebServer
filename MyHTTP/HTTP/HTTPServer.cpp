#include "HTTPServer.h"
#include "AsyncLog.h"
#include "DBConnPool.h"
#include <assert.h>

HTTPServer::HTTPServer(EventLoop* loop, const Socket& host, Trigger trigger,
                        int connNr, int workerNr, int wokerQueSize)
: TCPServer(loop, host, connNr), workerPool_(workerNr, wokerQueSize), trigger_(trigger)
{

}

/**
 * @brief: 启动HTTP服务器
 * @return {*}
 */    
void HTTPServer::startServer()
{
    // 设置工作线程池的工作函数
    workerPool_.setWorker(std::bind(&HTTPServer::worker, this, std::placeholders::_1));

    // 检查请求处理回调函数是否设置
    if(!onGET_) LOG_WARN << "HTTPServer: onGET CallBack Not Set";
    if(!onPOST_) LOG_WARN << "HTTPServer: onPOST CallBack Not Set";

    // 设置TCPServer的各个回调函数
    TCPServer::setNewConnCallBack(std::bind(&HTTPServer::handleNewConn, this, std::placeholders::_1));
    TCPServer::setCloseConnCallBack(std::bind(&HTTPServer::handleClose, this, std::placeholders::_1));

    // 启动TCPServer
    TCPServer::start();
}

/**
 * @brief: 新连接到达
 * @return {*}
 * @param {SPtr&} conn 新的TCP连接
 */    
void HTTPServer::handleNewConn(const TCPConnection::SPtr& conn)
{
    // 创建HTTPConnection并加入到表中
    HTTPConnection::SPtr newConn(new HTTPConnection(conn, &workerPool_));
    conns_.insert({conn->id(), newConn});

    // 设置触发模式
    conn->setTrigger(trigger_);
}

/**
 * @brief: 连接关闭的回调函数
 * @return {*}
 * @param {SPtr&} conn 待关闭的连接
 */    
void HTTPServer::handleClose(const TCPConnection::SPtr& conn)
{
    // 移除对应的HTTPConnection
    int nr = conns_.erase(conn->id());
    assert(nr == 1);
}

/**
 * @brief: 解析错误处理函数
 * @return {*}
 * @param {HTTPConnection*} conn 解析出错的HTTP连接
 */    
void HTTPServer::handleError(const HTTPConnection::SPtr& conn)
{
    // 输出相关信息
    HTTPRequest &request = conn->requestRef();
    LOG_TRACE << "HTTPConnection " << conn->id() << " Error";

    // 输出请求解析的相关信息
    request.print();
}

/**
 * @brief: 解析HTTP请求报文的工作函数
 * @return {*}
 * @param {SPtr&} conn HTTP连接
 */    
void HTTPServer::worker(HTTPConnection::SPtr conn)
{
    HTTPRequest &request = conn->requestRef();

    // 解析读取的数据并根据解析结果调用回调函数
    HTTPRequest::ParseRet parseRet = HTTPRequest::ParseRet::UNKNOWN;
    while (parseRet != HTTPRequest::ParseRet::NO_DATA && 
        parseRet != HTTPRequest::ParseRet::UNCOMPLETE &&
        parseRet != HTTPRequest::ParseRet::BAD)
    {
        parseRet = request.parse();
        LOG_TRACE << "PaeseRet : " << HTTPRequest::parseRetStr(parseRet);
        switch (parseRet)
        {
            case HTTPRequest::ParseRet::GET : onGET_(conn); break;
            case HTTPRequest::ParseRet::POST : onPOST_(conn); break;
            case HTTPRequest::ParseRet::BAD :
            case HTTPRequest::ParseRet::UNKNOWN: handleError(conn); break;
        }
    }
    
    // 若报文完整解析完成且未要求keep-alive, 则关闭连接
    if(parseRet == HTTPRequest::ParseRet::NO_DATA && conn->keepAlive() == false)
    {
        // printf("NOT_KEEP_ALIVE\n");
        conn->shutdown();
    }

    // 解锁buffer
    conn->setParsing(false);
}

/**
 * @brief: 创建一个HTTPServer并运行（函数运行loop，阻塞）
 * @return {*}
 * @param {Config} &config
 */    
void HTTPServer::createAndStart(Config config)
{
    // 日志输出到文件
    std::string processName = config.processName();
    AsyncLog log(processName);
    if(config.logType() == Config::LogType::FILE)
    {
        log.turnOn();
        log.start();
    }

    // 设置数据库连接池
    DBConnPool* dbPool = DBConnPool::getInstance();
    dbPool->init("localhost", "root", "WebServer", "ServerDB", config.dbConnNr());

    // 创建Server
    EventLoop loop;
    Socket host = Socket::create(AF_INET, SOCK_STREAM, 0, "0.0.0.0", config.port());
    host.setResueAddr(true);
    HTTPServer server(&loop, host, config.trigger(), config.eventLoopNr(), config.workerNr());

    // 设置回调函数
    server.setOnGET(config.onGET());
    server.setOnPOST(config.onPOST());

    // 启动Server
    server.startServer();

    // 启动事件循环
    loop.run();
}