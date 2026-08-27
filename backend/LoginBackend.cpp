#include "LoginBackend.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

LoginBackend::LoginBackend(QObject* parent, TcpClient* tcpclient)
    : QObject(parent), m_tcpClient(tcpclient), m_loginCompleted(false), m_ownTcpClient(false)
{
    // 使用传入的引用参数初始化TCP客户端
    // 注意：TcpClient 的信号槽统一由主后端（MainBackend）路由分发，
    // 这里不再直连信号，避免多个后端重复监听共享信号
    qDebug() << "LoginBackend: m_tcpClient=" << m_tcpClient;
}

LoginBackend::~LoginBackend()
{
    // 如果是自己创建的TcpClient，需要手动删除
    if (m_tcpClient) {
        m_tcpClient->disconnectFromServer();
        // 不要delete，因为已经设置了parent，Qt会自动清理
    }
}

void LoginBackend::startLogin(const QString& accountId, const QString& password)
{
    // 保存登录凭证
    m_accountId = accountId;
    m_password = password;
    m_loginCompleted = false;
    
    // 连接服务器（异步操作）
    m_tcpClient->connectToServer("192.168.20.128", 8899);
}

void LoginBackend::sendLoginRequest()
{
    // 构建登录数据包（JSON格式）
    QJsonObject loginData;
    loginData["type"] = "login";
    loginData["account"] = m_accountId;
    loginData["password"] = m_password;
    
    // 序列化JSON
    QJsonDocument doc(loginData);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
     /*
    toJson () 核心功能：序列化（对象 → 字节流）
    将 doc 中封装的 JSON 语法树，转换成连续的二进制字节数组 QByteArray，这是 TCP 能发送的合法数据格式。
    Compact	压缩紧凑格式，删除所有换行、空格、制表符,Indented	带换行缩进格式化，方便人阅读调试
    */

    // 添加换行符作为包分隔符（解决TCP粘包问题）
    QByteArray packet = jsonData + '\n';
    
    // 发送数据包
    m_tcpClient->sendData(packet);
    
    qDebug() << "发送登录请求:" << jsonData;
}

void LoginBackend::onTcpConnected()
{
    qDebug() << "TCP连接已建立";
    
    // 连接成功后，发送登录请求
    sendLoginRequest();
}

void LoginBackend::onTcpDisconnected()
{
    qDebug() << "TCP连接已断开";
    
    // 如果登录未完成，尝试重新连接
    if (!m_loginCompleted && !m_accountId.isEmpty() && !m_password.isEmpty()) {
        m_tcpClient->connectToServer("192.168.20.128", 8899);
    }
}

void LoginBackend::onTcpDataReceived(const QByteArray& data)
{
    qDebug() << "收到服务器数据:" << data;
    
    // 解析JSON响应
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析失败:" << error.errorString();
        emit loginFailed();
        return;
    }
    
    QJsonObject response = doc.object();
    QString type = response["type"].toString();
    
    if (type == "login_response") {
        bool success = response["success"].toBool();
        if (success) {
            m_loginCompleted = true;
            emit loginSuccess(m_accountId);
        } else {
            m_loginCompleted = true;
            qDebug() << "登录失败:" << response["message"].toString();
            emit loginFailed();
        }
    }
}

void LoginBackend::onTcpError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    qDebug() << "Socket错误:" << m_tcpClient->errorString();
    emit loginFailed();
}

void LoginBackend::onTcpConnectionTimeout()
{
    qDebug() << "登录连接超时（30秒）";
    emit loginTimeout();
}
