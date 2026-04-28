#ifndef QDLGCUSTOMSCENEOBJECTS_H
#define QDLGCUSTOMSCENEOBJECTS_H

#include <dlgEx.h>

namespace Ui
{
class CQDlgCustomSceneObjects;
}

class CQDlgCustomSceneObjects : public CDlgEx
{
    Q_OBJECT

  public:
    explicit CQDlgCustomSceneObjects(QWidget* parent = 0);
    ~CQDlgCustomSceneObjects();

    void refresh() override;

    void cancelEvent() override;

    bool inMainRefreshRoutine;

  private slots:
    void on_qqSize_editingFinished();

    void on_qqColor_clicked();

    void on_qqApplyMainProperties_clicked();

  private:
    Ui::CQDlgCustomSceneObjects* ui;
};

#endif // QDLGCUSTOMSCENEOBJECTS_H
