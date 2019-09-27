#include "statusBar.h"

CStatusBar::CStatusBar() : QPlainTextEdit()
{
}

CStatusBar::~CStatusBar()
{
}

void CStatusBar::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    moveCursor(QTextCursor::End);
//    moveCursor(QTextCursor::PreviousBlock);
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    ensureCursorVisible();
}
