#include "StringList.h"
#include <unordered_set>

StringList::StringList()
{

}

StringList::StringList(const std::string &str, const char* blank, int times)
{
    split(str, blank, times);
}

/**
 * @brief: 拆分字符串形成字符串列表
 * @return {*}
 * @param {string} &str 待拆分的字符串
 * @param {char*} blank 拆分字符
 */    
void StringList::split(const std::string &str, const char* blank, int times)
{
    // 构建拆分字符集合
    std::unordered_set<char> splitSet;
    if(blank == nullptr)
    {
        splitSet.insert(' ');
        splitSet.insert('\t');
    }
    else
    {
        while(*blank != '\0')
        {
            splitSet.insert(*blank);
            blank++;
        }
    }

    // 拆分字符串
    int start = 0;
    for (int end = 0; end < str.size(); end++)
    {
        // 找到拆分字符
        if(splitSet.find(str[end]) != splitSet.end())
        {
            if(times == 0 && (str[end] == ' ' || str[end] == '\t'))
                continue;
            else if(str[end] == ' ' || str[end] == '\t')
                times--;

            // 若[start, end)字符串不为空，则拆分
            if(end != start)
            {
                list.push_back(str.substr(start, end - start));
            }

            // 更新起点
            start = end + 1;
        }
    }

    // 添加末尾字符串
    if (str.size() > start)
    {
        list.push_back(str.substr(start));
    }
}

/**
 * @brief: 获取字符串列表的长度 
 * @return {*} size
 */    
int StringList::size()
{
    return list.size();
}

/**
 * @brief: 重载数组访问操作符
 * @return {*} 指定位置的字符串
 */    
std::string& StringList::operator[](int index)
{
    return list[index];
}