/*
 * MIT License Copyright (c) 2021, h1zzz
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qnetworkaccessmanager.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {}
