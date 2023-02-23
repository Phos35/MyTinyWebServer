#include "Timestamp.h"
#include <sys/time.h>
#include "StringList.h"

uint64_t Timestamp::MICRO_PER_MILLI = 1000;
uint64_t Timestamp::MICRO_PER_SECOND = 1000 * Timestamp::MICRO_PER_MILLI;
uint64_t Timestamp::MICRO_PER_MIN = 60 * Timestamp::MICRO_PER_SECOND;
uint64_t Timestamp::MICRO_PER_HOUR = 60 * Timestamp::MICRO_PER_MIN;
uint64_t Timestamp::SECOND_PER_DAY = 24 * Timestamp::MICRO_PER_HOUR;

Timestamp::Timestamp(std::string &time)
:microseconds_(0)
{
    StringList timeList(time, ":.");

    this->microseconds_ += strToll(timeList[0]) * MICRO_PER_HOUR;
    this->microseconds_ += strToll(timeList[1]) * MICRO_PER_MIN;
    this->microseconds_ += strToll(timeList[2]) * MICRO_PER_SECOND;
    this->microseconds_ += strToll(timeList[3]);
}

Timestamp Timestamp::now()
{
    timeval cur;
    gettimeofday(&cur, nullptr);

    uint64_t curMicro = cur.tv_sec * MICRO_PER_SECOND + cur.tv_usec;
    return Timestamp(curMicro);
}

std::string Timestamp::format(const char* formatStr)
{
    time_t seconds = static_cast<time_t>(microseconds_ / MICRO_PER_SECOND);
    tm cur;
    gmtime_r(&seconds, &cur);

    int state = 0;
    std::string date;
    for (const char *ch = formatStr; *ch != '\0'; ch++)
    {
        switch (state)
        {
        case 0:
        {
            if(*ch == '%')
                state = 1;
            else
                date.push_back(*ch);
        };
        break;

        case 1:
        {
            switch(*ch)
            {
            case 'y': date += zeroPreStr(cur.tm_year + 1900, 4); break;
            case 'm': date += zeroPreStr(cur.tm_mon + 1, 2); break;
            case 'd': date += zeroPreStr(cur.tm_mday, 2); break;
            case 'H': date += zeroPreStr(cur.tm_hour, 2); break;
            case 'M': date += zeroPreStr(cur.tm_min, 2); break;
            case 'S': date += zeroPreStr(cur.tm_sec, 2); break;
            case 's': date += zeroPreStr(microseconds_ % MICRO_PER_SECOND, 6); break;
            }
            state = 0;
        };
        break;
        }
    }

    return date;
}

/**
 * @brief: 时间戳增加microsec微妙
 * @return {*}
 * @param {uint64_t} microseconds_
 */    
Timestamp Timestamp::addMicrosec(uint64_t microsec)
{
    microseconds_ += microsec;
    return *this;
}

/**
 * @brief: 时间戳增加ms毫秒
 * @return {*}
 * @param {uint64_t} ms
 */    
Timestamp Timestamp::addMillisec(uint64_t ms)
{
    microseconds_ += ms * MICRO_PER_MILLI;
    return *this;
}

std::string Timestamp::zeroPreStr(int val, int len)
{
    std::string valStr = std::to_string(val);
    if(valStr.size() == len)
        return valStr;
    else
        return std::string(len - valStr.size(), '0') + valStr;
}

uint64_t Timestamp::strToll(const std::string& str)
{
    int nonZeroPos = str.find_first_not_of('0');

    if(nonZeroPos == std::string::npos) return 0;
    else return std::stoll(str.substr(nonZeroPos));
}