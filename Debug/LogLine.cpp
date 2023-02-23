#include "LogLine.h"

LogLine::LogLine(StringList& list)
:time_(list[1]), id_(std::stoi(list[6]))
{
    if(list[4] == "Established")
        type_ = Type::ESTABLISH;
    else if(list[4] == "Destroyed")
        type_ = Type::DESTROYED;
    else if(list[4] == "Deconstruct")
        type_ = Type::DECONSTRUCT;
    else
        type_ = Type::UNKNOWN;
}