#include "HTTPParser.h"
#include <iostream>
#include <fstream>

int main()
{
    std::ifstream datafile("data.txt");
    if(datafile.is_open() == false)
    {
        printf("File open failed\n");
        return -1;
    }

    char data[2048] = {0};
    datafile.read(data, 2048);
    std::cout << "Data: \n" << data << std::endl;
    // std::cin.getline(data, 128);

    HTTPRequest request;
    HTTPParser decoder(request);
    printf("ParseRet: %s\n", HTTPParser::parseRetStr(decoder.parse(data)).c_str());
    return 0;
}