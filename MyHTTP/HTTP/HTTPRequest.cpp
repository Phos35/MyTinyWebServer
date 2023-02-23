#include "HTTPRequest.h"
#include "../Utils/StringList.h"

HTTPRequest::HTTPRequest(Buffer* inputBuff)
:buff(inputBuff), 
 lineStart(0), curIndex(0), reqState(RequesState::REQUESTLINE)
{

}

/**
 * @brief: 设置输入缓冲区
 * @return {*}
 * @param {Buffer} *inputBuff 输入缓冲区指针
 */    
void HTTPRequest::setBuffer(Buffer *inputBuff)
{
    buff = inputBuff;
}

/**
 * @brief: 获取资源Url
 * @return {*}
 */    
std::string& HTTPRequest::getUrl()
{
    return url;
}

/**
 * @brief: 获取请求体
 * @return {*}
 */    
std::string& HTTPRequest::getContent()
{
    return content;
}

/**
 * @brief: 打印HTTPRequest的内容
 * @return {*}
 */    
void HTTPRequest::print()
{
    printf("Buffer: %s\n", buff->rdPtr());

    printf("%s %s %s\n", type == RequestType::GET ? "GET" : "POST", ver.c_str(), url.c_str());
    for (auto itr = header.begin(); itr != header.end(); itr++)
    {
        printf("%s: %s\n", itr->first.c_str(), itr->second.c_str());
    }
    printf("\n");

    if(content.empty() == false)
    {
        printf("%s\n\n", content.c_str());
    }
}

/**
 * @brief: 解析请求
 * @return {*ParseRet} 解析的结果状态
 */    
HTTPRequest::ParseRet HTTPRequest::parse()
{
    // 无数据
    if(buff->readable() == 0)
    {
        return ParseRet::NO_DATA;
    }

    // 初始化
    lineStart = buff->prespace() + lineStart;
    curIndex = buff->prespace() + curIndex;
    lineState = LineState::LINE_OK;
    const char *data = buff->data();
    printf("RECV\n%s\n", data + lineStart);

    // 解析数据
    while(lineState == LineState::LINE_OK)
    {
        // 解析一行
        lineState = parseLine();
        // 行数据不足
        if(lineState == LineState::LINE_OPEN)
        {
            return ParseRet::UNCOMPLETE;
        }
        // 行解析出错
        else if(lineState == LineState::LINE_BAD)
        {
            return ParseRet::BAD;
        }

        // 获取一行数据
        std::string line(data + lineStart, data + curIndex - 2);
        lineStart = curIndex;

        // 根据状态进行解析
        ParseRet ret = ParseRet::UNKNOWN;
        RequesState tmpS = reqState;
        switch (reqState)
        {
        case RequesState::REQUESTLINE:
            ret = parseRequestLine(line);
            break;

        case RequesState::HEADER:
            ret = parseHeader(line);
            break;

        case RequesState::CONTENT:
            ret = parseContent(line);
            break;

        default:
            return ParseRet::UNKNOWN;
        }

        // if(ret == ParseRet::UNKNOWN)
        // {
        //     printf("state: %s, parseRet: %s\n", stateStr(tmpS).c_str(), parseRetStr(ret).c_str());
        // }

        // 若解析完成或解析出错，则释放缓冲内容并直接返回结果
        if(ret != ParseRet::PART_SUCCESS)
        {
            buff->readAsString(curIndex);
            return ret;
        }
    }

    // 解析未完成，需要继续读取数据
    return ParseRet::UNCOMPLETE;
}

/**
 * @brief: 初始化请求状态
 * @return {*}
 */    
void HTTPRequest::resetState()
{
    reqState = RequesState::REQUESTLINE;
}

/**
 * @brief: 解析出一行数据
 * @return {*}
 */    
HTTPRequest::LineState HTTPRequest::parseLine()
{
    int end = buff->prespace() + buff->readable();
    for (; curIndex < end; curIndex++)
    {
        char curCh = buff->data()[curIndex];

        if(curCh == '\r')
        {
            // 一行结束
            if(curIndex + 1 < end && buff->data()[curIndex + 1] == '\n')
            {
                curIndex += 2;
                return LineState::LINE_OK;
            }
            // 数据结束，表明可能还有其他数据尚未接收到
            else if(curIndex + 1 == end)
            {
                curIndex++;
                return LineState::LINE_OPEN;
            }
            // 语法错误
            else
            {
                return LineState::LINE_BAD;
            }
        }
        else if(curCh == '\n')
        {
            // 一行结束
            if(curIndex > 0 && buff->data()[curIndex - 1] == '\r')
            {
                curIndex++;
                return LineState::LINE_OK;
            }
            // 不符合格式的行
            else
                return LineState::LINE_BAD;
        }
    }

    // 解析过程中未遇到\r 或者 \n，但目前处于CONTENT解析，则解析完成
    if(reqState == RequesState::CONTENT && curIndex == end)
    {
        curIndex += 2;
        return LineState::LINE_OK;
    }
    // 否则解析未完成，需要更多的数据
    else
        return LineState::LINE_OPEN;
}

/**
 * @brief: 解析请求行
 * @return {*}
 */    
HTTPRequest::ParseRet HTTPRequest::parseRequestLine(std::string& line)
{
    StringList strList(line);

    // 格式错误
    if (strList.size() != 3)
        return ParseRet::BAD;
    
    // 获取请求类型
    if(strList[0] == "GET")
        type = RequestType::GET;
    else
        type = RequestType::POST;

    // 获取URL
    url = strList[1];

    // 获取协议版本
    ver = strList[2];

    // 转换状态
    reqState = RequesState::HEADER;

    return ParseRet::PART_SUCCESS;
}

/**
 * @brief: 解析请求头
 * @return {*}
 */    
HTTPRequest::ParseRet HTTPRequest::parseHeader(std::string& line)
{
    // 空行，转换状态
    if(line.empty() == true)
    {
        // 若为POST请求，则需要转换至CONTENT
        if(type == RequestType::POST)
        {
            // header中断，则需要继续接收信息
            if(curIndex == buff->prespace() + buff->readable())
            {
                return ParseRet::UNCOMPLETE;
            }
            // header读取完毕，转入读取CONTENT
            else
            {
                reqState = RequesState::CONTENT;
                return ParseRet::PART_SUCCESS;
            }
        }
        // 若为GET请求，则解析结束
        else
        {
            return ParseRet::GET;
        }
    }

    StringList strList(line, ": ", 1);

    // 格式错误
    if (strList.size() < 2)
        return ParseRet::BAD;

    // 添加至header字段中
    header.insert({strList[0], strList[1]});
    // printf("Header {%s:%s}\n", strList[0].c_str(), strList[1].c_str());

    return ParseRet::PART_SUCCESS;
}

/**
 * @brief: 解析请求体
 * @return {*}
 */    
HTTPRequest::ParseRet HTTPRequest::parseContent(std::string& line)
{
    // 记录内容
    content += line;

    // 读取完成则返回POST
    if(curIndex == buff->prespace() + buff->readable() + 2)
        return ParseRet::POST;
    else
        return ParseRet::PART_SUCCESS;
}

/**
 * @brief: 获取解析结果对应的字符串
 * @return {*}
 * @param {ParseRet} ret
 */    
std::string HTTPRequest::parseRetStr(ParseRet ret)
{
    switch (ret)
    {
    case ParseRet::NO_DATA : return "NO_DATA"; break;
    case ParseRet::BAD : return "BAD"; break;
    case ParseRet::UNCOMPLETE : return "UNCOMPLETE"; break;
    case ParseRet::PART_SUCCESS : return "PART_SUCCESS"; break;
    case ParseRet::GET : return "GET"; break;
    case ParseRet::POST : return "POST"; break;
    default: return "UNKNOWN"; break;
    }
}

/**
 * @brief: 获取请求解析状态对应的字符串
 * @return {*}
 * @param {RequesState} s
 */    
std::string HTTPRequest::stateStr(RequesState s)
{
    switch (s)
    {
    case RequesState::REQUESTLINE : return "REQUESTLINE"; break;
    case RequesState::HEADER : return "HEADER"; break;
    case RequesState::CONTENT : return "CONTENT"; break;
    default:
        return "UNKNOWN STATE";
        break;
    }
}