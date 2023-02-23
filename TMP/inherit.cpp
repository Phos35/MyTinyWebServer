// #include <memory>
// #include <unordered_map>

// class Parent: public std::enable_shared_from_this<Parent>
// {
// public:
//     Parent(int content = 0)
//         : content_(content)
//     {
//     }

//     virtual void handle()
//     {
//         printf("Parent Handle\n");
//     }

//     int content() { return content_; }

// private:
//     int content_;
// };

// class Child : public Parent
// {
// public:
//     Child(int content = -1)
//     :Parent(content)
//     {

//     }

//     void handle() override
//     {
//         printf("Child Handle\n");
//     }

// private:
// };

// class ParentServer
// {
// public:
//     int id;
//     std::unordered_map<int, std::shared_ptr<Parent>> conns;

//     ParentServer():id(0){}

//     void addConn(std::shared_ptr<Parent>& conn)
//     {
//         printf("Conn %d ", id);
//         conn->handle();
//         conns[id++] = conn;
//     }

//     virtual void onNewConn(int content)
//     {
//         std::shared_ptr<Parent> conn(new Parent(content));
//         printf("Parent Server, ");
//         addConn(conn);
//     }

// private:
// };

// class ChildServer : public ParentServer
// {
// public:
//     void onNewConn(int content) override
//     {
//         std::shared_ptr<Parent> conn(new Child(content));
//         printf("Child Server, ");
//         addConn(conn);
//     }

// private:

// };

// int main()
// {
//     ParentServer p;
//     ChildServer c;
//     int content = 0;
//     while(content != -1)
//     {
//         printf("Please input a val: ");
//         scanf("%d", &content);
//         p.onNewConn(content);
//         c.onNewConn(content);
//     }

//     return 0;
// }