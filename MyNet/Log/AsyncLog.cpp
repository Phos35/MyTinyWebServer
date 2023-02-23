#include "AsyncLog.h"
AsyncLog::AsyncLog(std::string& processName, int rollSize, 
                   int dateCheckCount, int interval)
:processName(processName), rollSize(rollSize), 
 dateCheckCount(dateCheckCount), interval(interval),
 input1(new Buffer()), input2(new Buffer())
{

}

void AsyncLog::turnOn()
{
    Logger::setOutput(std::bind(&AsyncLog::append, this, std::placeholders::_1, std::placeholders::_2));
}

void AsyncLog::append(const char* buff, int len)
{
    std::lock_guard<Mutex> guard(mutex);
    // 若正在写的缓冲区已满，则交换正在输入的缓冲区和预备缓冲区, 填入数据并通知后端线程
    if(input1->available() < len)
    {
        inputBuffers.push_back(std::move(input1));

        input1 = std::move(input2);
        input1->append(buff, len);
        cond.notify_one();
    }
    // 若未满，则直接填入
    else
        input1->append(buff, len);
}

void AsyncLog::start()
{
    running = true;
    std::thread thread(std::bind(&AsyncLog::backFunc, this));
    thread.detach();
}

void AsyncLog::backFunc()
{
    // 准备资源
    LogFile file(processName, rollSize, dateCheckCount);
    BufferPtr back1(new Buffer());
    BufferPtr back2(new Buffer());
    BufferVec writtingVec;

    back1->clear();
    back2->clear();

    while(running)
    {
        {
            std::unique_lock<Mutex> lck(mutex);
            if(inputBuffers.empty())
            {
                cond.wait_for(lck, std::chrono::seconds(interval));
            }

            // 获取正在输入的buffer并补充
            inputBuffers.push_back(std::move(input1));
            input1 = std::move(back1);

            // 若预备输入缓冲为空，则补充
            if(!input2)
            {
                input2 = std::move(back2);
            }

            // 获取需要写入文件的缓冲区
            writtingVec.swap(inputBuffers);
        }

        // 写入缓冲区
        for (const auto &buffer : writtingVec)
        {
            file.append(buffer->data(), buffer->length());
        }

        // 补充back缓冲区
        back1 = std::move(writtingVec.back());
        writtingVec.pop_back();
        back1->clear();

        if (!back2)
        {
            back2 = std::move(writtingVec.back());
            writtingVec.pop_back();
            back2->clear();
        }

        // 刷新缓冲区
        file.flush();
    }

    file.flush();
}