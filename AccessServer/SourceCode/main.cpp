#include <QCoreApplication>
#include "daemons.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if(geteuid() != 0)
    {
        qDebug() << "权限不足！";
        return 0;
    }
    Daemons daemons;


    return a.exec();
}
