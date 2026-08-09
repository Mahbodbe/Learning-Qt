#include "QtWidgetsApplication.h"

QtWidgetsApplication::QtWidgetsApplication(QWidget *parent)
    : QMainWindow(parent)
{

    ui.setupUi(this);
	model = new QStandardItemModel(this);
	QString modernStyle = R"(
		MainWindow {
			border: 1px solid #d1d1d1;
		}
        QWidget {
            font-family: 'Segoe UI', 'Vazirmatn', sans-serif;
            font-size: 10pt;
            background-color: #f8f9fa;
            color: #1c1c1c;
        }

        QLineEdit, QDateEdit {
            background-color: #ffffff;
            border: 1px solid #d1d1d1;
            border-radius: 6px;
            padding: 6px 10px;
            selection-background-color: #0067c0;
        }

        QLineEdit:hover, QDateEdit:hover {
            border: 1px solid #868686;
        }

        QLineEdit:focus, QDateEdit:focus {
            border: 2px solid #0067c0;
            padding: 5px 9px;
        }

        QPushButton {
            background-color: #ffffff;
            border: 1px solid #d1d1d1;
            border-radius: 6px;
            padding: 6px 16px;
            font-weight: 500;
        }

        QPushButton:hover {
            background-color: #f3f3f3;
            border-color: #c7c7c7;
        }

        QPushButton:pressed {
            background-color: #e5e5e5;
        }

        QPushButton#saveButton {
            background-color: #0067c0;
            color: #ffffff;
            border: none;
            font-weight: 600;
        }

        QPushButton#saveButton:hover {
            background-color: #1875d1;
        }

        QPushButton#saveButton:pressed {
            background-color: #005aab;
        }

        QTableWidget {
            background-color: #ffffff;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            gridline-color: #f0f0f0;
            selection-background-color: #e5f3ff;
            selection-color: #000000;
            outline: none;
        }

        QHeaderView::section {
            background-color: #f3f3f3;
            color: #404040;
            padding: 8px;
            border: none;
            border-bottom: 2px solid #e0e0e0;
            font-weight: 600;
        }

        QToolBar {
            background-color: #ffffff;
            border-bottom: 1px solid #e5e5e5;
            spacing: 6px;
            padding: 4px;
        }

        QToolButton {
            border-radius: 4px;
            padding: 4px;
        }

        QToolButton:hover {
            background-color: #f0f0f0;
        }

        QMenuBar {
            background-color: #ffffff;
            border-bottom: 1px solid #f0f0f0;
        }

        QMenuBar::item:selected {
            background-color: #f0f0f0;
            border-radius: 4px;
        }
    )";

	this->setStyleSheet(modernStyle);
	
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
	connect(ui.actionNew, &QAction::triggered, this, &QtWidgetsApplication::ifNew);
	connect(ui.actionSaveAll, &QAction::triggered, this, &QtWidgetsApplication::writeServer);
	connect(ui.actionOpen, &QAction::triggered, this, &QtWidgetsApplication::openFile);
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
	notSave = true;
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
		notSave = (ui.appTable->rowCount() > 0);
	}
}

void QtWidgetsApplication::deleteAll() {
	if (ui.appTable->rowCount() == 0) return;

	int status = QMessageBox::question(this, tr("Delete Records?"),
		tr("Are you sure you want to delete all records?"),
		QMessageBox::Yes | QMessageBox::No);

	if (status == QMessageBox::Yes) {
		ui.appTable->setRowCount(0); 
		notSave = false;
	}
}

void QtWidgetsApplication::ifNew() {
	if (!checkIfSave()) return;
	ui.appTable->setRowCount(0);
	clearfield();
	notSave = false;

}

void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
	QFile file("debug.txt");
	if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
		QTextStream stream(&file);
		stream << msg << "\n";
	}
}

bool connectDB(QSqlDatabase& db) {
	qInstallMessageHandler(customMessageHandler);

	if (QSqlDatabase::contains("contact_db")) {
		db = QSqlDatabase::database("contact_db");
		if (db.isOpen()) return true;
		return db.open();
	}

	db = QSqlDatabase::addDatabase("QODBC", "contact_db");
	QString dsn = "DRIVER={MySQL ODBC 26.7 Unicode Driver};"
		"SERVER=127.0.0.1;"
		"DATABASE=contact_db;"
		"USER=root;"
		"PASSWORD=;"
		"PORT=3306;";
	db.setDatabaseName(dsn);

	if (!db.open()) {
		qDebug() << "DB Connection Error:" << db.lastError().text();
		return false;
	}

	qDebug() << "Database connection established!";
	return true;
}

void QtWidgetsApplication::writeServer() {
	int totalRows = ui.appTable->rowCount();

	if (totalRows == 0) {
		QMessageBox::information(this, tr("Write to Server"), tr("No records in table to save!"));
		return;
	}

	int status = QMessageBox::question(this, tr("Save Records?"),
		tr("Are you sure you want to write all records on server?"),
		QMessageBox::Yes | QMessageBox::No);

	if (status != QMessageBox::Yes) return;

	QSqlDatabase db;
	if (!connectDB(db)) {
		QMessageBox::critical(this, tr("Database Error"),
			tr("Could not connect to server!\nCheck debug.txt for logs."));
		return;
	}

	QSqlQuery createTableQuery(db);
	QString createTableSQL =
		"CREATE TABLE IF NOT EXISTS contacts ("
		"id INT AUTO_INCREMENT PRIMARY KEY, "
		"name VARCHAR(255), "
		"boc VARCHAR(100), "
		"phone_number VARCHAR(50)"
		");";
	if (!createTableQuery.exec(createTableSQL)) {
		QMessageBox::critical(this, tr("Database Error"),
			tr("Failed to create table:\n") + createTableQuery.lastError().text());
		return;
	}

	db.transaction();

	
	bool hasError = false;
	QString lastSQLError = "";

	for (int row = 0; row < totalRows; ++row) {
		QTableWidgetItem* nameItem = ui.appTable->item(row, 0);
		QTableWidgetItem* phoneItem = ui.appTable->item(row, 2);
		QTableWidgetItem* bOc = ui.appTable->item(row, 1);

		if (!nameItem || !bOc || !phoneItem) continue;

		QString fullName = nameItem->text().trimmed();
		QString phone = phoneItem->text().trimmed();
		QString boc = bOc->text().trimmed();

		QSqlQuery query(db);
		query.prepare("INSERT INTO contacts (name, boc, phone_number) VALUES (:Name, :boc, :phone)");
		query.bindValue(":Name", fullName);
		query.bindValue(":boc", boc);
		query.bindValue(":phone", phone);

		if (!query.exec()) {
			lastSQLError = query.lastError().text();
			qDebug() << "Insert error at row" << row << ":" << query.lastError().text();
			hasError = true;
			break;
		}
	}

	if (!hasError) {
		db.commit();
		notSave = false;
		QMessageBox::information(this, tr("RMS System"), tr("All records wrote to server successfully!"));
	}
	else {
		db.rollback(); 
		QMessageBox::critical(this, tr("Database Error"),
			tr("Failed to write records to server.\n\nError details:\n") + lastSQLError);
	
	}
}

void QtWidgetsApplication::openFile() {
	if (!checkIfSave()) return;
	QString fileName = QFileDialog::getOpenFileName(
		this,
		tr("Open Contacts File"),
		"",
		tr("CSV Files (*.csv);;Text Files (*.txt);;All Files (*)")
	);
	if (fileName.isEmpty()) return;
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::critical(this, tr("File Error"), tr("Could not open file for reading!"));
		return;
	}
	QTextStream in(&file);
	ui.appTable->setRowCount(0);
	clearfield();
	while (!in.atEnd()) {
		QString line = in.readLine().trimmed();
		if (line.isEmpty()) continue;

		QStringList parts = line.split(',');
		if (parts.size() >= 3) {
			int row = ui.appTable->rowCount();
			ui.appTable->insertRow(row);
			ui.appTable->setItem(row, 0, new QTableWidgetItem(parts[0].trimmed()));
			ui.appTable->setItem(row, 1, new QTableWidgetItem(parts[1].trimmed()));
			ui.appTable->setItem(row, 2, new QTableWidgetItem(parts[2].trimmed()));
		}
	}

	file.close();
	notSave = true;
	QMessageBox::information(this, tr("RMS System"), tr("File loaded successfully!"));
}

bool QtWidgetsApplication::checkIfSave() {
	if (notSave && ui.appTable->rowCount() > 0) {
		QMessageBox::StandardButton resBtn = QMessageBox::question(
			this,
			tr("Unsaved Changes"),
			tr("You have unsaved changes. Do you want to save them to the server first?"),
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
			QMessageBox::Save
		);

		if (resBtn == QMessageBox::Save) {
			writeServer();
			if (notSave) return false;
		}
		else if (resBtn == QMessageBox::Cancel) return false;
	}

	return true;
}