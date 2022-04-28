
#ifndef QDLGPRIMITIVES_H
#define QDLGPRIMITIVES_H

#include "vDialog.h"
#include "3Vector.h"

namespace Ui {
    class CQDlgPrimitives;
}

class CQDlgPrimitives : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgPrimitives(QWidget *parent = 0);
    ~CQDlgPrimitives();

    void refresh();

    void cancelEvent();
    void okEvent();

    void initialize(int type,const C3Vector* sizes);
    void _correctDependentValues();

    int primitiveType;
    int subdivX;
    int subdivY;
    int subdivZ;
    int faceSubdiv;
    int sides;
    float xSize;
    float ySize;
    float zSize;
    int discSubdiv;
    bool smooth;
    bool openEnds;
    bool pure;
    bool dynamic;
    bool sizesAreLocked;
    float density;

private slots:
    void on_qqPure_clicked();
    void on_qqXSize_editingFinished();
    void on_qqYSize_editingFinished();
    void on_qqZSize_editingFinished();
    void on_qqXSubdiv_editingFinished();
    void on_qqYSubdiv_editingFinished();
    void on_qqZSubdiv_editingFinished();
    void on_qqSides_editingFinished();
    void on_qqFaceSubdiv_editingFinished();
    void on_qqDiscSubdiv_editingFinished();
    void on_qqSmooth_clicked();
    void on_qqOpen_clicked();
    void on_qqOkCancel_accepted();
    void on_qqOkCancel_rejected();
    void on_qqDensity_editingFinished();

    void on_qqDynamic_clicked();

private:
    Ui::CQDlgPrimitives *ui;
};

#endif // QDLGPRIMITIVES_H
