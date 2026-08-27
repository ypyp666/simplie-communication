#ifndef MESSAGESTATUSINDICATOR_H
#define MESSAGESTATUSINDICATOR_H

#include <QWidget>
#include <QTimer>

// 消息状态指示器（自定义 QWidget）
// 两种状态：
//   Sending：发送中 → 旋转小圆圈动画
//   Failed ：发送失败（超时）→ 红色感叹号
// 直接自绘（paintEvent），不需要任何图片资源
class MessageStatusIndicator : public QWidget
{
    Q_OBJECT
public:
    enum Status {
        None = 0,    // 无状态（隐藏）
        Sending = 1, // 发送中（旋转圆圈动画）
        Failed = 2   // 发送失败（红色感叹号）
    };

    explicit MessageStatusIndicator(QWidget *parent = nullptr);

    void setStatus(Status status);  // 切换状态

signals:
    void retryClicked();  // 发送失败状态下点击红色感叹号 → 请求重发

protected:
    void paintEvent(QPaintEvent *event) override;  // 重绘自己
    void mousePressEvent(QMouseEvent *event) override;  // 点击感叹号重发

private:
    Status m_status;
    QTimer m_timer;  // 旋转动画定时器
    int m_angle;     // 当前旋转角度（0~359）
};

#endif // MESSAGESTATUSINDICATOR_H
