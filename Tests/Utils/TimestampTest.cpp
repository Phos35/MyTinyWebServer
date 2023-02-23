#include "../../Utils/Timestamp.h"
#include <iostream>

int main()
{
    std::string time;
    std::cin >> time;

    Timestamp test(time);
    printf("readTime: %s\n", test.format("%H:%M:%S.%s").c_str());
    return 0;
}