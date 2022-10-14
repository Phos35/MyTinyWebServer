#include "Acceptor.h"
#include "AsyncLog.h"

Acceptor::Acceptor(EventLoop *loop, const Socket &socket)
:loop_(loop), sock_(socket), listenChannel_(socket.fd(), loop)
{
    // 绑定地址和端口
    sock_.bind();
    // 设置端口复用
    sock_.setResueAddr(true);
    // 设置listenChannel的读回调事件
    listenChannel_.setRDCallBack(std::bind(&Acceptor::handleNewConn, this, std::placeholders::_1));
}

/**
 * @brief: 进入监听状态
 * @return {*}
 */    
int Acceptor::listen()
{
    // 断言在事件循环所属的线程中执行
    loop_->assertInLoopThread();

    // Socket进入listen状态，并让listenChannel关注读事件
    int ret = sock_.listen();
    listenChannel_.focusRead();
    return ret;
}

/**
 * @brief: socket 可读时创建新连接
 * @return {*}
 * @param {Timestamp} time fd可读的时刻
 */    
void Acceptor::handleNewConn(Timestamp time)
{
    Socket client = sock_.accept(false);
    client.setTCPNoDelay(true);
    // 接受失败
    if(client.valid() == false)
    {
        perror("Acceptor Accept Failed");
    }
    else
    {
        // 调用连接建立回调函数
        if(newConnCallBack_)
        {
            newConnCallBack_(client);
        }
        // 未设置连接建立回调函数，进行提示
        else
        {
            printf("Acceptor's NewConnCallBack Not Set\n");
        }
    }
}

Acceptor::~Acceptor()
{
    // 移除Channel
    listenChannel_.unfocusAll();
    listenChannel_.remove();
}