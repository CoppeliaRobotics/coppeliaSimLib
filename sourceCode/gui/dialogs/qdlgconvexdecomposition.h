#ifndef QDLGCONVEXDECOMPOSITION_H
#define QDLGCONVEXDECOMPOSITION_H

#include "vDialog.h"
#include "simTypes.h"

namespace Ui {
    class CQDlgConvexDecomposition;
}

class CQDlgConvexDecomposition : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgConvexDecomposition(QWidget *parent = 0);
    ~CQDlgConvexDecomposition();

    void cancelEvent();
    void okEvent();

    void refresh();


    bool individuallyConsiderMultishapeComponents;
    int maxIterations;
    bool useHACD;

    // HACD params:
    bool addExtraDistPoints;
    bool addFacesPoints;
    int nClusters;
    int maxHullVertices;
    double maxConcavity;
    double smallClusterThreshold;
    int maxTrianglesInDecimatedMesh;
    double maxConnectDist;

    // VHACD params:
    int resolution;
    int depth;
    double concavity;
    int planeDownsampling;
    int convexHullDownsampling;
    double alpha;
    double beta;
    double gamma;
    bool pca;
    bool voxelBasedMode;
    int maxNumVerticesPerCH;
    double minVolumePerCH;


private slots:

    void on_qqOkCancel_accepted();
    void on_qqOkCancel_rejected();
    void on_qqExtraDistPoints_clicked();
    void on_qqExtraFacesPoints_clicked();
    void on_qqClusters_editingFinished();
    void on_qqConcavity_editingFinished();
    void on_qqConnectDist_editingFinished();
    void on_qqTargetBlabla_editingFinished();
    void on_qqMaxHullVertices_editingFinished();
    void on_qqSmallClusterThreshold_editingFinished();
    void on_qqIndividualComponents_clicked();
    void on_qqMaxIterations_editingFinished();
    void on_qqPcaEnabled_clicked();
    void on_qqVoxelBased_clicked();
    void on_qqResolution_editingFinished();
    void on_qqDepth_editingFinished();
    void on_qqConcavity_2_editingFinished();
    void on_qqPlaneDownsampling_editingFinished();
    void on_qqConvexHullDownsampling_editingFinished();
    void on_qqAlpha_editingFinished();
    void on_qqBeta_editingFinished();
    void on_qqGamma_editingFinished();
    void on_qqMaxVerticesPerCh_editingFinished();
    void on_qqMinVolumePerCh_editingFinished();
    void on_qqHACD_clicked();
    void on_qqVHACD_clicked();

private:
    Ui::CQDlgConvexDecomposition *ui;
};

#endif // QDLGCONVEXDECOMPOSITION_H
