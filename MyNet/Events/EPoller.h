#ifndef EPOLLER_H
#define EPOLLER_H
#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include "Timestamp.h"
#include "Channel.h"

// 前向声明
class EventLoop;

class EPoller{
public:
    // 触发模式
    enum Trigger
    {
        LT,
        ET
    };

    EPoller(EventLoop *loop);
    ~EPoller();

    /**
     * @brief: 检查发生的事件
     * @return {*}
     * @param {List&} activeChannels 存储发生活跃事件的Channel
     * @param {uint32_t} ms 超时间隔；-1表示阻塞
     * @param {Timestamp&} time 事件发生事件
     */    
    int poll(Channel::List& activeChannels, uint32_t ms, Timestamp& time);

    /**
     * @brief: 更新Channel(包括添加和更新)
     * @return {*}
     * @param {Channel} *channel 待更新的Channel
     */    
    void updateChannel(Channel *channel);

    /**
     * @brief: 移除Channel
     * @return {*}
     * @param {Channel*} channel 待移除的Channel
     */    
    void removeChannel(Channel* channel);

    /**
     * @brief: 获取epoll事件的字符串
     * @return {*}
     * @param {uint32_t} event 事件
     */    
    static std::string eventStr(uint32_t event);

private:
    typedef std::vector<epoll_event> EventList;
    typedef std::unordered_map<int, Channel *> ChannelMap;

    static int ACTIVE_EVENTS_MIN_SIZE;  // activeEvents_的最小（初始大小）

    EventLoop *loop_;            // EPoller 所属的事件循环
    int epfd_;                   // epoll fd
    EventList activeEvents_;     // 存储活跃事件
    ChannelMap focusedChannels_; // EPoller 关注的Channel

    int littleEventsTimes;       // 填充事件数量小于activeEvents一半的次数

    /**
     * @brief: 将活动的事件填充至对应的Channel中
     * @return {*}
     * @param {int} eventNr 发生事件的数量
     * @param {List} &activeChannels 待填充的Channel列表
     */    
    void fillActiveChannels(int eventNr, Channel::List &activeChannels);

    /**
     * @brief: 更新指定Channel上的事件
     * @return {*}
     * @param {int} op 操作
     * @param {Channel*} channel 待更新的Channel
     */    
    void update(int op, Channel* channel);

    /**
     * @brief: 获取执行的操作的字符串
     * @return {*}
     * @param {int} op 操作
     */    
    std::string operationStr(int op);
};

#endif