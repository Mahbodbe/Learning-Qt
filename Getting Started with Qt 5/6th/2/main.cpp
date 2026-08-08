#include <QApplication>
#include "mainwindow.h"
int main(int argc, char* aregv[]) {

	QApplication app(argc, aregv);
	MainWindow window;
	window.resize(300, 300);
	window.show();

	return app.exec();
}