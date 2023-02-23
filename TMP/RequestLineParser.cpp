#include "RequestLineParser.h"
#include "RequestTargetParser.h"

RequestLineParser::ParseRet RequestLineParser::parse(const std::string &data)
{
    ParseRet ret = ParseRet::PART_FINISHED;
    while(ret == ParseRet::PART_FINISHED)
    {
        printf("Cur State: %s, ParseRet: %s, ptr_: %d\n\n", statetoStr(state_).c_str(), parseRetStr(ret).c_str(), ptr_);
        switch(state_)
        {
            case DFAState::METHOD: ret = parseMethod(data); break;
            case DFAState::TARGET: ret = parseTarget(data); break;
            case DFAState::VERSION: ret = parseVersion(data); break;
        }

        if(ret != ParseRet::PART_FINISHED)
        {
            return ret;
        }
    }

    return ParseRet::SERVER_ERR;
}

RequestLineParser::ParseRet RequestLineParser::parseMethod(const std::string& data)
{
    // 获取METHOD字段
    std::string methodStr;
    for (; ptr_ < data.size() && data[ptr_] != ' '; ptr_++)
    {
        methodStr.push_back(data[ptr_]);
    }

    // 若未遇到空格，则报文不全
    if(ptr_ == data.size())
    {
        return ParseRet::INCOMPLETE;
    }

    HTTPRequest::Method method = HTTPRequest::strtoMethod(methodStr);
    // METHOD字段不合法
    if (method == HTTPRequest::Method::UNKNOWN)
    {
        return ParseRet::ERR_FMT;
    }

    // 合法METHOD字段，存储并转移至下一状态
    request_.setMethod(method);
    ptr_++; // ptr目前处于空格处，后移一位，转移到下一步解析
    state_ = DFAState::TARGET;
    return ParseRet::PART_FINISHED;
}

RequestLineParser::ParseRet RequestLineParser::parseTarget(const std::string& data)
{
    RequestTargetParser targetDecoder(ptr_);
    ParseRet ret = targetDecoder.parse(data);
    request_.setTarget(targetDecoder.target()); // TODO 为Request赋值转移至TargetParser中进行

    if(ret == ParseRet::FINISHED)
    {
        ++ptr_; // 若解析成功，则此时ptr位于空格处，+1使其转移到下一个字段
        ret = ParseRet::PART_FINISHED;
        state_ = DFAState::VERSION;
    }

    return ret;
}

RequestLineParser::ParseRet RequestLineParser::parseVersion(const std::string& data)
{
    // 获取VERSION字段
    std::string version;
    for (; ptr_ < data.size() && data[ptr_] != '\r'; ptr_++)
    {
        version.push_back(data[ptr_]);
    }

    // 若未遇到\r\n，则报文不全
    if(ptr_ + 1 >= data.size())
    {
        return ParseRet::INCOMPLETE;
    }

    HTTPRequest::Version ver = HTTPRequest::strtoVersion(version);
    // REQUESTLINE 未以\r\n结尾或者VERSION字段不合法
    if (data[ptr_ + 1] != '\n' || ver == HTTPRequest::Version::UNKNOWN)
    {
        return ParseRet::ERR_FMT;
    }

    // 合法的VERSION字段，存储字段并完成REQUESTLINE的解析
    request_.setVersion(ver);
    ptr_ += 2;  // ptr_此时位于\r，+2使其转移到HEADER字段
    return ParseRet::FINISHED;
}

std::string RequestLineParser::statetoStr(DFAState s)
{
    switch(s)
    {
        case DFAState::METHOD: return "METHOD"; break;
        case DFAState::TARGET: return "TARGET"; break;
        case DFAState::VERSION: return "VERSION"; break;
        default: return "UNKNOWN"; break;
    }
}