#ifndef QDLGSCRIPTS_H
#define QDLGSCRIPTS_H

#include <dlgEx.h>

namespace Ui
{
class CQDlgScripts;
}

class CQDlgScripts : public CDlgEx
{
    Q_OBJECT

  public:
    explicit CQDlgScripts(QWidget *parent = 0);
    ~CQDlgScripts();

    void refresh();

    void cancelEvent();

    bool inMainRefreshRoutine;

  private slots:
    void on_qqSize_editingFinished();

    void on_qqColor_clicked();

    void on_qqApplyMainProperties_clicked();

    void on_qqEnabled_clicked();

    void on_qqParentProxy_clicked();

    void on_qqExecutionOrder_currentIndexChanged(int index);

private:
    Ui::CQDlgScripts *ui;
};

#endif // QDLGSCRIPTS_H
