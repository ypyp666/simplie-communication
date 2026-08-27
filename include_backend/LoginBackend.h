#pragma once
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QAbstractSocket>
#include "TcpClient.h"

class LoginBackend : public QObject
{
    Q_OBJECT
public:
    explicit LoginBackend(QObject* parent = nullptr, TcpClient* tcpclient = nullptr);
    ~LoginBackend();
    void onTcpDataReceived(const QByteArray& data);
    void startLogin(const QString& accountId, const QString& password);

signals:
    void loginSuccess(const QString& accountId);
    void loginFailed();
    void loginWaiting();
    void loginTimeout();

public slots:
    // TcpClient 信号由主后端统一路由后调用（不再直连信号）
    void onTcpConnected();
    void onTcpDisconnected();
    void onTcpError(QAbstractSocket::SocketError error);
    void onTcpConnectionTimeout();

private:
    TcpClient* m_tcpClient;
    QString m_accountId;
    QString m_password;
    bool m_loginCompleted;
    bool m_ownTcpClient;  // 是否自己创建的TcpClient

    void sendLoginRequest();
};
