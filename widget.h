#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QList>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_start_clicked();
    void on_pushButton_close_clicked();
    void on_pushButton_send_clicked();

    void newConnect();
    void processTextMessage(QString msg);
    void socketDisconnected();

private:
    Ui::Widget *ui;
    QWebSocketServer *webSocketServer;
    QWebSocket *pSocket;
    QList<QWebSocket *> m_clients;

private:
    void sendContacts(QWebSocket* pClient);
    void sendMsg(QJsonObject obj, QWebSocket* pClient);
    void sendOnlineStatus();
};

#endif // WIDGET_H
