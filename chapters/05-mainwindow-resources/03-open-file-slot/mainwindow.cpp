#include "mainwindow.h"

MainWindow::MainWindow() {
	setWindowTitle("SRM System");
	setFixedSize(500, 500);
	QPixmap newIcon("new.png");
	QPixmap openIcon("open.png");
	QPixmap closeIcon("save.png");

	fileMenu = menuBar()->addMenu("&file");
	
	quitAction = new QAction(closeIcon, "Quit", this);
	quitAction->setShortcuts(QKeySequence::Quit);

	newAction = new QAction(newIcon, "&New", this);
	newAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));

	openAction = new QAction(openIcon, "&Open", this);
	openAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));

	fileMenu->addAction(newAction);
	fileMenu->addAction(openAction);
	fileMenu->addSeparator();
	fileMenu->addAction(quitAction);

	helpMenu = menuBar()->addMenu("H&elp");
	
	aboutAction = new QAction("About", this);
	aboutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
	helpMenu->addAction(aboutAction);

	connect(quitAction, &QAction::triggered, this, &QApplication::quit);
}