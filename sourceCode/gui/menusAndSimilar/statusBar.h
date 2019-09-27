#pragma once

#include <QPlainTextEdit>
#include <QScrollBar>

class CStatusBar : public QPlainTextEdit
{
    Q_OBJECT
public:
    CStatusBar();
    virtual ~CStatusBar();

    void resizeEvent(QResizeEvent *e);
};
