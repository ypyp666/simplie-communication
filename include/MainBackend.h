#ifndef MAINBACKEND_H
#define MAINBACKEND_H

#include <QObject>
#include <QString>
#include <QThread>
#include <qstringview.h>
#include "LoginBackend.h"
#include "ChatBackend.h"
#include "DatabaseManager.h"
#include "TcpClient.h"
#include "FeatureStructs.h"  // 共享结构体：ContactInfo / MessageInfo / OutgoingMessage

class MainBackend : public QObject
{
    Q_OBJECT
public:
    MainBackend(QObject* parent = nullptr);
    ~MainBackend();

    ChatBackend* getChatBackend();
    TcpClient* m_tcpClient;
    QString m_userid;
    QString m_password;  
    bool m_loginCompleted; // 登录是否已完成，避免重复登录和无限重连
    DatabaseManager* m_databaseManager;

    static bool s_loggedIn;  // 全局登录状态（定义在 MainBackend.cpp），TCP 断线重连时判断要不要自动重连

    // 获取当前登录账号ID（登录成功后 m_userid 由登录流程写入，UI 通过它拿真实账号）
    QString currentUserId() const { return m_userid; }


signals:
    void loginSuccess(const QString& accountId);
    void loginFailed();
    void registerSuccess();
    void registerFailed();
    void loginWaiting();  // 登录等待中信号
    void loginTimeout();  // 登录连接超时信号
    void sendWaiting();  // 发送等待信号
    void messageSendSuccess(const QString& tempId, const QString& serverId);  // 发送成功（tempId=本地临时消息ID，serverId=服务器分配的ID）
    void messageSendFailed(const QString& tempId, const QString& serverId);  // 发送失败（未连接/服务器拒绝/超时等），UI变红色感叹号

    // === 聊天相关信号（由 ChatBackend 转发）===
    void contactsLoaded(const QList<ContactInfo>& contacts);
    void messagesLoaded(const QList<MessageInfo>& messages);
    void messageReceived(const MessageInfo& message);                // 接收成功（对方发来的新消息，UI显示+存库+回ACK）
    void messageReceiveFailed(const QString& serverId);              // 接收失败（携带服务器消息ID，回ACK让服务器重发）

    // === 数据库异步请求信号（主线程发出 → 后台DB线程执行）===
    void dbSaveMessageRequested(const MessageInfo& message);
    void dbSetAccountRequested(const QString& accountId);
    void dbUpdateMessageIDRequested(const QString& contactId, const QString& oldId, const QString& newId);

    // === 数据库结果信号（后台DB线程回传 → 主线程）===
    void messageSaved(bool success);
    void messageIdUpdated(bool success);
    void databaseInitialized(bool success);

public slots:
    void login(const QString& username, const QString& password);
    void registerUser(const QString& username, const QString& password);
    void JsonParsing(const QByteArray packet);
    void sendMessage(const MessageInfo& message);

    // === 聊天相关接口（转发到 ChatBackend）===
    void loadContacts();
    void loadMessages(const QString& contactId);
    void sendFile(const QString& contactId, const QString& filePath);
    // 输入内容记忆功能
    void saveInputContent(const QString& contactId, const QString& content);
    QString getInputContent(const QString& contactId);
    void clearInputContent(const QString& contactId);
    // 断开会话：直接关闭当前 TCP 连接（转发到 TcpClient）
    void disconnectSession();

    // === 数据库异步接口（只发请求信号，不阻塞主线程）===
    void saveMessage(const MessageInfo& message);
    void setCurrentAccountId(const QString& accountId);
    void onDbMessageIdUpdated(const QString& contactId, const QString& oldId, const QString& newId);
    

private slots:
    // 后台数据库线程的结果回传（在【主线程】执行）
    void onDbMessageSaved(bool success);
    void onDbInitialized(bool success);
    // 收到对方消息：回ACK + 存库 + 转发给UI
    void onMessageReceived(const MessageInfo& message);
    // 接收失败：发拉取请求 + 转发给UI
    void onMessageReceiveFailed(const QString& serverId);

    // === TcpClient 共享信号统一路由（胶水层，分发给各后端）===
    void onTcpConnected();
    void onTcpDisconnected();
    void onTcpError(QAbstractSocket::SocketError error);
    void onTcpConnectionTimeout();
  

private:
    LoginBackend* m_loginBackend;
    ChatBackend* m_chatBackend;
    QThread* m_dbThread;  // 后台数据库线程
};

#endif // MAINBACKEND_H
