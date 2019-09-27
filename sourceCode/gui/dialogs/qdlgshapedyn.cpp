
#include "vrepMainHeader.h"
#include "qdlgshapedyn.h"
#include "ui_qdlgshapedyn.h"
#include "tt.h"
#include "gV.h"
#include "propBrowser_engineProp_material.h"
#include "v_repStrings.h"
#include "app.h"
#include "qdlginertiatensor.h"
#include <QInputDialog>
#include "vMessageBox.h"

bool CQDlgShapeDyn::showDynamicWindow=false;

CQDlgShapeDyn::CQDlgShapeDyn(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgShapeDyn)
{
    _dlgType=SHAPE_DYN_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgShapeDyn::~CQDlgShapeDyn()
{
    delete ui;
}

void CQDlgShapeDyn::cancelEvent()
{ // no cancel event allowed
    showDynamicWindow=false;
    CDlgEx::cancelEvent();
    App::setFullDialogRefreshFlag();
}

void CQDlgShapeDyn::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeAndNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();
    bool sel=App::ct->objCont->isLastSelectionAShape();
    int sc=App::ct->objCont->getShapeNumberInSelection();
    bool notHeightfield=true;
    bool lastSelIsNotStatic=false;
    bool lastSelIsConvex=false;
    CShape* it=App::ct->objCont->getLastSelection_shape();
    CGeomProxy* geom=nullptr;
    if (sel)
    {
        geom=it->geomData;
        notHeightfield=(it->geomData->geomInfo->getPurePrimitiveType()!=sim_pure_primitive_heightfield);
        lastSelIsNotStatic=!it->getShapeIsDynamicallyStatic();
        lastSelIsConvex=geom->geomInfo->isConvex();
    }

    // Material properties:
    ui->qqRespondable->setEnabled(sel&&noEditModeAndNoSim);
    ui->a_1->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_2->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_3->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_4->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_5->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_6->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_7->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_8->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_9->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_10->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_11->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_12->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_13->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_14->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_15->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->a_16->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->qqAdjustEngineProperties->setEnabled(sel&&noEditModeAndNoSim&&it->getRespondable());
    ui->qqApplyMaterialProperties->setEnabled((sc>1)&&sel&&noEditModeAndNoSim);

    ui->qqRespondable->setChecked(sel&&it->getRespondable());
    ui->a_1->setChecked(sel&&(it->getDynamicCollisionMask()&0x0001));
    ui->a_2->setChecked(sel&&(it->getDynamicCollisionMask()&0x0002));
    ui->a_3->setChecked(sel&&(it->getDynamicCollisionMask()&0x0004));
    ui->a_4->setChecked(sel&&(it->getDynamicCollisionMask()&0x0008));
    ui->a_5->setChecked(sel&&(it->getDynamicCollisionMask()&0x0010));
    ui->a_6->setChecked(sel&&(it->getDynamicCollisionMask()&0x0020));
    ui->a_7->setChecked(sel&&(it->getDynamicCollisionMask()&0x0040));
    ui->a_8->setChecked(sel&&(it->getDynamicCollisionMask()&0x0080));
    ui->a_9->setChecked(sel&&(it->getDynamicCollisionMask()&0x0100));
    ui->a_10->setChecked(sel&&(it->getDynamicCollisionMask()&0x0200));
    ui->a_11->setChecked(sel&&(it->getDynamicCollisionMask()&0x0400));
    ui->a_12->setChecked(sel&&(it->getDynamicCollisionMask()&0x0800));
    ui->a_13->setChecked(sel&&(it->getDynamicCollisionMask()&0x1000));
    ui->a_14->setChecked(sel&&(it->getDynamicCollisionMask()&0x2000));
    ui->a_15->setChecked(sel&&(it->getDynamicCollisionMask()&0x4000));
    ui->a_16->setChecked(sel&&(it->getDynamicCollisionMask()&0x8000));

    // Mass/inertia properties:
    ui->qqDynamic->setEnabled(sel&&noEditModeAndNoSim&&notHeightfield);
    ui->qqSleepModeStart->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqComputeMassProperties->setEnabled(sel&&noEditModeAndNoSim&&lastSelIsNotStatic&&lastSelIsConvex);
    ui->qqMass->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqMassD2->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqMassT2->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqID2->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqIT2->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqTensorMatrix->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqIX->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqIY->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqIZ->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqPX->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqPY->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqPZ->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqAlpha->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqBeta->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqGamma->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqApplyMassAndInertiaProperties->setEnabled((sc>1)&&sel&&noEditModeAndNoSim);

    ui->qqDynamic->setChecked(sel&&lastSelIsNotStatic);
    ui->qqAutomaticToNonStatic->setChecked(sel&&it->getSetAutomaticallyToNonStaticIfGetsParent());
    ui->qqSleepModeStart->setChecked(sel&&it->getStartInDynamicSleeping());
    if (sel)
    {
        ui->qqMass->setText(tt::getEString(false,geom->geomInfo->getMass(),3).c_str());
        C3Vector v(it->geomData->geomInfo->getPrincipalMomentsOfInertia());
        ui->qqIX->setText(tt::getEString(false,v(0),3).c_str());
        ui->qqIY->setText(tt::getEString(false,v(1),3).c_str());
        ui->qqIZ->setText(tt::getEString(false,v(2),3).c_str());
        C4X4Matrix m(it->geomData->geomInfo->getLocalInertiaFrame().getMatrix());
        C3Vector e(m.M.getEulerAngles());
        ui->qqPX->setText(tt::getEString(true,m.X(0),3).c_str());
        ui->qqPY->setText(tt::getEString(true,m.X(1),3).c_str());
        ui->qqPZ->setText(tt::getEString(true,m.X(2),3).c_str());
        ui->qqAlpha->setText(tt::getAngleEString(true,e(0),2).c_str());
        ui->qqBeta->setText(tt::getAngleEString(true,e(1),2).c_str());
        ui->qqGamma->setText(tt::getAngleEString(true,e(2),2).c_str());
    }
    else
    {
        ui->qqMass->setText("");
        ui->qqIX->setText("");
        ui->qqIY->setText("");
        ui->qqIZ->setText("");
        ui->qqPX->setText("");
        ui->qqPY->setText("");
        ui->qqPZ->setText("");
        ui->qqAlpha->setText("");
        ui->qqBeta->setText("");
        ui->qqGamma->setText("");
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgShapeDyn::on_qqDynamic_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_STATIC_SHAPEDYNGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqSleepModeStart_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_STARTINSLEEPMODE_SHAPEDYNGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqAdjustEngineProperties_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CShape* it=App::ct->objCont->getLastSelection_shape();
        if (it!=nullptr)
        {
            CPropBrowserEngineMaterial dlg(this);//App::mainWindow);
            dlg.setModal(true);
            dlg.exec();
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ENGINEPARAMS_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
            CDynMaterialObject* mat=it->getDynMaterial();
            cmd.stringParams.push_back(mat->getObjectName());
            std::vector<int> iParams;
            std::vector<float> fParams;
            mat->getBulletIntParams(iParams);
            mat->getBulletFloatParams(fParams);
            cmd.intVectorParams.push_back(iParams);
            cmd.floatVectorParams.push_back(fParams);
            iParams.clear();
            fParams.clear();
            mat->getOdeIntParams(iParams);
            mat->getOdeFloatParams(fParams);
            cmd.intVectorParams.push_back(iParams);
            cmd.floatVectorParams.push_back(fParams);
            iParams.clear();
            fParams.clear();
            mat->getVortexIntParams(iParams);
            mat->getVortexFloatParams(fParams);
            cmd.intVectorParams.push_back(iParams);
            cmd.floatVectorParams.push_back(fParams);
            iParams.clear();
            fParams.clear();
            mat->getNewtonIntParams(iParams);
            mat->getNewtonFloatParams(fParams);
            cmd.intVectorParams.push_back(iParams);
            cmd.floatVectorParams.push_back(fParams);
            iParams.clear();
            fParams.clear();
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgShapeDyn::on_qqRespondable_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CShape* it=App::ct->objCont->getLastSelection_shape();
        if (it!=nullptr)
        {
            if ((!it->getRespondable())&&(!it->geomData->geomInfo->isPure())&&(!it->geomData->geomInfo->isConvex()))
                App::uiThread->messageBox_warning(App::mainWindow,strTranslate("Shape"),strTranslate(IDS_MAKING_NON_PURE_CONCAVE_SHAPE_RESPONDABLE_WARNING),VMESSAGEBOX_OKELI);
            App::appendSimulationThreadCommand(TOGGLE_RESPONDABLE_SHAPEDYNGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::_toggleRespondableBits(int bits)
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CShape* it=App::ct->objCont->getLastSelection_shape();
        if (it!=nullptr)
        {
            App::appendSimulationThreadCommand(SET_RESPONDABLEMASK_SHAPEDYNGUITRIGGEREDCMD,it->getObjectHandle(),it->getDynamicCollisionMask()^bits);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}


void CQDlgShapeDyn::on_a_1_clicked()
{
    _toggleRespondableBits(0x0001);
}

void CQDlgShapeDyn::on_a_2_clicked()
{
    _toggleRespondableBits(0x0002);
}

void CQDlgShapeDyn::on_a_3_clicked()
{
    _toggleRespondableBits(0x0004);
}

void CQDlgShapeDyn::on_a_4_clicked()
{
    _toggleRespondableBits(0x0008);
}

void CQDlgShapeDyn::on_a_5_clicked()
{
    _toggleRespondableBits(0x0010);
}

void CQDlgShapeDyn::on_a_6_clicked()
{
    _toggleRespondableBits(0x0020);
}

void CQDlgShapeDyn::on_a_7_clicked()
{
    _toggleRespondableBits(0x0040);
}

void CQDlgShapeDyn::on_a_8_clicked()
{
    _toggleRespondableBits(0x0080);
}

void CQDlgShapeDyn::on_a_9_clicked()
{
    _toggleRespondableBits(0x0100);
}

void CQDlgShapeDyn::on_a_10_clicked()
{
    _toggleRespondableBits(0x0200);
}

void CQDlgShapeDyn::on_a_11_clicked()
{
    _toggleRespondableBits(0x0400);
}

void CQDlgShapeDyn::on_a_12_clicked()
{
    _toggleRespondableBits(0x0800);
}

void CQDlgShapeDyn::on_a_13_clicked()
{
    _toggleRespondableBits(0x1000);
}

void CQDlgShapeDyn::on_a_14_clicked()
{
    _toggleRespondableBits(0x2000);
}

void CQDlgShapeDyn::on_a_15_clicked()
{
    _toggleRespondableBits(0x4000);
}

void CQDlgShapeDyn::on_a_16_clicked()
{
    _toggleRespondableBits(0x8000);
}

void CQDlgShapeDyn::on_qqMass_editingFinished()
{
    if (!ui->qqMass->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqMass->text().toFloat(&ok);
        CShape* shape=App::ct->objCont->getLastSelection_shape();
        if (ok&&(shape!=nullptr))
        {
            App::appendSimulationThreadCommand(SET_MASS_SHAPEDYNGUITRIGGEREDCMD,shape->getObjectHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqMassT2_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=MULTIPLY_MASSFORSELECTION_SHAPEDYNGUITRIGGEREDCMD;
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        cmd.floatParams.push_back(2.0);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqMassD2_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=MULTIPLY_MASSFORSELECTION_SHAPEDYNGUITRIGGEREDCMD;
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        cmd.floatParams.push_back(0.5f);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqIX_editingFinished()
{
    if (!ui->qqIX->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CShape* shape=App::ct->objCont->getLastSelection_shape();
        bool ok;
        float newVal=ui->qqIX->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C3Vector v(shape->geomData->geomInfo->getPrincipalMomentsOfInertia());
            v(0)=newVal;
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_PRINCIPALMOMENTOFINTERIA_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(shape->getObjectHandle());
            cmd.floatParams.push_back(v(0));
            cmd.floatParams.push_back(v(1));
            cmd.floatParams.push_back(v(2));
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqIY_editingFinished()
{
    if (!ui->qqIY->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CShape* shape=App::ct->objCont->getLastSelection_shape();
        bool ok;
        float newVal=ui->qqIY->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C3Vector v(shape->geomData->geomInfo->getPrincipalMomentsOfInertia());
            v(1)=newVal;
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_PRINCIPALMOMENTOFINTERIA_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(shape->getObjectHandle());
            cmd.floatParams.push_back(v(0));
            cmd.floatParams.push_back(v(1));
            cmd.floatParams.push_back(v(2));
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqIZ_editingFinished()
{
    if (!ui->qqIZ->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CShape* shape=App::ct->objCont->getLastSelection_shape();
        bool ok;
        float newVal=ui->qqIZ->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C3Vector v(shape->geomData->geomInfo->getPrincipalMomentsOfInertia());
            v(2)=newVal;
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_PRINCIPALMOMENTOFINTERIA_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(shape->getObjectHandle());
            cmd.floatParams.push_back(v(0));
            cmd.floatParams.push_back(v(1));
            cmd.floatParams.push_back(v(2));
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}


void CQDlgShapeDyn::on_qqIT2_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=MULTIPLY_INERTIAFORSELECTION_SHAPEDYNGUITRIGGEREDCMD;
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        cmd.floatParams.push_back(2.0);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqID2_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=MULTIPLY_INERTIAFORSELECTION_SHAPEDYNGUITRIGGEREDCMD;
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        cmd.floatParams.push_back(0.5f);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqPX_editingFinished()
{
    if (!ui->qqPX->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CShape* shape=App::ct->objCont->getLastSelection_shape();
        bool ok;
        float newVal=ui->qqPX->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C7Vector tr(shape->geomData->geomInfo->getLocalInertiaFrame());
            tr.X(0)=newVal;
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_COMMATRIX_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(shape->getObjectHandle());
            cmd.transfParams.push_back(tr);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqPY_editingFinished()
{
    if (!ui->qqPY->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CShape* shape=App::ct->objCont->getLastSelection_shape();
        bool ok;
        float newVal=ui->qqPY->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C7Vector tr(shape->geomData->geomInfo->getLocalInertiaFrame());
            tr.X(1)=newVal;
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_COMMATRIX_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(shape->getObjectHandle());
            cmd.transfParams.push_back(tr);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqPZ_editingFinished()
{
    if (!ui->qqPZ->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CShape* shape=App::ct->objCont->getLastSelection_shape();
        bool ok;
        float newVal=ui->qqPZ->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C7Vector tr(shape->geomData->geomInfo->getLocalInertiaFrame());
            tr.X(2)=newVal;
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_COMMATRIX_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(shape->getObjectHandle());
            cmd.transfParams.push_back(tr);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqAlpha_editingFinished()
{
    if (!ui->qqAlpha->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CShape* shape=App::ct->objCont->getLastSelection_shape();
        bool ok;
        float newVal=ui->qqAlpha->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C4X4Matrix m(shape->geomData->geomInfo->getLocalInertiaFrame().getMatrix());
            C3Vector e(m.M.getEulerAngles());
            e(0)=newVal*gv::userToRad;
            m.M.setEulerAngles(e);
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_COMMATRIX_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(shape->getObjectHandle());
            cmd.transfParams.push_back(m.getTransformation());
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqBeta_editingFinished()
{
    if (!ui->qqBeta->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CShape* shape=App::ct->objCont->getLastSelection_shape();
        bool ok;
        float newVal=ui->qqBeta->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C4X4Matrix m(shape->geomData->geomInfo->getLocalInertiaFrame().getMatrix());
            C3Vector e(m.M.getEulerAngles());
            e(1)=newVal*gv::userToRad;
            m.M.setEulerAngles(e);
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_COMMATRIX_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(shape->getObjectHandle());
            cmd.transfParams.push_back(m.getTransformation());
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqGamma_editingFinished()
{
    if (!ui->qqGamma->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CShape* shape=App::ct->objCont->getLastSelection_shape();
        bool ok;
        float newVal=ui->qqGamma->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C4X4Matrix m(shape->geomData->geomInfo->getLocalInertiaFrame().getMatrix());
            C3Vector e(m.M.getEulerAngles());
            e(2)=newVal*gv::userToRad;
            m.M.setEulerAngles(e);
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_COMMATRIX_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(shape->getObjectHandle());
            cmd.transfParams.push_back(m.getTransformation());
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqApplyMassAndInertiaProperties_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_DYNPARAMS_SHAPEDYNGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqTensorMatrix_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        if (App::ct->objCont->getSelSize()==0)
            return;
        CShape* shape=App::ct->objCont->getLastSelection_shape();
        CQDlgInertiaTensor theDialog(this);

        C7Vector tr(shape->geomData->geomInfo->getLocalInertiaFrame()); // relative to the shape frame
        tr=shape->getCumulativeTransformation()*tr; // relative to the absolute frame!
        tr.X.clear(); // we just wanna reorient the inertia matrix, no shifting!
        C3X3Matrix m(CGeomWrap::getNewTensor(shape->geomData->geomInfo->getPrincipalMomentsOfInertia(),tr));
        // m is now the absolute inertia tensor centered at the COM
        m.copyToInterface(theDialog.tensor);
        C3Vector com(shape->geomData->geomInfo->getLocalInertiaFrame().X);
        com*=shape->getCumulativeTransformation(); // relative to the absolute frame!
        com.copyTo(theDialog.com);
        theDialog.applyToSelection=false;
        theDialog.refresh();
        if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
        {
            m.copyFromInterface(theDialog.tensor);
            C4Vector rot;
            C3Vector pmoment;
            CGeomWrap::findPrincipalMomentOfInertia(m,rot,pmoment);
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
            {
                CShape* a=App::ct->objCont->getShape(App::ct->objCont->getSelID(i));
                if (a!=nullptr)
                {
                    if ((a==shape)||theDialog.applyToSelection)
                    {
                        a->geomData->geomInfo->setPrincipalMomentsOfInertia(pmoment);
                        a->geomData->geomInfo->setLocalInertiaFrame(a->getCumulativeTransformation().getInverse()*C7Vector(rot,C3Vector(theDialog.com)));
                    }
                }
            }
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // **************** UNDO THINGY ****************
        }
        refresh();
    }
}

void CQDlgShapeDyn::on_qqAutomaticToNonStatic_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SETTODYNAMICIFGETSPARENT_SHAPEDYNGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqApplyMaterialProperties_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_RESPONDABLEPARAMS_SHAPEDYNGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqComputeMassProperties_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float density=(float)QInputDialog::getDouble(this,"Body density","Uniform density",500.0,0.1,30000.0,1,&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=COMPUTE_MASSANDINERTIA_SHAPEDYNGUITRIGGEREDCMD;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                cmd.intParams.push_back(App::ct->objCont->getSelID(i));
            cmd.floatParams.push_back(density);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
