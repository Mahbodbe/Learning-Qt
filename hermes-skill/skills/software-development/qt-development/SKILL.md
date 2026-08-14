---
name: qt-development
description: Qt Creator / Qt C++ Development reference guide - project setup, key concepts, signals/slots, network, SQL, deployment, and common pitfalls.
category: software-development
tags:
  - qt
  - qt-creator
  - cpp
  - gui
  - cmake
  - qmake
---
# Qt Creator / Qt C++ Development - Reference Guide

## Project Setup with Qt Creator

### Prerequisites
- Qt Creator (4.x / 10.x+)
- Qt Framework 5.x or 6.x
- C++ Compiler (GCC/Clang/MSVC)
- QMake or CMake

### Project Structure
```
project/
├── src/
│   ├── main.cpp
│   ├── mainwindow.cpp
│   └── mainwindow.h
├── resources.qrc
├── CMakeLists.txt (or project.pro)
└── README.md
```

### CMakeLists.txt Template (Qt 6)
```cmake
cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Network)

add_executable(MyApp
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

target_link_libraries(MyApp PRIVATE Qt6::Widgets Qt6::Network)
```

### QMake (.pro) Template (Qt 5/6)
```pro
QT += widgets network
CONFIG += c++17

SOURCES += src/main.cpp \
           src/mainwindow.cpp

HEADERS += src/mainwindow.h

RESOURCES += resources.qrc
```

## Qt Creator Wizard Usage

### First Project with Wizard
1. File → New Project → Application (Qt) → Qt Widgets Application
2. Choose build system: CMake (recommended) or QMake
3. Select Qt version (Qt 6.x preferred)
4. Add necessary modules (widgets, network, etc.)
4. Build system generates `.pro` or `CMakeLists.txt`

### Project Build & Run
1. Configure Project (Select Kit - Desktop Qt 6.x GCC 64bit)
2. Build: `Ctrl+B` or Build → Build Project
3. Run: `Ctrl+R` or click green Run button

## Key Qt Concepts

### Signal & Slot Mechanism
```cpp
// Modern Qt 5/6 connect syntax
connect(sender, &SenderClass::signalName, receiver, &ReceiverClass::slotName);

// Lambda slot (C++11)
connect(button, &QPushButton::clicked, [=]() {
    qDebug() << "Button clicked!";
});
```

### Layouts
```cpp
// Vertical layout
QVBoxLayout *layout = new QVBoxLayout(this);
layout->addWidget(widget1);
layout->addWidget(widget2);
setLayout(layout);

// Horizontal layout
QHBoxLayout *hlayout = new QHBoxLayout();
hlayout->addWidget(btn1);
hlayout->addWidget(btn2);
layout->addLayout(hlayout);
```

### Qt Network (HTTP Requests)
```cpp
#include <QNetworkAccessManager>
#include <QNetworkReply>

QNetworkAccessManager *manager = new QNetworkAccessManager(this);
connect(manager, &QNetworkAccessManager::finished, this, &MyClass::onReply);

QNetworkRequest request(QUrl("https://api.example.com/data"));
manager->get(request);

void MyClass::onReply(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        // process JSON
    }
    reply->deleteLater();
}
```

### Qt SQL / SQLite
```cpp
#include <QtSql>

QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
db.setDatabaseName("parking.db");
if (!db.open()) qDebug() << "DB Error:" << db.lastError().text();

QSqlQuery query;
query.prepare("SELECT floor, slot_number FROM parking_slots WHERE is_occupied = 0 ORDER BY floor ASC, slot_number ASC LIMIT 1");
if (query.exec() && query.next()) {
    int floor = query.value(0).toInt();
    int slot = query.value(1).toInt();
}
```

### Q_PROPERTY for GUI Binding
```cpp
class ParkingModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(int occupiedSlots READ occupiedSlots NOTIFY occupiedSlotsChanged)
    
public:
    int occupiedSlots() const { return m_occupied; }
    
signals:
    void occupiedSlotsChanged();
};
```

## Essential Qt Modules

| Module | Include | Use Case |
|--------|---------|----------|
| `QtCore` | `<QtCore>` | Core non-GUI functionality |
| `QtWidgets` | `<QtWidgets>` | GUI widgets (buttons, dialogs, layouts) |
| `QtNetwork` | `<QtNetwork>` | HTTP, TCP, UDP, WebSocket |
| `QtSql` | `<QtSql>` | Database (SQLite, PostgreSQL, etc.) |
| `QtCharts` | `<QtCharts>` | Charts and graphs |
| `QtWebEngine` | `<QtWebEngine>` | Web browser component |
| `QtQuick` | `<QtQuick>` | QML/Qt Quick (declarative UI) |

## Build Systems

### CMake (Modern, Recommended)
```bash
cmake -B build -S .
cmake --build build
```

### QMake (Legacy)
```bash
qmake project.pro
make
```

## Common Pitfalls

1. **Missing `Q_OBJECT` macro** - Required for signals/slots to work
2. **Forgetting `deleteLater()`** - Memory leaks in QNetworkReply
3. **UI blocking** - Long operations in main thread freeze UI (use QThread/QtConcurrent)
4. **Missing `Q_OBJECT` in moc** - Run qmake/CMake after adding Q_OBJECT
5. **Missing `Q_DECLARE_METATYPE`** - For custom types in signals/slots
6. **Image deployment** - Use `qrc` resource system for images, not absolute paths

## Deployment

### Linux
```bash
# Use linuxdeployqt or linuxdeploy
linuxdeployqt AppDir/your_app -appimage
```

### Windows
```bash
windeployqt.exe your_app.exe
```

### macOS
```bash
macdeployqt YourApp.app
```

## Useful Resources
- Qt Documentation: https://doc.qt.io/
- Qt Examples: Built into Qt Creator (Welcome → Examples)
- Qt Forum: https://forum.qt.io/
- Benjamin Baka's "Getting Started with Qt 5" - Primary learning reference
- Detailed Repository Refactoring Report: [references/development-report.md](references/development-report.md)

## Session Notes & Reorganization Lessons (2026-08-09)
- Refactored `Learning-Qt` repository completely into a professional educational reference.
- Restructured all chapter and exercise directories from obscure single digits/numbers to highly descriptive chronological chapters (from `01-introduction` to `07-database-sql`).
- Updated all QMake (`.pro`) project files and MSVC Visual Studio solutions (`.sln` / `.vcxproj`) to reflect renamed source files and target executable configurations.
- Overhauled `README.md` and `README_FA.md` to establish a complete English-first curriculum map with visual mockup placeholders for student-facing screenshots.
- Provided a full Change Log and structured performance report inside `docs/development-report.md`.
- Documented full SQL connectivity techniques via QODBC drivers, parameter insertions (named and positional placeholders), custom message handlers (`qInstallMessageHandler`), transactions, and mapping backend tables to interactive frontend tables using `QSqlTableModel` and `QTableView`.
- Retained the standalone showcase project (`record-management-system`) with full CSV file reading, writing, transactions, and user validation dialog triggers.
- Git Write Pitfall Note: When using multiline heredoc (`cat << 'EOF' > ...`) inside `terminal` tool calls, unescaped Bash variables, backticks, or quotes inside Markdown codeblocks can truncate the file or corrupt formatting. Always prefer `write_file` or explicit Python file writes for complex multi-line text files like `README.md` to guarantee exact content integrity.
- Visual Studio / VS Code auto-recovery / temp files: Always ensure `.gitignore` explicitly blocks `enc_temp_folder/`, `.vs/`, `*.VC.db`, and `*.ipch` to avoid cluttering commits with auto-saved Visual Studio C++ temp source duplicates.