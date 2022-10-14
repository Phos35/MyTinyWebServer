#include "LogFile.h"

LogFile::LogFile(std::string& processName, int rollSize, int dateCheckCount)
:processName(processName), rollSize(rollSize), dateCheckCount(dateCheckCount),
 startTime(Timestamp::now()), lastRollTime(startTime)
{
    // 创建日志文件
    roll();
}

void LogFile::append(const char* buff, int len)
{
    // 将数据写入文件缓冲区
    file->append(buff, len);
    count++;

    // 若数据写入长度大于日志文件限定长度，则滚动日志文件
    if(file->written() >= rollSize)
    {
        roll();
    }
    // 当append次数超过dateCheckCount，查看日期是否更新
    if(count >= dateCheckCount)
    {
        Timestamp now = Timestamp::now();

        // 若日期更迭，则滚动日志文件
        if(now.day() != startTime.day())
        {
            roll();
        }
    }
}

void LogFile::flush()
{
    file->fflush();
}

void LogFile::roll()
{
    // 生成日志文件的名称
    std::string fileName = genFileName();

    // 更新file
    file.reset(new AppendFile(fileName));
}

std::string LogFile::genFileName()
{
    // 添加进程名称
    std::string fileName = processName + "_";

    // 添加日期
    fileName += startTime.format("%Y%m%d") + "_";

    // 添加进程id
    // fileName += std::to_string(getpid());

    // 添加结尾
    fileName += ".log";

    return fileName;
}