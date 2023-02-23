#ifndef REQUEST_LINE_PARSER
#define REQUEST_LINE_PARSER
#include "Parser.h"
#include "HTTPRequest.h"

class RequestLineParser : public Parser
{
public:
    typedef Parser::ParseRet ParseRet;

    // 自动机状态
    enum class DFAState
    {
        UNKNOWN,
        METHOD,
        TARGET,
        VERSION
    };

    RequestLineParser(HTTPRequest& request, uint32_t& ptr)
    :state_(DFAState::METHOD), request_(request), ptr_(ptr)
    {}

    /// @brief 解析请求行
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
    uint32_t& ptr_;             // 指向下一个要处理的字符的指针

    
    /// @brief 解析METHOD字段
    /// @param data 待解析的数据
    /// @return 解析结果
    ParseRet parseMethod(const std::string& data);

    /// @brief 解析Request-Target字段
    /// @param data 待解析的数据
    /// @return 解析结果
    ParseRet parseTarget(const std::string& data);

    /// @brief 解析VERSION字段
    /// @param data 待解析的数据
    /// @return 解析结果
    ParseRet parseVersion(const std::string& data);
};

#endif