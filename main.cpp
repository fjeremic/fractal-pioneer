#include "FractalPioneer.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FractalPioneer w;
    w.show();
    return a.exec();
}
