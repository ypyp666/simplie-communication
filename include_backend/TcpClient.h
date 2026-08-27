#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QByteArray>
#include <QTimer>


class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject* parent = nullptr);
    ~TcpClient();

    void connectToServer(const QString& host, int port);
    void reconnect();  // 用上次保存的主机/端口重新连接
    void disconnectFromServer();
    void sendData(const QByteArray& data);
    bool isConnected() const;
    QString errorString() const;

signals:
    void connected();
    void disconnected();
    void dataReceived(const QByteArray& data);
    void errorOccurred(QAbstractSocket::SocketError error);
    void connectionTimeout();

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);
    void onConnectTimeout();

private:
    QTcpSocket* m_socket;
    QByteArray m_recvBuffer;
    QTimer* m_connectTimeoutTimer;
    QString m_host;

    int m_port;
};

#endif // TCPCLIENT_H
