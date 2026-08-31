#ifndef FORGOTPASSWORDPAGE_H
#define FORGOTPASSWORDPAGE_H

#include <QWidget>
/*
ForgotPasswordPage（忘记密码页）
独立的找回密码页面组件，放进 LoginWindow 的 QStackedWidget 里作为其中一页。
自己不负责切页，点"返回登录"时发 backToLoginRequested 信号，由容器去切。
*/

class ForgotPasswordPage : public QWidget
{
    Q_OBJECT
public:
    explicit ForgotPasswordPage(QWidget *parent = nullptr);

signals:
    void backToLoginRequested();   // 点击"返回登录"

private:
    void setupUI();
};

#endif // FORGOTPASSWORDPAGE_H
