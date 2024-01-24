#ifndef IMAGESERVER_H
#define IMAGESERVER_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QTcpSocket>
#include <QTcpServer>
#include <QFileDialog>
#include <QMenuBar>
#include <QPainter>
#include <QLabel>

class ImageServer : public QMainWindow {
    Q_OBJECT

private:
    QString name_image;
    QTcpServer *serverTCP;
    QString filename;
    int server_status;
    QMap<int, QTcpSocket*> clients;

private slots:
    void onServer();
    void offServer();
    void openImage();
    void turnOffProgram();
    void startAbout();

    void readClient();
    void newUser();

    void setNewImage(QString imagename);
public:
    ImageServer(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *p = nullptr);
};

#endif
