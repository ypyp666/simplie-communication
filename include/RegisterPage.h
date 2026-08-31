#ifndef REGISTERPAGE_H
#define REGISTERPAGE_H

#include <QWidget>
/*
RegisterPage（注册页）
独立的注册页面组件，放进 LoginWindow 的 QStackedWidget 里作为其中一页。
自己不负责切页，点"返回登录"时发 backToLoginRequested 信号，由容器去切。
*/

class RegisterPage : public QWidget
{
    Q_OBJECT
public:
    explicit RegisterPage(QWidget *parent = nullptr);

signals:
    void backToLoginRequested();   // 点击"返回登录"

private:
    void setupUI();
};

#endif // REGISTERPAGE_H
