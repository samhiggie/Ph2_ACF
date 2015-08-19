#include "startup.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GUI::Startup startup;
    startup.show();

    return a.exec();
}
