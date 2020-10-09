
#ifndef QDLGSELECTION_H
#define QDLGSELECTION_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgSelection;
}

class CQDlgSelection : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgSelection(QWidget *parent = 0);
    ~CQDlgSelection();
    void refresh();

protected:
    std::vector<int> list;
    void processIt();

private slots:
    void on_simpleShapesButton_clicked();

    void on_groupedShapesButton_clicked();

    void on_dummiesButton_clicked();

    void on_camerasButton_clicked();

    void on_lightsButton_clicked();

    void on_proximitySensorsButton_clicked();

    void on_renderingSensorsButton_clicked();

    void on_forceSensorsButton_clicked();

    void on_jointsButton_clicked();

    void on_graphsButton_clicked();

    void on_pathsButton_clicked();

    void on_clearSelectionButton_clicked();

    void on_invertSelectionButton_clicked();

    void on_octreesButton_clicked();

    void on_pointCloudsButton_clicked();

private:
    Ui::CQDlgSelection *ui;
};

#endif // QDLGSELECTION_H
