#include "TCPConnection.h"
#include "AsyncLog.h"
#include <assert.h>

TCPConnection::TCPConnection(EventLoop* loop, uint32_t id, 
                             const Socket &host, const Socket& peer)
:loop_(loop), id_(id), host_(host), peer_(peer), channel_(peer.fd(), loop),
 state_(State::CONNECTING)
{
    channel_.setRDCallBack(std::bind(&TCPConnection::handleRead, this, std::placeholders::_1));
    channel_.setWRCallBack(std::bind(&TCPConnection::handleWrite, this));
    // TODO 设置其他回调函数
}

TCPConnection::~TCPConnection()
{
    LOG_ERROR << "Deconstruct " << "TCPConnection " << id_;
}

/**
 * @brief: 启动TCPConnection
 * @return {*}
 */    
void TCPConnection::establish()
{   
    // 断言在事件循环所处的线程中
    loop_->assertInLoopThread();

    // 若设置为ET模式，则需要关注ET
    if(trigger_ == Trigger::ET)
        channel_.focusET();

    // 关注读事件
    channel_.focusRead();
    state_ = State::CONNECTED;

    // 设置超时定时器 TODO 定时器设置应在更上层进行
    Timestamp closeTime = Timestamp::now().addMillisec(10000);
    timer_ = loop_->runAt(std::bind(&TCPConnection::handleClose, shared_from_this()), closeTime);
    LOG_TRACE << "Connection " << id_ << " Set Close Timer: " << timer_.id();

    LOG_ERROR << "Established " << "TCPConnection " << id_ << " fd " << peer_.fd()
              << " Loop wakefd " << loop_->wakefd();
}

/**
 * @brief: 连接关闭，销毁相应资源
 * @return {*}
 */    
void TCPConnection::destroy()
{
    // 断言状态为DISCONNECTING
    assert(state_ == State::DISCONNECTING);

    // 断言在事件循环所处的线程中
    loop_->assertInLoopThread();

    state_ = State::DISCONNECTED;

    // 从事件循环中移除channel
    channel_.remove();

    LOG_ERROR << "Destroyed " << "TCPConnection " << id_ << " fd " << peer_.fd();

    // 关闭Socket
    peer_.close();

    // 取消定时器
    loop_->cancelTimer(timer_);
}

/**
 * @brief: 处理读事件 -- 避免ONESHOT的设计
 * @return {*}
 * @param {Timestamp} time 读事件发生的时刻
 */
void TCPConnection::handleRead(Timestamp time)
{
    // 若连接已断开，则退出
    if(state_ != State::CONNECTED)
    {
        LOG_WARN << "Connection " << id_ << " Handleread In " << stateStr(state_);
        return;
    }

    if(input_.handling() == true)
        LOG_TRACE << "TCPConnection " << id_ << " Buffer Busy";
    // input buffer可用，则读入数据
    if(input_.handling() == false)
    {
        if(trigger_ == Trigger::LT)
            readLT(time);
        else
            readET(time);
    }
    // input不可用
    // 若为LT模式，事件还会再次触发，忽略此次读事件即可
    // 若为ET模式，则事件不会再次触发，需要设置定时器再读
    else if(trigger_ == Trigger::ET) 
    {
        Timestamp nextTime = Timestamp::now().addMillisec(2);
        loop_->runAt(std::bind(&TCPConnection::handleRead, shared_from_this(), time), nextTime);
    }
}

/**
 * @brief: LT模式下的读
 * @return {*}
 * @param {Timestamp} time 读事件发生的时间
 */
void TCPConnection::readLT(Timestamp time)
{
    // 读取内容
    int readed = input_.readfd(channel_.fd(), trigger_);
    LOG_TRACE << "TCPConnection " << id_ << " Read " << readed << " Bytes";

    // 读取错误
    if (readed == -1)
    {
        LOG_ERROR << "TCPConnection " << id_ << " Read Failed: " << strerror(errno);
    }
    // 对端关闭
    else if(readed == 0)
    {
        LOG_TRACE << "TCPConnection " << id_ << " Peer Closed";
        handleClose();
    }
    // 正常读取，调用读事件回调函数
    else
    {
        if(readCallBack_)
        {
            readCallBack_(shared_from_this());
        }
        else
        {
            printf("TCPConnection %d Not Set ReadCallBack\n", id_);
        }
    }
}

/**
 * @brief: ET模式下的读取
 * @return {*}
 * @param {Timestamp} time 读事件发生的时间
 */
void TCPConnection::readET(Timestamp time)
{
    while(true)
    {
        int readed = input_.readfd(channel_.fd(), trigger_);
        LOG_TRACE << "TCPConnection " << id_ << " Read " << readed << " Bytes";
        
        // 读取发生错误
        if(readed == -1)
        {
            // 若为EAGAIN 或 EWOULDBLOCK，则结束读取
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                if(readCallBack_)
                    readCallBack_(shared_from_this());
                else
                    printf("TCPConnection %d Not Set ReadCallBack\n", id_);
                break;
            }
            // 否则输出错误信息
            else LOG_ERROR << "TCPConnection " << id_ << " Read Failed: " << strerror(errno);
        }
        // 对端关闭
        else if(readed == 0)
        {
            handleClose();
            break;
        }
    }
}

/**
 * @brief: 发送数据
 * @return {*}
 * @param {string} &content 待发送的内容
 */    
void TCPConnection::send(const std::string &content)
{
    if(loop_->isInLoopThread())
    {
        sendInLoop(content);
    }
    else
    {
        loop_->runInLoop(std::bind(&TCPConnection::sendInLoop, shared_from_this(), content));
    }
}

/**
 * @brief: 在 TCPConnection 所属的事件循环中执行发送数据的任务
 * @return {*}
 * @param {string} content
 */    
void TCPConnection::sendInLoop(std::string content)
{
    // 对端先关闭连接，则停止输出
    if(state_ != State::CONNECTED)
    {
        LOG_WARN << "TCPConnection" << id_ << " Send Failed, Current State: " << stateStr(state_);
        return;
    }

    // 断言在事件循环所处的线程
    loop_->assertInLoopThread();

    // 第一次发送数据
    int wrote = 0;
    if (channel_.writing() == false)
    {
        int sent = 0;
        // LT模式首次写
        if(trigger_ == Trigger::LT)
            sent = firstWriteLT(content);
        // ET模式首次写
        else
            sent = firstWriteET(content);
        
        // 写入发生错误
        if(sent == -1)
            handleClose();
        
        LOG_TRACE << "TCPConnection " << id_ << " First Write " << sent << " Bytes";

        wrote += sent;
    }

    // 若有数据尚未写入，则需要放入缓冲区并关注读事件
    if(wrote < content.size() && wrote != -1)
    {
        output_.append(content.data() + wrote, content.size() - wrote);
        channel_.focusWrite();
    }
}

/**
 * @brief: LT模式下的首次写
 * @return {*} 写入成功则返回写入的字节数；失败则返回-1
 * @param {string} &content 待写入的内容
 */
int TCPConnection::firstWriteLT(const std::string &content)
{
    return write(peer_.fd(), content.data(), content.size());
}

/**
 * @brief: ET模式下的首次写入
 * @return {*} 写入成功则返回写入的字节数；失败则返回-1
 * @param {string} &content 待写入的内容
 */
int TCPConnection::firstWriteET(const std::string &content)
{
    int totalSent = 0;
    while (true)
    {
        int sent = write(peer_.fd(), content.data() + totalSent, content.size() - totalSent);
        if(sent == -1)
        {
            // 写入结束
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return totalSent;
            }
            // 发生错误
            else
            {
                return -1;
            }
        }

        totalSent += sent;
        if(totalSent == content.size())
            return totalSent;
    }
    return -1;
}

/**
 * @brief: 优雅关闭连接，即只关闭读端，写端待数据写完后再关闭
 * @return {*}
 */    
void TCPConnection::shutdown()
{
    // 若在本线程中，则直接执行
    if(loop_->isInLoopThread())
    {
        shutdownInLoop();
    }
    else
    {
        loop_->runInLoop(std::bind(&TCPConnection::shutdownInLoop, shared_from_this()));
    }
}

/**
 * @brief: 将优雅关闭连接的任务转移到IO事件循环中执行
 * @return {*}
 */    
void TCPConnection::shutdownInLoop()
{
    // 若对端已关闭连接，则无需shutdown
    if(state_ != CONNECTED)
    {
        LOG_WARN << "TCPConnection " << id_ << " Peer May Close, Now State: " << stateStr(state_);
        return;
    }

    // 若数据已经写完，则直接关闭
    if(channel_.writing() == false)
    {
        handleClose();
    }
    // 数据仍在写，且对端未关闭连接
    else if(state_ == State::CONNECTED)
    {
        state_ = State::SHUTDOWN;
    }
}

/**
 * @brief: 处理写事件(只处理当缓冲区中存在内容时的写)
 * @return {*}
 */
void TCPConnection::handleWrite()
{
    if(channel_.writing() == true)
    {
        int wrote = 0;
        // LT模式写
        if(trigger_ == Trigger::LT)
            wrote = writeLT();
        // ET模式写
        else
            wrote = writeET();

        // 发生错误
        if (wrote == -1)
        {
            LOG_ERROR << "TCPConnection " << id_ << " Write Failed";
            channel_.unfocusWrite();
            handleClose();
            return;
        }

        output_.readAsString(wrote);

        // 数据写完，取消写事件
        if(output_.readable() == 0)
        {
            channel_.unfocusWrite();

            // 若连接已优雅关闭，则进行最后的资源销毁
            if(state_ == State::SHUTDOWN)
            {
                LOG_TRACE << "TCPConenction " << id_ << " Shutdown Finished, Now Closing";
                handleClose();
            }
        }
    }
}

/**
 * @brief: LT模式下的写
 * @return {*} 写入成功则返回写入的字节数；写入失败则返回-1
 */
int TCPConnection::writeLT()
{
    int wrote = write(peer_.fd(), output_.rdPtr(), output_.readable());
    LOG_TRACE << "TCPConnection " << id_ << " LTWrite " << wrote << " Bytes";
    return wrote;
}

/**
 * @brief: ET模式下的写
 * @return {*} 写入成功则返回写入的字节数；写入失败则返回-1
 */
int TCPConnection::writeET()
{
    int totalWrote = 0;
    while(true)
    {
        int wrote = write(peer_.fd(), output_.rdPtr() + totalWrote, output_.readable() - totalWrote);
        LOG_TRACE << "TCPConnection " << id_ << " ETWrite " << wrote << " Bytes";

        if(wrote == -1)
        {
            // 写入结束
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return totalWrote;
            }
            // 发生错误
            else
            {
                LOG_TRACE << "TCPConnection ET Write Failed: " << strerror(errno);
                return -1;
            }
        }

        totalWrote += wrote;
        //  写完数据
        if(totalWrote == output_.readable())
        {
            return totalWrote;
        }
    }
    return -1;
}

/**
 * @brief: 处理关闭事件
 * @return {*}
 */
void TCPConnection::handleClose()
{
    // 设置状态为DISCONNECTING
    if(state_ == State::CONNECTED || (state_ == State::SHUTDOWN && channel_.writing() == false))
    {
        state_ = State::DISCONNECTING;

        // 取消定时关闭的定时器
        // loop_->cancelTimer(timer_);

        // 调用连接关闭回调函数
        if(closeCallBack_)
        {
            channel_.unfocusAll();
            closeCallBack_(id_);
        }
        else
        {
            printf("TCPConnection %d Not Set CloseCallBack\n", id_);
        }
    }
}

/**
 * @brief: 状态字符串 
 * @return {*}
 * @param {State} s 状态
 */    
std::string TCPConnection::stateStr(State s)
{
    switch(s)
    {
        case State::CONNECTING : return "CONNECTING"; break;
        case State::CONNECTED : return "CONNECTED"; break;
        case State::DISCONNECTING : return "DISCONNECTING"; break;
        case State::DISCONNECTED : return "DISCONNECTED"; break;
        case State::SHUTDOWN : return "SHUTDOWN"; break;
        default: return "UNKNOWN"; break;
    }
}