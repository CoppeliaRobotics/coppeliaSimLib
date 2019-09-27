
#ifndef QDLGENVIRONMENT_H
#define QDLGENVIRONMENT_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgEnvironment;
}

class CQDlgEnvironment : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgEnvironment(QWidget *parent = 0);
    ~CQDlgEnvironment();

    void refresh();

private slots:
    void on_qqBackgroundColorUp_clicked();

    void on_qqBackgroundColorDown_clicked();

    void on_qqAmbientLightColor_clicked();

    void on_qqFogAdjust_clicked();

    void on_qqMaxTriangleSize_editingFinished();

    void on_qqSaveCalcStruct_clicked();

    void on_qqShapeTexturesDisabled_clicked();

    void on_qqNextSaveIsDefinitive_clicked();

    void on_qqAcknowledgments_textChanged();

    void on_qqCleanUpHashNames_clicked();

    void on_qqCleanUpGhosts_clicked();

    void on_qqMinRelTriangleSize_editingFinished();

    void on_qqExtensionString_editingFinished();

private:
    Ui::CQDlgEnvironment *ui;
};

#endif // QDLGENVIRONMENT_H
