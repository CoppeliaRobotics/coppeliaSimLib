
#ifndef QDLGRENDERINGSENSORS_H
#define QDLGRENDERINGSENSORS_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgVisionSensors;
}

class CQDlgVisionSensors : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgVisionSensors(QWidget *parent = 0);
    ~CQDlgVisionSensors();

    void refresh();

    void cancelEvent();

    bool inMainRefreshRoutine;

private slots:
    void on_qqEnabled_clicked();

    void on_qqExplicitHandling_clicked();

    void on_qqExternalInput_clicked();

    void on_qqLocalLights_clicked();

    void on_qqShowVolume_clicked();

    void on_qqShowFog_clicked();

    void on_qqNearPlane_editingFinished();

    void on_qqFarPlane_editingFinished();

    void on_qqPerspectiveAngleOrOrthographicSize_editingFinished();

    void on_qqResX_editingFinished();

    void on_qqResY_editingFinished();

    void on_qqSizeX_editingFinished();

    void on_qqSizeY_editingFinished();

    void on_qqSizeZ_editingFinished();

    void on_qqAdjustImageColor_clicked();

    void on_qqApplyMainProperties_clicked();

    void on_qqCasingColor_clicked();

    void on_qqApplyColors_clicked();

    void on_qqIgnoreRGB_clicked();

    void on_qqIgnoreDepth_clicked();

    void on_qqIgnorePacket1_clicked();

    void on_qqRenderModeCombo_currentIndexChanged(int index);

private:
    Ui::CQDlgVisionSensors *ui;
};

#endif // QDLGRENDERINGSENSORS_H
