
#ifndef QDLGMODELTHUMBNAILVISU_H
#define QDLGMODELTHUMBNAILVISU_H

#include <vDialog.h>
#include <thumbnail.h>

namespace Ui {
    class CQDlgModelThumbnailVisu;
}

class CQDlgModelThumbnailVisu : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgModelThumbnailVisu(QWidget *parent = 0);
    ~CQDlgModelThumbnailVisu();

    void cancelEvent();
    void okEvent();

    void applyThumbnail(const CThumbnail* thumbnail);

private slots:
    void on_qqYesNo_accepted();

    void on_qqYesNo_rejected();

private:
    Ui::CQDlgModelThumbnailVisu *ui;
};

#endif // QDLGMODELTHUMBNAILVISU_H
