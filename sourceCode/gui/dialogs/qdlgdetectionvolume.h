#ifndef QDLGDETECTIONVOLUME_H
#define QDLGDETECTIONVOLUME_H

#include "dlgEx.h"
#include "convexVolume.h"
#include "proximitySensor.h"
#include "mill.h"

namespace Ui {
    class CQDlgDetectionVolume;
}

class CQDlgDetectionVolume : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgDetectionVolume(QWidget *parent = 0);
    ~CQDlgDetectionVolume();

    void refresh();
    void cancelEvent();

    CConvexVolume* getCurrentConvexVolume();

    bool inMainRefreshRoutine;

    static bool showVolumeWindow;

private slots:
    void on_qqOffset_editingFinished();

    void on_qqRadius_editingFinished();

    void on_qqRange_editingFinished();

    void on_qqRadiusFar_editingFinished();

    void on_qqSizeX_editingFinished();

    void on_qqAngle_editingFinished();

    void on_qqSizeY_editingFinished();

    void on_qqFaceCount_editingFinished();

    void on_qqSizeFarX_editingFinished();

    void on_qqFaceCountFar_editingFinished();

    void on_qqSizeFarY_editingFinished();

    void on_qqSubdivisions_editingFinished();

    void on_qqInsideGap_editingFinished();

    void on_qqSubdivisionsFar_editingFinished();

    void on_qqApplyAll_clicked();

private:
    Ui::CQDlgDetectionVolume *ui;
};

#endif // QDLGDETECTIONVOLUME_H
