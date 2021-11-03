
#ifndef QDLGMULTISHAPEEDITION_H
#define QDLGMULTISHAPEEDITION_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgMultishapeEdition;
}

class CQDlgMultishapeEdition : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgMultishapeEdition(QWidget *parent = 0);
    ~CQDlgMultishapeEdition();

    void refresh();

    void cancelEvent();

private slots:
    void on_qqBackfaceCulling_clicked();

    void on_qqShowEdges_clicked();

    void on_qqShadingAngle_editingFinished();

    void on_qqAdjustColor_clicked();

    void on_qqTexture_clicked();

private:
    Ui::CQDlgMultishapeEdition *ui;
};

#endif // QDLGMULTISHAPEEDITION_H
