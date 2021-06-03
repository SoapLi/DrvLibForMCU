#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMqtt/qmqttclient.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private:
    Ui::MainWindow *ui;
       QMqttClient*m_client;

private slots:
       void updateLogStateChange();
       void brokerDisconnected();

       void on_pushButton_connect_clicked();
       void on_pushButton_pub_clicked();
       void on_pushButtonSub_clicked();
};
#endif // MAINWINDOW_H
