
#ifndef QDLGPATHPLANNINGPARAMS_H
#define QDLGPATHPLANNINGPARAMS_H

#include "vDialog.h"
#include "3Vector.h"
#include <QAbstractButton>

namespace Ui {
    class CQDlgPathPlanningParams;
}

class CQDlgPathPlanningParams : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgPathPlanningParams(QWidget *parent = 0);
    ~CQDlgPathPlanningParams();

    void refresh();

    void cancelEvent();
    void okEvent();

//  int taskID;
    int planningType;
    int holonomicType;
    bool visualizeArea;
    float searchRangeMin[4];
    float searchRangeMax[4];
    int searchDir[4];
    float stepSize;
    float angularStepSize;
    C3Vector gammaAxis;
    float minTurningCircleDiameter;
    bool inMainRefreshRoutine;

private slots:
    void on_qqClose_clicked(QAbstractButton *button);

    void on_qqHoloTypeCombo_currentIndexChanged(int index);

    void on_qqDeltaX_editingFinished();

    void on_qqDeltaY_editingFinished();

    void on_qqdeltaZ_editingFinished();

    void on_qqStepSizeL_editingFinished();

    void on_qqStepSizeA_editingFinished();

    void on_qqMinTurningCircle_editingFinished();

    void on_qqVisualizeArea_clicked();

    void on_qqMinX_editingFinished();

    void on_qqMinY_editingFinished();

    void on_qqMinZ_editingFinished();

    void on_qqMinD_editingFinished();

    void on_qqMinX_2_editingFinished();

    void on_qqMinY_2_editingFinished();

    void on_qqMinZ_2_editingFinished();

    void on_qqMinD_2_editingFinished();

    void on_qqMinusX_clicked();

    void on_qqPlusX_clicked();

    void on_qqMinusY_clicked();

    void on_qqPlusY_clicked();

    void on_qqMinusZ_clicked();

    void on_qqPlusZ_clicked();

    void on_qqMinusD_clicked();

    void on_qqPlusD_clicked();

private:
    void _setSearchRange(float searchMin[4],float searchR[4]);
    void _setSearchDirection(int dir[4]);

    Ui::CQDlgPathPlanningParams *ui;
};

#endif // QDLGPATHPLANNINGPARAMS_H
