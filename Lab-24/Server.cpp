#include "Server.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QString>
#include <QLabel>

Server::Server(QWidget *parent) : QMainWindow(parent)
{

    QWidget *ui_area = new QWidget;
    setCentralWidget(ui_area);

    QAction* startServer = new QAction("&Запустить сервер");
    QAction* stopServer = new QAction("&Остановить сервер");
    QAction* openImage = new QAction("&Открыть изображение");
    QAction* exit = new QAction("&Выход");

    auto fileMenu = menuBar()->addMenu(tr("&Файл"));
    fileMenu->addAction(startServer);
    fileMenu->addAction(stopServer);
    fileMenu->addAction(openImage);
    fileMenu->addAction(exit);
    connect(startServer, SIGNAL(triggered()), this, SLOT(startServ()));
    connect(stopServer, SIGNAL(triggered()), this, SLOT(stopServ()));
    connect(openImage, SIGNAL(triggered()), this, SLOT(openImg()));
    connect(exit, SIGNAL(triggered()), this, SLOT(Exit()));

    auto helpMenu = menuBar()->addMenu(tr("&Помощь"));
    QAction* about = new QAction("&О программе");
    helpMenu->addAction(about);
    connect(about, SIGNAL(triggered()), this, SLOT(showAboutWindow()));
    filename = "";

    QVBoxLayout *l = new QVBoxLayout(this);
    mImgViewer = new ImgViewer;
    l->addWidget(mImgViewer);

    ui_area->setLayout(l);
    resize(640,480);
}
void Server::startServ()
{
    qDebug() << "Starting server...";

    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newuser()));
    if (!tcpServer->listen(QHostAddress::Any, 31337) && server_status==0) {
        qDebug() <<  QObject::tr("Unable to start the server: %1.").arg(tcpServer->errorString());
    } else {
        server_status=1;
        qDebug() << tcpServer->isListening() << "TCPSocket listen on port";
        qDebug() << "Server started!";
    }
}
void Server::stopServ()
{
    qDebug() << "Stopping server...";

    if(server_status==1){
        foreach(int i, SClients.keys())
        {
            SClients[i]->close();
            SClients.remove(i);
        }
        tcpServer->close();
        qDebug() << "Server stopped!";
        server_status=0;
    }
    delete tcpServer;
}
void Server::newuser()
{
    if(server_status==1)
    {
        qDebug() << "New connection!";
        QTcpSocket* clientSocket = tcpServer->nextPendingConnection();
        int idusersocs = clientSocket->socketDescriptor();
        SClients[idusersocs] = clientSocket;
        connect(SClients[idusersocs], SIGNAL(readyRead()), this, SLOT(readClient()));
    }
}
void Server::readClient()
{
    QTcpSocket* clientSocket = (QTcpSocket*)sender();
    int idusersocs = clientSocket->socketDescriptor();

    QTextStream os(clientSocket);
    os.setAutoDetectUnicode(true);
    if(filename == "")
    {
        os << "HTTP/1.0 200 Ok\r\n"
            "Content-Type: text/html; charset=\"utf-8\"\r\n"
            "\r\n"
            "<h1>Nothing to see here</h1>";
    }
    else
    {
        clientSocket->write("HTTP/1.1 200 OK\r\n"
            "Content-Type: image/png; charset=\"utf-8\"\r\n"
            "\r\n");
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QByteArray mydata=file.readAll();
        clientSocket->write(mydata);
    }
    os << "\r\n\r\n";

    clientSocket->close();
    SClients.remove(idusersocs);

}
void Server::openImg()
{
    filename = QFileDialog::getOpenFileName(this, tr("Open Image"), "./", tr("Image Files (*.png)"));
    mImgViewer->setNewImg(filename);
}
void Server::showAboutWindow()
{
    QWidget* aboutWidget = new QWidget;

    aboutWidget->setWindowTitle("О программе");
    QLabel* info = new QLabel(aboutWidget);
    info->setText(QString("Автор: Мухамед Нугманов\r\nQT v: 5.15.3"));

    aboutWidget->show();
    aboutWidget->resize(400,300);
}
void Server::Exit()
{
    this->close();
}
Server::~Server()
{

}
