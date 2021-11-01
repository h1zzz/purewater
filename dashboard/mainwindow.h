/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include <qmainwindow.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};

#endif /* mainwindow.h */
