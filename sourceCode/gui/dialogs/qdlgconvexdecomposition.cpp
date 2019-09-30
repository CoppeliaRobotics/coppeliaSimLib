#include "qdlgconvexdecomposition.h"
#include "ui_qdlgconvexdecomposition.h"
#include "tt.h"
#include "gV.h"
#include "app.h"

CQDlgConvexDecomposition::CQDlgConvexDecomposition(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgConvexDecomposition)
{
    // First set-up default values:
    individuallyConsiderMultishapeComponents=false;
    maxIterations=4;
    useHACD=true;;

    // HACD params:
    addExtraDistPoints=true;
    addFacesPoints=true;
    nClusters=1;
    maxHullVertices=200; // from 100 to 200 on 5/2/2014
    maxConcavity=100.0f;
    smallClusterThreshold=0.25f;
    maxTrianglesInDecimatedMesh=500;
    maxConnectDist=30.0f;

    // VHACD params:
    resolution=100000;
    depth=20;
    concavity=0.0025f;
    planeDownsampling=4;
    convexHullDownsampling=4;
    alpha=0.05f;
    beta=0.05f;
    gamma=0.00125f;
    pca=false;
    voxelBasedMode=true;
    maxNumVerticesPerCH=64;
    minVolumePerCH=0.0001f;

    ui->setupUi(this);
}

CQDlgConvexDecomposition::~CQDlgConvexDecomposition()
{
    delete ui;
}

void CQDlgConvexDecomposition::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgConvexDecomposition::okEvent()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgConvexDecomposition::refresh()
{
    ui->qqIndividualComponents->setChecked(individuallyConsiderMultishapeComponents);
    ui->qqMaxIterations->setText(tt::FNb(0,maxIterations,false).c_str());

    // HACD:
    ui->qqHACD->setChecked(useHACD);
    ui->qqExtraDistPoints->setChecked(addExtraDistPoints);
    ui->qqExtraFacesPoints->setChecked(addFacesPoints);
    ui->qqClusters->setText(tt::getIString(false,nClusters).c_str());
    ui->qqConcavity->setText(tt::getFString(false,maxConcavity,3).c_str());
    ui->qqConnectDist->setText(tt::getFString(false,maxConnectDist,3).c_str());
    ui->qqTargetBlabla->setText(tt::getIString(false,maxTrianglesInDecimatedMesh).c_str());
    ui->qqMaxHullVertices->setText(tt::getIString(false,maxHullVertices).c_str());
    ui->qqSmallClusterThreshold->setText(tt::getFString(false,smallClusterThreshold,3).c_str());

    // VHACD:
    ui->qqVHACD->setChecked(!useHACD);
    ui->qqPcaEnabled->setChecked(pca);
    ui->qqVoxelBased->setChecked(voxelBasedMode);
    ui->qqResolution->setText(tt::getIString(false,resolution).c_str());
    ui->qqDepth->setText(tt::getIString(false,depth).c_str());
    ui->qqConcavity_2->setText(tt::getFString(false,concavity,3).c_str());
    ui->qqPlaneDownsampling->setText(tt::getIString(false,planeDownsampling).c_str());
    ui->qqConvexHullDownsampling->setText(tt::getIString(false,convexHullDownsampling).c_str());
    ui->qqAlpha->setText(tt::getFString(false,alpha,3).c_str());
    ui->qqBeta->setText(tt::getFString(false,beta,3).c_str());
    ui->qqGamma->setText(tt::getFString(false,gamma,3).c_str());
    ui->qqMaxVerticesPerCh->setText(tt::getIString(false,maxNumVerticesPerCH).c_str());
    ui->qqMinVolumePerCh->setText(tt::getEString(false,minVolumePerCH,3).c_str());
}

void CQDlgConvexDecomposition::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgConvexDecomposition::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgConvexDecomposition::on_qqExtraDistPoints_clicked()
{
    addExtraDistPoints=!addExtraDistPoints;
    refresh();
}

void CQDlgConvexDecomposition::on_qqExtraFacesPoints_clicked()
{
    addFacesPoints=!addFacesPoints;
    refresh();
}

void CQDlgConvexDecomposition::on_qqClusters_editingFinished()
{
    if (!ui->qqClusters->isModified())
        return;
    bool ok;
    int newVal=ui->qqClusters->text().toInt(&ok);
    if (ok)
        nClusters=tt::getLimitedInt(1,100,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqConcavity_editingFinished()
{
    if (!ui->qqConcavity->isModified())
        return;
    bool ok;
    float newVal=ui->qqConcavity->text().toFloat(&ok);
    if (ok)
        maxConcavity=tt::getLimitedFloat(0.01f,100000.0f,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqConnectDist_editingFinished()
{
    if (!ui->qqConnectDist->isModified())
        return;
    bool ok;
    float newVal=ui->qqConnectDist->text().toFloat(&ok);
    if (ok)
        maxConnectDist=tt::getLimitedFloat(0.001f,1000.0f,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqTargetBlabla_editingFinished()
{
    if (!ui->qqTargetBlabla->isModified())
        return;
    bool ok;
    int newVal=ui->qqTargetBlabla->text().toInt(&ok);
    if (ok)
        maxTrianglesInDecimatedMesh=tt::getLimitedInt(4,100000,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqMaxHullVertices_editingFinished()
{
    if (!ui->qqMaxHullVertices->isModified())
        return;
    bool ok;
    int newVal=ui->qqMaxHullVertices->text().toInt(&ok);
    if (ok)
        maxHullVertices=tt::getLimitedInt(4,100000,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqSmallClusterThreshold_editingFinished()
{
    if (!ui->qqSmallClusterThreshold->isModified())
        return;
    bool ok;
    float newVal=ui->qqSmallClusterThreshold->text().toFloat(&ok);
    if (ok)
        smallClusterThreshold=tt::getLimitedFloat(0.01f,1.0f,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqIndividualComponents_clicked()
{
    individuallyConsiderMultishapeComponents=!individuallyConsiderMultishapeComponents;
    refresh();
}

void CQDlgConvexDecomposition::on_qqMaxIterations_editingFinished()
{
    if (!ui->qqMaxIterations->isModified())
        return;
    bool ok;
    int newVal=ui->qqMaxIterations->text().toInt(&ok);
    if (ok)
        maxIterations=tt::getLimitedInt(1,10,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqPcaEnabled_clicked()
{
    pca=!pca;
    refresh();
}

void CQDlgConvexDecomposition::on_qqVoxelBased_clicked()
{
    voxelBasedMode=!voxelBasedMode;
    refresh();
}

void CQDlgConvexDecomposition::on_qqResolution_editingFinished()
{
    if (!ui->qqResolution->isModified())
        return;
    bool ok;
    int newVal=ui->qqResolution->text().toInt(&ok);
    if (ok)
        resolution=tt::getLimitedInt(10000,64000000,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqDepth_editingFinished()
{
    if (!ui->qqDepth->isModified())
        return;
    bool ok;
    int newVal=ui->qqDepth->text().toInt(&ok);
    if (ok)
        depth=tt::getLimitedInt(1,32,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqConcavity_2_editingFinished()
{
    if (!ui->qqConcavity_2->isModified())
        return;
    bool ok;
    float newVal=ui->qqConcavity_2->text().toFloat(&ok);
    if (ok)
        concavity=tt::getLimitedFloat(0.0f,1.0f,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqPlaneDownsampling_editingFinished()
{
    if (!ui->qqPlaneDownsampling->isModified())
        return;
    bool ok;
    int newVal=ui->qqPlaneDownsampling->text().toInt(&ok);
    if (ok)
        planeDownsampling=tt::getLimitedInt(1,16,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqConvexHullDownsampling_editingFinished()
{
    if (!ui->qqConvexHullDownsampling->isModified())
        return;
    bool ok;
    int newVal=ui->qqConvexHullDownsampling->text().toInt(&ok);
    if (ok)
        convexHullDownsampling=tt::getLimitedInt(1,16,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqAlpha_editingFinished()
{
    if (!ui->qqAlpha->isModified())
        return;
    bool ok;
    float newVal=ui->qqAlpha->text().toFloat(&ok);
    if (ok)
        alpha=tt::getLimitedFloat(0.0f,1.0f,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqBeta_editingFinished()
{
    if (!ui->qqBeta->isModified())
        return;
    bool ok;
    float newVal=ui->qqBeta->text().toFloat(&ok);
    if (ok)
        beta=tt::getLimitedFloat(0.0f,1.0f,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqGamma_editingFinished()
{
    if (!ui->qqGamma->isModified())
        return;
    bool ok;
    float newVal=ui->qqGamma->text().toFloat(&ok);
    if (ok)
        gamma=tt::getLimitedFloat(0.0f,1.0f,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqMaxVerticesPerCh_editingFinished()
{
    if (!ui->qqMaxVerticesPerCh->isModified())
        return;
    bool ok;
    int newVal=ui->qqMaxVerticesPerCh->text().toInt(&ok);
    if (ok)
        maxNumVerticesPerCH=tt::getLimitedInt(4,1024,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqMinVolumePerCh_editingFinished()
{
    if (!ui->qqMinVolumePerCh->isModified())
        return;
    bool ok;
    float newVal=ui->qqMinVolumePerCh->text().toFloat(&ok);
    if (ok)
        minVolumePerCH=tt::getLimitedFloat(0.0f,0.01f,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqHACD_clicked()
{
    useHACD=true;
    refresh();
}

void CQDlgConvexDecomposition::on_qqVHACD_clicked()
{
    useHACD=false;
    refresh();
}
