
#ifndef QDLGFOG_H
#define QDLGFOG_H

#include <dlgEx.h>

namespace Ui {
    class CQDlgFog;
}

class CQDlgFog : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgFog(QWidget *parent = 0);
    ~CQDlgFog();

    void refresh();

private slots:
    void on_qqFogEnabled_clicked();

    void on_qqLinear_clicked();

    void on_qqExp_clicked();

    void on_qqExp2_clicked();

    void on_qqStartDistance_editingFinished();

    void on_qqEndDistance_editingFinished();

    void on_qqDensity_editingFinished();

    void on_qqAdjustColor_clicked();

private:
    Ui::CQDlgFog *ui;
};

#endif // QDLGFOG_H
