
#ifndef QDLGPATHS_H
#define QDLGPATHS_H

#include <dlgEx.h>
#include <path_old.h>

namespace Ui {
    class CQDlgPaths;
}

class CQDlgPaths : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgPaths(QWidget *parent = 0);
    ~CQDlgPaths();

    void refresh();

    void cancelEvent();

    bool inMainRefreshRoutine;

    CPathCont_old* getPathCont();
    CPath_old* getPath();

private slots:
    void on_qqShowOrientation_clicked();

    void on_qqShowPathLine_clicked();

    void on_qqShowPosition_clicked();

    void on_qqAdjustColor_clicked();

    void on_qqLineSize_editingFinished();

    void on_qqControlPointSize_editingFinished();

    void on_qqDistanceCombo_currentIndexChanged(int index);

    void on_qqShowShapingDialog_clicked();

    void on_qqCopyToClipboard_clicked();

    void on_qqCreateEquivalentObject_clicked();

private:
    Ui::CQDlgPaths *ui;
};

#endif // QDLGPATHS_H
