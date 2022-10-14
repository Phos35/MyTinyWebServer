#ifndef LOG_STREAM_H
#define LOG_STREAM_H
#include "FixedBuffer.h"
#include <string>

class LogStream
{
public:
    /**
     * @brief: 获取数据指针 
     * @return {*}
     */    
    char *data()
    {
        return buffer.data();
    }

    /**
     * @brief: 获取缓冲区数据长度
     * @return {*}
     */    
    int length()
    {
        return buffer.length();
    }

    /*重载 <<*/
    LogStream& operator <<(char val)
    {
        buffer.append(&val, 1);
        return *this;
    }

    LogStream& operator <<(bool val)
    {
        buffer.append(val == true ? "1" : "0", 1);
        return *this;
    }

    LogStream& operator <<(int val)
    {
        std::string str = std::to_string(val);
        buffer.append(str.c_str(), str.size());
        return *this;
    }

    LogStream& operator <<(long val)
    {
        std::string str = std::to_string(val);
        buffer.append(str.c_str(), str.size());
        return *this;
    }

    LogStream& operator <<(uint32_t val)
    {
        std::string str = std::to_string(val);
        buffer.append(str.c_str(), str.size());
        return *this;
    }

    LogStream& operator <<(float val)
    {
        std::string str = std::to_string(val);
        buffer.append(str.c_str(), str.size());
        return *this;
    }

    LogStream& operator <<(double val)
    {
        std::string str = std::to_string(val);
        buffer.append(str.c_str(), str.size());
        return *this;
    }

    LogStream& operator << (const char* str)
    {
        if(str != nullptr)
        {
            buffer.append(str, strlen(str));
        }
        else
        {
            buffer.append("NULL", 5);
        }
        return *this;
    }

    LogStream& operator<<(const std::string& str)
    {
        if(str.empty() == false)
        {
            buffer.append(str.c_str(), str.size());
        }
        else
        {
            buffer.append("NULL", 5);
        }
        return *this;
    }

private:
    typedef FixedBuffer<SMALL_BUFFER_SIZE> Buffer;

    Buffer buffer;    // 存储数据的缓冲区
};


#endif