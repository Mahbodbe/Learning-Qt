#include "QtWidgetsApplication.h"

QtWidgetsApplication::QtWidgetsApplication(QWidget *parent)
    : QMainWindow(parent)
{

    ui.setupUi(this);
	model = new QStandardItemModel(this);
	//ui.appTable->setModel(model);
	signal();
}

QtWidgetsApplication::~QtWidgetsApplication()
{}

void QtWidgetsApplication::signal() {
	//connect(ui.actionQuit, &QAction::triggered, this, &QApplication::quit);
	//connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(aboutDialog()));
	//connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(saveButtonClicked()));
	//connect(ui.saveButton, SIGNAL(clicked()), this, SLOT(saveButtonClicked()));
	connect(ui.actionAbout, &QAction::triggered, this, &QtWidgetsApplication::aboutDialog);
	connect(ui.actionSave, &QAction::triggered, this, &QtWidgetsApplication::saveButtonClicked);
	connect(ui.saveButton, &QPushButton::clicked, this, &QtWidgetsApplication::saveButtonClicked);
}

void QtWidgetsApplication::saveButtonClicked() {
	int row = ui.appTable->rowCount();
	ui.appTable->insertRow(row);
	ui.appTable->setItem(row, 0, new QTableWidgetItem(ui.nameLineEdit->text()));
	ui.appTable->setItem(row, 1, new QTableWidgetItem(ui.dateEdit->date().toString()));
	ui.appTable->setItem(row, 2, new QTableWidgetItem(ui.phoneNumberLineEdit->text()));

	clearfield();
	QMessageBox::information(this, tr("RMS System"), tr("Record saved successfully!"));
}

void QtWidgetsApplication::aboutDialog()  {
	QMessageBox::about(this, "About RMS System", "RMS System 2.0" "<p>Copyright &copy; 2005 Inc." "This is a simple application to demonstrate the use of windows, " "tool bars, menus and dialog boxes");
}

void QtWidgetsApplication::clearfield() {
	ui.nameLineEdit->clear();
	ui.phoneNumberLineEdit->setText("");
	QDate dateOfBirth(1980, 1, 1);
	ui.dateEdit->setDate(dateOfBirth);
}