#include "HeadersParser.h"

HeadersParser::ParseRet HeadersParser::parse(const std::string &data)
{
    ParseRet partialRet = ParseRet::PART_FINISHED;
    std::string key, val;
    while (partialRet == ParseRet::PART_FINISHED)
    {
        printf("Cur State: %s, ParseRet: %s, ptr_: %d, char: %c\n\n", statetoStr(state_).c_str(), parseRetStr(partialRet).c_str(), ptr_, data[ptr_]);
        switch (state_)
        {
            case DFAState::KEY: partialRet = parseKey(data, key); break;
            case DFAState::VAL: partialRet = parseVal(data, val); break;
        }

        if(partialRet != ParseRet::PART_FINISHED)
        {
            return partialRet;
        }
        else if(state_ == DFAState::KEY && key.empty() == false)
        {
            request_.addHeader(key, val);
            key.clear();
            val.clear();
        }
    }

    return ParseRet::SERVER_ERR;
}

HeadersParser::ParseRet HeadersParser::parseKey(const std::string &data, std::string &key)
{
    // 报文不全
    if(ptr_ + 1 >= data.size())
    {
        return ParseRet::INCOMPLETE;
    }

    // 若遇到\r，则判断是否结束HEADER解析
    if(data[ptr_] == '\r')
    {
        // \r后未接\n，格式错误
        if(data[ptr_ + 1] != '\n')
        {
            return ParseRet::ERR_FMT;
        }
        // 遇到空行，结束HEADER解析
        else
        {
            ptr_ += 2; // 此时ptr_位于\r处，+2 使其转移至下一个字段处
            return ParseRet::FINISHED;
        }
    }

    // 获取字段名称
    for (; ptr_ < data.size() && data[ptr_] != ':'; ptr_++)
    {
        // 字段名不允许出现空格
        if(data[ptr_] == ' ')
            return ParseRet::ERR_FMT;
        // 全小写存储
        key.push_back(std::tolower(data[ptr_]));
    }
    ptr_++; // ptr_ 此时位于:处，+1转移到字段值处
    state_ = DFAState::VAL;
    return ParseRet::PART_FINISHED;
}

HeadersParser::ParseRet HeadersParser::parseVal(const std::string &data, std::string &val)
{
    // 忽略字段值的前置空格
    for (; ptr_ < data.size() && data[ptr_] == ' '; ptr_++);

    // 获取字段值
    for (; ptr_ < data.size() && data[ptr_] != '\r'; ptr_++)
    {
        val.push_back(std::tolower(data[ptr_]));
    }

    // 报文不全
    if(ptr_ + 1 >= data.size())
    {
        return ParseRet::INCOMPLETE;
    }

    // 字段值未以\r\n结尾
    if(data[ptr_ + 1] != '\n')
    {
        return ParseRet::ERR_FMT;
    }

    ptr_ += 2; // 此时ptr_位于\r处，+2使其转移到下一个字段
    state_ = DFAState::KEY;
    return ParseRet::PART_FINISHED;
}

std::string HeadersParser::statetoStr(DFAState s)
{
    switch(s)
    {
        case DFAState::KEY: return "KEY"; break;
        case DFAState::VAL: return "VAL"; break;
        default: return "UNKNOWN"; break;
    }
}