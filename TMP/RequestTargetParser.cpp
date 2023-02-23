#include "RequestTargetParser.h"

RequestTargetParser::ParseRet RequestTargetParser::parse(const std::string& data)
{
    for (; ptr_ < data.size(); ptr_++)
    {
        switch (data[ptr_])
        {
            case '%': target_.path_.push_back(hexStrToChar(data)); break;
            case '+': target_.path_.push_back(' '); break;
            case '?': decodeQuery(data); break;
            case ' ': decodeState_ = ParseRet::FINISHED; break;
            default: target_.path_.push_back(data[ptr_]); break;
        }

        if (decodeState_ != ParseRet::PART_FINISHED)
        {
            return decodeState_;
        }
    }

    return ParseRet::INCOMPLETE;
}

char RequestTargetParser::hexStrToChar(const std::string& data)
{
    char ch = 0;
    if(ptr_ + 2 >= data.size())
        decodeState_ = ParseRet::INCOMPLETE;

    int end = ptr_ + 2;
    for (int i = ptr_ + 1; i <= end; i++)
    {
        // printf("ptr : %c\n", data[i]);
        if (isdigit(data[i]))
            ch = ch * 16 + data[i] - '0';
        else if(isalpha(data[i]))
            ch = ch * 16 + std::toupper(data[i]) - 'A' + 10;
        else
        {
            decodeState_ = ParseRet::ERR_FMT;
        }
    }

    // 更新ptr
    ptr_ += 2;

    return ch;
}

void RequestTargetParser::decodeQuery(const std::string &data)
{
    /*通过指针cur来切换key、val读取*/
    std::string key, val, *cur = &key;
    for (ptr_++; ptr_ < data.size(); ptr_++)
    {
        char ch = data[ptr_];
        switch (data[ptr_])
        {
            case ' ': decodeState_ = ParseRet::FINISHED;
            case '&': 
            {
                target_.queries_[key] = val;
                key.clear();
                val.clear();
                cur = &key;
            }break;
            case '=': cur = &val; break;
            case '%': ch = hexStrToChar(data);
            default: cur->push_back(ch); break;
        }

        if(decodeState_ != ParseRet::PART_FINISHED)
            return;
    }

    decodeState_ = ParseRet::INCOMPLETE;
}