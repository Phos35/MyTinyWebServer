#include "HTTPRequest.h"
#include <functional>

void HTTPRequest::print()
{
    printf("Method: %s\n", methodtoStr(method_).c_str());
    printf("Request-Target: %s\n", targettoStr(target_).c_str());
    printf("Protocol Version: %s\n\n", versiontoStr(ver_).c_str());

    static std::function<void(Table &, const char *)> printTable = [](Table &t, const char *name)
    {
        printf("%s:\n", name);
        for (auto itr = t.begin(); itr != t.end();itr++)
        {
            printf("%s : %s\n", itr->first.c_str(), itr->second.c_str());
        }
        printf("\n");
    };

    if(headers_.empty() == false)
        printTable(headers_, "Headers");

    if(target_.queries_.empty() == false)
        printTable(target_.queries_, "Queries");

    if(args_.empty() == false)
        printTable(args_, "Arguments");

    if(content_.empty() == false)
        printf("Content:\n%s\n\n", content_.c_str());
}

std::string HTTPRequest::methodtoStr(Method method)
{
    switch(method)
    {
        case Method::GET: return "GET"; break;
        case Method::HEAD: return "HEAD"; break;
        case Method::POST: return "POST"; break;
        case Method::PUT: return "PUT"; break;
        case Method::DELETE: return "DELETE"; break;
        case Method::CONNECT: return "CONNECT"; break;
        case Method::OPTIONS: return "OPTIONS"; break;
        case Method::TRACE: return "TRACE"; break;
        default: return "UNKNOWN METHOD"; break;
    }
}

HTTPRequest::Method HTTPRequest::strtoMethod(const std::string& str)
{
    static MethodTable methodTable =
    {
        {"GET", HTTPRequest::Method::GET}, {"HEAD", HTTPRequest::Method::HEAD},
        {"POST", HTTPRequest::Method::POST},{"PUT", HTTPRequest::Method::PUT},
        {"DELETE", HTTPRequest::Method::DELETE},{"CONNECT", HTTPRequest::Method::CONNECT},
        {"OPTIONS", HTTPRequest::Method::OPTIONS},{"TRACE", HTTPRequest::Method::TRACE}
    };

    auto itr = methodTable.find(str);
    if(itr == methodTable.end())
        return Method::UNKNOWN;
    else
        return itr->second;
}

std::string HTTPRequest::versiontoStr(Version ver)
{
    switch(ver)
    {
        case Version::HTTP10: return "HTTP/1.0"; break;
        case Version::HTTP11: return "HTTP/1.1"; break;
        case Version::HTTP2: return "HTTP/2"; break;
        case Version::HTTP3: return "HTTP/3"; break;
        default: return "UNKNOWN Version"; break;
    }
}

HTTPRequest::Version HTTPRequest::strtoVersion(const std::string &str)
{
    static std::unordered_map<std::string, Version> verTable = 
    {
        {"HTTP/1.0", Version::HTTP10},{"HTTP/1.1", Version::HTTP11},
        {"HTTP/2", Version::HTTP2},{"HTTP/3", Version::HTTP3},
    };

    auto itr = verTable.find(str);
    if(itr == verTable.end())
        return Version::UNKNOWN;
    else
        return itr->second;
}

std::string HTTPRequest::targettoStr(RequestTarget &target)
{
    if(target.queries_.empty() == true)
        return target.path_;

    std::string queryStr;
    for (auto& kv : target.queries_)
    {
        queryStr += (kv.first + "=" + kv.second + "&");
    }
    queryStr.pop_back();

    return target.path_ + "?" + queryStr;
}