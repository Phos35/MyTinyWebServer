#include "Parser.h"

std::string Parser::parseRetStr(Parser::ParseRet val)
{
    switch(val)
    {
        case ParseRet::ERR_FMT : return "ERR_FMT"; break;
        case ParseRet::SERVER_ERR : return "SERVER_ERR"; break;
        case ParseRet::INCOMPLETE : return "INCOMPLETE"; break;
        case ParseRet::PART_FINISHED : return "PART_FINISHED"; break;
        case ParseRet::FINISHED : return "FINISHED"; break;
        default: return "UNKNOWN"; break;
    }
}