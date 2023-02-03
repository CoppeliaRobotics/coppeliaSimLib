
#ifndef QDLGLAYERS_H
#define QDLGLAYERS_H

#include <dlgEx.h>

namespace Ui {
    class CQDlgLayers;
}

class CQDlgLayers : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgLayers(QWidget *parent = 0);
    ~CQDlgLayers();

    void refresh();
    void clickedLayer(int bit);

private slots:
    void on_a_1_clicked();
    void on_a_2_clicked();
    void on_a_3_clicked();
    void on_a_4_clicked();
    void on_a_5_clicked();
    void on_a_6_clicked();
    void on_a_7_clicked();
    void on_a_8_clicked();
    void on_a_9_clicked();
    void on_a_10_clicked();
    void on_a_11_clicked();
    void on_a_12_clicked();
    void on_a_13_clicked();
    void on_a_14_clicked();
    void on_a_15_clicked();
    void on_a_16_clicked();

    void on_invert_clicked();

    void on_qqDynamicContentOnly_clicked();

private:
    Ui::CQDlgLayers *ui;
};

#endif // QDLGLAYERS_H
