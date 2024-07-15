#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->pushButton_close->setEnabled(false);
    webSocketServer = new QWebSocketServer("server", QWebSocketServer::NonSecureMode);

}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_start_clicked()
{
    QString port = ui->spinBox_Port->text();
//    qDebug() << "端口为" << port;
    if(webSocketServer->listen(QHostAddress::LocalHost, port.toInt()))
    {
        ui->listWidget_client->addItem(QString("服务开启成功，监听端口：%1").arg(port));
        ui->pushButton_close->setEnabled(true);
        ui->pushButton_start->setEnabled(false);
        connect(webSocketServer, &QWebSocketServer::newConnection, this, &Widget::newConnect);
    }else{
        connect(webSocketServer, &QWebSocketServer::closed, this, &Widget::socketDisconnected);
        qDebug() << "打开端口失败";
    }

}

void Widget::on_pushButton_close_clicked()
{
    ui->pushButton_close->setEnabled(false);
    ui->pushButton_start->setEnabled(true);
    webSocketServer->close();
}

// 群发功能
void Widget::on_pushButton_send_clicked()
{
    QString msg = ui->textEdit_sendMsg->toPlainText();
    for(int i=0; i < m_clients.size(); i++)
    {
        QWebSocket* m_client = m_clients.at(i);
        qDebug() << m_client->peerAddress()<< ":" << m_client->peerPort();
        m_client->sendTextMessage(msg);
    }

}

// 处理新一个客户端的连接信息
void Widget::newConnect()
{
    pSocket = webSocketServer->nextPendingConnection();
    QString name = pSocket->peerName();
    QString address = pSocket->peerAddress().toString();
//    qint16 port = pSocket->peerPort();
    ui->listWidget_client->addItem(QString("新客户端连接, %1 %2:%3").arg(name).arg(address).arg(pSocket->peerPort()));
    // 处理收到消息
    connect(pSocket, &QWebSocket::textMessageReceived, this, &Widget::processTextMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &Widget::socketDisconnected);
    m_clients << pSocket;
}

void Widget::processTextMessage(QString msg)
{
    QWebSocket *pClient = qobject_cast<QWebSocket*> (sender());
    QString receiveMsg = QString("clientID: %1:%2  received: %3").arg(pClient->peerAddress().toString()).arg(pClient->peerPort()).arg(msg);
    ui->listWidget_reciveMsg->addItem(receiveMsg);
    // 接下来对websocekt消息进行处理
    // 转换json数据为可读数据
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
    if(!doc.isObject())
    {
        qDebug() << "不是json数据";
        return;
    }
    /*
     * 这里协议规定一下
     * 当获取的值为1时则是获取联系人列表
     * 当获取的值为2时则是聊天内容转发
     *
     */
    QJsonObject obj = doc.object();
    int type = obj["type"].toInt();
//    qDebug() << type;
    switch (type) {
    case 1:
        // 执行获取联系人列表
        sendContacts(pClient);
        break;
    case 2:
        sendMsg(obj, pClient);
        // 执行消息转发
        break;
    }
}

void Widget::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket*> (sender());
    ui->listWidget_client->addItem(QString("客户端关闭, %1 %2:%3")
                                   .arg(pClient->peerName())
                                   .arg(pClient->peerAddress().toString())
                                   .arg(pClient->peerPort()));
    m_clients.removeOne(pClient);
}

void Widget::sendContacts(QWebSocket *pClient)
{
    // 获取联系人列表，暂时定为内网地址加端口
    QJsonObject info;
    QJsonArray contactArr;
    for(int i=0; i<m_clients.size(); i++)
    {
        QWebSocket* client = m_clients.at(i);
        contactArr.append(QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort()));
    }
    info.insert("type", 1);
    info.insert("contacts", contactArr);
    QJsonDocument doc(info);
    pClient->sendTextMessage(doc.toJson());

}

void Widget::sendMsg(QJsonObject obj, QWebSocket *pClient)
{
    QString msg = obj["msg"].toString();
    QString contact = obj["contact"].toString();
    // 当前要聊天的对象
    for(int i=0; i < m_clients.size(); i++)
    {
        QWebSocket* client = m_clients.at(i);
        QString clienName = QString("%1:%2").arg(client->peerAddress().toString()).arg(client->peerPort());
        if(clienName == contact)
        {
            QJsonObject obj;
            obj.insert("type", 2);
            obj.insert("msg", msg);
            obj.insert("from", contact);
            QJsonDocument doc(obj);
            client->sendTextMessage(doc.toJson());
            break;
        }
    }
}


