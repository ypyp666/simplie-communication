#ifndef CHATAREA_H
#define CHATAREA_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QWidget>
#include <QLabel>
#include <QMap>
#include "ChatBackend.h"
#include "MessageStatusIndicator.h"

class ChatInput;
class MessageItem;

class ChatArea : public QWidget
{
    Q_OBJECT
public:
    explicit ChatArea(QWidget *parent = nullptr);
    void setMessages(const QList<MessageInfo>& messages);
    void setContactName(const QString& name);
    void addMessage(const MessageInfo& message);
    void setMessageStatus(const QString& messageId, MessageStatusIndicator::Status status);  // 按消息ID切换发送状态（发送中/失败）
    void clearMessages();
    void clearInput();  // 清空输入框
    void setInputVisible(bool visible);  // 设置输入框可见性
    void setInputContent(const QString& content);  // 设置输入框内容
    QString getInputContent();  // 获取输入框内容

signals:
    void sendMessage(const QString& content);
    void sendFile(const QString& filePath);
    void retrySend(const QString& messageId);  // 用户点击失败感叹号 → 请求重发该消息

private:
    QVBoxLayout* mainLayout;
    QVBoxLayout* messagesLayout;
    QScrollArea* scrollArea;
    QWidget* messagesWidget;
    QLabel* headerLabel;
    QWidget* headerLabelWidget;
    QHBoxLayout* headerLayout;
    QLabel* headerOnlineLabel;
    ChatInput* chatInput;  // 输入框作为聊天区域的一部分
    QMap<QString, MessageItem*> m_messageItems;  // 消息ID → 消息气泡项（供按ID切换发送状态）
};

#endif // CHATAREA_H