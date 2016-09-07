#include "CWinFlash.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CWinFlash w;
    w.show();

    return a.exec();
}
