
#ifndef QDLGROTATION_H
#define QDLGROTATION_H

#include "dlgEx.h"
#include "7Vector.h"

namespace Ui {
    class CQDlgRotation;
}

class QComboBox;

class CQDlgRotation : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgRotation(QWidget *parent = 0);
    ~CQDlgRotation();

    void refresh();

    void cancelEvent();


protected:
    void _enableCoordinatePart(bool enableState,bool enableButtons,bool alsoRadioButtons);
    void _enableTransformationPart(bool enableState,bool enableButtons,bool alsoRadioButtons);
    void _setDefaultValuesCoordinatePart(bool alsoRadioButtons);
    void _setDefaultValuesTransformationPart(bool alsoRadioButtons);
    void _setValuesTransformationPart(bool alsoRadioButtons);

    // Coord part
    bool _setCoord_userUnit(double newValueInUserUnit,int index);
    C7Vector _getNewTransf(const C7Vector& transf,double newValueInUserUnit,int index);
    bool _applyCoord();

    // Transf part
    bool _applyTransformation(int axis);
    void _transform(C7Vector& tr,bool self,int axis);

    static int coordMode; //0=abs,1=rel to parent
    static double rotAngles[3];
    static int transfMode; //0=abs,1=rel to parent,2=rel to self
    static int currentTab; //0=mouse transl., 1=pos, 2=transl., 3=scaling

    static int manipulationModePermission;
    static int manipulationRotationRelativeTo;
    static double manipulationRotationStepSize;
    static bool objectRotationSettingsLocked;

    int lastLastSelectionID;

private slots:
    void on_qqCoordWorld_clicked();

    void on_qqCoordParent_clicked();

    void on_qqCoordAlpha_editingFinished();

    void on_qqCoordBeta_editingFinished();

    void on_qqCoordGamma_editingFinished();

    void on_qqCoordApplyOr_clicked();

    void on_qqTransfWorld_clicked();

    void on_qqTransfParent_clicked();

    void on_qqTransfOwn_clicked();

    void on_qqTransfAlpha_editingFinished();

    void on_qqTransfBeta_editingFinished();

    void on_qqTransfGamma_editingFinished();

    void on_qqTransfApplyOr_clicked();

    void on_qqTransfApplyOrX_clicked();

    void on_qqTransfApplyOrY_clicked();

    void on_qqTransfApplyOrZ_clicked();

    void on_tabWidget_currentChanged(int index);

    // Mouse manip
    void on_qqOrWorld_clicked();
    void on_qqOrParent_clicked();
    void on_qqOrOwn_clicked();
    void on_qqOrA_clicked();
    void on_qqOrB_clicked();
    void on_qqOrG_clicked();
    void on_qqOrCombo_activated(int index);


private:
    void _selectItemOfCombobox(QComboBox* theBox,int itemData);
    int _getIndexOfComboboxItemWithData(QComboBox* theBox,int itemData);

    Ui::CQDlgRotation *ui;
};

#endif // QDLGROTATION_H
