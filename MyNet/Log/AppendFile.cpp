#include "AppendFile.h"
AppendFile::AppendFile(std::string fileName)
:file(fopen(fileName.c_str(), "ae")), writtenBytes(0)
{
    // 设置流缓冲区
    setbuffer(file, buffer, sizeof(buffer));
}

AppendFile::~AppendFile()
{
    // 刷新缓冲区
    fflush();

    // 关闭文件
    fclose(file);
}

void AppendFile::append(const char* buff, int len)
{
    int written = 0;

    while(written < len)
    {
        int remain = len - written;
        int ret = write(buff, remain);

        // 发生错误
        if(ret == 0)
        {
            fprintf(stderr, "AppendFile Error: %s\n", strerror(ferror(file)));
            break;
        }

        written += ret;
    }

    written += len;
}

int AppendFile::write(const char* buf, int len)
{
    return ::fwrite_unlocked(buf, 1, len, file);
}

void AppendFile::fflush()
{
    ::fflush(file);
}


int AppendFile::written()
{
    return writtenBytes;
}