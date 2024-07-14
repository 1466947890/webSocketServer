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

void Widget::on_pushButton_send_clicked()
{

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
}

void Widget::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket*> (sender());
    ui->listWidget_client->addItem(QString("客户端关闭, %1 %2:%3")
                                   .arg(pClient->peerName())
                                   .arg(pClient->peerAddress().toString())
                                   .arg(pClient->peerPort()));
}


