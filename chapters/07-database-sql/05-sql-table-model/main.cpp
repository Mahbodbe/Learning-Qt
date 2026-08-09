#include <QApplication>
#include <QtSql>
#include <QDebug>
#include <QFile>
#include <QTextStream>

void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    QFile file("debug.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << msg << "\n";
    }
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    qInstallMessageHandler(customMessageHandler);

    QSqlDatabase db_conn = QSqlDatabase::addDatabase("QODBC", "contact_db");
    QString dsn = "DRIVER={MySQL ODBC 26.7 Unicode Driver};"
        "SERVER=127.0.0.1;"
        "DATABASE=contact_db;"
        "USER=root;"
        "PASSWORD=;"
        "PORT=3306;";
    db_conn.setDatabaseName(dsn);

    if (!db_conn.open()) {
        qDebug() << db_conn.lastError().text();
        return 1;
    }
    else {
        qDebug() << "Database connection established !";
    }

    QSqlTableModel* contactsTableModel = new QSqlTableModel(nullptr, db_conn);
    contactsTableModel->setTable("contacts");
    contactsTableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    contactsTableModel->select();

    for (int i = 0; i < contactsTableModel->rowCount(); ++i) {
        QSqlRecord record = contactsTableModel->record(i);
        QString id = record.value("id").toString();
        QString last_name = record.value("last_name").toString();
        QString first_name = record.value("first_name").toString();
        QString phone_number = record.value("phone_number").toString();
        qDebug() << id << " : " << first_name << " : " << last_name << " : " << phone_number;
    }

    int row = contactsTableModel->rowCount();
    contactsTableModel->insertRows(row, 1);
    contactsTableModel->setData(contactsTableModel->index(row, 1), "Stokes");
    contactsTableModel->setData(contactsTableModel->index(row, 2), "Nick");
    contactsTableModel->setData(contactsTableModel->index(row, 3), "+443569948");
    contactsTableModel->submitAll();

    qDebug() << "\nCustom filter: \n";
    contactsTableModel->setFilter("id=12 AND last_name LIKE 'Stokes'");
    contactsTableModel->select();

    for (int i = 0; i < contactsTableModel->rowCount(); ++i) {
        QSqlRecord record = contactsTableModel->record(i);
        QString id = record.value("id").toString();
        QString last_name = record.value("last_name").toString();
        QString first_name = record.value("first_name").toString();
        QString phone_number = record.value("phone_number").toString();
        qDebug() << id << " : " << first_name << " : " << last_name << " : " << phone_number;
    }

    delete contactsTableModel;
    return 0;
}