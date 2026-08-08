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
        qDebug() << "Database connection error:" << db_conn.lastError().text();
        return 1;
    }

    qDebug() << "Database connection established !";

    QSqlQuery statement(db_conn);

    if (statement.exec("SELECT first_name, last_name, phone_number FROM contacts")) {

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
    }
    else {
        qDebug() << "Query execution failed:" << statement.lastError().text();
    }

    QSqlQuery insert_statement(db_conn);

    insert_statement.prepare("INSERT INTO contacts (last_name, first_name, phone_number) "
        "VALUES (?, ?, ?)");

    insert_statement.addBindValue("Sidle");
    insert_statement.addBindValue("Sara");
    insert_statement.addBindValue("+14495849555");
    insert_statement.exec();


    insert_statement.prepare("INSERT INTO contacts (last_name, first_name, phone_number)"
        "VALUES (?, ?, ?)");
    insert_statement.bindValue(2, "+144758849555");
    insert_statement.bindValue(1, "Brass");
    insert_statement.bindValue(0, "Jim");
    insert_statement.exec();

    insert_statement.prepare("INSERT INTO contacts (last_name, first_name, phone_number)" 
    "VALUES (:last_name, :first_name, :phone_number)");
    insert_statement.bindValue(":last_name", "Brown");
    insert_statement.bindValue(":first_name", "Warrick");
    insert_statement.bindValue(":phone_number", "+7494588594");
    insert_statement.exec();

    return 0;
}