# Implementation Plan: Restructuring `Learning-Qt`

This plan outlines the actions required to turn the `Learning-Qt` repository into a premium, clean, and highly educational learning source.

## 1. Directory and File Reorganization
Currently, the files are structured inside "Getting Started with Qt 5" and "project" using generic names like `first`, `second`, `third`, `4th`, `1`, `2`, `3`, etc. We will migrate and map them to descriptive names.

### Reorganization Mapping:

| Current Path | Proposed Name / Path | Educational Description |
|---|---|---|
| `Getting Started with Qt 5` | `chapters/` | Grouping book chapters together |
| `Getting Started with Qt 5/first` | `chapters/01-introduction/` | Boilerplate wizard-generated window |
| `Getting Started with Qt 5/second/hello_world` | `chapters/02-hello-world/` | QLabel hover stylesheet experiment |
| `Getting Started with Qt 5/third/1` | `chapters/03-basic-widgets/01-button-tooltip/` | QPushButton & custom assets |
| `Getting Started with Qt 5/third/2` | `chapters/03-basic-widgets/02-label-alignment/` | Long formatted text rendering |
| `Getting Started with Qt 5/third/3` | `chapters/03-basic-widgets/03-form-grid-layouts/` | QFormLayout and QGridLayout comparison |
| `Getting Started with Qt 5/third/4` | `chapters/03-basic-widgets/04-login-vbox-layout/` | QVBoxLayout for structures |
| `Getting Started with Qt 5/third/5` | `chapters/03-basic-widgets/05-url-exporter-hbox/` | QHBoxLayout application |
| `Getting Started with Qt 5/third/6` | `chapters/03-basic-widgets/06-visual-studio-wizard/` | Standard MSVC Wizard structure |
| `Getting Started with Qt 5/4th/1` | `chapters/04-signals-slots/01-quit-connection/` | Connect clicked() to quit() |
| `Getting Started with Qt 5/4th/2` | `chapters/04-signals-slots/02-dial-label-link/` | Connect dial to QLabel |
| `Getting Started with Qt 5/4th/3` | `chapters/04-signals-slots/03-dial-lcd-label/` | Connect dial to LCD & Label |
| `Getting Started with Qt 5/4th/4` | `chapters/04-signals-slots/04-slider-dial-sync/` | Two-way slider-dial link |
| `Getting Started with Qt 5/5th/1` | `chapters/05-mainwindow-resources/01-menubar-actions/` | Advanced main window actions |
| `Getting Started with Qt 5/5th/2` | `chapters/05-mainwindow-resources/02-toolbar-status/` | Action icons & Status Bar |
| `Getting Started with Qt 5/5th/3` | `chapters/05-mainwindow-resources/03-open-file-slot/` | File Open and reading logic |
| `Getting Started with Qt 5/5th/4` | `chapters/05-mainwindow-resources/04-save-file-slot/` | File Save and writing logic |
| `Getting Started with Qt 5/5th/5` | `chapters/05-mainwindow-resources/05-text-edit-editor/` | Combined simple Rich Text Editor |
| `Getting Started with Qt 5/5th/6` | `chapters/05-mainwindow-resources/06-word-processor-full/`| Complete feature-packed Notepad |
| `Getting Started with Qt 5/6th/1` | `chapters/06-events-drag-drop/01-mouse-events/` | MousePress / Release intercepts |
| `Getting Started with Qt 5/6th/2` | `chapters/06-events-drag-drop/02-key-events/` | Keyboard Press intercepting |
| `Getting Started with Qt 5/6th/3` | `chapters/06-events-drag-drop/03-text-drag-drop/` | Custom DragTextEdit container |
| `Getting Started with Qt 5/6th/4` | `chapters/06-events-drag-drop/04-file-drag-drop/` | Drag and Drop external files |
| `Getting Started with Qt 5/7th/1` | `chapters/07-database-sql/01-connection-odbc/` | Connect to MySQL via QODBC |
| `Getting Started with Qt 5/7th/2` | `chapters/07-database-sql/02-select-query/` | Reading and outputting dataset |
| `Getting Started with Qt 5/7th/3` | `chapters/07-database-sql/03-bind-insert/` | Placeholder parameter insertions |
| `Getting Started with Qt 5/7th/4` | `chapters/07-database-sql/04-sql-crud/` | Full standard INSERT, UPDATE, DELETE |
| `Getting Started with Qt 5/7th/5` | `chapters/07-database-sql/05-sql-table-model/` | Programmatic QSqlTableModel |
| `Getting Started with Qt 5/7th/6` | `chapters/07-database-sql/06-sql-table-view/` | QTableView + Save Changes button |
| `project` | `showcase-projects/` | Advanced application directories |
| `project/1/1` | `showcase-projects/record-management-system/` | Record Management System (RMS) |

## 2. Compilation and Build Reference Safety
After renaming folders:
- Standard C++ `#include` directives must remain correct (they use local project headers).
- `.pro` project configuration files will have their internal file references updated to match new folder environments.
- `.sln` and `.vcxproj` files will have their UUIDs and file paths safely mapped.

## 3. Comprehensive Documentation & Roadmap
- Complete overhaul of `README.md` and `README_FA.md` in English/Persian, highlighting a structured curriculum.
- Build clean placeholder links for visual UI screenshots.
- Create `docs/development-report.md` detailing all renamed paths, fixed references, and screenshot instructions.
