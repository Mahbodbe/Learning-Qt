#include <QMainWindow>
#include <QMoveEvent>
#include <QStatusBar>

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(QWidget* parent = 0);
private:
	void moveEvent(QMoveEvent* event);

};