#include "ChatArea.h"
#include "MessageItem.h"
#include "ChatInput.h"
#include <QTimer>
#include <QScrollBar>

ChatArea::ChatArea(QWidget *parent) : QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    headerLabelWidget = new QWidget(this);
    headerLabelWidget->setStyleSheet(R"(
        background-color: white;
        border-radius: 5px;
        box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
    )");
    
    headerLayout = new QHBoxLayout(headerLabelWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(0);

    headerLabel = new QLabel("请选择一个联系人开始聊天", this);
    headerLabel->setStyleSheet(R"(
        background-color: transparent;
        color: black;
        padding: 15px 20px;
        font-size: 16px;
        font-weight: 600;
    )");

    headerLayout->addWidget(headerLabel);

    mainLayout->addWidget(headerLabelWidget);

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            border: none;
            background-color: #e8eef3;
        }
        QScrollBar:vertical {
            width: 0px;
            background-color: transparent;
        }
        QScrollBar::handle:vertical {
            background-color: transparent;
            border-radius: 3px;
        }
        QScrollArea:hover QScrollBar:vertical {
            width: 6px;
        }
        QScrollArea:hover QScrollBar::handle:vertical {
            background-color: #c0c0c0;
        }
        QScrollArea:focus QScrollBar:vertical {
            width: 6px;
        }
        QScrollArea:focus QScrollBar::handle:vertical {
            background-color: #c0c0c0;
        }
    )");
    
    mainLayout->addWidget(scrollArea, 1);

    messagesWidget = new QWidget();
    messagesLayout = new QVBoxLayout(messagesWidget);
    messagesLayout->setContentsMargins(0, 10, 0, 10);
    messagesLayout->setSpacing(0);
    messagesLayout->addStretch(1);

    scrollArea->setWidget(messagesWidget);

    // 创建输入框，作为聊天区域的一部分
    chatInput = new ChatInput(this);
    chatInput->setStyleSheet("background-color: white; border-top: 1px solid #e0e0e0;");
    chatInput->hide();  // 默认隐藏
    mainLayout->addWidget(chatInput);

    // 连接输入框信号到 ChatArea 信号
    connect(chatInput, &ChatInput::sendMessage, this, &ChatArea::sendMessage);
    connect(chatInput, &ChatInput::sendFile, this, &ChatArea::sendFile);
}

void ChatArea::addMessage(const MessageInfo& message)
{
    MessageItem* item = new MessageItem(message, this);
    m_messageItems.insert(message.id, item);  // 记录 消息ID→Item，供发送状态切换使用
    // 消息项内点击失败感叹号 → ChatArea 转发重发请求
    connect(item, &MessageItem::retryRequested, this, &ChatArea::retrySend);
    
    messagesLayout->insertWidget(messagesLayout->count() - 1, item);
    //弹簧布局占一个单位，确保消息添加完成后滚动到最底部
    QLabel* timeLabel = new QLabel(message.sendTime.toString("HH:mm"), this);
    timeLabel->setStyleSheet("color: #999; font-size: 11px;");
    timeLabel->setAlignment(Qt::AlignCenter);
    messagesLayout->insertWidget(messagesLayout->count() - 1, timeLabel);

    QTimer::singleShot(10, this, [this]() -> void {
        QScrollBar* scrollBar = scrollArea->verticalScrollBar();//获取垂直滚动条
        if (scrollBar == nullptr) {
            return;
        }
        scrollBar->setValue(scrollBar->maximum());//滚动到最底部
    });//添加消息后，滚动到最底部，延迟10ms秒，确保消息添加完成后再滚动
 

}
    
   /*
   []() {
    // 滚到底部
};Lambda 写法（现代 C++），[this] = 把当前类的 this 指针捕获进来，
作用：在函数里能访问当前类的成员变量（scrollArea、this、成员函数）
   */

void ChatArea::setMessages(const QList<MessageInfo>& messages)
{
    clearMessages();
    for (const auto& msg : messages) {
        addMessage(msg);
    }
}

// 按消息ID切换发送状态：转发给对应的消息气泡项
void ChatArea::setMessageStatus(const QString& messageId, MessageStatusIndicator::Status status)
{
    auto it = m_messageItems.find(messageId);
    if (it != m_messageItems.end()) {
        it.value()->setStatus(status);
    }
}

void ChatArea::setContactName(const QString& name)
{
    headerLabel->setText(name);
}

void ChatArea::clearMessages()
{
    QLayoutItem* item;
    while ((item = messagesLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    m_messageItems.clear();  // 同时清除 消息ID→Item 映射，避免悬挂指针
    messagesLayout->addStretch(1);
}

void ChatArea::clearInput()
{
    if (chatInput) {
        // 清空输入框内容
        QTextEdit* inputEdit = chatInput->findChild<QTextEdit*>();
        if (inputEdit) {
            inputEdit->clear();
        }
    }
}

void ChatArea::setInputVisible(bool visible)
{
    if (chatInput) {
        if (visible) {
            chatInput->show();
        } else {
            chatInput->hide();
        }
    }
}

void ChatArea::setInputContent(const QString& content)
{
    if (chatInput) {
        QTextEdit* inputEdit = chatInput->findChild<QTextEdit*>();
        if (inputEdit) {
            inputEdit->setPlainText(content);
            // 将光标移动到文本末尾
            QTextCursor cursor = inputEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            inputEdit->setTextCursor(cursor);
        }
    }
}

QString ChatArea::getInputContent()
{
    if (chatInput) {
        QTextEdit* inputEdit = chatInput->findChild<QTextEdit*>();
        if (inputEdit) {
            return inputEdit->toPlainText();
        }
    }
    return "";
}