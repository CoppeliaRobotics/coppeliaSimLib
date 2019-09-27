
#ifndef QDLGHEIGHTFIELDDIMENSION_H
#define QDLGHEIGHTFIELDDIMENSION_H

#include "vDialog.h"

namespace Ui {
    class CQDlgHeightfieldDimension;
}

class CQDlgHeightfieldDimension : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgHeightfieldDimension(QWidget *parent = 0);
    ~CQDlgHeightfieldDimension();

    void cancelEvent();
    void okEvent();

    void refresh();

    float xSize;
    float ySize;
    float zScaling;
    float xSizeTimesThisGivesYSize;

private slots:
    void on_qqSizeX_editingFinished();

    void on_qqSizeY_editingFinished();

    void on_qqScalingZ_editingFinished();

    void on_qqOk_accepted();

private:
    Ui::CQDlgHeightfieldDimension *ui;
};

#endif // QDLGHEIGHTFIELDDIMENSION_H
