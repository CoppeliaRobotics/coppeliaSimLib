
#ifndef QDLGSHAPES_H
#define QDLGSHAPES_H

#include <dlgEx.h>

namespace Ui {
    class CQDlgShapes;
}

class CQDlgShapes : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgShapes(QWidget *parent = 0);
    ~CQDlgShapes();

    void refresh();

    void cancelEvent();

private slots:
    void on_qqBackfaceCulling_clicked();

    void on_qqWireframe_clicked();

    void on_qqInvertFaces_clicked();

    void on_qqShowEdges_clicked();

    void on_qqShadingAngle_editingFinished();

    void on_qqApplyMain_clicked();

    void on_qqEditDynamics_clicked();

    void on_qqAdjustOutsideColor_clicked();

    void on_qqApplyColors_clicked();

    void on_qqTexture_clicked();

    void on_qqGeometry_clicked();

    void on_qqDirtTexture_clicked();

    void on_qqClearTextures_clicked();

    void on_qqEditMultishape_clicked();

    void on_qqHiddenBorder_clicked();

private:
    Ui::CQDlgShapes *ui;
};

#endif // QDLGSHAPES_H
