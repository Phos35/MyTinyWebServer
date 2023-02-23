#include "HTTPServer.h"
#include "ThreadPool.hpp"
#include "HTTPResponse.h"
#include "AsyncLog.h"
#include "StringList.h"
#include "DBConnPool.h"
#include "Config.h"
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <sys/stat.h>
#include <signal.h>

class IgnoreSigPipe{
public:
    IgnoreSigPipe(){
        ::signal(SIGPIPE, SIG_IGN);
    }
};

IgnoreSigPipe initObj;

// 资源根路径
std::string resourceRoot = "/home/ubuntu/MyTinyWebServer/Resource";

// 解析Url，获取资源路径
std::string parseUrl(std::string& url, std::string& path)
{
    // 非法URL，返回错误页面
    if(url[0] != '/')
        return "text/html";

    // 若为/，则返回首页
    if(url == "/")
    {
        path = resourceRoot + "/html/home.html";
        return "text/html";
    }

    // 构建资源路径
    path = resourceRoot + url;

    // 返回资源类型
    int dotIndex = url.find_last_of('.');
    std::string type = url.substr(dotIndex + 1);

    // favicon.ico
    if(type == "ico")
        return "image/*";

    // html类型
    if(type == "html")
        return "text/html";
    
    if(type == "css")
        return "text/css";
    
    if(type == "jpg")
        return "image/jpg";
    
    if(type == "png")
        return "image/png";

    if(type == "mp4")
        return "video/mp4";

    if(type == "gif")
        return "image/gif";

    return "NULL";
}

// 根据资源路径获取资源
std::string getResource(std::string& path)
{
    // 获取文件大小
    struct stat fileState;
    if(stat(path.c_str(), &fileState) == -1)
    {
        perror("Stat File");
    }
    int size = fileState.st_size;

    // 打开文件
    FILE* file = fopen(path.c_str(), "rb");
    if (file == nullptr)
    {
        perror(path.c_str());
        return std::string();
    }

    // 读取文件
    std::vector<char> ret(size, 0);
    int readed = fread(ret.data(), 1, size, file);
    fclose(file);

    return std::string(ret.begin(), ret.end());
}

void respondError(const HTTPConnection::SPtr& conn)
{
    std::string errorPath = "/html/internalError.html";
    // 解析URL，获取资源路径
    std::string path;
    std::string type = parseUrl(errorPath, path);

    // 获取对应的资源
    std::string content = getResource(path);

    HTTPResponse response;
    // 设置头部
    response.setResponseLine("HTTP/1.1", 500, "Internal Error");

    // 添加头部字段
    response.addHeader("Content-Type", type);
    response.addHeader("Content-Length", std::to_string(content.size()));

    // 设置content
    response.setContent(content);

    // 发送数据
    std::string responseStr = response.data();
    conn->respond(responseStr);
    conn->shutdown();
}

void respondGET(const HTTPConnection::SPtr& conn)
{
    HTTPRequest & request = conn->requestRef();
    
    // 拆分URL和请求参数
    StringList fullURL(request.getUrl(), "?");

    // 解析URL，获取资源路径
    LOG_INFO << "URL: " << fullURL[0];
    std::string path;
    std::string type = parseUrl(fullURL[0], path);

    // 获取对应的资源
    std::string content = getResource(path);
    if(content.empty() == true)
    {
        path = (resourceRoot + "/html/internalError.html");
        type = "text/html";
        content = getResource(path);
    }

    HTTPResponse response;
    // 设置头部
    response.setResponseLine("HTTP/1.1", 200, "OK");

    // 添加头部字段
    response.addHeader("Content-Type", type);
    response.addHeader("Content-Length", std::to_string(content.size()));

    // 设置content
    response.setContent(content);

    // 发送数据
    std::string responseStr = response.data();
    conn->respond(responseStr);
}

int checkUser(std::string& user, std::string& pwd)
{
    DBConnPool *dbPool = DBConnPool::getInstance();
    DBConn conn;
    dbPool->getConn(conn);
    MYSQL *sqlConn = conn.getRaw();

    // 查询
    std::string command = "select passwd from user where username=\'" + user + "\';";
    int queryRet = mysql_query(sqlConn, command.c_str());
    MYSQL_RES *result = mysql_store_result(sqlConn);
    uint64_t num = mysql_num_rows(result);

    if(num != 1)
    {
        return 0;
    }
    else
    {
        MYSQL_ROW row;
        row = mysql_fetch_row(result);
        if (row[0] == pwd)
        {
            return 1;
        }
        return 2;
    }
}

void login(const HTTPConnection::SPtr& conn)
{
    HTTPRequest & request = conn->requestRef();

    // 获取Content并拆分获取参数
    std::string reqContent = request.getContent();
    StringList argLine(reqContent, "&");
    std::vector<StringList> args(argLine.size());
    for (int i = 0; i < argLine.size(); i++)
    {
        args[i].split(argLine[i], "=");
    }

    // 根据参数检验用户登录是否成功
    int login = checkUser(args[0][1], args[1][1]);

    // 成功则跳转到图片页面
    std::string path;
    std::string type;
    if (login == 1)
    {
        type = "text/html";
        path = resourceRoot + "/html/video.html";
    }
    else if(login == 0)
    {
        type = "text/html";
        path = resourceRoot + "/html/noUser.html";
    }
    else if(login == 2)
    {
        type = "text/html";
        path = resourceRoot + "/html/wrongPwd.html";
    }

    // 获取对应的资源
    std::string content = getResource(path);
    if(content.empty() == true)
    {
        path = (resourceRoot + "/html/internalError.html");
        content = getResource(path);
    }

    HTTPResponse response;
    // 设置头部
    response.setResponseLine("HTTP/1.1", 200, "OK");

    // 添加头部字段
    response.addHeader("Content-Type", type);
    response.addHeader("Content-Length", std::to_string(content.size()));

    // 设置content
    response.setContent(content);

    // 发送数据
    std::string responseStr = response.data();
    conn->respond(responseStr);
}

int storeUser(std::string& user, std::string& pwd)
{
    DBConnPool *dbPool = DBConnPool::getInstance();
    DBConn conn;
    dbPool->getConn(conn);
    MYSQL *sqlConn = conn.getRaw();

    // 添加用户
    std::string command = "insert into user values(\'" + user + "\',\'" + pwd + "\');";
    return mysql_query(sqlConn, command.c_str());
}

void logon(const HTTPConnection::SPtr& conn)
{
    HTTPRequest & request = conn->requestRef();

    // 获取Content并拆分获取参数
    std::string reqContent = request.getContent();
    StringList argLine(reqContent, "&");
    std::vector<StringList> args(argLine.size());
    for (int i = 0; i < argLine.size(); i++)
    {
        args[i].split(argLine[i], "=");
    }

    // 根据参数注册用户
    int logon = storeUser(args[0][1], args[1][1]);

    std::string path;
    std::string type;
    // 注册成功则跳转到登录页面
    if(logon == 0)
    {
        type = "text/html";
        path = resourceRoot + "/html/login.html";
    }
    else
    {
        type = "text/html";
        path = resourceRoot + "/html/logonFailed.html";
    }

    // 获取对应的资源
    std::string content = getResource(path);
    if(content.empty() == true)
    {
        path = (resourceRoot + "/html/internalError.html");
        content = getResource(path);
    }

    HTTPResponse response;
    // 设置头部
    response.setResponseLine("HTTP/1.1", 200, "OK");

    // 添加头部字段
    response.addHeader("Content-Type", type);
    response.addHeader("Content-Length", std::to_string(content.size()));

    // 设置content
    response.setContent(content);

    // 发送数据
    std::string responseStr = response.data();
    conn->respond(responseStr);
}

void respondPOST(const HTTPConnection::SPtr& conn)
{
    HTTPRequest & request = conn->requestRef();

    // 根据URL判断为注册或者登录
    std::string url = request.getUrl();
    std::string type = url.substr(url.find_last_of('/') + 1);
    if (type == "login")
    {
        login(conn);
    }
    else if (type == "logon")
    {
        logon(conn);
    }
    else
    {
        respondError(conn);
    }
}

int main(int argc, char* argv[])
{
    // 设置参数
    Config config;
    config.loadConfig(argc, argv);
    config.setOnGET(respondGET);
    config.setOnPOST(respondPOST);
    
    // 启动并运行HTTPServer
    HTTPServer::createAndStart(config);

    return 0;
}