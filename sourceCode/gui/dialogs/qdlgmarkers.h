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

  private:
    Ui::CQDlgMarkers* ui;
};

#endif // QDLGMARKERS_H
