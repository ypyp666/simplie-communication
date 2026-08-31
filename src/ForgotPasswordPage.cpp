#include "ForgotPasswordPage.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

ForgotPasswordPage::ForgotPasswordPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void ForgotPasswordPage::setupUI()
{
    // ========== 忘记密码页自己的根布局 ==========
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(40, 40, 40, 40);

    // 页标题
    QLabel* titleLabel = new QLabel("找回密码", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel { color: #333333; font-size: 20px; font-weight: bold; }");
    layout->addWidget(titleLabel);
    layout->addSpacing(10);

    // 账号输入框
    QLineEdit* fAccountEdit = new QLineEdit(this);
    fAccountEdit->setPlaceholderText("请输入账号");
    fAccountEdit->setFixedHeight(45);
    fAccountEdit->setStyleSheet(R"(
        QLineEdit {
            border-radius: 8px;
            border: none;
            padding-left: 15px;
            font-size: 14px;
            background-color: white;
        }
    )");
    layout->addWidget(fAccountEdit);

    // 新密码输入框
    QLineEdit* fNewPwdEdit = new QLineEdit(this);
    fNewPwdEdit->setPlaceholderText("请输入新密码");
    fNewPwdEdit->setFixedHeight(45);
    fNewPwdEdit->setEchoMode(QLineEdit::Password);
    fNewPwdEdit->setStyleSheet(R"(
        QLineEdit {
            border-radius: 8px;
            border: none;
            padding-left: 15px;
            font-size: 14px;
            background-color: white;
        }
    )");
    layout->addWidget(fNewPwdEdit);

    // 确认新密码输入框
    QLineEdit* fConfirmEdit = new QLineEdit(this);
    fConfirmEdit->setPlaceholderText("请再次输入新密码");
    fConfirmEdit->setFixedHeight(45);
    fConfirmEdit->setEchoMode(QLineEdit::Password);
    fConfirmEdit->setStyleSheet(R"(
        QLineEdit {
            border-radius: 8px;
            border: none;
            padding-left: 15px;
            font-size: 14px;
            background-color: white;
        }
    )");
    layout->addWidget(fConfirmEdit);

    // 提交按钮（先做个占位效果，具体逻辑后面再接后端）
    QPushButton* fSubmitBtn = new QPushButton("确认修改", this);
    fSubmitBtn->setFixedHeight(45);
    fSubmitBtn->setStyleSheet(R"(
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
    layout->addWidget(fSubmitBtn);
    connect(fSubmitBtn, &QPushButton::clicked, this, [=]() {
        QMessageBox::information(this, "提示", "找回密码功能开发中");
    });

    // 返回登录按钮（点击通知容器切回登录页）
    QPushButton* fBackBtn = new QPushButton("返回登录", this);
    fBackBtn->setStyleSheet(R"(
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
    layout->addWidget(fBackBtn);
    layout->addStretch();
    connect(fBackBtn, &QPushButton::clicked, this, &ForgotPasswordPage::backToLoginRequested);
}
