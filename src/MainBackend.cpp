#include "MainBackend.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <qstringview.h>


bool MainBackend::s_loggedIn = false;  // 全局登录状态的静态定义

MainBackend::MainBackend(QObject* parent)
    : QObject(parent)
{
    // ===== 数据库后台线程初始化 =====
    // 注册跨线程信号槽需要传递的自定义类型（不注册，QueuedConnection 无法传递）
    qRegisterMetaType<MessageInfo>();//告诉Qt，MessageInfo是一个自定义类型，需要注册一下
    qRegisterMetaType<QList<MessageInfo>>();//告诉Qt，QList<MessageInfo>是一个自定义类型，需要注册一下

    // 1. 创建后台数据库线程
    m_dbThread = new QThread(this);
    m_dbThread->setObjectName("DbWorkerThread");

    // 2. 把 DatabaseManager 单例整个搬到后台线程"住下"
    //    moveToThread 必须发生在 connect 之前！
    //    之后它的所有槽函数都会在后台线程执行，数据库连接也都在后台线程创建，
    //    主线程只管发请求信号，DB 再慢也不卡 UI
    m_databaseManager = &DatabaseManager::instance();
    m_databaseManager->moveToThread(m_dbThread);

    // 3. 启动线程（started 事件循环跑起来后，排队过来的请求才会被处理）
    m_dbThread->start();

    // 4. 请求信号（主线程）→ DatabaseManager 槽（后台线程）：跨线程自动 QueuedConnection
    connect(this, &MainBackend::dbSaveMessageRequested,
            m_databaseManager, &DatabaseManager::saveMessage);
    connect(this, &MainBackend::dbSetAccountRequested,
            m_databaseManager, &DatabaseManager::setCurrentAccountId);
    connect(this, &MainBackend::dbUpdateMessageIDRequested,
            m_databaseManager, &DatabaseManager::updateMessageId);



    // 5. 结果信号（后台线程发出）→ 本类槽（主线程）：跨线程自动 QueuedConnection
    connect(m_databaseManager, &DatabaseManager::messageSaved,
            this, &MainBackend::onDbMessageSaved);
    connect(m_databaseManager, &DatabaseManager::databaseInitialized,
                    this, &MainBackend::onDbInitialized);
    connect(m_databaseManager, &DatabaseManager::messageIdUpdated,
            this, [=](bool success){
                if(!success) {
                     qDebug() << "更新消息ID失败";
                }
            });
    // 创建登录后端和聊天后端
    m_tcpClient = new TcpClient(this);
    m_loginBackend = new LoginBackend(this,m_tcpClient);
    m_chatBackend = new ChatBackend(this, m_tcpClient);
    
    connect(m_tcpClient, &TcpClient::dataReceived, this, &MainBackend::JsonParsing);
    // TcpClient 共享信号统一路由到主后端，再分发给需要的后端（各后端不再直连）
    connect(m_tcpClient, &TcpClient::connected, this, &MainBackend::onTcpConnected);
    connect(m_tcpClient, &TcpClient::disconnected, this, &MainBackend::onTcpDisconnected);
    connect(m_tcpClient, &TcpClient::errorOccurred, this, &MainBackend::onTcpError);
    connect(m_tcpClient, &TcpClient::connectionTimeout, this, &MainBackend::onTcpConnectionTimeout);
    // 转发 ChatBackend 的发送结果信号（成功/失败都带tempId，UI据此切换气泡状态）
    connect(m_chatBackend, &ChatBackend::sendSuccess,
            this, &MainBackend::messageSendSuccess);
    connect(m_chatBackend, &ChatBackend::sendFailed,
            this, &MainBackend::messageSendFailed);
    // 转发 LoginBackend 的信号到 MainBackend
    connect(m_loginBackend, &LoginBackend::loginSuccess, 
            this, [=](){
                s_loggedIn = true;              // 更新全局登录状态
                m_chatBackend->setUserId(m_userid);  // 同步登录账号给聊天后端（拉取重试需要）
                emit loginSuccess(m_userid);
                // 登录成功即拉取服务器缓存中未确认的消息（首次登录：TCP连上→登录成功→拉取）
                m_chatBackend->sendPullRequest(m_userid);
            });
    connect(m_loginBackend, &LoginBackend::loginFailed, 
            this,[=](){
                s_loggedIn = false;             // 登录失败，重置全局登录状态
                emit loginFailed();
            });
    connect(m_loginBackend, &LoginBackend::loginWaiting, 
            this, [=](){
                emit loginWaiting();
            });
    connect(m_loginBackend, &LoginBackend::loginTimeout, 
            this, [=](){
                emit loginTimeout();
            });
    // 转发 ChatBackend 的聊天相关信号
    connect(m_chatBackend, &ChatBackend::contactsLoaded,
            this, &MainBackend::contactsLoaded);
    connect(m_chatBackend, &ChatBackend::messagesLoaded,
            this, &MainBackend::messagesLoaded);
    connect(m_chatBackend, &ChatBackend::newMessageReceived,
            this, &MainBackend::onMessageReceived);
    connect(m_chatBackend, &ChatBackend::messageReceiveFailed,
            this, &MainBackend::onMessageReceiveFailed);
}

MainBackend::~MainBackend()
{
    // 退出后台数据库线程，防止程序退出时线程还在跑
    if (m_dbThread) {
        m_dbThread->quit();      // 通知事件循环退出
        m_dbThread->wait(3000);  // 等待线程真正结束
    }
    // 析构时自动清理子对象（通过 Qt 的父子机制）
}

ChatBackend* MainBackend::getChatBackend()
{
    return m_chatBackend;
}

void MainBackend::login(const QString& userId, const QString& password)
{
    m_userid = userId;
    m_password = password;
    s_loggedIn = false;  // 新的登录会话开始，重置全局登录状态
    // 转发调用到 LoginBackend
    m_loginBackend->startLogin(userId, password);
}

void MainBackend::sendMessage(const MessageInfo& message)
{
    OutgoingMessage out;
    out.type = "repost";               // 告诉服务器这是一个转发消息
    out.accountId = message.accountId; // 当前登录账号ID
    out.senderId = message.senderId;
    out.targetId = message.targetId;
    out.content = message.content;
    out.sendTime = message.sendTime;
    out.tempId = message.id;
    emit sendWaiting();
    m_chatBackend->startSendMessage(message.contactId, out);

    saveMessage(message);  // 异步保存到数据库（后台线程执行，不阻塞UI）
}

// ===== 数据库异步接口 =====
// 只发请求信号，实际执行在后台数据库线程，主线程不等待、不阻塞

// 异步保存消息
void MainBackend::saveMessage(const MessageInfo& message)
{
    emit dbSaveMessageRequested(message);
}

// 异步切换当前账号（后台线程打开对应账号的消息库）
void MainBackend::setCurrentAccountId(const QString& accountId)
{
    emit dbSetAccountRequested(accountId);
}

// 后台数据库线程回传：消息保存结果（在主线程执行）
void MainBackend::onDbMessageSaved(bool success)
{
    qDebug() << "数据库保存消息结果:" << success;
    emit messageSaved(success);
}

// 后台数据库线程回传：数据库初始化结果（在主线程执行）
void MainBackend::onDbInitialized(bool success)
{
    qDebug() << "数据库初始化结果:" << success;
    emit databaseInitialized(success);
}

// 收到对方消息：回ACK（胶水转发） + 存库 + 转发给UI
void MainBackend::onMessageReceived(const MessageInfo& message)
{
    // 1. 回ACK的业务逻辑在 ChatBackend，主后端只做胶水转发
    m_chatBackend->sendReceiveAck(message.id, true);

    // 2. 存库由 ChatWindow 统一处理（按已读状态覆盖），主后端不再重复保存
    // 3. 转发给UI显示
    emit messageReceived(message);
}

// 接收失败：发拉取请求（业务在 ChatBackend）+ 转发给UI
void MainBackend::onMessageReceiveFailed(const QString& serverId)
{
    // 1. 拉取请求的业务逻辑在 ChatBackend，主后端只做胶水转发
    m_chatBackend->sendPullRequest(m_userid);  // 拉取发给自己的消息

    // 2. 转发给UI（如果需要提示用户）
    emit messageReceiveFailed(serverId);
}

void MainBackend::onDbMessageIdUpdated(const QString& contactId, const QString& oldId, const QString& newId)
{
    emit dbUpdateMessageIDRequested(contactId, oldId, newId);
}

void MainBackend::loadContacts()
{
    m_chatBackend->loadContacts();
}

void MainBackend::loadMessages(const QString& contactId)
{
    m_chatBackend->loadMessages(contactId);
}

void MainBackend::sendFile(const QString& contactId, const QString& filePath)
{
    m_chatBackend->sendFile(contactId, filePath);
}

void MainBackend::saveInputContent(const QString& contactId, const QString& content)
{
    m_chatBackend->saveInputContent(contactId, content);
}

QString MainBackend::getInputContent(const QString& contactId)
{
    return m_chatBackend->getInputContent(contactId);
}

void MainBackend::clearInputContent(const QString& contactId)
{
    m_chatBackend->clearInputContent(contactId);
}

// 断开会话：直接关闭当前 TCP 连接
void MainBackend::disconnectSession()
{
    if (m_tcpClient) {
        m_tcpClient->disconnectFromServer();
    }
}

// ===== TcpClient 共享信号统一路由（胶水层） =====
// TCP连接成功：根据全局登录状态决定走"登录"还是直接"拉取"
void MainBackend::onTcpConnected()
{
    if (s_loggedIn) {
        // 已登录过（断线重连场景）：无需重新登录，直接拉取服务器缓存的未确认消息
        m_chatBackend->sendPullRequest(m_userid);

    } else {
        // 未登录（首次登录场景）：交给登录后端发登录请求，登录成功后再拉取
        m_loginBackend->onTcpConnected();
    }
}

// TCP断开：所有需要感知断线的后端都通知到
void MainBackend::onTcpDisconnected()
{
    if (!s_loggedIn) {
        // 已登录过无需再重新登录，直接拉取服务器缓存的未确认消息
        m_loginBackend->onTcpDisconnected();
        return;
    }
    m_chatBackend->onTcpDisconnected();
    m_tcpClient->reconnect();
}

// TCP错误：分发给所有后端
void MainBackend::onTcpError(QAbstractSocket::SocketError error)
{
    m_loginBackend->onTcpError(error);
    m_chatBackend->onTcpError(error);
}

// TCP连接超时：分发给所有后端
void MainBackend::onTcpConnectionTimeout()
{
    if (!s_loggedIn) {
        m_loginBackend->onTcpConnectionTimeout();
        return;
    }
    m_chatBackend->onTcpConnectionTimeout();
}

void MainBackend::registerUser(const QString& username, const QString& password)
{
    Q_UNUSED(username);
    Q_UNUSED(password);
    emit registerSuccess();
}

void MainBackend::JsonParsing(const QByteArray packet)
{
    qDebug() << "收到服务器数据:" << packet;
    
    // 解析JSON响应
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(packet, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析失败:" << error.errorString();
       // emit loginFailed();
        return;
    }
    
    QJsonObject response = doc.object();
    QString type = response["type"].toString();
    if (type == "login_response") {
        m_loginBackend->onTcpDataReceived(packet);
    } else if (type == "repost_response") {
        m_chatBackend->onTcpDataReceived(packet);
    }
    else if (type == "repost") {
        m_chatBackend->onTcpRepost(packet);
    }
    else if (type == "pull_response") {
        // 服务器发完所有缓存消息后发来的拉取确认包，交给聊天后端处理（停止拉取定时器）
        m_chatBackend->onPullResponse(packet);
    }
}