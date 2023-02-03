
#ifndef QDLGTRANSLATIONROTATIONCONT_H
#define QDLGTRANSLATIONROTATIONCONT_H

#include <dlgEx.h>
#include <QVBoxLayout>

namespace Ui {
    class CQDlgTranslationRotationCont;
}

class CQDlgTranslationRotationCont : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgTranslationRotationCont(QWidget *parent = 0);
    ~CQDlgTranslationRotationCont();

    void refresh();
    void cancelEvent();
    void okEvent();

    QVBoxLayout* bl;

    int currentPage;
    int desiredPage;

    CDlgEx* pageDlgs[2];
    int originalHeights[2];

private slots:

private:
    Ui::CQDlgTranslationRotationCont *ui;
};

#endif // QDLGTRANSLATIONROTATIONCONT_H
