#include "../LogLine.h"
#include "../../Utils/StringList.h"
#include <iostream>
#include <fstream>
#include <unordered_map>

struct Info{
    LogLine established;
    LogLine destroyed;
    LogLine deconstruct;
};

int main()
{
    std::ifstream log("log.txt");
    if(log.is_open() == false)
    {
        printf("File Open Failed\n");
        return -1;
    }

    std::string line(1024, 0);
    std::unordered_map<int, Info> conns;
    while (line[0] != '-')
    {
        log.getline((char *)line.data(), 1024);
        StringList list(line, " ");

        LogLine logLine(list);
        switch(logLine.type())
        {
        case LogLine::Type::ESTABLISH : conns[logLine.id()].established = logLine; break;
        case LogLine::Type::DESTROYED : conns[logLine.id()].destroyed = logLine; break;
        case LogLine::Type::DECONSTRUCT : conns[logLine.id()].deconstruct = logLine; break;
        default:
            printf("TypeError, id : %d\n", logLine.id());
            return -1;
        }

        if(logLine.type() == LogLine::Type::DECONSTRUCT)
        {
            Timestamp established = conns[logLine.id()].established.time();
            Timestamp destroyed = conns[logLine.id()].destroyed.time();
            Timestamp deconstruct = conns[logLine.id()].deconstruct.time();
            
            if(destroyed.microseconds() - established.microseconds() >= 10 * Timestamp::MICRO_PER_SECOND)
                printf("Conn %d, Establish: %s, Destroyed: %s, Deconstruct: %s\n", logLine.id(), established.format("%H:%M:%S.%s").c_str(), destroyed.format("%H:%M:%S.%s").c_str(), deconstruct.format("%H:%M:%S.%s").c_str());

        }
    }
    printf("Finished\n");
    return 0;
}