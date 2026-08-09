#include <QApplication>
#include <QMainWindow>
#include <QLabel>

int main(int argc, char* argv[]) {

	QApplication app(argc, argv);
	QMainWindow mainWindow;
	mainWindow.show();

	return app.exec();

}