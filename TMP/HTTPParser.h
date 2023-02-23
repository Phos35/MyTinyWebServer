#ifndef HTTP_DECODER
#define HTTP_DECODER
#include <string>
#include <unordered_map>
#include "HTTPRequest.h"
#include "Parser.h"
#include "RequestTargetParser.h"

// HTTP报文解码器
class HTTPParser : public Parser
{
public:
    typedef Parser::ParseRet ParseRet;

    // 状态机状态
    enum DFAState
    {
        REQUEST_LINE,
        HEADER,
        CONTENT
    };

    HTTPParser(HTTPRequest& request)
    :state_(HTTPParser::DFAState::REQUEST_LINE), ptr_(0), 
     request_(request)
    {}

    /// @brief 解析HTTP请求报文
    /// @param data 带解析的数据
    /// @return 解析结果
    ParseRet parse(const std::string& data) override;

    /// @brief 解析REQUEST-LINE字段
    /// @param data 待解析的数据
    /// @return 解析结果
    ParseRet parseRequestLine(const std::string& data);

    /// @brief 解析HEADER字段
    /// @param data 待解析的数据
    /// @return 解析结果
    ParseRet parseHeaders(const std::string& data);

    /// @brief 解析CONTENT字段
    /// @param data 待解析的数据
    /// @return 解析结果
    ParseRet parseContent(const std::string& data);

    /// @brief DFA状态转化为对应的字符串
    /// @param state DFA状态
    /// @return 状态对应的字符串
    static std::string statetoStr(DFAState state);

private:
    HTTPRequest& request_;               // HTTP请求

    DFAState state_;                     // 状态机状态
    uint32_t ptr_;                       // 指向下一个要读取的字符的指针
};

#endif