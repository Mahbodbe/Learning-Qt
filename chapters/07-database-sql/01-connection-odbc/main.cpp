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

/*
 use contact_db;
 CREATE TABLE IF NOT EXISTS contacts (
 id INT AUTO_INCREMENT,
 last_name VARCHAR(255) NOT NULL,
 first_name VARCHAR(255) NOT NULL,
 phone_number VARCHAR(255) NOT NULL,
 PRIMARY KEY (id)
) ENGINE=INNODB;
*/

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
        qDebug() << db_conn.lastError();
        return 1;
    }
    else {
        qDebug() << "Database connection established !";
    }

    return 0;
}