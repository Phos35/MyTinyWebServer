# HTTP报文解析

# 一、主从状态机整体

![](http://43.138.43.178:8000/image/HTTPRequestParser.png)





## 二、从状态机

- ![](http://43.138.43.178:8000/image/SubHTTPRequestParser.png)
- 从状态机每次解析一行，当成功解析一行后其状态将为LINE_OPEN，并从其设置的curIndex和lineStart即可获取到完整的一行



## 三、主状态机

- ![](http://43.138.43.178:8000/image/MainHTTPRequestParser.png)
- 主状态机获取从状态机完整解析的行后根据所在部分的格式（请求行、请求头、请求体）进行解析
  - 请求行
    一行，空格分隔的三个部分：请求方法、URL、协议版本
  - 请求头
    "key: value"形式的键值对，以/r/n的空行为结束
  - 请求体
    格式任意的内容部分，长度限制为头部字段中的"Content-Length"的值