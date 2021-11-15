
#ifndef QDLGCAMERAS_H
#define QDLGCAMERAS_H
#include "dlgEx.h"

namespace Ui {
    class CQDlgCameras;
}

class CQDlgCameras : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgCameras(QWidget *parent = 0);
    ~CQDlgCameras();

    void refresh();

    void cancelEvent();

    bool inMainRefreshRoutine;

private slots:
    void on_qqPerspectiveProjectionAngle_editingFinished();

    void on_qqOrthographicProjectionSize_editingFinished();

    void on_qqNearClipping_editingFinished();

    void on_qqFarClipping_editingFinished();

    void on_qqTrackedCombo_currentIndexChanged(int index);

    void on_qqShowFog_clicked();

    void on_qqPerspectiveMode_clicked();

    void on_qqManipProxy_clicked();

    void on_qqAlongX_clicked();

    void on_qqAlongY_clicked();

    void on_qqAlongZ_clicked();

    void on_qqFullRotation_clicked();

    void on_qqTilting_clicked();

    void on_qqColorA_clicked();

    void on_qqColorB_clicked();

    void on_qqSize_editingFinished();

    void on_qqLocalLights_clicked();

    void on_qqAllowPicking_clicked();

    void on_qqRenderModeCombo_currentIndexChanged(int index);

    void on_qqShowVolume_clicked();

private:
    Ui::CQDlgCameras *ui;
};

#endif // QDLGCAMERAS_H
