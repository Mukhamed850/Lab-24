#include "ImageServer.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QString>
#include <QLabel>

ImageServer::ImageServer(QWidget *parent) : QMainWindow(parent) {
    // Инициализация основного виджета
    QWidget *mainWidget = new QWidget;
    setCentralWidget(mainWidget);

    // Создание действий для меню
    QAction* ActStartServer = new QAction("&Запустить сервер");
    QAction* ActStopServer  = new QAction("&Остановить сервер");
    QAction* ActOpenImage   = new QAction("&Открыть изображение");
    QAction* ActExit        = new QAction("&Выход");

    // Добавление действий в меню
    auto fileMenu = menuBar()->addMenu(tr("&Файл"));
    fileMenu->addAction(ActStartServer);
    fileMenu->addAction(ActStopServer);
    fileMenu->addAction(ActOpenImage);
    fileMenu->addAction(ActExit);

    // Подключение слотов к сигналам для каждого действия
    connect(ActStartServer, SIGNAL(triggered()), this, SLOT(onServer()));
    connect(ActStopServer,  SIGNAL(triggered()), this, SLOT(offServer()));
    connect(ActOpenImage,   SIGNAL(triggered()), this, SLOT(openImage()));
    connect(ActExit,        SIGNAL(triggered()), this, SLOT(turnOffProgram()));

    // Создание действия "О программе" и добавление его в меню "Помощь"
    auto helpMenu = menuBar()->addMenu(tr("&Помощь"));
    QAction* about = new QAction("&О программе");
    helpMenu->addAction(about);

    // Подключение слота для отображения окна "О программе"
    connect(about, SIGNAL(triggered()), this, SLOT(startAbout()));

    // Инициализация переменной для хранения пути к изображению
    filename = "";


    QVBoxLayout *l = new QVBoxLayout(this);
    mainWidget->setLayout(l);

    // Установка начального размера окна
    resize(700,500);
}

void ImageServer::onServer() {
    // Метод запуска сервера
    qDebug() << "Starting...";

    // Создание объекта сервера
    serverTCP = new QTcpServer(this);

    // Подключение слота для обработки новых подключений
    connect(serverTCP, SIGNAL(newConnection()), this, SLOT(newUser()));

    // Попытка запуска сервера
    if (!serverTCP->listen(QHostAddress::Any, 31337) && server_status==0)
        qDebug() <<  QObject::tr("Unable to start the server: %1.").arg(serverTCP->errorString());
    else {
        // Установка статуса сервера и вывод сообщений о запуске
        server_status = 1;
        qDebug() << serverTCP->isListening() << "TCPSocket listen on port";
        qDebug() << "Started!";
    }
}

void ImageServer::offServer() {
    // Метод остановки сервера
    qDebug() << "Stopping...";

    if(server_status) {
        // Закрытие соединений с клиентами
        foreach(int i, clients.keys()) {
            clients[i]->close();
            clients.remove(i);
        }

        // Закрытие сервера и вывод сообщения об остановке
        serverTCP->close();
        qDebug() << "Stopped!";
        server_status = 0;
    }

    // Освобождение памяти, выделенной для объекта сервера
    delete serverTCP;
}

void ImageServer::newUser() {
    // Метод обработки нового подключения клиента
    if(server_status) {
        qDebug() << "New connection!";

        // Создание сокета для нового клиента
        QTcpSocket* clientSocket = serverTCP->nextPendingConnection();
        int id_socket_users = clientSocket->socketDescriptor();

        // Добавление клиента в QMap
        clients[id_socket_users] = clientSocket;

        // Подключение слота для обработки данных от клиента
        connect(clients[id_socket_users], SIGNAL(readyRead()), this, SLOT(readClient()));
    }
}

void ImageServer::readClient() {
    // Метод обработки данных от клиента
    QTcpSocket* clientSocket = (QTcpSocket*)sender();
    int id_socket_users = clientSocket->socketDescriptor();

    QTextStream os(clientSocket);
    os.setAutoDetectUnicode(true);

    // Отправка HTTP-ответа
    if(filename == "")
        os << "HTTP/1.0 200 Ok\r\nContent-Type: text/html; charset=\"utf-8\"\r\n\r\n<h1>Nothing to see here</h1>";
    else {
        clientSocket->write("HTTP/1.1 200 OK\r\nContent-Type: image/png; charset=\"utf-8\"\r\n\r\n");
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QByteArray mydata=file.readAll();
        clientSocket->write(mydata);
    }

    // Завершение соединения и удаление клиента из QMap
    os << "\r\n\r\n";
    clientSocket->close();
    clients.remove(id_socket_users);
}

void ImageServer::startAbout() {
    // Метод отображения окна "О программе"
    QWidget* aboutWidget = new QWidget;
    aboutWidget->setWindowTitle("О программе");
    QLabel* info = new QLabel(aboutWidget);
    info->setText(QString("Автор: Мухамед Нугманов\r\nВерсия Qt: 5.15.3"));
    aboutWidget->show();
    aboutWidget->resize(300, 150);
}

void ImageServer::openImage() {
    // Метод открытия изображения
    filename = QFileDialog::getOpenFileName(this, tr("Открыть изображение"), "./", tr("Файлы изображения (*.png)"));
    setNewImage(filename);
}

void ImageServer::turnOffProgram() {
    // Метод закрытия программы
    this->close();
}

void ImageServer::paintEvent(QPaintEvent *) {
    // Метод отрисовки изображения в графическом окне
    QPainter painter(this);
    painter.drawImage(QRectF((qreal)(this->width())/2 - (qreal)(this->width())/3, 0, (qreal)(this->width()*2)/3, (qreal)(this->height()*2)/3), QImage(name_image));
}

void ImageServer::setNewImage(QString imagename) {
    // Метод установки нового изображения и обновление отображения
    this->name_image = imagename;
    update();
}
