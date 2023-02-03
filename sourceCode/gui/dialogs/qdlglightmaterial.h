
#ifndef QDLGLIGHTMATERIAL_H
#define QDLGLIGHTMATERIAL_H

#include <dlgEx.h>
#include <qdlgcolor.h>
#include <colorObject.h>

namespace Ui {
    class CQDlgLightMaterial;
}

class CQDlgLightMaterial : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgLightMaterial(QWidget *parent = 0);
    ~CQDlgLightMaterial();

    void refresh();

    void cancelEvent();

    bool needsDestruction();
    bool isLinkedDataValid();
    bool doesInstanceSwitchRequireDestruction();

    int _objType;
    int _objID1;
    int _objID2;
    int _lastSelectedObjectID;
    size_t _objectSelectionSize;

    static void displayMaterialDlg(int objType,int objID1,int objID2,QWidget* theParentWindow);
    void _initializeDlg(int objType,int objID1,int objID2);
    void _adjustCol(int colComponent);

private slots:
    void on_qqDiffuseAdjust_clicked();

    void on_qqSpecularAdjust_clicked();

private:
    Ui::CQDlgLightMaterial *ui;
};

#endif // QDLGLIGHTMATERIAL_H
