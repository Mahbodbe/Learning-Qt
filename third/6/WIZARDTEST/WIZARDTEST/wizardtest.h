#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_wizardtest.h"

class WIZARDTEST : public QMainWindow
{
    Q_OBJECT

public:
    WIZARDTEST(QWidget *parent = nullptr);
    ~WIZARDTEST();

private:
    Ui::WIZARDTESTClass ui;
};
