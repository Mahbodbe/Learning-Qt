#include <QApplication>
#include <QDial>
#include <QLabel>
#include <QVBoxLayout>
#include <QLCDNumber>
#include <QSlider>



int main(int argc, char* argv[]) {

	QApplication app(argc, argv);
	QWidget* window = new QWidget;
	QVBoxLayout* layout = new QVBoxLayout;
	QSlider* lengthSlider = new QSlider(Qt::Horizontal);
	QDial* volumeDial = new QDial;
	QLCDNumber* volumeLCD = new QLCDNumber;
	
	volumeLCD->setPalette(Qt::blue);
	lengthSlider->setTickInterval(10);
	lengthSlider->setSingleStep(1);
	
	lengthSlider->setTickPosition(QSlider::TicksAbove);
	lengthSlider->setMaximum(0);
	lengthSlider->setMaximum(250);

	volumeDial->setNotchesVisible(true);
	volumeDial->setMinimum(0);
	volumeDial->setMaximum(250);


	layout->addWidget(volumeDial);
	layout->addWidget(lengthSlider);
	layout->addWidget(volumeLCD);

	//QObject::connect(lengthSlider, SIGNAL(valueChanged(int)), volumeLCD , SLOT(display(int)));
	//QObject::connect(volumeDial, SIGNAL(valueChanged(int)), volumeLCD, SLOT(display(int)));

	// for connecting slider to dial and dial to slider
	QObject::connect(lengthSlider, SIGNAL(valueChanged(int)), volumeDial, SLOT(setValue(int)));
	QObject::connect(volumeDial, SIGNAL(valueChanged(int)), lengthSlider, SLOT(setValue(int)));
	// after connecting them to each other no need to connect both to LCD
	QObject::connect(lengthSlider, SIGNAL(valueChanged(int)), volumeLCD, SLOT(display(int)));


	window->setLayout(layout);
	window->show();
	return app.exec();
}