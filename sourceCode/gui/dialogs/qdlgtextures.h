
#ifndef QDLGTEXTURES_H
#define QDLGTEXTURES_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgTextures;
}

class CQDlgTextures : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgTextures(QWidget *parent = 0);
    ~CQDlgTextures();

    void refresh();

    void cancelEvent();
    bool needsDestruction();
    bool isLinkedDataValid();
    bool doesInstanceSwitchRequireDestruction();

    int _objType;
    int _objID1;
    int _objID2;

    void _setTextureConfig(int index);
    void _setTextureScaling(int index);
    void _setTextureBooleanProperty(int index);

    static void displayDlg(int objType,int objID1,int objID2,QWidget* theParentWindow);
    void _initializeDlg(int objType,int objID1,int objID2);

private slots:
    void on_qqAlpha_editingFinished();

    void on_qqBeta_editingFinished();

    void on_qqGamma_editingFinished();

    void on_qqX_editingFinished();

    void on_qqY_editingFinished();

    void on_qqZ_editingFinished();

    void on_qqU_editingFinished();

    void on_qqV_editingFinished();

    void on_qqInterpolate_clicked();

    void on_qqRepeatU_clicked();

    void on_qqRepeatV_clicked();

    void on_qqRemoveSelect_clicked();

    void on_qqLoad_clicked();

    void on_qqMapMode_currentIndexChanged(int index);

    void on_qqApplyMode_currentIndexChanged(int index);

private:
    Ui::CQDlgTextures *ui;

    bool inMainRefreshRoutine;
};

#endif // QDLGTEXTURES_H
