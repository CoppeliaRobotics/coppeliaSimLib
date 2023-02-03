
#ifndef QDLGLIGHTS_H
#define QDLGLIGHTS_H

#include <dlgEx.h>

namespace Ui {
    class CQDlgLights;
}

class CQDlgLights : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgLights(QWidget *parent = 0);
    ~CQDlgLights();

    void refresh();

    void cancelEvent();

private slots:
    void on_qqEnabled_clicked();

    void on_qqLocal_clicked();

    void on_qqSize_editingFinished();

    void on_qqSpotExponent_editingFinished();

    void on_qqConstantFactor_editingFinished();

    void on_qqLinearFactor_editingFinished();

    void on_qqQuadraticFactor_editingFinished();

    void on_qqLightColor_clicked();

    void on_qqCasingColor_clicked();

    void on_qqSpotCutoff_editingFinished();

private:
    Ui::CQDlgLights *ui;
};

#endif // QDLGLIGHTS_H
