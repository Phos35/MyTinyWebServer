#ifndef LOG_LINE_H
#define LOG_LINE_H
#include <string>
#include "../Utils/Timestamp.h"
#include "../Utils/StringList.h"

class LogLine{
public:
    LogLine():type_(Type::UNKNOWN), id_(-1){}
    LogLine(StringList& list);

    enum Type
    {
        UNKNOWN,
        ESTABLISH,
        DESTROYED,
        DECONSTRUCT
    };

    /*get*/
    Timestamp time() { return time_; }
    Type type() { return type_; }
    int id() { return id_; }

private:
    Timestamp time_;     // 事件发生的时间
    Type type_;          // 时间类型
    int id_;             // TCP连接的id
};

#endif