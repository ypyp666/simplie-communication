#include "MessageItem.h"
#include <QTimer>

MessageItem::MessageItem(const MessageInfo& message, QWidget *parent)
    : QWidget(parent), m_messageId(message.id), m_statusIndicator(nullptr)
{
    setupUI(message);
}

// 切换消息发送状态：转发给状态指示器（自己发送的消息才有效）
void MessageItem::setStatus(MessageStatusIndicator::Status status)
{
    if (m_statusIndicator) {
        m_statusIndicator->setStatus(status);
    }
}

void MessageItem::setupUI(const MessageInfo& message)
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 10, 15, 10);
    mainLayout->setSpacing(10);

    // ========== 头像部分 ==========
    QFrame* avatarFrame = new QFrame(this);
    avatarFrame->setFixedSize(40, 40);
    
    QString avatarBgColor = message.isSelf ? "#4a90d9" : "#67c23a";
    avatarFrame->setStyleSheet(QString(R"(
        QFrame {
            border-radius: 20px;
            background-color: %1;
        }
    )").arg(avatarBgColor));
    
    QLabel* avatarLabel = new QLabel(avatarFrame);
    avatarLabel->setFixedSize(40, 40);
    avatarLabel->setStyleSheet(R"(
        color: white;
        font-size: 16px;
    )");
    avatarLabel->setText(message.isSelf ? "我" : message.senderId.left(1).toUpper());
    avatarLabel->setAlignment(Qt::AlignCenter);

    // ========== 消息气泡部分 ==========
    QWidget* bubbleWidget = new QWidget(this);
    // ✅ 关键1：气泡用垂直布局，后面要加时间标签，水平布局放不下
    QVBoxLayout* bubbleLayout = new QVBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(10, 6, 10, 6);
    bubbleLayout->setSpacing(4);

    QString bubbleColor = message.isSelf ? "#4a90d9" : "#ffffff";
    QString textColor = message.isSelf ? "white" : "#333333";

    if (message.isFile) {
        QLabel* fileLabel = new QLabel(bubbleWidget);
        QString fileSizeStr;
        if (message.fileSize < 1024) {
            fileSizeStr = QString("%1 B").arg(message.fileSize);
        } else if (message.fileSize < 1024 * 1024) {
            fileSizeStr = QString("%1 KB").arg(message.fileSize / 1024);
        } else {
            fileSizeStr = QString("%1 MB").arg(message.fileSize / (1024 * 1024));
        }

        QString html = QString(R"(
            <div style="display: flex; align-items: center; gap: 8px;">
                <span style="font-size: 20px;">📄</span>
                <div>
                    <div style="font-size: 13px; color: %1; font-weight: 500;">%2</div>
                    <div style="font-size: 11px; color: %3;">%4</div>
                </div>
            </div>
        )").arg(textColor, message.fileName, "#999", fileSizeStr);
        fileLabel->setText(html);
        bubbleLayout->addWidget(fileLabel);
    } else {
        QTextEdit* contentEdit = new QTextEdit(bubbleWidget);
        contentEdit->setPlainText(message.content);
        contentEdit->setStyleSheet(QString(R"(
            QTextEdit {
                color: %1;
                font-size: 14px;
                background-color: transparent;
                border: none;
                padding: 0px;
            }
        )").arg(textColor));
        
        contentEdit->setReadOnly(true);//设置为只读，防止用户编辑
        contentEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏垂直滚动条
        contentEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏水平滚动条
        contentEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);//设置为单词边界换行或任意位置换行
        
        // 关键：自适应宽度逻辑
        /*
        QTextEdit = 显示控件 + 内部有一个 QTextDocument（文档对象）
        你看到的文字、换行、排版，全是 QTextDocument 算出来的
        QTextEdit 只负责 “画出来”，不负责计算最佳宽高
        所以自适应的核心，永远是 操作 QTextDocument
        */ 
        QTimer::singleShot(10, this, [contentEdit, bubbleWidget]() {
            QTextDocument* doc = contentEdit->document();
            
            // 先让文档不限制宽度，计算理想宽度
            doc->setPageSize(QSizeF(10000, 10000));
            doc->adjustSize();
            
            // 获取文本的理想宽度（不换行时的宽度）
            qreal idealWidth = doc->idealWidth();//qreal为Qt的浮点数类型
            
            // 实际宽度 = min(理想宽度, 最大宽度800)
            int actualWidth = qMin((int)idealWidth, 800);
            
            // 设置文档页面宽度，让文本知道在哪里换行
            doc->setPageSize(QSizeF(actualWidth - 20, 10000)); 
            doc->adjustSize();
            
            // 设置文本编辑框的尺寸
            contentEdit->setFixedWidth(actualWidth);
            contentEdit->setFixedHeight(doc->size().height() + 8);
            
            // 设置气泡容器的尺寸（加上内边距）
            bubbleWidget->setFixedWidth(actualWidth + 32);
        });
        
        bubbleLayout->addWidget(contentEdit);
    }

    if (message.isOffline) {
        QLabel* offlineLabel = new QLabel("(离线)", bubbleWidget);
        offlineLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(message.isSelf ? "#ffb366" : "#ff9800"));
        offlineLabel->setAlignment(Qt::AlignRight);
        bubbleLayout->addWidget(offlineLabel);
    }

    bubbleWidget->setStyleSheet(QString(R"(
        QWidget {
            border-radius: 18px;
            background-color: %1;
        }
    )").arg(bubbleColor));

    // ✅ 关键4：限制气泡最大宽度，不要用样式表的max-width，代码设置更可靠
    bubbleWidget->setMaximumWidth(800);
    bubbleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // ========== 布局顺序区分 ==========
    if (message.isSelf) {
        mainLayout->addStretch(1);
        // 自己发送的消息：气泡左侧显示状态指示器（发送中旋转圆圈 / 失败红色感叹号）
        // 历史消息默认隐藏，只有新发的消息才通过 setStatus 显示
        m_statusIndicator = new MessageStatusIndicator(this);
        // 点击失败感叹号 → 转发重发请求（带上本消息ID）
        connect(m_statusIndicator, &MessageStatusIndicator::retryClicked, this, [this]() {
            emit retryRequested(m_messageId);
        });
        mainLayout->addWidget(m_statusIndicator, 0, Qt::AlignVCenter);
        mainLayout->addWidget(bubbleWidget, 1);
        mainLayout->addWidget(avatarFrame);
    } else {
        mainLayout->addWidget(avatarFrame);
        mainLayout->addWidget(bubbleWidget, 1);
        mainLayout->addStretch(1);
    }

    // 让整个MessageItem高度自适应内容
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}