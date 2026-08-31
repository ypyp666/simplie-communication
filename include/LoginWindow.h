#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QStackedWidget>
#include "LoginPage.h"
#include "RegisterPage.h"
#include "ForgotPasswordPage.h"
/*
LoginWindow（登录窗口 = 页面容器）
它本身不再放任何页面内容，只负责：
1. 用 QStackedWidget 装三个页面：LoginPage(登录页) / RegisterPage(注册页) / ForgotPasswordPage(忘记密码页)
2. 在页面之间切换（注册页/忘记密码页的"返回登录"、登录页的"去注册/去忘记密码"）
3. 作为模态弹窗（exec() 阻塞），登录成功后 accept() 放行进入主界面

窗口级的事（标题、固定大小、模态、背景渐变）在这里做；
页面级的事（UI 控件、悬浮效果、登录流程）在各自的 Page 类里做。

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

private:
    // 页面容器与三个页面
    QStackedWidget* m_stackedWidget;
    LoginPage* m_loginPage;
    RegisterPage* m_registerPage;
    ForgotPasswordPage* m_forgotPage;
};

#endif // LOGINWINDOW_H
