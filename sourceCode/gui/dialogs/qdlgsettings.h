
#ifndef QDLGSETTINGS_H
#define QDLGSETTINGS_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgSettings;
}


class QComboBox;

class CQDlgSettings : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgSettings(QWidget *parent = 0);
    ~CQDlgSettings();

    void refresh();

private slots:
    void on_translationStepSize_activated(int index);

    void on_rotationStepSize_activated(int index);

    void on_removeIdenticalVerticesCheckbox_clicked();

    void on_removeIdenticalVerticesTolerance_editingFinished();

    void on_removeIdenticalTriangles_clicked();

    void on_ignoreTriangleWinding_clicked();

    void on_worldReference_clicked();

    void on_boundingBoxDisplay_clicked();

    void on_undoRedo_clicked();

    void on_hideConsole_clicked();

    void on_qqAutoSave_clicked();

    void on_qqAdjustOpenGl_clicked();

    void on_qqHideHierarchy_clicked();

private:
    Ui::CQDlgSettings *ui;

    void _selectItemOfCombobox(QComboBox* theBox,int itemData);
    int _getIndexOfComboboxItemWithData(QComboBox* theBox,int itemData);
};

#endif // QDLGSETTINGS_H
