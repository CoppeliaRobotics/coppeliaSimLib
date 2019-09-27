
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

    void initialize(int type,const C3Vector* sizes); //0=plane, 1=box, 2=sphere, 3=cylinder, 4=disc
    void _adjustValuesForPurePrimitive();

    int primitiveType;
    int subdivX;
    int subdivY;
    int subdivZ;
    int faces;
    int sides;
    float xSize;
    float ySize;
    float zSize;
    int discSubdiv;
    bool smooth;
    int openEnds;
    bool pure;
    bool dynamic;
    bool cone;
    bool sizesAreLocked;
    float density;
    bool negativeVolume;
    float negativeVolumeScaling;

private slots:
    void on_qqPure_clicked();
    void on_qqXSize_editingFinished();
    void on_qqYSize_editingFinished();
    void on_qqZSize_editingFinished();
    void on_qqXSubdiv_editingFinished();
    void on_qqYSubdiv_editingFinished();
    void on_qqZSubdiv_editingFinished();
    void on_qqSides_editingFinished();
    void on_qqFaces_editingFinished();
    void on_qqDiscSubdiv_editingFinished();
    void on_qqSmooth_clicked();
    void on_qqOpen_clicked();
    void on_qqCone_clicked();
    void on_qqOkCancel_accepted();
    void on_qqOkCancel_rejected();
    void on_qqDensity_editingFinished();
    void on_qqNegativeVolume_clicked();
    void on_qqNegativeVolumeScaling_editingFinished();

    void on_qqDynamic_clicked();

private:
    Ui::CQDlgPrimitives *ui;
};

#endif // QDLGPRIMITIVES_H
