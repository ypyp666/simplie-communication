#include "LoginWindow.h"
#include <QMessageBox>
#include <QTimer>
#include <QMouseEvent>

LoginWindow::LoginWindow(MainBackend* backend, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("登录");
    setFixedSize(400, 500);
    setModal(true);// 设置为模态对话框，用户只能在登录窗口操作，必须登录后才能进入主界面
    this->m_backend = backend;
    
    // 初始化登录动画相关
    m_loginAnimationTimer = new QTimer(this);
    m_dotsCount = 0;
    
    // 设置窗口圆角和阴影
    setStyleSheet(R"(
        LoginWindow {
            border-radius: 12px;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f0f5ff, stop:1 #e0e8f5);
        }
    )");
    
    setupUI();
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    
    // ========== 第一层：头像区域 ==========
    QHBoxLayout* avatarLayout = new QHBoxLayout();
    avatarLayout->setSpacing(10);
    
    // 头像标签
    avatarLabel = new QLabel(this);
    avatarLabel->setFixedSize(80, 80);
    avatarLabel->setStyleSheet(R"(
        QLabel {
            border-radius: 40px;
            background-color: #4a90d9;
            border: 3px solid white;
        }
    )");
    avatarLabel->installEventFilter(this);// 安装事件过滤器，监听头像点击事件
    //当 avatarLabel 发生任何事件（鼠标进入、离开、点击等），都会先经过 LoginWindow::eventFilter() 处理
    // 添加账号按钮（默认隐藏）
    addAccountBtn = new QPushButton("+", this);
    addAccountBtn->setFixedSize(30, 30);
    addAccountBtn->setStyleSheet(R"(
        QPushButton {
            border-radius: 15px;
            background-color: #4a90d9;
            color: white;
            font-size: 18px;
            border: none;
        }
        QPushButton:hover {
            background-color: #3a80c9;
        }
    )");
    addAccountBtn->hide();
    
    avatarLayout->addStretch();
    avatarLayout->addWidget(avatarLabel);
    avatarLayout->addWidget(addAccountBtn);
    avatarLayout->addStretch();
    
    mainLayout->addLayout(avatarLayout);
    
    // ========== 第二层：账号输入 ==========
    QHBoxLayout* accountLayout = new QHBoxLayout();
    accountLayout->setSpacing(0);
    
    accountEdit = new QLineEdit(this);
    accountEdit->setPlaceholderText("请输入账号");
    accountEdit->setFixedHeight(45);
    accountEdit->setStyleSheet(R"(
        QLineEdit {
            border-radius: 8px 0 0 8px;
            border: none;
            padding-left: 15px;
            font-size: 14px;
            background-color: white;
        }
        QLineEdit:focus {
            border-color: #4a90d9;
            outline: none;
        }
    )");//  border-radius: 8px 0 0 8px;左上圆角，右上圆角，右下圆角，左下圆角
    //outline 是控件获得焦点时，外围自动出现的高亮虚线 / 实线外框，不属于边框 border，不会占用布局空间，只是视觉提示当前选中了这个输入框、按钮。
    accountEdit->installEventFilter(this);
    
    // 下拉选择按钮（默认透明）
    accountDropdownBtn = new QPushButton(this);
    accountDropdownBtn->setFixedSize(45, 45);

    accountDropdownBtn->setFocusPolicy(Qt::ClickFocus);  
    accountDropdownBtn->setStyleSheet(R"(
        QPushButton {
            border-radius: 0 8px 8px 0;
            border: none;
            background-color: white;
        }
        QPushButton:hover {
            background-color: #f5f5f5;
        }
    )");
   // 资源里放 arrow-down.svg
    accountDropdownBtn->setIcon(QIcon(":/res/icon/arrow-down.svg"));
    accountDropdownBtn->setIconSize(QSize(16, 16));
    accountDropdownBtn->installEventFilter(this);
    accountDropdownBtn->hide();

    accountLayout->addWidget(accountEdit);
    accountLayout->addWidget(accountDropdownBtn);
    
    mainLayout->addLayout(accountLayout);
    
    // ========== 第三层：密码输入 ==========
    QHBoxLayout* passwordLayout = new QHBoxLayout();
    passwordLayout->setSpacing(0);
    
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->setFixedHeight(45);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(R"(
        QLineEdit {
            border-radius: 8px 0 0 8px;
            border: none;
            padding-left: 15px;
            font-size: 14px;
            background-color: white;
        }
        QLineEdit:focus {
            border-color: #4a90d9;
            outline: none;
        }
    )");
    passwordEdit->installEventFilter(this);
    
    // 密码可见切换按钮（默认透明）
    passwordToggleBtn = new QPushButton(this);
    passwordToggleBtn->setFixedSize(45, 45);
    passwordToggleBtn->setFocusPolicy(Qt::NoFocus);  // 禁止 Tab 键选中
    passwordToggleBtn->setStyleSheet(R"(
        QPushButton {
            border-radius: 0 8px 8px 0;
            background-color: white;
            border: none;
            padding: 0;
            cursor: pointer;
        }
        QPushButton:hover {
            background-color: #f5f5f5;
        }
    )");
    passwordToggleBtn->setIcon(QIcon(":/res/icon/eyes_show.svg"));
    passwordToggleBtn->setIconSize(QSize(14, 14));
    passwordToggleBtn->installEventFilter(this);
    passwordToggleBtn->hide();
    
    passwordLayout->addWidget(passwordEdit);
    passwordLayout->addWidget(passwordToggleBtn);
    
    mainLayout->addLayout(passwordLayout);
    
    // ========== 第四层：登录按钮 ==========
    loginBtn = new QPushButton("登录", this);
    loginBtn->setFixedHeight(45);
    loginBtn->setStyleSheet(R"(
        QPushButton {
            border-radius: 8px;
            background-color:white;
            font-size: 16px;
            font-weight: bold;
            border: none;
        }
        QPushButton:hover {
            background-color: #f5f5f5;
        }
        QPushButton:pressed {
            background-color: #2a70b9;
        }
    )");
    
    mainLayout->addWidget(loginBtn);
    
    // ========== 第五层：忘记密码和注册 ==========
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(0);
    
    forgotPwdBtn = new QPushButton("忘记密码", this);
    forgotPwdBtn->setStyleSheet(R"(
        QPushButton {
            color: #666666;
            font-size: 14px;
            border: none;
            background-color: transparent;
            text-align: left;
        }
        QPushButton:hover {
            color: #4a90d9;
        }
    )");
    
    registerBtn = new QPushButton("注册账号", this);
    registerBtn->setStyleSheet(R"(
        QPushButton {
            color: #4a90d9;
            font-size: 14px;
            border: none;
            background: transparent;
            text-align: right;
        }
        QPushButton:hover {
            color: #3a80c9;
        }
    )");
    
    bottomLayout->addWidget(forgotPwdBtn);
    bottomLayout->addStretch();
    bottomLayout->addWidget(registerBtn);
    
    mainLayout->addLayout(bottomLayout);
    
    // 连接信号槽
    connect(loginBtn, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(passwordToggleBtn, &QPushButton::clicked, this, &LoginWindow::onPasswordToggle);
    connect(accountEdit, &QLineEdit::textChanged, this, &LoginWindow::updateLoginButtonState);
    connect(passwordEdit, &QLineEdit::textChanged, this, &LoginWindow::updateLoginButtonState);
    connect(this,&LoginWindow::loginAquiard,m_backend,&MainBackend::login);
    // 专门监听后端登录结果信号，在这里判断是否关闭窗口
    connect(m_backend, &MainBackend::loginSuccess, this, [=](const QString& accountId){
        // 服务器验证成功，关闭登录弹窗，放行进入主界面
        stopLoginAnimation();
        emit loginSuccess(accountId, accountId); // 转发登录成功信号
        accept();
    });
    connect(m_backend, &MainBackend::loginFailed, this, [=](){
        // 服务器验证失败，弹窗提示，窗口保持打开
        stopLoginAnimation();
        QMessageBox::warning(this, "登录失败", "账号或密码错误");
    });
    // 监听登录等待信号
    connect(m_backend, &MainBackend::loginWaiting, this, &LoginWindow::onLoginWaiting);
    // 监听登录超时信号
    connect(m_backend, &MainBackend::loginTimeout, this, &LoginWindow::onLoginTimeout);
    // 登录动画定时器
    connect(m_loginAnimationTimer, &QTimer::timeout, this, &LoginWindow::updateLoginButtonText);
    // 初始化登录按钮状态
    updateLoginButtonState();
}

 bool LoginWindow::eventFilter(QObject* obj, QEvent* event)//QObject类的原生虚函数，用于拦截事件并进行处理
{
    // 头像悬浮：10秒延迟消失
    if (obj == avatarLabel || obj == addAccountBtn) {
        if (event->type() == QEvent::Enter)//鼠标进入事件
         {
            addAccountBtn->show();
            // 重置10秒计时器
            //QTimer::singleShot(10000, addAccountBtn, &QPushButton::hide);
        } else if (event->type() == QEvent::Leave)//鼠标离开事件
         {
            // 重新启动5秒计时器
            QTimer::singleShot(5000, addAccountBtn, &QPushButton::hide);
        }
    }
  
    /* 
     QPushButton* relatedBtn = (obj == accountEdit) 
            ? qobject_cast<QPushButton*>(accountEdit->parent()->findChild<QPushButton*>())
            : passwordToggleBtn;
        这段逻辑保留，逻辑讲解：
        1. 如果obj是accountEdit，那么relatedBtn就是accountDropdownBtn，并且用
        qobject_cast将accountDropdownBtn转换为QPushButton*类型。（qobject_cast是Qt提供的类型转换函数，用于将QObject*安全转换为指定类型的指针）
        accountEdit->parent()拿到accountEdit的父对象，即LoginWindow。
        findChild<QPushButton*>()在LoginWindow中查找accountDropdownBtn，返回accountDropdownBtn的指针。
        因为accountDropdownBtn是按钮组件并且accountEdit的子对象，所以可以使用findChild()函数来查找它。
        2. 如果obj是passwordEdit，那么relatedBtn就是passwordToggleBtn。
        3. 这段逻辑的目的是根据obj的类型，选择对应的下拉按钮或密码可见切换按钮。
        */
    // 账号输入框区域：悬浮显示下拉按钮
    if (obj == accountEdit || obj == passwordEdit) {
        QPushButton* relatedBtn = (obj == accountEdit) 
            ? accountDropdownBtn
            : passwordToggleBtn;
        
        if (relatedBtn) {
            if (event->type() == QEvent::Enter)//鼠标进入事件
            {
                relatedBtn->setStyleSheet(R"(
                    QPushButton {
                        border-radius: 0 8px 8px 0;
                        border: 1px solid #e0e0e0;
                        border-left: none;
                        background-color: white;
                    }
                    QPushButton:hover {
                        background-color: #f5f5f5;
                    }
                )");
                relatedBtn->show();
            } else if (event->type() == QEvent::Leave) {
                relatedBtn->setStyleSheet(R"(
                    QPushButton {
                        border-radius: 0 8px 8px 0;
                        border: 1px solid #e0e0e0;
                        border-left: none;
                        background-color: white;
                    }
                    QPushButton:hover {
                        background-color: #f5f5f5;
                    }
                )");
            }
        }
    }
    
    // 密码按钮本身悬浮
    if (obj == passwordToggleBtn) {
        if (event->type() == QEvent::Enter) {
            passwordToggleBtn->setStyleSheet(R"(
                QPushButton {
                    border-radius: 0 8px 8px 0;
                    border: 1px solid #e0e0e0;
                    border-left: none;
                    background-color: white;
                }
                QPushButton:hover {
                    background-color: #f5f5f5;
                }
            )");
            passwordToggleBtn->show();
        } else if (event->type() == QEvent::Leave) {
            passwordToggleBtn->setStyleSheet(R"(
                QPushButton {
                    border-radius: 0 8px 8px 0;
                    border: 1px solid #e0e0e0;
                    border-left: none;
                    background-color: white;
                }
                QPushButton:hover {
                    background-color: #f5f5f5;
                }
            )");
            QTimer::singleShot(500, passwordToggleBtn, &QPushButton::hide);
        }
    }

    if(obj==accountDropdownBtn)
    {
     if(event->type()==QEvent::Enter)//鼠标进入事件
     {
        accountDropdownBtn->show();
     }
     else if(event->type()==QEvent::Leave)//鼠标离开事件
     {
       QTimer::singleShot(500, accountDropdownBtn, &QPushButton::hide);
     }
    }
    
    return QDialog::eventFilter(obj, event);
}

void LoginWindow::onLoginClicked()
{
    QString account = accountEdit->text();
    QString password = passwordEdit->text();

    if (account.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入账号和密码");
        return;
    }
    emit loginAquiard(account, password);
    // 模拟登录成功（实际应该调用后端验证）
}

void LoginWindow::onPasswordToggle()
{
    if (passwordEdit->echoMode() == QLineEdit::Password)//判断当前输入框回显模式：是否为密码隐藏模式
    {
        passwordEdit->setEchoMode(QLineEdit::Normal);//设置为正常模式
        passwordToggleBtn->setIcon(QIcon(":/res/icon/eyes_clicked.svg"));
    } else 
    {
        passwordEdit->setEchoMode(QLineEdit::Password);//设置为密码模式
        passwordToggleBtn->setIcon(QIcon(":/res/icon/eyes_show.svg"));
    }
}

void LoginWindow::onAvatarHoverEnter()
{
    addAccountBtn->show();
}

void LoginWindow::onAvatarHoverLeave()
{
    // 延迟10秒后隐藏
    QTimer::singleShot(10000, addAccountBtn, &QPushButton::hide);
}

void LoginWindow::updateLoginButtonState()
{
    bool canLogin = !accountEdit->text().isEmpty() && !passwordEdit->text().isEmpty();
    
    if (canLogin) {
        // 可登录状态：蓝色按钮，可点击，有悬浮效果
        loginBtn->setEnabled(true);
        loginBtn->setStyleSheet(R"(
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
            QPushButton:pressed {
                background-color: #2a70b9;
            }
        )");
    } else {
        // 不可登录状态：灰色按钮，禁用，无悬浮效果
        loginBtn->setEnabled(false);
        loginBtn->setStyleSheet(R"(
            QPushButton {
                border-radius: 8px;
                background-color: #cccccc;
                color: #999999;
                font-size: 16px;
                font-weight: bold;
                border: none;
            }
        )");
    }
}

void LoginWindow::onLoginWaiting()
{
    // 保存原始按钮文本
    m_originalLoginText = loginBtn->text();
    
    // 禁用登录按钮，防止重复点击
    loginBtn->setEnabled(false);
    
    // 初始化点的数量
    m_dotsCount = 0;
    
    // 更新按钮文本为"正在登录中"
    loginBtn->setText("正在登录中");
    
    // 设置登录中的样式（灰色背景）
    loginBtn->setStyleSheet(R"(
        QPushButton {
            border-radius: 8px;
            background-color: #999999;
            color: white;
            font-size: 16px;
            font-weight: bold;
            border: none;
        }
    )");
    
    // 启动定时器，每500ms更新一次点的数量
    m_loginAnimationTimer->start(500);
}

void LoginWindow::updateLoginButtonText()
{
    // 更新点的数量（0->1->2->3->0循环）
    m_dotsCount = (m_dotsCount + 1) % 4;
    
    // 构建新的按钮文本
    QString text = "正在登录中";
    for (int i = 0; i < m_dotsCount; i++) {
        text += ".";
    }
    
    // 更新按钮文本
    loginBtn->setText(text);
}

void LoginWindow::stopLoginAnimation()
{
    // 停止定时器
    m_loginAnimationTimer->stop();
    
    // 恢复原始按钮文本
    loginBtn->setText(m_originalLoginText);
    
    // 恢复按钮状态
    updateLoginButtonState();
}

void LoginWindow::onLoginTimeout()
{
    // 停止登录动画
    stopLoginAnimation();
    
    // 弹出提示框
    QMessageBox::warning(this, "连接超时", "连接超时，请检查网络设置");
}
