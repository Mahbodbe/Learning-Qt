#include "QtWidgetsApplication.h"

QtWidgetsApplication::QtWidgetsApplication(QWidget *parent)
    : QMainWindow(parent)
{

    ui.setupUi(this);
	model = new QStandardItemModel(this);
	
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
	connect(ui.clearAllButton, &QPushButton::clicked, this, &QtWidgetsApplication::deleteAll);
	connect(ui.actionDeleteRecord, &QAction::triggered, this, &QtWidgetsApplication::deleteRecord);
	connect(ui.actionDeleteAll, &QAction::triggered, this, &QtWidgetsApplication::deleteAll);
	
}

void QtWidgetsApplication::saveButtonClicked() {

	if (ui.nameLineEdit->text().trimmed().isEmpty() || ui.phoneNumberLineEdit->text().trimmed().isEmpty()) {
		QMessageBox::warning(this,
			tr("Validation Error"),
			tr("Please fill in all required fields!"));
		return; 
	}

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

void QtWidgetsApplication::deleteRecord() {
	int totalRows = ui.appTable->rowCount();

	if (totalRows == 0) {
		QMessageBox::information(this, tr("Delete Record"), tr("No records to delete."));
		return;
	}

	bool ok;
	int rowId = QInputDialog::getInt(this, tr("Select Row to delete"),
		tr("Please enter Row ID of record (e.g. 1):"),
		1, 1, totalRows, 1, &ok);
	if (ok) {
		ui.appTable->removeRow(rowId - 1);
	}
}

void QtWidgetsApplication::deleteAll() {
	if (ui.appTable->rowCount() == 0) return;

	int status = QMessageBox::question(this, tr("Delete Records?"),
		tr("Are you sure you want to delete all records?"),
		QMessageBox::Yes | QMessageBox::No);

	if (status == QMessageBox::Yes) {
		ui.appTable->setRowCount(0); 
	}
}