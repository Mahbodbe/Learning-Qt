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

    QString table_definition = "CREATE TABLE IF NOT EXISTS contacts ("
        "id INT AUTO_INCREMENT,"
        "last_name VARCHAR(255) NOT NULL,"
        "first_name VARCHAR(255) NOT NULL,"
        "phone_number VARCHAR(255) NOT NULL,"
        "PRIMARY KEY (id)"
        ") ENGINE=INNODB;";
    QSqlQuery table_creator(table_definition, db_conn);

    QSqlQuery insert_statement(db_conn);
    insert_statement.prepare("INSERT INTO contacts (last_name, first_name, phone_number) VALUES (?, ?, ?)");
    insert_statement.addBindValue("Sidle");
    insert_statement.addBindValue("Sara");
    insert_statement.addBindValue("+14495849555");
    insert_statement.exec();

    insert_statement.prepare("INSERT INTO contacts (last_name, first_name, phone_number) VALUES (?, ?, ?)");
    insert_statement.bindValue(0, "Brass");
    insert_statement.bindValue(1, "Jim");
    insert_statement.bindValue(2, "+144758849555");
    insert_statement.exec();

    insert_statement.prepare("INSERT INTO contacts (last_name, first_name, phone_number) VALUES (:last_name, :first_name, :phone_number)");
    insert_statement.bindValue(":last_name", "Brown");
    insert_statement.bindValue(":first_name", "Warrick");
    insert_statement.bindValue(":phone_number", "+7494588594");
    insert_statement.exec();

    QSqlQuery delete_statement(db_conn);
    delete_statement.exec("DELETE FROM contacts WHERE first_name = 'Warrick'");
    qDebug() << "Number of rows affected: " << delete_statement.numRowsAffected();

    QSqlQuery update_statement(db_conn);
    update_statement.exec("UPDATE contacts SET first_name='Jude' WHERE id=1");
    qDebug() << "Number of rows affected: " << update_statement.numRowsAffected();

    QSqlQuery statement("SELECT * FROM contacts", db_conn);
    QSqlRecord record = statement.record();
    int fnIdx = record.indexOf("first_name");
    int lnIdx = record.indexOf("last_name");
    int phoneIdx = record.indexOf("phone_number");

    while (statement.next()) {
        QString firstName = statement.value(fnIdx).toString();
        QString lastName = statement.value(lnIdx).toString();
        QString phoneNumber = statement.value(phoneIdx).toString();
        qDebug() << firstName << " - " << lastName << " - " << phoneNumber;
    }

    return 0;
}