#include "../../Utils/StringList.h"
#include <iostream>

int main()
{
    char splits[32] = {0};
    std::string line(1024, 0);
    while (line != "end")
    {
        printf("Input Splits: ");
        std::cin.getline(splits, 32);
        printf("Input String: ");
        std::cin.getline((char*)line.data(), 1024);

        StringList list(line.substr(0, line.find_first_of('\0')), splits);
        printf("List Size: %d\n", list.size());
        for (int i = 0; i < list.size(); i++)
        {
            printf("%d : %s\n", i, list[i].c_str());
        }
        printf("\n");
    }
}