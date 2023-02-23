#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
#include <string>
#include <unordered_map>

/* HTTP请求报文格式
<METHOD> SP <REQUEST_TARGET> SP <VERSION> CRLF
<HEADER> CRLF
...
CRLF
<CONTENT>
*/

/* REQUEST_TARGET格式
1. origin-form
<ABSOLUTE_PATH> [?<QUERY>]

2. absolute-form proxy使用，暂不做解析实现
<ABSOLUTE_URI>

3. authority-form 只用于CONNECT请求，用于多个代理之间的隧道建立，暂不做解析实现
<URI_HOST> : <PORT>

4. asteris-form 只用于OPTIONS请求
*
*/


class HTTPRequest
{
public:
    // 请求方法
    enum class Method
    {
        UNKNOWN,
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        CONNECT,
        OPTIONS,
        TRACE
    };

    // HTTP版本
    enum class Version
    {
        UNKNOWN,
        HTTP10,
        HTTP11,
        HTTP2,
        HTTP3
    };

    typedef std::unordered_map<std::string, Method> MethodTable;
    typedef std::unordered_map<std::string, Version> VersionTable;
    typedef std::unordered_map<std::string, std::string> Table;
    typedef Table QueryTable;
    typedef Table ArgTable;
    typedef Table HeaderTable;

    // 请求行中目标资源结构体
    struct RequestTarget
    {
        std::string path_;
        QueryTable queries_;
    };

    /**
     * @brief: 输出Targe详细信息 
     * @return {*}
     */    
    void print();

    /**
     * @brief: 请求方法转字符串
     * @return {*}
     * @param {Method} method 待转换的请求方法
     */    
    static std::string methodtoStr(Method method);

    /**
     * @brief: 请求方法字符串转枚举类型
     * @return {*}
     * @param {string&} str
     */    
    static Method strtoMethod(const std::string& str);

    /**
     * @brief: HTTP版本转字符串
     * @return {*}
     * @param {Version} ver 待转换的版本
     */    
    static std::string versiontoStr(Version ver);

    /**
     * @brief: HTTP版本字符串转化为对应的枚举类型
     * @return {*}
     * @param {string} &str
     */    
    static Version strtoVersion(const std::string &str);

    /*get系列*/
    Method method() { return method_; }
    RequestTarget target() { return target_; }
    std::string path() { return target_.path_; }
    const QueryTable queryTable() { return target_.queries_; }
    std::string query(const std::string &key) 
    {
        auto itr = target_.queries_.find(key);
        if(itr == target_.queries_.end())
            return std::string();
        else
            return itr->second;
    }
    Version version() { return ver_; }
    std::string getHeader(const std::string& key)
    {
        HeaderTable::iterator itr = headers_.find(key);
        if(itr == headers_.end())
        {
            printf("Invalid Key\n");
            return std::string();
        }
        else
        {
            return itr->second;
        }
    }

    /*set系列*/
    void setMethod(Method val) { method_ = val; }
    void setTarget(const RequestTarget &val) { target_ = val; }
    void setVersion(Version val) { ver_ = val; }
    void addHeader(const std::string &key, const std::string &val) { headers_.insert({key, val}); }
    void setContent(const std::string &val) { content_ = val; }

private:
    Method method_;         // HTTP请求方法
    RequestTarget target_;  // 请求的目标
    Version ver_;           // HTTP协议版本

    HeaderTable headers_;   // Header字段表
    ArgTable args_;         // POST请求参数

    std::string content_;   // 请求体

    /**
     * @brief: 将RequestTarget转为对应的字符串
     * @return {*}
     * @param {RequestTarget} &target
     */    
    std::string targettoStr(RequestTarget &target);
};

#endif