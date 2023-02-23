#include "HTTPParser.h"
#include "RequestLineParser.h"
#include "HeadersParser.h"

/**
 * @brief: 报文解析的入口
 * @return {*}
 */    
HTTPParser::ParseRet HTTPParser::parse(const std::string& data)
{
    // TODO Target_TOO_LONG 414

    // 开始解析
    ParseRet partialRet = ParseRet::PART_FINISHED;
    while (partialRet == ParseRet::PART_FINISHED)
    {
        printf("Cur State: %s, ParseRet: %s, ptr_: %d, char: %c\n\n", statetoStr(state_).c_str(), parseRetStr(partialRet).c_str(), ptr_, data[ptr_]);
        // 状态机转移
        switch(state_)
        {
            case DFAState::REQUEST_LINE : partialRet = parseRequestLine(data); break;
            case DFAState::HEADER : partialRet = parseHeaders(data); break;
            case DFAState::CONTENT : partialRet = parseContent(data); break;
            default : partialRet = ParseRet::SERVER_ERR; break;
        }
        
        if (partialRet != ParseRet::PART_FINISHED)
        {
            request_.print();
            return partialRet;
        }
    }
    return ParseRet::SERVER_ERR;
}

HTTPParser::ParseRet HTTPParser::parseRequestLine(const std::string& data)
{
    RequestLineParser requestLineParser(request_, ptr_);
    ParseRet ret = requestLineParser.parse(data);

    if(ret == ParseRet::FINISHED)
    {
        state_ = DFAState::HEADER;
        ret = ParseRet::PART_FINISHED;
    }

    return ret;
}

HTTPParser::ParseRet HTTPParser::parseHeaders(const std::string& data)
{
    HeadersParser headersParser(request_, ptr_);
    ParseRet ret = headersParser.parse(data);

    if(ret == ParseRet::FINISHED)
    {
        state_ = DFAState::CONTENT;
        ret = ParseRet::PART_FINISHED;
    }
    return ret;
}

HTTPParser::ParseRet HTTPParser::parseContent(const std::string& data)
{
    if(request_.method() != HTTPRequest::Method::POST)
    {
        return ParseRet::FINISHED;
    }

    int readLength = std::stoi(request_.getHeader("content-length"));
    if(ptr_ + readLength > data.size())
    {
        return ParseRet::INCOMPLETE;
    }
    else
    {
        request_.setContent(data.substr(ptr_, readLength));
        return ParseRet::FINISHED;
    }
}

std::string HTTPParser::statetoStr(DFAState state)
{
    switch(state)
    {
        case DFAState::REQUEST_LINE: return "REQUEST_LINE"; break;
        case DFAState::HEADER: return "HEADER"; break;
        case DFAState::CONTENT: return "CONTENT"; break;
        default: return "UNKNOEN"; break;
    }
}