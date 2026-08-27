#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QWidget>
#include <QLabel>

class StatusBar : public QWidget
{
    Q_OBJECT
public:
    explicit StatusBar(QWidget *parent = nullptr);

public slots:
    void setStatus(const QString& status);

private:
    QLabel* statusLabel;
};

#endif // STATUSBAR_H
