#ifndef TIMESTAMP_H
#define TIMESTAMP_H
#include <string>

class Timestamp{
public:
    static uint64_t MICRO_PER_MILLI;
    static uint64_t MICRO_PER_SECOND;
    static uint64_t MICRO_PER_MIN;
    static uint64_t MICRO_PER_HOUR;
    static uint64_t SECOND_PER_DAY;

    Timestamp(uint64_t micorseconds = 0)
    :microseconds_(micorseconds){}

    Timestamp(std::string &time);

    /**
     * @brief: 获取当前时间戳
     * @return {*}
     */    
    static Timestamp now();

    /**
     * @brief: 格式化时间字符串
     * @return {*}
     * @param {char} *format 格式：%Y-年，%m-月，%d-日，%H-小时
     *                             %M-分钟，%S-秒，%s-毫秒
     */    
    std::string format(const char *formatStr);

    /**
     * @brief: 时间戳增加microsec微妙
     * @return {*}
     * @param {uint64_t} microseconds_
     */    
    Timestamp addMicrosec(uint64_t microsec);

    /**
     * @brief: 时间戳增加ms毫秒
     * @return {*}
     * @param {uint64_t} ms
     */    
    Timestamp addMillisec(uint64_t ms);

    /**
     * @brief: 重载小于运算符，便于比较和排序
     * @return {*}
     */    
    bool operator<(const Timestamp &another) const { return this->microseconds_ < another.microseconds_; }

    /**
     * @brief: 获取时间戳微秒数
     * @return {*}
     */    
    uint64_t microseconds() { return microseconds_; }

    /*日期get函数*/
    uint64_t day() { return microseconds_ / MICRO_PER_SECOND / SECOND_PER_DAY; }

    /**
     * @brief: 将数字字符串转换为longlong型
     * @return {*}
     * @param {string&} str 待转换的字符串
     */    
    uint64_t strToll(const std::string& str);

private:
    uint64_t microseconds_;     // 微妙              

    /**
     * @brief: 将val转化为0前缀字符串
     * @return {*}
     * @param {int} val 代转化值
     * @param {int} len 转化字符串长度
     */    
    std::string zeroPreStr(int val, int len);
};
#endif