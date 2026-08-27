#ifndef CHATINPUT_H
#define CHATINPUT_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QFileDialog>

class ChatInput : public QWidget
{
    Q_OBJECT
public:
    explicit ChatInput(QWidget *parent = nullptr);

signals:
    void sendMessage(const QString& content);
    void sendFile(const QString& filePath);

private slots:
    void onSendClicked();
    void onFileClicked();
    void onEnterPressed();
    void adjustHeight();

private:
    QVBoxLayout* layout;
    QTextEdit* inputEdit;
    QPushButton* fileButton;
    QPushButton* sendButton;
};

#endif // CHATINPUT_H