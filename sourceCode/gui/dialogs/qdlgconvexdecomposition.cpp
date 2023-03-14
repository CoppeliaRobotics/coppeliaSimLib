#include <qdlgconvexdecomposition.h>
#include <ui_qdlgconvexdecomposition.h>
#include <tt.h>
#include <utils.h>
#include <app.h>

CQDlgConvexDecomposition::CQDlgConvexDecomposition(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgConvexDecomposition)
{
    // First set-up default values:
    useHACD=true; // i.e. do not use V-HACD, HACD somehow still better at producing smaller compounds
    // HACD params:
    addExtraDistPoints=true;
    addFacesPoints=true;
    nClusters=1;
    maxHullVertices=200; // from 100 to 200 on 5/2/2014
    maxConcavity=100.0;
    smallClusterThreshold=0.25;
    maxTrianglesInDecimatedMesh=500;
    maxConnectDist=30.0;

    // VHACD params:
    resolution=100000;
    depth=20;
    concavity=0.0025;
    planeDownsampling=4;
    convexHullDownsampling=4;
    alpha=0.05;
    beta=0.05;
    gamma=0.00125;
    pca=false;
    voxelBasedMode=true;
    maxNumVerticesPerCH=64;
    minVolumePerCH=0.0001;

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
    // HACD:
    ui->qqHACD->setChecked(useHACD);
    ui->qqExtraDistPoints->setChecked(addExtraDistPoints);
    ui->qqExtraFacesPoints->setChecked(addFacesPoints);
    ui->qqClusters->setText(utils::getIntString(false,nClusters).c_str());
    ui->qqConcavity->setText(utils::getSizeString(false,maxConcavity).c_str());
    ui->qqConnectDist->setText(utils::getSizeString(false,maxConnectDist).c_str());
    ui->qqTargetBlabla->setText(utils::getIntString(false,maxTrianglesInDecimatedMesh).c_str());
    ui->qqMaxHullVertices->setText(utils::getIntString(false,maxHullVertices).c_str());
    ui->qqSmallClusterThreshold->setText(utils::getSizeString(false,smallClusterThreshold).c_str());

    // VHACD:
    ui->qqVHACD->setChecked(!useHACD);
    ui->qqPcaEnabled->setChecked(pca);
    ui->qqVoxelBased->setChecked(voxelBasedMode);
    ui->qqResolution->setText(utils::getIntString(false,resolution).c_str());
    ui->qqDepth->setText(utils::getIntString(false,depth).c_str());
    ui->qqDepth->setVisible(false);
    ui->depthLabel->setVisible(false);
    ui->qqConcavity_2->setText(utils::getSizeString(false,concavity).c_str());
    ui->qqPlaneDownsampling->setText(utils::getIntString(false,planeDownsampling).c_str());
    ui->qqConvexHullDownsampling->setText(utils::getIntString(false,convexHullDownsampling).c_str());
    ui->qqAlpha->setText(utils::getSizeString(false,alpha).c_str());
    ui->qqBeta->setText(utils::getSizeString(false,beta).c_str());
    ui->qqGamma->setText(utils::getSizeString(false,gamma).c_str());
    ui->qqGamma->setVisible(false);
    ui->gammaLabel->setVisible(false);
    ui->qqMaxVerticesPerCh->setText(utils::getIntString(false,maxNumVerticesPerCH).c_str());
    ui->qqMinVolumePerCh->setText(utils::getVolumeString(minVolumePerCH).c_str());
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
    double newVal=ui->qqConcavity->text().toDouble(&ok);
    if (ok)
        maxConcavity=tt::getLimitedFloat(0.01,100000.0,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqConnectDist_editingFinished()
{
    if (!ui->qqConnectDist->isModified())
        return;
    bool ok;
    double newVal=ui->qqConnectDist->text().toDouble(&ok);
    if (ok)
        maxConnectDist=tt::getLimitedFloat(0.001,1000.0,newVal);
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
    double newVal=ui->qqSmallClusterThreshold->text().toDouble(&ok);
    if (ok)
        smallClusterThreshold=tt::getLimitedFloat(0.01,1.0,newVal);
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
    double newVal=ui->qqConcavity_2->text().toDouble(&ok);
    if (ok)
        concavity=tt::getLimitedFloat(0.0,1.0,newVal);
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
    double newVal=ui->qqAlpha->text().toDouble(&ok);
    if (ok)
        alpha=tt::getLimitedFloat(0.0,1.0,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqBeta_editingFinished()
{
    if (!ui->qqBeta->isModified())
        return;
    bool ok;
    double newVal=ui->qqBeta->text().toDouble(&ok);
    if (ok)
        beta=tt::getLimitedFloat(0.0,1.0,newVal);
    refresh();
}

void CQDlgConvexDecomposition::on_qqGamma_editingFinished()
{
    if (!ui->qqGamma->isModified())
        return;
    bool ok;
    double newVal=ui->qqGamma->text().toDouble(&ok);
    if (ok)
        gamma=tt::getLimitedFloat(0.0,1.0,newVal);
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
    double newVal=ui->qqMinVolumePerCh->text().toDouble(&ok);
    if (ok)
        minVolumePerCH=tt::getLimitedFloat(0.0,0.1,newVal);
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
