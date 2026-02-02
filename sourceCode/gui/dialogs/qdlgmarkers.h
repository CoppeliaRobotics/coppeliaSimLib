#ifndef QDLGMARKERS_H
#define QDLGMARKERS_H

#include <dlgEx.h>

namespace Ui
{
class CQDlgMarkers;
}

class CQDlgMarkers : public CDlgEx
{
    Q_OBJECT

  public:
    explicit CQDlgMarkers(QWidget* parent = 0);
    ~CQDlgMarkers();

    void refresh() override;

    void cancelEvent() override;

    bool inMainRefreshRoutine;

  private slots:
    void on_qqSize_editingFinished();

    void on_qqColor_clicked();

    void on_qqApplyMainProperties_clicked();

  private:
    Ui::CQDlgMarkers* ui;
};

#endif // QDLGMARKERS_H
