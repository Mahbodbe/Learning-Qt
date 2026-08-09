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

    return 0;
}