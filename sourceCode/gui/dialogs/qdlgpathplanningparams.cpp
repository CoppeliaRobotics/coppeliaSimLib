
#include "qdlgpathplanningparams.h"
#include "ui_qdlgpathplanningparams.h"
#include "gV.h"
#include "tt.h"
#include "app.h"


void CQDlgPathPlanningParams::_setSearchRange(float searchMin[4],float searchR[4])
{
    for (int i=0;i<3;i++)
    {
        float v=searchMin[i];
        tt::limitValue(-1000.0f,1000.0f,v);
        searchRangeMin[i]=v;
        v=searchR[i];
        tt::limitValue(0.0001f,2000.0f,v);
        searchRangeMax[i]=v;
    }
    float v=tt::getNormalizedAngle(searchMin[3]);
    searchRangeMin[3]=v;
    v=searchR[3];
    tt::limitValue(0.1f*degToRad_f,piValTimes2_f,v);
    searchRangeMax[3]=v;
}

void CQDlgPathPlanningParams::_setSearchDirection(int dir[4])
{
    for (int i=0;i<4;i++)
    {
        searchDir[i]=dir[i];
        if (searchDir[i]<-1)
            searchDir[i]=-1;
        if (searchDir[i]>1)
            searchDir[i]=1;
    }
}

CQDlgPathPlanningParams::CQDlgPathPlanningParams(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgPathPlanningParams)
{
    ui->setupUi(this);
}

CQDlgPathPlanningParams::~CQDlgPathPlanningParams()
{
    delete ui;
}

void CQDlgPathPlanningParams::cancelEvent()
{
//  defaultModalDialogEndRoutine(false);
}

void CQDlgPathPlanningParams::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgPathPlanningParams::refresh()
{
    inMainRefreshRoutine=true;

    int t=planningType;
    int ht=holonomicType;

    ui->qqHoloTypeCombo->setEnabled(t==sim_pathplanning_holonomic);
    ui->qqDeltaX->setEnabled(t==sim_pathplanning_holonomic);
    ui->qqDeltaY->setEnabled(t==sim_pathplanning_holonomic);
    ui->qqdeltaZ->setEnabled(t==sim_pathplanning_holonomic);
    ui->qqStepSizeA->setEnabled(t==sim_pathplanning_holonomic);
    ui->qqMinTurningCircle->setEnabled(t==sim_pathplanning_nonholonomic);

    ui->qqMinX->setEnabled((t==sim_pathplanning_nonholonomic)||(ht!=sim_holonomicpathplanning_abg));
    ui->qqMinX_2->setEnabled((t==sim_pathplanning_nonholonomic)||(ht!=sim_holonomicpathplanning_abg));
    ui->qqMinusX->setEnabled((t==sim_pathplanning_holonomic)&&(ht!=sim_holonomicpathplanning_abg));
    ui->qqPlusX->setEnabled((t==sim_pathplanning_holonomic)&&(ht!=sim_holonomicpathplanning_abg));

    ui->qqMinY->setEnabled((t==sim_pathplanning_nonholonomic)||((ht!=sim_holonomicpathplanning_xg)&&(ht!=sim_holonomicpathplanning_xg)&&(ht!=sim_holonomicpathplanning_abg)&&(ht!=sim_holonomicpathplanning_xabg)));
    ui->qqMinY_2->setEnabled((t==sim_pathplanning_nonholonomic)||((ht!=sim_holonomicpathplanning_xg)&&(ht!=sim_holonomicpathplanning_xg)&&(ht!=sim_holonomicpathplanning_abg)&&(ht!=sim_holonomicpathplanning_xabg)));
    ui->qqMinusY->setEnabled((t==sim_pathplanning_holonomic)&&((ht!=sim_holonomicpathplanning_xg)&&(ht!=sim_holonomicpathplanning_xg)&&(ht!=sim_holonomicpathplanning_abg)&&(ht!=sim_holonomicpathplanning_xabg)));
    ui->qqPlusY->setEnabled((t==sim_pathplanning_holonomic)&&((ht!=sim_holonomicpathplanning_xg)&&(ht!=sim_holonomicpathplanning_xg)&&(ht!=sim_holonomicpathplanning_abg)&&(ht!=sim_holonomicpathplanning_xabg)));

    ui->qqMinZ->setEnabled((t==sim_pathplanning_holonomic)&&((ht==sim_holonomicpathplanning_xyz)||(ht==sim_holonomicpathplanning_xyzg)||(ht==sim_holonomicpathplanning_xyzabg)));
    ui->qqMinZ_2->setEnabled((t==sim_pathplanning_holonomic)&&((ht==sim_holonomicpathplanning_xyz)||(ht==sim_holonomicpathplanning_xyzg)||(ht==sim_holonomicpathplanning_xyzabg)));
    ui->qqMinusZ->setEnabled((t==sim_pathplanning_holonomic)&&((ht==sim_holonomicpathplanning_xyz)||(ht==sim_holonomicpathplanning_xyzg)||(ht==sim_holonomicpathplanning_xyzabg)));
    ui->qqPlusZ->setEnabled((t==sim_pathplanning_holonomic)&&((ht==sim_holonomicpathplanning_xyz)||(ht==sim_holonomicpathplanning_xyzg)||(ht==sim_holonomicpathplanning_xyzabg)));

    ui->qqMinD->setEnabled((t==sim_pathplanning_holonomic)&&((ht==sim_holonomicpathplanning_xg)||(ht==sim_holonomicpathplanning_xyg)||(ht==sim_holonomicpathplanning_xyzg)));
    ui->qqMinD_2->setEnabled((t==sim_pathplanning_holonomic)&&((ht==sim_holonomicpathplanning_xg)||(ht==sim_holonomicpathplanning_xyg)||(ht==sim_holonomicpathplanning_xyzg)));
    ui->qqMinusD->setEnabled((t==sim_pathplanning_holonomic)&&((ht==sim_holonomicpathplanning_xg)||(ht==sim_holonomicpathplanning_xyg)||(ht==sim_holonomicpathplanning_xyzg)));
    ui->qqPlusD->setEnabled((t==sim_pathplanning_holonomic)&&((ht==sim_holonomicpathplanning_xg)||(ht==sim_holonomicpathplanning_xyg)||(ht==sim_holonomicpathplanning_xyzg)));

    ui->qqVisualizeArea->setChecked(visualizeArea);

    ui->qqHoloTypeCombo->clear();

    ui->qqStepSizeL->setText(tt::getFString(false,stepSize,3).c_str());

    if (t==sim_pathplanning_holonomic)
    {
        const char* texts[9]={strTranslate("X-Y"),strTranslate("X-Delta"),strTranslate("X-Y-Z"),strTranslate("X-Y-Delta"),strTranslate("Alpha-Beta-Gamma"),
                        strTranslate("X-Y-Z-Delta"),strTranslate("X-Alpha-Beta-Gamma"),strTranslate("X-Y-Alpha-Beta-Gamma"),
                        strTranslate("X-Y-Z-Alpha-Beta-Gamma")};
        const int datas[9]={sim_holonomicpathplanning_xy,sim_holonomicpathplanning_xg,sim_holonomicpathplanning_xyz,sim_holonomicpathplanning_xyg,
                        sim_holonomicpathplanning_abg,sim_holonomicpathplanning_xyzg,sim_holonomicpathplanning_xabg,
                        sim_holonomicpathplanning_xyabg,sim_holonomicpathplanning_xyzabg};
        for (int i=0;i<9;i++)
            ui->qqHoloTypeCombo->addItem(texts[i],QVariant(datas[i]));

        for (int i=0;i<ui->qqHoloTypeCombo->count();i++)
        {
            if (ui->qqHoloTypeCombo->itemData(i).toInt()==holonomicType)
            {
                ui->qqHoloTypeCombo->setCurrentIndex(i);
                break;
            }
        }

        C3Vector a(gammaAxis);

        ui->qqDeltaX->setText(tt::getFString(true,a(0),2).c_str());
        ui->qqDeltaY->setText(tt::getFString(true,a(1),2).c_str());
        ui->qqdeltaZ->setText(tt::getFString(true,a(2),2).c_str());
        ui->qqStepSizeA->setText(tt::getAngleFString(false,angularStepSize,2).c_str());
        ui->qqMinTurningCircle->setText("");
    }
    else
    {
        ui->qqDeltaX->setText("");
        ui->qqDeltaY->setText("");
        ui->qqdeltaZ->setText("");
        ui->qqStepSizeA->setText("");
        ui->qqMinTurningCircle->setText(tt::getFString(false,minTurningCircleDiameter,3).c_str());
    }

    if ((t==sim_pathplanning_nonholonomic)||(ht!=sim_holonomicpathplanning_abg))
    {
        ui->qqMinX->setText(tt::getEString(true,searchRangeMin[0],1).c_str());
        ui->qqMinX_2->setText(tt::getEString(true,searchRangeMax[0],1).c_str());
        if (t==sim_pathplanning_nonholonomic)
        {
            ui->qqMinusX->setChecked(false);
            ui->qqPlusX->setChecked(false);
        }
        else
        {
            ui->qqMinusX->setChecked(searchDir[0]<=0);
            ui->qqPlusX->setChecked(searchDir[0]>=0);
        }
    }
    else
    {
        ui->qqMinX->setText("");
        ui->qqMinX_2->setText("");
        ui->qqMinusX->setChecked(false);
        ui->qqPlusX->setChecked(false);
    }

    if ((t==sim_pathplanning_nonholonomic)||((ht!=sim_holonomicpathplanning_xg)&&(ht!=sim_holonomicpathplanning_xg)&&(ht!=sim_holonomicpathplanning_abg)&&(ht!=sim_holonomicpathplanning_xabg)))
    {
        ui->qqMinY->setText(tt::getEString(true,searchRangeMin[1],1).c_str());
        ui->qqMinY_2->setText(tt::getEString(true,searchRangeMax[1],1).c_str());
        if (t==sim_pathplanning_nonholonomic)
        {
            ui->qqMinusY->setChecked(false);
            ui->qqPlusY->setChecked(false);
        }
        else
        {
            ui->qqMinusY->setChecked(searchDir[1]<=0);
            ui->qqPlusY->setChecked(searchDir[1]>=0);
        }
    }
    else
    {
        ui->qqMinY->setText("");
        ui->qqMinY_2->setText("");
        ui->qqMinusY->setChecked(false);
        ui->qqPlusY->setChecked(false);
    }

    if ((t==sim_pathplanning_holonomic)&&((ht==sim_holonomicpathplanning_xyz)||(ht==sim_holonomicpathplanning_xyzg)||(ht==sim_holonomicpathplanning_xyzabg)))
    {
        ui->qqMinZ->setText(tt::getEString(true,searchRangeMin[2],1).c_str());
        ui->qqMinZ_2->setText(tt::getEString(true,searchRangeMax[2],1).c_str());
        ui->qqMinusZ->setChecked(searchDir[2]<=0);
        ui->qqPlusZ->setChecked(searchDir[2]>=0);
    }
    else
    {
        ui->qqMinZ->setText("");
        ui->qqMinZ_2->setText("");
        ui->qqMinusZ->setChecked(false);
        ui->qqPlusZ->setChecked(false);
    }


    if ((t==sim_pathplanning_holonomic)&&((ht==sim_holonomicpathplanning_xg)||(ht==sim_holonomicpathplanning_xyg)||(ht==sim_holonomicpathplanning_xyzg)))
    {
        ui->qqMinD->setText(tt::getEString(true,searchRangeMin[3]*radToDeg_f,1).c_str());
        ui->qqMinD_2->setText(tt::getEString(true,searchRangeMax[3]*radToDeg_f,1).c_str());
        ui->qqMinusD->setChecked(searchDir[3]<=0);
        ui->qqPlusD->setChecked(searchDir[3]>=0);
    }
    else
    {
        ui->qqMinD->setText("");
        ui->qqMinD_2->setText("");
        ui->qqMinusD->setChecked(false);
        ui->qqPlusD->setChecked(false);
    }

    inMainRefreshRoutine=false;
}

void CQDlgPathPlanningParams::on_qqClose_clicked(QAbstractButton *button)
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgPathPlanningParams::on_qqHoloTypeCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        holonomicType=ui->qqHoloTypeCombo->itemData(ui->qqHoloTypeCombo->currentIndex()).toInt();
        refresh();
    }
}

void CQDlgPathPlanningParams::on_qqDeltaX_editingFinished()
{
    if (!ui->qqDeltaX->isModified())
        return;
    bool ok;
    float newVal=ui->qqDeltaX->text().toFloat(&ok);
    if (ok)
        gammaAxis(0)=newVal;
    refresh();
}

void CQDlgPathPlanningParams::on_qqDeltaY_editingFinished()
{
    if (!ui->qqDeltaY->isModified())
        return;
    bool ok;
    float newVal=ui->qqDeltaY->text().toFloat(&ok);
    if (ok)
        gammaAxis(1)=newVal;
    refresh();
}

void CQDlgPathPlanningParams::on_qqdeltaZ_editingFinished()
{
    if (!ui->qqdeltaZ->isModified())
        return;
    bool ok;
    float newVal=ui->qqdeltaZ->text().toFloat(&ok);
    if (ok)
        gammaAxis(2)=newVal;
    refresh();
}

void CQDlgPathPlanningParams::on_qqStepSizeL_editingFinished()
{
    if (!ui->qqStepSizeL->isModified())
        return;
    bool ok;
    float newVal=ui->qqStepSizeL->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0001f,10.0f,newVal);
        stepSize=newVal;
        if (sin(44.0*degToRad)*minTurningCircleDiameter<stepSize)
            minTurningCircleDiameter=stepSize/sin(44.0*degToRad);
    }
    refresh();
}

void CQDlgPathPlanningParams::on_qqStepSizeA_editingFinished()
{
    if (!ui->qqStepSizeA->isModified())
        return;
    bool ok;
    float newVal=ui->qqStepSizeA->text().toFloat(&ok);
    if (ok)
    {
        newVal*=gv::userToRad;
        tt::limitValue(0.1f*degToRad_f,90.0f*degToRad_f,newVal);
        angularStepSize=newVal;
    }
    refresh();
}

void CQDlgPathPlanningParams::on_qqMinTurningCircle_editingFinished()
{
    if (!ui->qqMinTurningCircle->isModified())
        return;
    bool ok;
    float newVal=ui->qqMinTurningCircle->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.001f,10.0f,newVal);
        minTurningCircleDiameter=newVal;
        if (sin(44.0*degToRad)*minTurningCircleDiameter<stepSize)
            stepSize=minTurningCircleDiameter*sin(44.0*degToRad);
    }
    refresh();
}

void CQDlgPathPlanningParams::on_qqVisualizeArea_clicked()
{
    visualizeArea=!visualizeArea;
    refresh();
}

void CQDlgPathPlanningParams::on_qqMinX_editingFinished()
{
    if (!ui->qqMinX->isModified())
        return;
    bool ok;
    float newVal=ui->qqMinX->text().toFloat(&ok);
    if (ok)
    {
        searchRangeMin[0]=newVal;
        _setSearchRange(searchRangeMin,searchRangeMax);
    }
    refresh();
}

void CQDlgPathPlanningParams::on_qqMinY_editingFinished()
{
    if (!ui->qqMinY->isModified())
        return;
    bool ok;
    float newVal=ui->qqMinY->text().toFloat(&ok);
    if (ok)
    {
        searchRangeMin[1]=newVal;
        _setSearchRange(searchRangeMin,searchRangeMax);
    }
    refresh();
}

void CQDlgPathPlanningParams::on_qqMinZ_editingFinished()
{
    if (!ui->qqMinZ->isModified())
        return;
    bool ok;
    float newVal=ui->qqMinZ->text().toFloat(&ok);
    if (ok)
    {
        searchRangeMin[2]=newVal;
        _setSearchRange(searchRangeMin,searchRangeMax);
    }
    refresh();
}

void CQDlgPathPlanningParams::on_qqMinD_editingFinished()
{
    if (!ui->qqMinD->isModified())
        return;
    bool ok;
    float newVal=ui->qqMinD->text().toFloat(&ok);
    if (ok)
    {
        searchRangeMin[3]=newVal*gv::userToRad;
        _setSearchRange(searchRangeMin,searchRangeMax);
    }
    refresh();
}

void CQDlgPathPlanningParams::on_qqMinX_2_editingFinished()
{
    if (!ui->qqMinX_2->isModified())
        return;
    bool ok;
    float newVal=ui->qqMinX_2->text().toFloat(&ok);
    if (ok)
    {
        searchRangeMax[0]=newVal;
        _setSearchRange(searchRangeMin,searchRangeMax);
    }
    refresh();
}

void CQDlgPathPlanningParams::on_qqMinY_2_editingFinished()
{
    if (!ui->qqMinY_2->isModified())
        return;
    bool ok;
    float newVal=ui->qqMinY_2->text().toFloat(&ok);
    if (ok)
    {
        searchRangeMax[1]=newVal;
        _setSearchRange(searchRangeMin,searchRangeMax);
    }
    refresh();
}

void CQDlgPathPlanningParams::on_qqMinZ_2_editingFinished()
{
    if (!ui->qqMinZ_2->isModified())
        return;
    bool ok;
    float newVal=ui->qqMinZ_2->text().toFloat(&ok);
    if (ok)
    {
        searchRangeMax[2]=newVal;
        _setSearchRange(searchRangeMin,searchRangeMax);
    }
    refresh();
}

void CQDlgPathPlanningParams::on_qqMinD_2_editingFinished()
{
    if (!ui->qqMinD_2->isModified())
        return;
    bool ok;
    float newVal=ui->qqMinD_2->text().toFloat(&ok);
    if (ok)
    {
        searchRangeMax[3]=newVal*gv::userToRad;
        _setSearchRange(searchRangeMin,searchRangeMax);
    }
    refresh();
}

void CQDlgPathPlanningParams::on_qqMinusX_clicked()
{
    if (searchDir[0]==0)
        searchDir[0]=1;
    else
    {
        if (searchDir[0]>0)
            searchDir[0]=0;
        else
            searchDir[0]=1;
    }
    _setSearchDirection(searchDir);
    refresh();
}

void CQDlgPathPlanningParams::on_qqPlusX_clicked()
{
    if (searchDir[0]==0)
        searchDir[0]=-1;
    else
    {
        if (searchDir[0]<0)
            searchDir[0]=0;
        else
            searchDir[0]=-1;
    }
    _setSearchDirection(searchDir);
    refresh();
}

void CQDlgPathPlanningParams::on_qqMinusY_clicked()
{
    if (searchDir[1]==0)
        searchDir[1]=1;
    else
    {
        if (searchDir[1]>0)
            searchDir[1]=0;
        else
            searchDir[1]=1;
    }
    _setSearchDirection(searchDir);
    refresh();
}

void CQDlgPathPlanningParams::on_qqPlusY_clicked()
{
    if (searchDir[1]==0)
        searchDir[1]=-1;
    else
    {
        if (searchDir[1]<0)
            searchDir[1]=0;
        else
            searchDir[1]=-1;
    }
    _setSearchDirection(searchDir);
    refresh();
}

void CQDlgPathPlanningParams::on_qqMinusZ_clicked()
{
    if (searchDir[2]==0)
        searchDir[2]=1;
    else
    {
        if (searchDir[2]>0)
            searchDir[2]=0;
        else
            searchDir[2]=1;
    }
    _setSearchDirection(searchDir);
    refresh();
}

void CQDlgPathPlanningParams::on_qqPlusZ_clicked()
{
    if (searchDir[2]==0)
        searchDir[2]=-1;
    else
    {
        if (searchDir[2]<0)
            searchDir[2]=0;
        else
            searchDir[2]=-1;
    }
    _setSearchDirection(searchDir);
    refresh();
}

void CQDlgPathPlanningParams::on_qqMinusD_clicked()
{
    if (searchDir[3]==0)
        searchDir[3]=1;
    else
    {
        if (searchDir[3]>0)
            searchDir[3]=0;
        else
            searchDir[3]=1;
    }
    _setSearchDirection(searchDir);
    refresh();
}

void CQDlgPathPlanningParams::on_qqPlusD_clicked()
{
    if (searchDir[3]==0)
        searchDir[3]=-1;
    else
    {
        if (searchDir[3]<0)
            searchDir[3]=0;
        else
            searchDir[3]=-1;
    }
    _setSearchDirection(searchDir);
    refresh();
}
