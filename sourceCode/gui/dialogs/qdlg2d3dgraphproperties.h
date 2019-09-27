
#ifndef QDLG2D3DGRAPHPROPERTIES_H
#define QDLG2D3DGRAPHPROPERTIES_H

#include "dlgEx.h"
#include <QListWidgetItem>
#include <QAbstractButton>

namespace Ui {
    class CQDlg2D3DGraphProperties;
}

class CQDlg2D3DGraphProperties : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlg2D3DGraphProperties(QWidget *parent = 0);
    ~CQDlg2D3DGraphProperties();

    void cancelEvent();

    void refresh();
    void dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);

    void updateObjectsInList();
    int getSelectedObjectID();
    void selectObjectInList(int objectID);

    bool needsDestruction();
    bool isLinkedDataValid();
    bool doesInstanceSwitchRequireDestruction();

    void _initialize(int graphHandle,bool xyGraph);
    static void display(int graphHandle,bool xyGraph,QWidget* theParentWindow);


private slots:
    void onDeletePressed();

    void on_qqAddNewCurve_clicked();

    void on_qqList_itemSelectionChanged();

    void on_qqList_itemChanged(QListWidgetItem *item);

    void on_qqVisible_clicked();

    void on_qqDisplayLabel_clicked();

    void on_qqLinkPoints_clicked();

    void on_qqAdjustColor_clicked();

    void on_qqDuplicate_clicked();

    void on_qqWidth_editingFinished();

    void on_qqRelativeToGraph_clicked();

    void on_qqRelativeToWorld_clicked();

    void on_qqAlwaysOnTop_clicked();

private:
    int _graphHandle;
    bool _inListSelectionRoutine;
    bool _xyGraph;

    Ui::CQDlg2D3DGraphProperties *ui;
};

#endif // QDLG2D3DGRAPHPROPERTIES_H
