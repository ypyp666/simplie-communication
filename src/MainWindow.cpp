#include "MainWindow.h"
#include "ChatWindow.h"
#include "StatusBar.h"

MainWindow::MainWindow(QWidget *parent, MainBackend* backend) : QMainWindow(parent)
{
    m_backend = backend;
    setWindowTitle("我的聊天软件");
    setMinimumSize(900, 600);
    resize(1000, 700);

    initUI();
    initPages();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    StatusBar* statusPage = new StatusBar(this);
    statusPage->setFixedWidth(30);
    mainLayout->addWidget(statusPage);
}

void MainWindow::initPages()
{
    pageStack = new QStackedWidget(this);
    mainLayout->addWidget(pageStack);

    // 添加聊天页面（传入主后端对象）
    ChatWindow* chatWindow = new ChatWindow(this, m_backend);
    pageStack->addWidget(chatWindow);


    // 默认显示聊天页面
    pageStack->setCurrentIndex(CHAT_PAGE_INDEX);
}

void MainWindow::switchToPage(int index)
{
    if (index >= 0 && index < pageStack->count()) {
        pageStack->setCurrentIndex(index);
        emit pageChanged(index);
    }
}