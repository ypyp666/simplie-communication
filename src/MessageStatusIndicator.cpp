#include "MessageStatusIndicator.h"
#include <QPainter>
#include <QPen>
#include <QMouseEvent>

MessageStatusIndicator::MessageStatusIndicator(QWidget *parent)
    : QWidget(parent), m_status(None), m_angle(0)
{
    setFixedSize(18, 18);  // 指示器大小

    // 旋转动画：每 50ms 转 30°，360° 一圈循环
    m_timer.setInterval(50);
    connect(&m_timer, &QTimer::timeout, this, [this]() {
        m_angle = (m_angle + 30) % 360;
        update();  // 触发重绘 paintEvent
    });

    hide();  // 默认隐藏，只有发送中/失败才显示
}

void MessageStatusIndicator::setStatus(Status status)
{
    if (m_status == status) {
        return;
    }
    m_status = status;

    if (status == Sending) {
        m_angle = 0;      // 每次进入发送中，从 0° 重新转
        m_timer.start();  // 启动旋转动画
        show();
        unsetCursor();    // 发送中不需要手型
    } else if (status == Failed) {
        m_timer.stop();   // 停止动画，显示静止的红色感叹号
        show();
        setCursor(Qt::PointingHandCursor);  // 失败时可点击重发，悬停显示手型
    } else {
        m_timer.stop();
        hide();
        unsetCursor();
    }
    update();
}

// 点击红色感叹号 → 触发重发（仅在发送失败状态下有效）
void MessageStatusIndicator::mousePressEvent(QMouseEvent *event)
{
    if (m_status == Failed && event->button() == Qt::LeftButton) {
        emit retryClicked();
    }
    QWidget::mousePressEvent(event);
}

void MessageStatusIndicator::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 抗锯齿，让圆圈/感叹号边缘平滑

    const int size = qMin(width(), height());
    QRectF rect((width() - size) / 2.0, (height() - size) / 2.0, size, size);

    if (m_status == Sending) {
        // ===== 发送中：旋转小圆圈（一段弧线，缺口让它看起来在转） =====
        QPen pen(QColor("#4a90d9"), 2.0, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        QRectF arcRect = rect.adjusted(2, 2, -2, -2);
        // drawArc 的角度单位是 1/16 度；留 90° 缺口，更像加载动画
        painter.drawArc(arcRect, m_angle * 16, 270 * 16);
    } else if (m_status == Failed) {
        // ===== 发送失败：红色圆形底 + 白色感叹号 =====
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#e64545"));
        painter.drawEllipse(rect);

        qreal cx = rect.center().x();
        painter.setPen(QPen(Qt::white, 2.0, Qt::SolidLine, Qt::RoundCap));
        // 感叹号的竖线
        painter.drawLine(QPointF(cx, rect.top() + 4.5), QPointF(cx, rect.center().y() + 1.0));
        // 感叹号下面的圆点
        painter.setBrush(Qt::white);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(cx, rect.bottom() - 4.0), 1.6, 1.6);
    }
}
