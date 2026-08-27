#pragma once
#include <mutex>
#include <unordered_map>
#include <string>
#include <memory>

struct Session
{
    std::string account;    // 登录账号，未登录为空
    int fd = -1;            // socket文件描述符
    std::string clientIp;   // 客户端IP地址
    bool state=false;       //登录状态，只有登录上才可以进行其他操作
};
using SessionPtr = std::shared_ptr<Session>;

class OnlineSessionManager {
    public:
    static OnlineSessionManager& Instance();

    OnlineSessionManager(const OnlineSessionManager&) = delete;
    OnlineSessionManager& operator=(const OnlineSessionManager&) = delete;
    // 添加在线会话（登录成功调用）
    void addSession(const std::string& account, SessionPtr sess);

    // 根据账号查找会话（发消息时用）
    SessionPtr findSession(const std::string& account);

    // 用户下线，移除会话
    void removeSession(const std::string& account);

    ~OnlineSessionManager()=default;

    private:
    OnlineSessionManager()=default;

    std::mutex m_mtx;//互斥锁对象，一个 mutex 同一时间只允许一个线程拿到锁；别的线程再来拿锁，就会卡住阻塞，直到别人释放锁。
    // key:账号 value:会话智能指针
    std::unordered_map<std::string, SessionPtr> m_onlineMap;

};