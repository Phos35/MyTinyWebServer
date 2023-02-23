#ifndef STRINGLIST_H
#define STRINGLIST_H
#include <string>
#include <vector>

class StringList{
public:
    StringList();
    StringList(const std::string &str, const char* blank = nullptr, int times = -1);

    /**
     * @brief: 拆分字符串形成字符串列表
     * @return {*}
     * @param {string} &str 待拆分的字符串
     * @param {char*} blank 拆分字符
     */    
    void split(const std::string &str, const char* blank = nullptr, int times = -1);

    /**
     * @brief: 获取字符串列表的长度 
     * @return {*} size
     */    
    int size();

    /**
     * @brief: 重载数组访问操作符
     * @return {*} 指定位置的字符串
     */    
    std::string &operator[](int index);

private:
    std::vector<std::string> list;      // 字符串列表
};

#endif