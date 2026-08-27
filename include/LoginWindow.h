#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPixmap>
#include <QTimer>
#include "MainBackend.h"
/*
QDialog（你的 LoginWindow 登录框）
专门做弹窗、模态交互窗口，核心用途：登录、弹窗提示、选择框、配置弹窗。
自带专属 exec() 阻塞循环，天生模态，用来中断主流程，等待用户做出选择（登录 / 取消）。
没有菜单栏、工具栏、状态栏、中心部件这套标准布局框架；
尺寸灵活，一般是小窗口；
关闭 / 确认后直接结束局部事件循环，返回 Accepted/Rejected。
*/ 

class LoginWindow : public QDialog//
{
    Q_OBJECT
public:
    explicit LoginWindow(MainBackend* backend, QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    void loginSuccess(const QString& accountId, const QString& accountName);
    void loginAquiard(const QString& accountId, const QString& passward);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onLoginClicked();
    void onPasswordToggle();
    void onAvatarHoverEnter();
    void onAvatarHoverLeave();
    void updateLoginButtonState();
    void onLoginWaiting();
    void updateLoginButtonText();
    void stopLoginAnimation();
    void onLoginTimeout();

private:
    void setupUI();

    // 头像相关
    QLabel* avatarLabel;
    QPushButton* addAccountBtn;
    
    // 账号输入相关
    QLineEdit* accountEdit;
    QPushButton*  accountDropdownBtn; // 新增，和passwordToggleBtn对齐
    
    // 密码输入相关
    QLineEdit* passwordEdit;
    QPushButton* passwordToggleBtn;
    
    // 按钮相关
    QPushButton* loginBtn;
    QPushButton* forgotPwdBtn;
    QPushButton* registerBtn;

    //后端相关
    MainBackend* m_backend;

    // 登录动画相关
    QTimer* m_loginAnimationTimer;
    int m_dotsCount;
    QString m_originalLoginText;
};

#endif // LOGINWINDOW_H
