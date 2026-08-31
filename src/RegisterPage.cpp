#include "RegisterPage.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

RegisterPage::RegisterPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void RegisterPage::setupUI()
{
    // ========== 注册页自己的根布局 ==========
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(40, 40, 40, 40);

    // 页标题
    QLabel* titleLabel = new QLabel("注册账号", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel { color: #333333; font-size: 20px; font-weight: bold; }");
    layout->addWidget(titleLabel);
    layout->addSpacing(10);

    // 账号输入框
    QLineEdit* regAccountEdit = new QLineEdit(this);
    regAccountEdit->setPlaceholderText("请输入账号");
    regAccountEdit->setFixedHeight(45);
    regAccountEdit->setStyleSheet(R"(
        QLineEdit {
            border-radius: 8px;
            border: none;
            padding-left: 15px;
            font-size: 14px;
            background-color: white;
        }
    )");
    layout->addWidget(regAccountEdit);

    // 密码输入框
    QLineEdit* regPasswordEdit = new QLineEdit(this);
    regPasswordEdit->setPlaceholderText("请输入密码");
    regPasswordEdit->setFixedHeight(45);
    regPasswordEdit->setEchoMode(QLineEdit::Password);
    regPasswordEdit->setStyleSheet(R"(
        QLineEdit {
            border-radius: 8px;
            border: none;
            padding-left: 15px;
            font-size: 14px;
            background-color: white;
        }
    )");
    layout->addWidget(regPasswordEdit);

    // 确认密码输入框
    QLineEdit* regConfirmEdit = new QLineEdit(this);
    regConfirmEdit->setPlaceholderText("请再次输入密码");
    regConfirmEdit->setFixedHeight(45);
    regConfirmEdit->setEchoMode(QLineEdit::Password);
    regConfirmEdit->setStyleSheet(R"(
        QLineEdit {
            border-radius: 8px;
            border: none;
            padding-left: 15px;
            font-size: 14px;
            background-color: white;
        }
    )");
    layout->addWidget(regConfirmEdit);

    // 注册按钮（先做个占位效果，具体逻辑后面再接后端）
    QPushButton* regSubmitBtn = new QPushButton("注册", this);
    regSubmitBtn->setFixedHeight(45);
    regSubmitBtn->setStyleSheet(R"(
        QPushButton {
            border-radius: 8px;
            background-color: #4a90d9;
            color: white;
            font-size: 16px;
            font-weight: bold;
            border: none;
        }
        QPushButton:hover {
            background-color: #3a80c9;
        }
    )");
    layout->addWidget(regSubmitBtn);
    connect(regSubmitBtn, &QPushButton::clicked, this, [=]() {
        QMessageBox::information(this, "提示", "注册功能开发中");
    });

    // 返回登录按钮（透明文字链接，点击通知容器切回登录页）
    QPushButton* backToLoginBtn = new QPushButton("返回登录", this);
    backToLoginBtn->setStyleSheet(R"(
        QPushButton {
            color: #4a90d9;
            font-size: 14px;
            border: none;
            background: transparent;
            text-align: center;
        }
        QPushButton:hover {
            color: #3a80c9;
        }
    )");
    layout->addWidget(backToLoginBtn);
    layout->addStretch();
    connect(backToLoginBtn, &QPushButton::clicked, this, &RegisterPage::backToLoginRequested);
}
