#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
#include "Buffer.h"
#include "EventLoop.h"
#include "Channel.h"
#include <unordered_map>

class HTTPRequest
{
public:
    // 请求类型
    enum class RequestType{
        GET,
        POST
    };

    // 主状态机的状态
    enum class RequesState{
        REQUESTLINE,    // 解析请求行中
        HEADER,         // 解析请求头中
        CONTENT         // 解析请求体
    };

    // 从状态机的状态
    enum class LineState{
        LINE_OPEN,      // 行解析中
        LINE_BAD,       // 行解析错误
        LINE_OK         // 行解析成功
    };

    // 请求解析结果
    enum class ParseRet{
        PART_SUCCESS,   // 部分解析完成
        UNCOMPLETE,     // 请求不完整
        GET,            // 完整GET请求
        POST,           // 完整POST请求
        BAD,            // 请求格式错误
        UNKNOWN,        // 未知错误
        NO_DATA,        // 缓冲区无数据
    };

    typedef std::string URL;

    HTTPRequest(Buffer* inputBuff = nullptr);

    /**
     * @brief: 设置输入缓冲区
     * @return {*}
     * @param {Buffer} *inputBuff 输入缓冲区指针
     */    
    void setBuffer(Buffer *inputBuff);

    /**
     * @brief: 解析请求
     * @return {*ParseRet} 解析的结果状态
     */    
    ParseRet parse();

    /**
     * @brief: 初始化请求状态
     * @return {*}
     */    
    void resetState();

    /**
     * @brief: 获取资源Url
     * @return {*}
     */    
    std::string& getUrl();

    /**
     * @brief: 获取请求体
     * @return {*}
     */    
    std::string &getContent();

    /**
     * @brief: 返回request是否正在解析
     * @return {*}
     */    
    bool parsing() { return buff->handling(); }

    /**
     * @brief: 设置request正在处理的标志
     * @return {*}
     * @param {bool} val
     */    
    void setParsing(bool val) { buff->setHandling(val); }

    /**
     * @brief: 返回HTTPConnection是否keepalive
     * @return {*}
     */    
    bool keepAlive() { return header.find("Connextion") != header.end() && header["Connection"] == "keep-alive"; }

    /**
     * @brief: 打印HTTPRequest的内容
     * @return {*}
     */    
    void print();

    /**
     * @brief: 获取解析结果对应的字符串
     * @return {*}
     * @param {ParseRet} ret
     */    
    static std::string parseRetStr(ParseRet ret);

    /**
     * @brief: 获取请求解析状态对应的字符串
     * @return {*}
     * @param {RequesState} s
     */    
    static std::string stateStr(RequesState s);

private:
    typedef std::unordered_map<std::string, std::string> Header;

    Buffer* buff;                   // 保存请求报文的Buffer

    RequestType type;               // 请求类型
    std::string ver;                // 协议版本
    URL url;                        // 请求url
    Header header;                  // 请求头
    std::string content;            // 请求体

    int lineStart;                  // 正在解析的行的起始位置
    int curIndex;                   // 正在解析的字符的位置
    RequesState reqState;           // 请求解析的状态（主状态机状态）
    LineState lineState;            // 行解析状态（从状态机状态）

    /**
     * @brief: 解析出一行数据
     * @return {*}
     */    
    LineState parseLine();

    /**
     * @brief: 解析请求行
     * @return {*}
     */    
    ParseRet parseRequestLine(std::string& line);

    /**
     * @brief: 解析请求头
     * @return {*}
     */    
    ParseRet parseHeader(std::string& line);

    /**
     * @brief: 解析请求体
     * @return {*}
     */    
    ParseRet parseContent(std::string& line);
};

#endif