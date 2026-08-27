#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>
#include"MainBackend.h"

/*
 QMainWindow（你的 MainWindow 主窗口）
 专门做主窗口，核心用途：显示聊天界面、状态信息、账号管理等。
 常用布局：水平布局，包含聊天页面、状态页面、账号管理页面等。
 尺寸固定，一般是大窗口；
 不存在阻塞函数，显示直接show()即可。
 不存在模态问题，所有操作都是异步的。
 程序主体长期运行，等待用户交互。
 主窗口关闭后，程序退出。
*/ 

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)
public:
    explicit MainWindow(QWidget *parent = nullptr, MainBackend* backend = nullptr);
    ~MainWindow();
    MainBackend* m_backend;

signals:
    // 页面切换信号
    void pageChanged(int index);

public slots:
    // 切换到指定页面
    void switchToPage(int index);

private:
    void initUI();
    void initPages();

    QWidget* centralWidget;
    QHBoxLayout* mainLayout;
    QStackedWidget* pageStack;  // 页面管理器

    // 页面索引常量
    static const int CHAT_PAGE_INDEX = 0;
    static const int STATUS_PAGE_INDEX = 1;
};

#endif // MAINWINDOW_H