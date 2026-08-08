#include "dragTextEdit.h"


DragTextEdit::DragTextEdit(QWidget* parent) : QTextEdit(parent)
{
	setAcceptDrops(true);
}


void DragTextEdit::dragMoveEvent(QDragMoveEvent* event)
{
	event->acceptProposedAction();
}
void DragTextEdit::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
}
void DragTextEdit::dragEnterEvent(QDragEnterEvent* event)
{
	event->acceptProposedAction();
}

void DragTextEdit::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    if (mimeData->hasUrls())
    {
        const QUrl url = mimeData->urls().first();

        if (url.isLocalFile())
        {
            QFile file(url.toLocalFile());

            if (file.open(QFile::ReadOnly | QFile::Text))
            {
                QString contents = QString::fromUtf8(file.readAll());
                setText(contents);

                file.close();

                event->acceptProposedAction();
                return;
            }
        }
    }

    event->ignore();
}
