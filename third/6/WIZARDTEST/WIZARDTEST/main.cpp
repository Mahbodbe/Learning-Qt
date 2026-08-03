#include "wizardtest.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WIZARDTEST w;
    w.show();
    return a.exec();
}
