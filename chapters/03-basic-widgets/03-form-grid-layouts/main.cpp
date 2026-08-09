#include <QApplication>
#include <QPushButton>
#include <QGridLayout>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QStringList>
#include <QFormLayout>


//int main(int argc, char* argv[]) {
//	
//	QApplication app(argc, argv);
//	QWidget* window = new QWidget;
//	
//	QLabel* nameLabel = new QLabel("Open Happiness");
//	QLineEdit *firstNameLineEdit = new QLineEdit;
//	QLineEdit *lastNameLineEdit = new QLineEdit;
//	
//	QSpinBox *ageSpinBox = new QSpinBox;
//	ageSpinBox->setRange(1, 100);
//	
//	QComboBox* employmentStatusComboBox = new QComboBox;
//	QStringList employmentStatus = { "Unemployed", "Employed", "NA" };
//	employmentStatusComboBox->addItems(employmentStatus);
//	
//	QGridLayout* layout = new QGridLayout;
//	layout->addWidget(nameLabel, 0, 0);
//	layout->addWidget(firstNameLineEdit, 0, 1);
//	layout->addWidget(lastNameLineEdit, 0, 2);
//	layout->addWidget(ageSpinBox, 1, 0);
//	layout->addWidget(employmentStatusComboBox, 1, 1, 1, 2);
//	window->setLayout(layout);
//
//	window->show();
//	return app.exec();
//}

int main(int argc, char* argv[]) {
		
	QApplication app(argc, argv);
	QWidget* window = new QWidget;

	QLineEdit* firstNameLineEdit = new QLineEdit;
	QLineEdit *lastNameLineEdit = new QLineEdit;
	QSpinBox *ageSpinBox = new QSpinBox;
	ageSpinBox->setRange(1, 100);
	 
	QComboBox *employmentStatusComboBox = new QComboBox;
	QStringList employmentStatus = { "Unemployed", "Employed", "NA" };
	employmentStatusComboBox->addItems(employmentStatus);

	QFormLayout* personalInfoformLayout = new QFormLayout;
	personalInfoformLayout->addRow("First Name:", firstNameLineEdit);
	personalInfoformLayout->addRow("Last Name:", lastNameLineEdit);
	personalInfoformLayout->addRow("Age", ageSpinBox);
	personalInfoformLayout->addRow("Employment Status", employmentStatusComboBox);
	window->setLayout(personalInfoformLayout);
	window->show();

	return app.exec();
}