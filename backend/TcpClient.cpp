#include "TcpClient.h"
#include <QDebug>

TcpClient::TcpClient(QObject* parent)
    : QObject(parent), m_port(0)
{
    m_socket = new QTcpSocket(this);
    // 创建 TCP 套接字
    
    m_connectTimeoutTimer = new QTimer(this);
    m_connectTimeoutTimer->setSingleShot(true);//设置为单次触发，避免重复触发
    
    // 连接信号槽
    connect(m_socket, &QTcpSocket::connected, this, &TcpClient::onSocketConnected);
    // 连接成功后,触发时机：connectToHost() 发起 TCP 握手，三次握手全部完成，成功连上服务端时触发
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClient::onSocketDisconnected);
    //触发时机：连接关闭时触发
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClient::onSocketReadyRead);
    //触发时机：操作系统内核缓冲区有服务端发来的数据，Qt 通知你可以读取。
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &TcpClient::onSocketError);
    //触发时机：套接字发生错误时触发，专门捕获所有网络错误，参数会传入错误枚举
    /*
   细讲QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred)这个信号
   可变参数模板是c++11的新特性，用于处理不同参数类型，QOverload是一个模板类，可以绑定到不同参数类型的信号槽。of()方法用于指定参数类型。
    QAbstractSocket里有多个重载版本的errorOccurred信号，用于处理套接字错误。
    直接绑定会有二义性，需要指定参数类型。用of()方法来过滤出我们需要的信号。
   */
    connect(m_connectTimeoutTimer, &QTimer::timeout, this, &TcpClient::onConnectTimeout);
 /*
    向操作系统申请一块 TCP 通信资源；
    创建 Qt 对象，提供信号槽（readyRead、connected、disconnected）异步回调，不卡 UI；
    绑定父对象 MainBackend，生命周期和全局后端同步，程序退出自动释放连接。
    在 Qt 层创建一个管理 TCP 逻辑的对象；
    调用操作系统 socket() 系统调用，在内核分配一个空的套接字文件描述符（fd）；
    初始化收发缓冲区、绑定信号槽。
    */}

TcpClient::~TcpClient()
{
    disconnectFromServer();
         /*
        优雅关闭接口：1.本地发送 FIN 报文，启动四次挥手；2.不会立刻销毁 fd，会先把本地缓冲区剩余数据发送完毕；3.socket 进入 ClosingState（正在关闭）。
        */

        // 确保套接字已关闭，释放资源
        m_socket->deleteLater();
     
     
}

void TcpClient::connectToServer(const QString& host, int port)
{
    m_host = host;
    m_port = port;
    
      // 如果已经连接，先断开（异步）
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
            // 断开后会触发 onSocketDisconnected 信号，在那里重新连接
    }
     // 启动30秒登录超时计时器
    m_connectTimeoutTimer->start(30000);
       // 连接到服务器（IP地址暂时不关心，用本地回环地址测试）
    m_socket->connectToHost(host, port);
}

void TcpClient::reconnect()
{
    // 还没连过（没有可用地址）就直接忽略，避免空重连
    if (m_host.isEmpty() || m_port <= 0) {
        qDebug() << "TcpClient::reconnect: 尚未连接过，无可用地址";
        return;
    }
    connectToServer(m_host, m_port);
}

void TcpClient::disconnectFromServer()
{
    m_connectTimeoutTimer->stop();
    
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
        m_socket->waitForDisconnected();
    }
           //waitForDisconnected()主动阻塞等待连接断开 等待套接字关闭，确保所有数据发送完毕

}

void TcpClient::sendData(const QByteArray& data)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(data);
        m_socket->flush();
    }
}

bool TcpClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

QString TcpClient::errorString() const
{
    return m_socket->errorString();
}

void TcpClient::onSocketConnected()
{
    m_connectTimeoutTimer->stop();
    emit connected();
}

void TcpClient::onSocketDisconnected()
{
    m_connectTimeoutTimer->stop();
    
    if (m_socket->bytesAvailable() > 0) {// 连接断开时，检查是否还有未读取的数据
        m_recvBuffer.append(m_socket->readAll());// 保留读取所有可读数据
        onSocketReadyRead();
    }
    
    emit disconnected();
}

void TcpClient::onSocketReadyRead()
{
    m_recvBuffer.append(m_socket->readAll());// 将新数据追加到接收缓冲区
    
    while (true) {
         // 查找换行符位置
        int newlinePos = m_recvBuffer.indexOf('\n');
        // 如果没有完整的包，等待下次数据
        if (newlinePos == -1) {
            break;
        }
             // 提取一个完整的数据包（不包含换行符）
        QByteArray packet = m_recvBuffer.left(newlinePos);
            // 从缓冲区移除已处理的数据包
        m_recvBuffer.remove(0, newlinePos + 1);
        
        emit dataReceived(packet);
    }
}

void TcpClient::onSocketError(QAbstractSocket::SocketError error)
{
    m_connectTimeoutTimer->stop();
    emit errorOccurred(error);
}

void TcpClient::onConnectTimeout()
{
    if (m_socket->state() == QAbstractSocket::ConnectingState) {
        m_socket->abort();
        emit connectionTimeout();
    }
}
