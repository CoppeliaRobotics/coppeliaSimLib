
#ifndef QDLGTEXTURELOADOPTIONS_H
#define QDLGTEXTURELOADOPTIONS_H

#include "vDialog.h"

namespace Ui {
    class CQDlgTextureLoadOptions;
}

class CQDlgTextureLoadOptions : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgTextureLoadOptions(QWidget *parent = 0);
    ~CQDlgTextureLoadOptions();

    void cancelEvent();
    void okEvent();

    void refresh();

    static int scaleTo;
    static bool scale;

private slots:

    void on_qqOk_accepted();

    void on_qqScale_clicked();

    void on_qq256_clicked();

    void on_qq512_clicked();

    void on_qq1024_clicked();

    void on_qq2048_clicked();

    void on_qq128_clicked();

private:
    Ui::CQDlgTextureLoadOptions *ui;
};

#endif // QDLGTEXTURELOADOPTIONS_H
