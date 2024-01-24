#include <QApplication>
#include "ImageServer.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    ImageServer w;

    w.show();
    return app.exec();
}
