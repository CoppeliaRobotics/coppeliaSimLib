
#ifndef QDLGPATHSHAPING_H
#define QDLGPATHSHAPING_H

#include <dlgEx.h>
#include <path_old.h>

namespace Ui {
    class CQDlgPathShaping;
}

class CQDlgPathShaping : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgPathShaping(QWidget *parent = 0);
    ~CQDlgPathShaping();

    void refresh();
    void cancelEvent();

    bool inMainRefreshRoutine;

    bool _extractOneFloat(std::string& txt,double& val);

    static bool showWindow;

private slots:
    void on_qqEnabled_clicked();

    void on_qqFollowOrientation_clicked();

//  void on_qqConvexHull_clicked();

    void on_qqAdjustColor_clicked();

    void on_qqGenerateShape_clicked();

    void on_qqMaxLength_editingFinished();

    void on_qqTypeCombo_currentIndexChanged(int index);

    void on_qqCyclic_clicked();

    void on_qqScalingFactor_editingFinished();

    void on_qqCoordinates_textChanged();

private:
    Ui::CQDlgPathShaping *ui;
};

#endif // QDLGPATHSHAPING_H
