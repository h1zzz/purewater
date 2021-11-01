/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include <qapplication.h>
#include <qdebug.h>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow win;
    win.show();

    return app.exec();
}
