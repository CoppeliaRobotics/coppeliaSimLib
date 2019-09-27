
#ifndef QDLGSIMPLESEARCH_H
#define QDLGSIMPLESEARCH_H

#include "vDialog.h"

namespace Ui {
    class CQDlgSimpleSearch;
}

class CQDlgSimpleSearch : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgSimpleSearch(QWidget *parent = 0);
    ~CQDlgSimpleSearch();

    void init();

    void cancelEvent();
    void okEvent();

    static std::string textToSearch;
    static bool matchCase;

private slots:

    void on_qqMatchCase_clicked();

    void on_qqFind_clicked();

    void on_qqText_returnPressed();

private:
    Ui::CQDlgSimpleSearch *ui;
};

#endif // QDLGSIMPLESEARCH_H
