
#ifndef QDLGMATERIAL_H
#define QDLGMATERIAL_H

#include <dlgEx.h>
#include <qdlgcolor.h>
#include <colorObject.h>

namespace Ui {
    class CQDlgMaterial;
}

class CQDlgMaterial : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgMaterial(QWidget *parent = 0);
    ~CQDlgMaterial();

    void refresh();

    void cancelEvent();

    bool needsDestruction();
    bool isLinkedDataValid();
    bool doesInstanceSwitchRequireDestruction();

    bool inMainRefreshRoutine;

    int _objType;
    int _objID1;
    int _objID2;
    int _lastSelectedObjectID;
    size_t _objectSelectionSize;

    static void displayMaterialDlg(int objType,int objID1,int objID2,QWidget* theParentWindow);
    static void displayMaterialDlgModal(int objType,int objID1,int objID2,QWidget* theParentWindow);
    void _initializeDlg(int objType,int objID1,int objID2);
    void _adjustCol(int colComponent);

private slots:
    void on_qqAmbientAdjust_clicked();

    void on_qqSpecularAdjust_clicked();

    void on_qqEmissiveAdjust_clicked();

    void on_qqPulsationAdjust_clicked();

    void on_qqShininess_editingFinished();

    void on_qqOpacityEnable_clicked();

    void on_qqOpacity_editingFinished();

    void on_qqColorName_editingFinished();

    void on_qqAuxiliaryAdjust_clicked();

    void on_qqExtensionString_editingFinished();

private:
    Ui::CQDlgMaterial *ui;
};

#endif // QDLGMATERIAL_H
