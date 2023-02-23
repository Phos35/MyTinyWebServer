#include "Logger.h"

std::unordered_map<Logger::Level, std::string> Logger::LevelStr = {
    {Logger::Level::DEBUG, "DEBUG"},
    {Logger::Level::TRACE, "TRACE"},
    {Logger::Level::INFO, "INFO"},
    {Logger::Level::WARN, "WARN"},
    {Logger::Level::ERROR, "ERROR"}
};

Logger::CallBack Logger::outputCallBack = &Logger::defaulOutput;

Logger::Logger(String file, int line, Level level)
:sourceFile(std::move(file)), line(line), level(level)
{
    // 获取当前时间
    time = Timestamp::now();

    // 获取线程id
    long threadID = pthread_self();

    // 填入date, thread_id, level
    logStream << time.format("%y-%m-%d %H:%M:%S.%s") << " " << threadID << " " << LevelStr[level] << " ";
}

LogStream& Logger::stream()
{
    return logStream;
}

void Logger::defaulOutput(const char* buff, int len)
{
    int written = 0;
    while(written != len)
    {
        int n = printf("%s", buff + written);
        printf("\n");
        written += n;
    }
}

void Logger::setOutput(Logger::CallBack cb)
{
    outputCallBack = cb;
}

Logger::~Logger()
{
    // 写入file 和 line
    logStream << " - " << sourceFile << ":" << line << "\n";

    // 通过输出回调函数输出至指定位置
    if(outputCallBack)
    {
        outputCallBack(logStream.data(), logStream.length());
    }
}