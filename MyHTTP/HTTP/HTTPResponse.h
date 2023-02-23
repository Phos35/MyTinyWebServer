#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include <string>
#include <unordered_map>

class HTTPResponse
{
public:
    /**
     * @brief: 设置HTTP响应的响应行
     * @return {*}
     * @param {string} version 协议版本
     * @param {int} code 状态码
     * @param {string} description 状态码描述
     */    
    void setResponseLine(std::string version, int code, std::string description);

    /**
     * @brief: 添加 Header 字段
     * @return {*}
     * @param {string} key 字段名
     * @param {string} val 字段值
     */    
    void addHeader(std::string key, std::string val);

    /**
     * @brief: 设置响应体
     * @return {*}
     * @param {string} &content
     */    
    void setContent(std::string &content);

    /**
     * @brief: 生成响应报文的数据
     * @return {*}
     */    
    std::string data();

private:
    typedef std::unordered_map<std::string, std::string> Header;

    std::string ver;        // 协议版本
    int code;               // 响应码
    std::string description;// 响应码描述

    Header header;          // 响应头

    std::string content;    // 响应体

    /**
     * @brief: 添加响应行
     * @return {*}
     * @param {string} &response
     */    
    void addResponseLine(std::string &response);

    /**
     * @brief: 添加头部字段
     * @return {*}
     * @param {string} &response
     */    
    void addHeaders(std::string &response);

    /**
     * @brief: 添加content
     * @return {*}
     * @param {string} &response
     */    
    void addContent(std::string &response);
};

#endif