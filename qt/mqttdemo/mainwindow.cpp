#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_client = new QMqttClient(this);

    connect(m_client, &QMqttClient::stateChanged, this, &MainWindow::updateLogStateChange);
    connect(m_client, &QMqttClient::disconnected, this, &MainWindow::brokerDisconnected);
    connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
    const QString content = QDateTime::currentDateTime().toString()
              + QLatin1String(" Received Topic: ")
              + topic.name()
              + QLatin1String(" Message: ")
              + message
              + QLatin1Char('\n');
        qDebug()<<content;
           ui->textEditLog->insertPlainText(content);

    });
    connect(m_client, &QMqttClient::pingResponseReceived, this, [this]() {
    const QString content = QDateTime::currentDateTime().toString()
              + QLatin1String(" PingResponse")
              + QLatin1Char('\n');
        qDebug()<<content;
         ui->textEditLog->insertPlainText(content);
    });
    updateLogStateChange();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::updateLogStateChange()
{
    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(": State Change")
                    + QString::number(m_client->state())
                    + QLatin1Char('\n');
      qDebug()<<content;
       ui->textEditLog->insertPlainText(content);
}

void MainWindow::brokerDisconnected()
{

}

void MainWindow::on_pushButton_connect_clicked()
{
    m_client->setHostname(ui->lineEditHost->text());
    m_client->setPort(ui->lineEditPort->text().toInt());
    m_client->setUsername(ui->lineEditUserName->text());
    m_client->setPassword(ui->lineEditPassword->text());
     m_client->connectToHost();
}

void MainWindow::on_pushButton_pub_clicked()
{
    if (m_client->publish(ui->lineEditTopic->text(), ui->lineEditMessage->text().toUtf8()) == -1)
    {
        qDebug()<<"on_pushButton_pub_clicked ERR!!!!";
    }
    else
    {
        qDebug()<<"on_pushButton_pub_clicked SUCCESS!!!!";
    }
}

void MainWindow::on_pushButtonSub_clicked()
{
    auto subscription = m_client->subscribe(ui->lineEditSub->text());
    if (!subscription) {
        qDebug()<<"on_pushButtonSub_clicked ERR!!!!";
        return;
    }
    else
    {
        qDebug()<<"on_pushButtonSub_clicked SUCCESS!!!!";
    }

}
