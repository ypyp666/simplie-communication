#include "ChatInput.h"

ChatInput::ChatInput(QWidget *parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 0, 15, 15);
    layout->setSpacing(10);

    fileButton = new QPushButton("📎", this);
    fileButton->setFixedSize(40, 40);
    fileButton->setStyleSheet(R"(
        QPushButton {
            border: none;
            border-radius: 50%;
            background-color: #f0f0f0;
            font-size: 18px;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
        }
    )");
    layout->addWidget(fileButton, 0, Qt::AlignLeft);//左对齐

    inputEdit = new QTextEdit(this);
    inputEdit->setPlaceholderText("输入消息...");
    inputEdit->setStyleSheet(R"(
        QTextEdit {
            border: 1px solid #e0e0e0;
            border-radius: 10px;
            padding: 12px 15px;
            font-size: 14px;
            background-color: white;
        }
        QTextEdit:focus {
            border-color: #4a90d9;
            outline: none;
        }
    )");
    inputEdit->setMaximumHeight(100);  // 最大高度限制
    inputEdit->setMinimumHeight(60);   // 最小高度
    inputEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    inputEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    inputEdit->document()->setDocumentMargin(0);
    inputEdit->setStyleSheet(R"(
        QTextEdit {
            border: 1px solid #e0e0e0;
            border-radius: 10px;
            padding: 12px 15px;
            font-size: 14px;
            background-color: white;
        }
        QTextEdit:focus {
            border-color: #4a90d9;
            outline: none;
        }
        QTextEdit QScrollBar:vertical {
            width: 6px;
            background-color: transparent;
        }
        QTextEdit QScrollBar::handle:vertical {
            background-color: #c0c0c0;
            border-radius: 3px;
        }
    )");
    layout->addWidget(inputEdit);

    sendButton = new QPushButton("发送", this);
    sendButton->setFixedSize(60, 30);
    sendButton->setStyleSheet(R"(
        QPushButton {
            border: none;
            border-radius: 15px;
            background-color: #4a90d9;
            color: white;
            font-size: 14px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #3a80c9;
        }
        QPushButton:disabled {
            background-color: #a0a0a0;
        }
    )");
    sendButton->setEnabled(false);
    layout->addWidget(sendButton);
    
    // 设置 ChatInput 整体的最小和最大高度，防止挤压聊天区域
    setMaximumHeight(150);

    connect(fileButton, &QPushButton::clicked, this, &ChatInput::onFileClicked);
    connect(sendButton, &QPushButton::clicked, this, &ChatInput::onSendClicked);
    connect(inputEdit, &QTextEdit::textChanged, [this]() {
        sendButton->setEnabled(!inputEdit->toPlainText().trimmed().isEmpty());
    });//根据文本框状态设置登录按钮是否可点击
}

void ChatInput::onSendClicked()
{
    QString content = inputEdit->toPlainText().trimmed();//去掉首尾空格
    if (!content.isEmpty()) {
        emit sendMessage(content);
        inputEdit->clear();
        sendButton->setEnabled(false);
    }
}

void ChatInput::onFileClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", "所有文件 (*.*)");
    if (!filePath.isEmpty()) {
        emit sendFile(filePath);
    }
}

void ChatInput::onEnterPressed()
{
    if (sendButton->isEnabled()) {
        onSendClicked();
    }
}

void ChatInput::adjustHeight()
{
    // 根据文本内容动态调整输入框高度
    int docHeight = (int)inputEdit->document()->size().height();
    if (docHeight < 60) docHeight = 60;
    if (docHeight > 120) docHeight = 120;
    inputEdit->setFixedHeight(docHeight);
}