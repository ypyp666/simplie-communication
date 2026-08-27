#include "ChatBackend.h"
#include "TcpClient.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <qstringview.h>

ChatBackend::ChatBackend(QObject *parent,TcpClient* tcpclient) : QObject(parent),m_tcpClient(tcpclient)
{
    m_pullTimer = new QTimer(this);
    //m_pullTimer->setInterval(30000);设置30秒触发一次，每次触发后30秒再触发一次
    m_pullTimer->setSingleShot(true);
    // 注意：TcpClient 的信号槽统一由主后端（MainBackend）路由分发，
    // 这里不再直连信号，避免多个后端重复监听共享信号
    // 断线时发送失败由 UI 红色感叹号提示，用户手动点击重发，不做自动重连补发
    connect(m_pullTimer, &QTimer::timeout, this, &ChatBackend::onPullTimerTimeout);
}

// 登录成功后由主后端同步当前登录账号（拉取重试 onPullTimerTimeout 需要用到 m_userid）
void ChatBackend::setUserId(const QString& userId)
{
    m_userid = userId;
}

void ChatBackend::loadContacts()
{
    QList<ContactInfo> contacts;

    ContactInfo c1;
    c1.id = "12345";
    c1.name = "张三";
    c1.avatar = "";
    c1.lastMessage = "明天一起吃饭？";
    c1.lastTime = QDateTime::currentDateTime().addSecs(-15 * 60);
    c1.isOnline = true;
    c1.unreadCount = 2;
    contacts.append(c1);

    ContactInfo c2;
    c2.id = "10010";
    c2.name = "管理员大人";
    c2.avatar = "";
    c2.lastMessage = "你好，我是管理员";
    c2.lastTime = QDateTime::currentDateTime().addSecs(-10 * 60);
    c2.isOnline = false;
    c2.unreadCount = 0;
    contacts.append(c2);
    emit contactsLoaded(contacts);
}

void ChatBackend::loadMessages(const QString& contactId)
{
    QList<MessageInfo> messages;

    QString currentAccount = "self";

    MessageInfo m1;
    m1.id = "msg1";
    m1.accountId = currentAccount;
    m1.senderId = contactId;
    m1.targetId = currentAccount;
    m1.content = "嗨，最近忙什么呢？";
    m1.sendTime = QDateTime::currentDateTime().addSecs(-60 * 60);
    m1.isSelf = false;
    m1.isFile = false;
    m1.isOffline = false;
    messages.append(m1);

    MessageInfo m2;
    m2.id = "msg2";
    m2.accountId = currentAccount;
    m2.senderId = currentAccount;
    m2.targetId = contactId;
    m2.content = "没什么，在写一个聊天软件";
    m2.sendTime = QDateTime::currentDateTime().addSecs(-60 * 60 + 5 * 60);
    m2.isSelf = true;
    m2.isFile = false;
    m2.isOffline = false;
    messages.append(m2);

    MessageInfo m3;
    m3.id = "msg3";
    m3.accountId = currentAccount;
    m3.senderId = contactId;
    m3.targetId = currentAccount;
    m3.content = "听起来不错！用什么语言写的？";
    m3.sendTime = QDateTime::currentDateTime().addSecs(-60 * 60 + 10 * 60);
    m3.isSelf = false;
    m3.isFile = false;
    m3.isOffline = false;
    messages.append(m3);

    MessageInfo m4;
    m4.id = "msg4";
    m4.accountId = currentAccount;
    m4.senderId = currentAccount;
    m4.targetId = contactId;
    m4.content = "C++ 和 Qt，全栈开发";
    m4.sendTime = QDateTime::currentDateTime().addSecs(-60 * 60 + 15 * 60);
    m4.isSelf = true;
    m4.isFile = false;
    m4.isOffline = false;
    messages.append(m4);

    MessageInfo m5;
    m5.id = "msg5";
    m5.accountId = currentAccount;
    m5.senderId = currentAccount;
    m5.targetId = contactId;
    m5.fileName = "project_doc.pdf";
    m5.filePath = "/documents/project_doc.pdf";
    m5.fileSize = 1024 * 1024 * 2;
    m5.sendTime = QDateTime::currentDateTime().addSecs(-30 * 60);
    m5.isSelf = true;
    m5.isFile = true;
    m5.isOffline = false;
    messages.append(m5);

    MessageInfo m6;
    m6.id = "msg6";
    m6.accountId = currentAccount;
    m6.senderId = contactId;
    m6.targetId = currentAccount;
    m6.content = "明天一起吃饭？";
    m6.sendTime = QDateTime::currentDateTime().addSecs(-15 * 60);
    m6.isSelf = false;
    m6.isFile = false;
    m6.isOffline = false;
    messages.append(m6);

    emit messagesLoaded(messages);
}

void ChatBackend::startSendMessage(const QString& contactId, const OutgoingMessage& message)
{
    if (!m_tcpClient->isConnected()) {
        // 未连接：只自动重连，不自动重发这条消息（重发交给用户手动点击）
        qDebug() << "TCP连接未建立，触发自动重连（不重发当前消息）";
        m_tcpClient->reconnect();
        emit sendFailed(message.tempId, "");  // 未连接，拿不到服务器ID，UI显示红色感叹号待用户重发
        return;
    }
    sendMessage(contactId, message);
}

void ChatBackend::sendMessage(const QString& contactId, const OutgoingMessage& message)
{
    QJsonObject messageJson;
    messageJson["type"] = "repost";
    messageJson["accountId"] = message.accountId;
    messageJson["sendId"] = message.senderId;
    messageJson["targetId"] = contactId;
    messageJson["content"] = message.content;
    messageJson["sendTime"] = message.sendTime.toString(Qt::ISODate);  // QDateTime → ISO字符串
    messageJson["tempId"] = message.tempId;  // 临时ID
    QJsonDocument doc(messageJson);
    QByteArray messageJsonStr = doc.toJson(QJsonDocument::Compact);
    qDebug() << "发送消息：" << messageJsonStr;
    QByteArray packet = messageJsonStr + "\n";
    m_tcpClient->sendData(packet);

}

void ChatBackend::sendFile(const QString& contactId, const QString& filePath)
{
    Q_UNUSED(contactId);
    Q_UNUSED(filePath);
}

void ChatBackend::markMessagesAsRead(const QString& contactId)
{
    Q_UNUSED(contactId);
}

void ChatBackend::connectToServer()
{
}

void ChatBackend::disconnectFromServer()
{
}


// 保存输入框内容
void ChatBackend::saveInputContent(const QString& contactId, const QString& content)
{
    if (!content.isEmpty()) {
        inputCache[contactId] = content;
    } else {
        inputCache.remove(contactId);
    }
}

// 获取输入框内容
QString ChatBackend::getInputContent(const QString& contactId)
{
    return inputCache.value(contactId, "");
}

// 清除输入框内容（发送消息后调用）
void ChatBackend::clearInputContent(const QString& contactId)
{
    inputCache.remove(contactId);
}

void ChatBackend::onTcpDisconnected()
{
    qDebug() << "TCP连接已断开";
}

void ChatBackend::onTcpError(QAbstractSocket::SocketError error)
{
    qDebug() << "TCP连接错误：" << m_tcpClient->errorString();
}

void ChatBackend::onTcpConnectionTimeout()
{
    qDebug() << "TCP连接超时";
}
void ChatBackend::onTcpDataReceived(const QByteArray& packet)
{
    qDebug() << "[ChatBackend::onTcpDataReceived] 收到原始包:" << packet;  // 调试用：完整打印服务器回包
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(packet, &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析失败:" << error.errorString();
        emit sendFailed("", "");  // 包损坏拿不到tempId和serverId，UI找不到对应气泡会忽略
        return;
    }
    QJsonObject response = doc.object();
    QString type = response["type"].toString();
    if (type == "repost_response") {
       bool success = response["success"].toBool();
       QString tempId = response["tempId"].toString();    // 服务器原样回传的临时ID
       QString serverId = response["serverId"].toString();  // 服务器分配的消息ID
       qDebug() << "收到发消息确认(sendSuccess) success=" << success
                << "tempId=" << tempId << "serverId=" << serverId;
       if (success) {
           emit sendSuccess(tempId, serverId);
       } else {
           emit sendFailed(tempId, serverId);
       }
    }
}
void ChatBackend::onTcpRepost(const QByteArray& packet)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(packet, &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析失败:" << error.errorString();
        return;
    }
    QJsonObject messageJson = doc.object();
    QString type = messageJson["type"].toString();
    if (type == "repost") {
        MessageInfo message;
        message.id = messageJson["ID"].toString();           // 服务器分配的消息ID
        message.accountId = messageJson["accountId"].toString();
        message.senderId = messageJson["sendId"].toString();
        message.targetId = messageJson["targetId"].toString();
        message.content = messageJson["content"].toString();
        message.sendTime = QDateTime::fromString(messageJson["sendTime"].toString(), Qt::ISODate);
        message.contactId = message.senderId;  // 对方发来的消息，对话对方=发送者
        message.isSelf = false;                // 对方发的，不是自己发的
        message.isRead = false;               // 收到时默认未读
        message.isFile = false;
        message.isOffline = false;
        emit newMessageReceived(message);
    }
}

// 回复接收确认：收到消息后回ACK给服务器（成功=true移除缓存，失败=false让服务器重发）
void ChatBackend::sendReceiveAck(const QString& serverId, bool success)
{
    QJsonObject ack;
    ack["type"] = "receive_ack";
    ack["success"] = success;
    ack["serverId"] = serverId;  // 服务器分配的消息ID
    QJsonDocument doc(ack);
    QByteArray packet = doc.toJson(QJsonDocument::Compact) + "\n";
    qDebug() << "回复接收确认：" << packet;
    m_tcpClient->sendData(packet);
}

// 发送拉取请求：接收失败时向服务器请求重发未收到的缓存消息
void ChatBackend::sendPullRequest(const QString& targetId)
{
    //m_pullTimer->start(30000);//30秒触发一次（暂时不用拉取重试计时，注释掉）
    QJsonObject pullReq;
    pullReq["type"] = "pull_msg";
    pullReq["targetId"] = targetId;  // 拉取发给自己的消息
    QJsonDocument doc(pullReq);
    QByteArray packet = doc.toJson(QJsonDocument::Compact) + "\n";
    qDebug() << "发送拉取请求：" << packet;
    m_tcpClient->sendData(packet);
}

void ChatBackend::onPullTimerTimeout()
{ 
    pullCount++;//拉取请求次数增加
    if (pullCount <=3) {//最多3次
        qDebug() << "拉取请求次数增加：" << pullCount;

        sendPullRequest(m_userid);//重新发送拉取请求
    }
    else {
        m_pullTimer->stop();//停止定时器
        m_tcpClient->disconnectFromServer();//断开TCP连接
    }
}

// 拉取请求确认：服务器把该账号所有缓存消息重发完后发来确认包
// 【内容由你实现】建议：收到确认说明本次拉取已完成，停止定时器并重置次数，避免继续重拉
void ChatBackend::onPullResponse(const QByteArray& packet)
{
    Q_UNUSED(packet);
    m_pullTimer->stop();
    pullCount = 0;QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(packet, &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析失败:" << error.errorString();
        return;
    }
    QJsonObject response = doc.object();
    QString type = response["type"].toString();
    QString serverId = response["Id"].toString();  // 服务器分配的消息ID
    if (type == "pull_response") {
        bool success = response["success"].toBool();
        if (success) {
            qDebug() << "拉取请求成功";
            MessageInfo message;
            message.id = response["ID"].toString();           // 服务器分配的消息ID
            message.accountId = response["accountId"].toString();
            message.senderId = response["sendId"].toString();
            message.targetId = response["targetId"].toString();
            message.content = response["content"].toString();
            message.sendTime = QDateTime::fromString(response["sendTime"].toString(), Qt::ISODate);
            message.contactId = message.senderId;            // 对方发来的消息，对话对方=发送者
            message.isSelf = false;                           // 对方发的，不是自己发的
            message.isRead = false;                           // 收到时默认未读
            message.isFile = false;
            message.isOffline = false;
            emit newMessageReceived(message);
        } else {
            qDebug() << "拉取请求失败";
        }
    }
    // TODO: 用户实现，例如：
    // m_pullTimer->stop();   // 拉取完成，停止30秒重试定时器
    // pullCount = 0;         // 重置重试次数
    // 也可先解析 packet 校验 type 是否为 pull_response 再处理
}
