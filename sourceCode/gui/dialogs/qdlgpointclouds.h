
#ifndef QDLGPOINTCLOUDS_H
#define QDLGPOINTCLOUDS_H

#include <dlgEx.h>

namespace Ui {
    class CQDlgPointclouds;
}

class CQDlgPointclouds : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgPointclouds(QWidget *parent = 0);
    ~CQDlgPointclouds();

    void refresh();

    void cancelEvent();

private slots:
    void on_qqMaxCellSize_editingFinished();

    void on_qqColor_clicked();

    void on_qqShowOctree_clicked();

    void on_qqRandomColors_clicked();

    void on_qqPointSize_editingFinished();

    void on_qqClear_clicked();

    void on_qqInsert_clicked();

    void on_qqMaxPointCount_editingFinished();

    void on_qqBuildResolution_editingFinished();

//  void on_qqReconstructCalcStructure_clicked();

    void on_qqNoOctreeStructure_clicked();

    void on_qqEmissiveColor_clicked();

    void on_qqDisplayRatio_editingFinished();

    void on_qqSubtract_clicked();

    void on_qqSubtractionTolerance_editingFinished();

    void on_qqInsertionTolerance_editingFinished();

private:
    Ui::CQDlgPointclouds *ui;
};

#endif // QDLGPOINTCLOUDS_H
