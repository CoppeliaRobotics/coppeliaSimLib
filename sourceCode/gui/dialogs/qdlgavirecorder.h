
#ifndef CQDLGAVIRECORDER_H
#define CQDLGAVIRECORDER_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgAviRecorder;
}

class CQDlgAviRecorder : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgAviRecorder(QWidget *parent = 0);
    ~CQDlgAviRecorder();

    void refresh();
    bool inRefreshRoutine;
private slots:
    void on_launchAtSimulationStart_clicked();

    void on_recordNow_clicked();

    void on_showCursor_clicked();

    void on_showButtons_clicked();

    void on_hideInfos_clicked();

    void on_selectLocation_clicked();

    void on_displayedFramesVsRecordedFrame_editingFinished();

    void on_frameRate_editingFinished();

    void on_qqOutputTypeCombo_currentIndexChanged(int index);

    void on_autoFrameRate_clicked();

    void on_recordDesktopInstead_clicked();

    void on_recordWindowInstead_clicked();

private:
    Ui::CQDlgAviRecorder *ui;
};

#endif // CQDLGAVIRECORDER_H
