#pragma once

#include <QMainWindow>
#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QDate>
#include <QDateEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QtGui>
#include <Qt>
#include <QToolBar>
#include <QTableView>
#include <QHeaderView>
#include <QInputDialog>

#include "ui_MainWindow.h"

class QtWidgetsApplication : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication(QWidget *parent = nullptr);
    ~QtWidgetsApplication();

private:
    Ui::MainWindow ui;
    void signal();
    QAction* actionQuit;

    QLineEdit* nameLineEdit;
    QStandardItemModel* model;
    


private slots:
    void saveButtonClicked();
    void aboutDialog();
    void clearfield();
};
