#include "HTTPResponse.h"
#include "../Utils/StringList.h"
#include "Logger.h"
#include <fstream>
#include <sstream>

/**
 * @brief: 设置HTTP响应的响应行
 * @return {*}
 * @param {string} version 协议版本
 * @param {int} code 状态码
 * @param {string} description 状态码描述
 */    
void HTTPResponse::setResponseLine(std::string version, int code, std::string description)
{
    this->ver = version;
    this->code = code;
    this->description = description;
}

/**
 * @brief: 添加 Header 字段
 * @return {*}
 * @param {string} key 字段名
 * @param {string} val 字段值
 */    
void HTTPResponse::addHeader(std::string key, std::string val)
{
    header[key] = val;
}

/**
 * @brief: 设置响应体
 * @return {*}
 * @param {string} &content
 */    
void HTTPResponse::setContent(std::string &content)
{
    this->content = content;
}

/**
 * @brief: 生成响应报文的数据
 * @return {*}
 */    
std::string HTTPResponse::data()
{
    std::string ret;

    // 添加响应行
    addResponseLine(ret);

    // 添加头部字段
    addHeaders(ret);

    // 添加空行
    ret += "\r\n";

    // 添加content
    addContent(ret);

    return ret;
}

/**
 * @brief: 添加响应行
 * @return {*}
 * @param {string} &response
 */    
void HTTPResponse::addResponseLine(std::string &response)
{
    response += ver;
    response += " ";
    response += std::to_string(code);
    response += " ";
    response += description;
    response += "\r\n";
}

/**
 * @brief: 添加头部字段
 * @return {*}
 * @param {string} &response
 */    
void HTTPResponse::addHeaders(std::string &response)
{
    for (auto itr = header.begin(); itr != header.end(); itr++)
    {
        response += itr->first;
        response += ": ";
        response += itr->second;
        response += "\r\n";
    }
}

/**
 * @brief: 添加content
 * @return {*}
 * @param {string} &response
 */    
void HTTPResponse::addContent(std::string &response)
{
    response += content;
}