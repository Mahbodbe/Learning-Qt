#include <QApplication>
#include <QtSql>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <Qt>
#include <QTableView>
#include <QHeaderView>

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
    enum{
        ID = 0,
        LastName = 1,
        FirstName = 2,
        PhoneNumber = 3,
    };

    QSqlTableModel* contactsTableModel = new QSqlTableModel(0, db_conn);
    contactsTableModel->setTable("contacts");
    contactsTableModel->select();
    contactsTableModel->setHeaderData(ID, Qt::Horizontal, QObject::tr("ID"));
    contactsTableModel->setHeaderData(LastName, Qt::Horizontal, QObject::tr("Last Name"));
    contactsTableModel->setHeaderData(FirstName, Qt::Horizontal, QObject::tr("First Name"));
    contactsTableModel->setHeaderData(PhoneNumber, Qt::Horizontal, QObject::tr("Phone Number"));
    //contactsTableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    QTableView* contactsTableView = new QTableView();
    contactsTableView->setModel(contactsTableModel);
    contactsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    contactsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    QHeaderView* header = contactsTableView->horizontalHeader();
    header->setStretchLastSection(true);
    QWidget window;
    QVBoxLayout* layout = new QVBoxLayout();
    QPushButton* saveToDbPushButton = new QPushButton("Save Changes");
    layout->addWidget(contactsTableView);
    layout->addWidget(saveToDbPushButton);

    QObject::connect(saveToDbPushButton, SIGNAL(clicked()), contactsTableModel, SLOT(submitAll()));
    window.setLayout(layout);
    window.show();
    return app.exec();

}