#ifndef HEADERS_PARSER_H
#define HEADERS_PARSER_H
#include "Parser.h"
#include "HTTPRequest.h"

class HeadersParser : public Parser
{
public:
    typedef Parser::ParseRet ParseRet;

    // 自动机状态
    enum class DFAState
    {
        KEY,
        VAL
    };

    HeadersParser(HTTPRequest& request, uint32_t& ptr)
    :state_(DFAState::KEY), request_(request), ptr_(ptr)
    {}

    /// @brief 解析HEADER字段
    /// @param data 待解析的数据
    /// @return 解析结果
    ParseRet parse(const std::string &data) override;

    /// @brief 获取状态对应的字符串
    /// @param s 状态
    /// @return 状态对应字符串
    static std::string statetoStr(DFAState s);

private:
    HTTPRequest &request_;      // HTTP请求

    DFAState state_;            // 解析状态
    uint32_t &ptr_;              // 指向下一个要读取的字符的指针


    /// @brief 解析字段名
    /// @param data 待解析的数据
    /// @param key 存储字段名
    /// @return  解析结果
    ParseRet parseKey(const std::string &data, std::string &key);

    /// @brief 解析字段值
    /// @param data 待解析的数据
    /// @param val 存储字段值
    /// @return 解析结果
    ParseRet parseVal(const std::string &data, std::string &val);
};

#endif