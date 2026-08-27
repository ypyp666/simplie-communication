#include "StatusBar.h"
#include <QHBoxLayout>

StatusBar::StatusBar(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("background-color: #f5f5f5; border-top: 1px solid #e0e0e0;");

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);

    statusLabel = new QLabel("就绪", this);
    statusLabel->setStyleSheet("color: #666; font-size: 12px;");

    layout->addWidget(statusLabel);
    layout->addStretch();
}

void StatusBar::setStatus(const QString& status)
{
    statusLabel->setText(status);
}
