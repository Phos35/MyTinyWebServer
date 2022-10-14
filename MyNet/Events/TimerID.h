#ifndef TIMERID_H
#define TIMERID_H
#include <stdint.h>
#include "Timer.h"

class TimerID{
public:
    TimerID(uint32_t id = 0, Timer* timer = nullptr)
    :id_(id), timer_(timer){}

    /*成员get函数*/
    uint32_t id() { return id_; }
    Timer *timer() { return timer_; }

private:
    uint32_t id_;
    Timer *timer_;
};

#endif