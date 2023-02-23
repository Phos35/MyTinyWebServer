#ifndef DECODER_H
#define DECODER_H
#include <string>

class Parser{
public:
    // 解析结果
    enum class ParseRet
    {
        ERR_FMT,
        SERVER_ERR,
        INCOMPLETE,
        PART_FINISHED,
        FINISHED,
    };

    /**
     * @brief: 解析报文
     * @return {*}
     */    
    virtual ParseRet parse(const std::string& data) = 0;

    /**
     * @brief: 将解析结果转化为字符串 
     * @return {*}
     * @param {ParseRet} val
     */    
    static std::string parseRetStr(ParseRet val);

private:

};

#endif