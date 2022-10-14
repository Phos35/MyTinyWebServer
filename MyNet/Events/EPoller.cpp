#include "EPoller.h"
#include "AsyncLog.h"
#include "EventLoop.h"
#include <unistd.h>
#include <sys/epoll.h>
#include <assert.h>

// activeEvents的初始大小
int EPoller::ACTIVE_EVENTS_MIN_SIZE = 16;

EPoller::EPoller(EventLoop *loop)
:loop_(loop), activeEvents_(16), littleEventsTimes(0)
{
    // 创建epfd
    epfd_ = epoll_create(5);
    if(epfd_ == -1)
    {
        perror("EPollfd Create Failed");
        exit(-1);
    }
}

/**
 * @brief: 检查发生的事件
 * @return {*}
 * @param {List&} activeChannels 存储发生活跃事件的Channel
 * @param {uint32_t} ms 超时间隔；-1表示阻塞
 * @param {Timestamp&} time 事件发生事件
 */    
int EPoller::poll(Channel::List& activeChannels, uint32_t ms, Timestamp& time)
{
    int nr = epoll_wait(epfd_, activeEvents_.data(), activeEvents_.size(), ms);
    
    // 非信号中断错误
    time = Timestamp::now();
    if (nr == -1 && errno != EINTR)
    {
        LOG_ERROR << time.format("%y-%m-%d %H:%M:%S.%s") << "EPOLL_WAIT ERROR" << strerror(errno);
    }
    // 超时
    else if(nr == 0)
    {
        LOG_WARN << time.format("%y-%m-%d %H:%M:%S.%s") << "EPOLL_WAIT Timeout";
    }
    // 记录事件
    else
    {
        LOG_TRACE << " happend " << nr << " events, "
                  << "total fd nr: " << static_cast<int>(focusedChannels_.size())
                  << ", activeEvetnts size : " << (int)activeEvents_.size();
        // printf("Happend %d events\n", nr);

        // 填充事件
        fillActiveChannels(nr, activeChannels);

        /*动态伸缩*/
        int size = activeEvents_.size();
        // 此次事件填满activeEvents，大小扩展至原来的2倍
        if (nr == size)
            activeEvents_.resize(size * 2);
        // 3次事件小于activeEvents的一半，缩小activeEvents至原来的一半,
        // 但不能小于初始大小
        else if(nr < size / 2 && nr > ACTIVE_EVENTS_MIN_SIZE)
        {
            ++littleEventsTimes;
            if(littleEventsTimes == 3)
            {
                activeChannels.resize(std::max(size / 2, ACTIVE_EVENTS_MIN_SIZE));
                littleEventsTimes = 0;
            }
        }
    }

    return nr;
}

/**
 * @brief: 更新Channel(包括添加和更新)
 * @return {*}
 * @param {Channel} *channel 待更新的Channel
 */    
void EPoller::updateChannel(Channel *channel)
{
    // 检验是否在事件循环所属的线程中
    loop_->assertInLoopThread();

    // 若channel尚未加入到EPoller关注的列表中，则进行添加
    if(channel->state() == Channel::State::NOT_ADD)
    {
        // 修改channel状态并添加到关注列表中
        channel->setState(Channel::State::ADDED);
        focusedChannels_.insert({channel->fd(), channel});

        // 关注Channel的事件
        update(EPOLL_CTL_ADD, channel);
    }
    // 若已添加，则进行更新
    else
    {
        assert(focusedChannels_.find(channel->fd()) != focusedChannels_.end() 
               && "Update Channel Not Exist");

        // 更新事件
        update(EPOLL_CTL_MOD, channel);
    }
}

/**
 * @brief: 移除Channel
 * @return {*}
 * @param {Channel*} channel 待移除的Channel
 */    
void EPoller::removeChannel(Channel* channel)
{
    // 检验是否在事件循环所属的线程中
    loop_->assertInLoopThread();

    assert(channel->state() == Channel::State::ADDED);

    // 设置Channel的状态为NOT_ADD
    channel->setState(Channel::State::NOT_ADD);

    // 从EPoller关注的Channel中移除channel
    int nr = focusedChannels_.erase(channel->fd());
    assert(nr == 1);

    // 取消关注的事件
    update(EPOLL_CTL_DEL, channel);
}

/**
 * @brief: 将活动的事件填充至对应的Channel中
 * @return {*}
 * @param {int} eventNr 发生事件的数量
 * @param {List} &activeChannels 待填充的Channel列表
 */    
void EPoller::fillActiveChannels(int eventNr, Channel::List &activeChannels)
{
    for (auto itr = activeEvents_.begin(); itr != activeEvents_.end() && eventNr > 0; itr++)
    {
        // 获取事件对应的Channel
        Channel *channel = static_cast<Channel*>(itr->data.ptr);

        // 设置Channel的活动事件并加入到activeCahnnels中
        channel->setActiveEvents(itr->events);
        activeChannels.push_back(channel);

        eventNr--;
    }

    assert(eventNr == 0);
}

/**
 * @brief: 更新指定Channel上的事件
 * @return {*}
 * @param {int} op 操作
 * @param {Channel*} channel 待更新的Channel
 */    
void EPoller::update(int op, Channel* channel)
{
    // 获取fd和对应的事件
    int fd = channel->fd();
    uint32_t events = channel->focusedEvents();

    // 记录epoll_ctl操作
    LOG_TRACE << "EPOLL_CTL_" << operationStr(op) << ", fd = " 
              << fd << ", events = " << eventStr(events);

    // 创建epoll_event并进行更新
    epoll_event e = {
        .events = events,
        .data = {channel}
    };
    int ret = epoll_ctl(epfd_, op, fd, &e);
    if(ret == -1)
    {
        LOG_ERROR << fd << " epoll_ctl failed: " << strerror(errno);
    }
}

/**
 * @brief: 获取执行的操作的字符串
 * @return {*}
 * @param {int} op 操作
 */    
std::string EPoller::operationStr(int op)
{
    switch(op)
    {
        case EPOLL_CTL_ADD: return "ADD"; break;
        case EPOLL_CTL_MOD: return "MOD"; break;
        case EPOLL_CTL_DEL: return "DEL"; break;
        default: return "UNKNOWN"; break;
    }
}

/**
 * @brief: 获取epoll事件的字符串
 * @return {*}
 * @param {int} event 事件
 */    
std::string EPoller::eventStr(uint32_t event)
{
    std::string ret = "{ ";
    if (event & EPOLLIN) ret += "EPOLLIN, ";
    if (event & EPOLLOUT) ret += "EPOLLOUT, ";
    if (event & EPOLLRDHUP) ret += "EPOLLRDHUP, ";
    if (event & EPOLLPRI) ret += "EPOLLPRI, ";
    if (event & EPOLLERR) ret += "EPOLLERR, ";
    if (event & EPOLLHUP) ret += "EPOLLHUP, ";
    if (event & EPOLLET) ret += "EPOLLET, ";
    if (event & EPOLLONESHOT) ret += "EPOLLONESHOT, ";
    if (event & EPOLLWAKEUP) ret += "EPOLLWAKEUP, ";
    if (event & EPOLLEXCLUSIVE) ret += "EPOLLEXCLUSIVE, ";
    ret += "}";
    return ret;
}

EPoller::~EPoller()
{
    close(epfd_);
}