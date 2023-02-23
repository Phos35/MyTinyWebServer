#ifndef REQUEST_TARGET_DECODER_H
#define REQUEST_TARGET_DECODER_H
#include <string>
#include "Parser.h"
#include "HTTPRequest.h"
/*
目前只实现origin-form的解析，其余类型后续更新 TODO
*/

class RequestTargetParser : public Parser
{
public:
    typedef HTTPRequest::QueryTable QueryTable;
    typedef HTTPRequest::RequestTarget Target;
    typedef Parser::ParseRet ParseRet;

    RequestTargetParser(uint32_t& ptr)
        :decodeState_(ParseRet::PART_FINISHED), ptr_(ptr)
    {}

    /**
     * @brief: 解析HTTP请求行中的Request-Target
     * @return {*}
     * @param {string&} data 待解析的HTTP报文
     */        
    ParseRet parse(const std::string& data) override;

    /*get 系列函数*/
    Target &target() { return target_; }

private:
    ParseRet decodeState_;     // 当前的解析状态
    uint32_t &ptr_;             // 指向下一个待解析字符的指针
    Target target_;             // 解析出的目标资源

    /**
     * @brief: 将十六进制字符串转换成对应的字符
     * @return {*} 若成功解析，则返回对应的字符;
     * @param {string&} data 待解析的HTTP报文
     */
    char hexStrToChar(const std::string& data);

    /**
     * @brief: 解析Reqeust-Target中的query参数
     * @return {*}
     * @param {string} &data 待解析的报文
     */    
    void decodeQuery(const std::string &data);
};

#endif