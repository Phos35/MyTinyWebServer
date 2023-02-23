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

// 解析URL请求资源的路径
std::string parsePath(const std::string& url)
{
    // 请求index.html
    if(url == "/")
        return resourceRoot + "/html/index.html";
    
    // 请求其他类型的资源
    return resourceRoot + url;
}

// 获取指定的资源
std::string getResource(const std::string& path)
{
    printf("Path: %s\n", path.c_str());
    std::string content;
    std::ifstream target(path);
    if(target.is_open() == false)
    {
        printf("File %s open failed! %s\n", path.c_str(), strerror(errno));
    }
    else
    {
        // 获取文件内容
        std::istreambuf_iterator<char> begin(target);
        std::istreambuf_iterator<char> end;
        content = std::string(begin, end);
    }

    // 关闭文件并返回指定内容
    return content;
}

// 响应未找到对应资源的错误
void handleNotFound(const HTTPConnection::SPtr & conn)
{
    HTTPResponse response;
    response.setResponseLine("HTTP/1.1", 404, "NOT FOUND");
    response.addHeader("Content-Type", "text/html");
    response.addHeader("Content-Length", "0");
    conn->respond(response.data());
}

std::string mimeType(std::string& url)
{
    StringList urlList(url, ".");
    std::string resourceType;
    std::string mime;
    if (urlList.size() == 0)
        mime = "text/plain";
    else
    {
        resourceType = urlList[urlList.size() - 1];
        if(resourceType == "js")
            mime = "application/javascript";
        else if(resourceType == "ico")
            mime = "image/*";
        else
            mime = "text/" + resourceType;
    }

    printf("Resource %s, type: %s\n", url.c_str(), mime.c_str());
    return mime;
}

// GET请求处理 
// TOD Request解析时添加请求资源类型的参数，同时自动将/转化为index.html的路径
// TODO response构造时传入ResponseLine和Content参数，基础的Header由Response自动构造
void handleGET(const HTTPConnection::SPtr & conn)
{
    printf("Handle GET\n");
    // 获取请求资源的路径
    std::string resourcePath = parsePath(conn->requestRef().getUrl());
    printf("Resource Path: %s\n", resourcePath.c_str());
    
    // 获取资源
    std::string content = getResource(resourcePath);

    HTTPResponse response;
    // 构造响应报文返回
    response.setResponseLine("HTTP/1.1", 200, "OK");
    response.setContent(content);
    response.addHeader("Content-Type", mimeType(resourcePath));
    response.addHeader("Content-Length", std::to_string(content.size()));
    // printf("Response: %s\n", response.data().c_str());
    conn->respond(response.data());
}

// 解析post请求中的参数
std::unordered_map<std::string, std::string> parseArgs(std::string& args)
{
    std::unordered_map<std::string, std::string> argMap;
    StringList argList(args, "&");
    for(int i = 0; i < argList.size(); i++)
    {
        StringList kvList(argList[i], "=");
        printf("(key, value) : (%s, %s)\n", kvList[0].c_str(), kvList[1].c_str());
        argMap[kvList[0]] = kvList[1];
    }
    return argMap;
}

// 将十六进制的ASCII码字符串转换成对应的字符
char hexASCIItoChar(const std::string& hexStr)
{
    char ret = 0;
    for (int i = 0; i < hexStr.size(); i++)
    {
        if(isdigit(hexStr[i]))
            ret = ret * 16 + hexStr[i] - '0';
        else
            ret = ret * 16 + hexStr[i] - 'A' + 10;
    }
    return ret;
}

// 将HTTP报文中的特殊字符转化为正常字符
std::string cancelHTTPFormat(std::string& ori)
{
    std::string ret;
    for (int i = 0; i < ori.size(); i++)
    {
        switch(ori[i])
        {
        case '+' : ret.push_back(' '); break;
        case '%' : 
            {
                ret.push_back(hexASCIItoChar(ori.substr(i + 1, 2)));
                i += 2;
            };
            break;
        default: ret.push_back(ori[i]); break;
        }
    }

    printf("Before: %s, After: %s\n", ori.c_str(), ret.c_str());
    return ret;
}

// POST请求处理
// TODO 参数拆分整合进decoder中
// TODO request解析过程中需要将特殊字符转化为正常字符，如+转化为空格等等
void handlePOST(const HTTPConnection::SPtr & conn)
{
    printf("POST\n");
    // 拆分获取参数
    std::unordered_map<std::string, std::string> argMap = parseArgs(conn->requestRef().getContent());

    // 参数错误
    auto typeItr = argMap.find("type");
    if (typeItr == argMap.end())
    {
        HTTPResponse response;
        response.setResponseLine("HTTP/1.1", 403, "Forbidden");
        response.addHeader("Content-Length", "0");
        conn->respond(response.data());
        return;
    }

    // 解析指令
    if(typeItr->second == "cmd")
    {
        // 获取指令具体内容 TODO 检验格式是否合法
        StringList cmdList(cancelHTTPFormat(argMap["content"]), " ");
        if(cmdList[0] == "markdown")
        {
            argMap["type"] = "md";
            argMap["url"] = "/markdown/" + cmdList[1];
        }
        else if(cmdList[0] == "image")
        {
            argMap["type"] = "image";
            argMap["url"] = "/image/" + cmdList[1];
        }
    }

    std::string content;
    if (typeItr->second == "md" || typeItr->second == "image")
    {
        std::string resourcePath = parsePath(argMap["url"]);

        // 获取文件内容
        content = getResource(resourcePath);
    }
    // 获取inttrioduction内容
    else if(typeItr->second == "introduction")
    {
        // content.push_back('\"');
        content += getResource(resourceRoot + "/txt/introduction.txt");
        // content.push_back('\"');
    }

    // 若文件内容为空，则未找到对应的文件，返回404
    if(content.empty() == true)
    {
        handleNotFound(conn);
        return;
    }
    // 否则正常响应
    else
    {
        // printf("Response: %s\n", content.c_str());
        HTTPResponse response;
        response.setResponseLine("HTTP/1.1", 200, "OK");
        response.addHeader("Content-Type", "/text/plain");
        response.addHeader("Content-Length", std::to_string(content.size()));
        response.setContent(content);
        conn->respond(response.data());
    }
}

int main(int argc, char* argv[])
{
    // 设置参数
    Config config;
    config.loadConfig(argc, argv);
    config.setOnGET(handleGET);
    config.setOnPOST(handlePOST);
    
    // 启动并运行HTTPServer
    HTTPServer::createAndStart(config);

    return 0;
}