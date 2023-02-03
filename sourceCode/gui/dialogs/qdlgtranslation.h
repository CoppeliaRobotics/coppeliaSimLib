
#ifndef QDLGTRANSLATION_H
#define QDLGTRANSLATION_H

#include <dlgEx.h>
#include <simMath/7Vector.h>

namespace Ui {
    class CQDlgTranslation;
}

class QComboBox;

class CQDlgTranslation : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgTranslation(QWidget *parent = 0);
    ~CQDlgTranslation();

    void refresh();

    void cancelEvent();

protected:
    void _enableCoordinatePart(bool enableState,bool enableButtons,bool alsoRadioButtons);
    void _enableTranslationPart(bool enableState,bool enableButtons,bool alsoRadioButtons);
    void _enableScalingPart(bool enableState,bool enableButtons,bool alsoRadioButtons);

    void _setDefaultValuesCoordinatePart(bool alsoRadioButtons);
    void _setDefaultValuesTranslationPart(bool alsoRadioButtons);
    void _setDefaultValuesScalingPart(bool alsoRadioButtons);
    void _setValuesTranslationPart(bool alsoRadioButtons);
    void _setValuesScalingPart(bool alsoRadioButtons);

    // Coord part
    bool _setCoord_userUnit(double newValueInUserUnit,int index);
    C7Vector _getNewTransf(const C7Vector& transf,double newValueInUserUnit,int index);
    bool _applyCoord(int mask);
    void _copyTransf(const C7Vector& tr,C7Vector& trIt,int mask);

    // Transf part
    bool _applyTranslation(int axis);
    bool _applyScaling(int axis);
    void _transformTranslation(C7Vector& tr,bool self,int axis);
    void _transformScaling(C7Vector& tr,int axis);

    static int coordMode; //0=abs,1=rel to parent
    static double translationValues[3];
    static double scalingValues[3];
    static int translateMode; //0=abs,1=rel to parent,2=rel to self
    static int scaleMode; //0=abs,1=rel to parent

    static int manipulationModePermission;
    static int manipulationTranslationRelativeTo;
    static double manipulationTranslationStepSize;
    static bool objectTranslationSettingsLocked;

    static int currentTab; //0=mouse transl., 1=pos, 2=transl., 3=scaling

    int lastLastSelectionID;

private slots:
    void on_qqCoordWorld_clicked();

    void on_qqCoordParent_clicked();

    void on_qqCoordX_editingFinished();

    void on_qqCoordY_editingFinished();

    void on_qqCoordZ_editingFinished();

    void on_qqCoordApplyPos_clicked();

    void on_qqCoordApplyX_clicked();

    void on_qqCoordApplyY_clicked();

    void on_qqCoordApplyZ_clicked();

    void on_qqTransfWorld_clicked();

    void on_qqTransfParent_clicked();

    void on_qqTransfOwn_clicked();

    void on_qqTransfX_editingFinished();

    void on_qqTransfY_editingFinished();

    void on_qqTransfZ_editingFinished();

    void on_qqTransfApplyPos_clicked();

    void on_qqTransfSX_editingFinished();

    void on_qqTransfSY_editingFinished();

    void on_qqTransfSZ_editingFinished();

    void on_qqTransfApplyScale_clicked();

    void on_qqScaleWorld_clicked();

    void on_qqScaleParent_clicked();

    void on_qqTransfApplyScaleX_clicked();

    void on_qqTransfApplyScaleY_clicked();

    void on_qqTransfApplyScaleZ_clicked();

    void on_qqTransfApplyPosX_clicked();

    void on_qqTransfApplyPosY_clicked();

    void on_qqTransfApplyPosZ_clicked();

    void on_tabWidget_currentChanged(int index);

    // Mouse manip
    void on_qqPosWorld_clicked();
    void on_qqPosParent_clicked();
    void on_qqPosOwn_clicked();
    void on_qqPosX_clicked();
    void on_qqPosY_clicked();
    void on_qqPosZ_clicked();
    void on_qqPosCombo_activated(int index);


private:
    Ui::CQDlgTranslation *ui;

    void _selectItemOfCombobox(QComboBox* theBox,int itemData);
    int _getIndexOfComboboxItemWithData(QComboBox* theBox,int itemData);
};

#endif // QDLGTRANSLATION_H
