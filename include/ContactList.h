#ifndef CONTACTLIST_H
#define CONTACTLIST_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include "ChatBackend.h"

class ContactList : public QWidget
{
    Q_OBJECT
public:
    explicit ContactList(QWidget *parent = nullptr);
    void setContacts(const QList<ContactInfo>& contacts);

signals:
    void contactSelected(const QString& contactId, const QString& contactName);

private slots:
    void onItemClicked(QListWidgetItem* item);

private:
    QListWidget* listWidget;
    QVBoxLayout* layout;
};

#endif // CONTACTLIST_H