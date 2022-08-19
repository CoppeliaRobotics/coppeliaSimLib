#include "qdlgshapedyn.h"
#include "ui_qdlgshapedyn.h"
#include "tt.h"
#include "gV.h"
#include "simStrings.h"
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
    bool noEditModeAndNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped();
    bool sel=App::currentWorld->sceneObjects->isLastSelectionAShape();
    int sc=App::currentWorld->sceneObjects->getShapeCountInSelection();
    bool notHeightfield=true;
    bool lastSelIsNotStatic=false;
    bool lastSelIsConvex=false;
    CShape* it=App::currentWorld->sceneObjects->getLastSelectionShape();
    if (sel)
    {
        notHeightfield=(it->getMeshWrapper()->getPurePrimitiveType()!=sim_primitiveshape_heightfield);
        lastSelIsNotStatic=!it->getShapeIsDynamicallyStatic();
        lastSelIsConvex=it->getMeshWrapper()->isConvex();
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
    ui->qqSleepModeStart->setVisible(App::userSettings->showOldDlgs);
    ui->qqSleepModeStart->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqComputeMassProperties->setEnabled(sel&&noEditModeAndNoSim&&lastSelIsNotStatic);
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
        ui->qqMass->setText(tt::getEString(false,it->getMeshWrapper()->getMass(),3).c_str());
        C3Vector v(it->getMeshWrapper()->getPrincipalMomentsOfInertia());
        ui->qqIX->setText(tt::getEString(false,v(0),3).c_str());
        ui->qqIY->setText(tt::getEString(false,v(1),3).c_str());
        ui->qqIZ->setText(tt::getEString(false,v(2),3).c_str());
        C4X4Matrix m(it->getMeshWrapper()->getLocalInertiaFrame().getMatrix());
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
        App::appendSimulationThreadCommand(TOGGLE_STATIC_SHAPEDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqSleepModeStart_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_STARTINSLEEPMODE_SHAPEDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqAdjustEngineProperties_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CShape* it=App::currentWorld->sceneObjects->getLastSelectionShape();
        if (it!=nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_MATERIAL_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
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
        CShape* it=App::currentWorld->sceneObjects->getLastSelectionShape();
        if (it!=nullptr)
        {
            if ((!it->getRespondable())&&(!it->getMeshWrapper()->isPure())&&(!it->getMeshWrapper()->isConvex()))
                App::uiThread->messageBox_warning(App::mainWindow,"Shape",IDS_MAKING_NON_PURE_CONCAVE_SHAPE_RESPONDABLE_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
            App::appendSimulationThreadCommand(TOGGLE_RESPONDABLE_SHAPEDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::_toggleRespondableBits(int bits)
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CShape* it=App::currentWorld->sceneObjects->getLastSelectionShape();
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
        CShape* shape=App::currentWorld->sceneObjects->getLastSelectionShape();
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
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
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
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
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
        CShape* shape=App::currentWorld->sceneObjects->getLastSelectionShape();
        bool ok;
        float newVal=ui->qqIX->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C3Vector v(shape->getMeshWrapper()->getPrincipalMomentsOfInertia());
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
        CShape* shape=App::currentWorld->sceneObjects->getLastSelectionShape();
        bool ok;
        float newVal=ui->qqIY->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C3Vector v(shape->getMeshWrapper()->getPrincipalMomentsOfInertia());
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
        CShape* shape=App::currentWorld->sceneObjects->getLastSelectionShape();
        bool ok;
        float newVal=ui->qqIZ->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C3Vector v(shape->getMeshWrapper()->getPrincipalMomentsOfInertia());
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
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
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
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
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
        CShape* shape=App::currentWorld->sceneObjects->getLastSelectionShape();
        bool ok;
        float newVal=ui->qqPX->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C7Vector tr(shape->getMeshWrapper()->getLocalInertiaFrame());
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
        CShape* shape=App::currentWorld->sceneObjects->getLastSelectionShape();
        bool ok;
        float newVal=ui->qqPY->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C7Vector tr(shape->getMeshWrapper()->getLocalInertiaFrame());
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
        CShape* shape=App::currentWorld->sceneObjects->getLastSelectionShape();
        bool ok;
        float newVal=ui->qqPZ->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C7Vector tr(shape->getMeshWrapper()->getLocalInertiaFrame());
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
        CShape* shape=App::currentWorld->sceneObjects->getLastSelectionShape();
        bool ok;
        float newVal=ui->qqAlpha->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C4X4Matrix m(shape->getMeshWrapper()->getLocalInertiaFrame().getMatrix());
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
        CShape* shape=App::currentWorld->sceneObjects->getLastSelectionShape();
        bool ok;
        float newVal=ui->qqBeta->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C4X4Matrix m(shape->getMeshWrapper()->getLocalInertiaFrame().getMatrix());
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
        CShape* shape=App::currentWorld->sceneObjects->getLastSelectionShape();
        bool ok;
        float newVal=ui->qqGamma->text().toFloat(&ok);
        if (ok&&(shape!=nullptr))
        {
            C4X4Matrix m(shape->getMeshWrapper()->getLocalInertiaFrame().getMatrix());
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
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount()-1;i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqTensorMatrix_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        if (App::currentWorld->sceneObjects->getSelectionCount()==0)
            return;
        CShape* shape=App::currentWorld->sceneObjects->getLastSelectionShape();
        CQDlgInertiaTensor theDialog(this);

        C7Vector tr(shape->getMeshWrapper()->getLocalInertiaFrame()); // relative to the shape frame
        tr=shape->getFullCumulativeTransformation()*tr; // relative to the absolute frame!
        tr.X.clear(); // we just wanna reorient the inertia matrix, no shifting!
        C3X3Matrix m(CMeshWrapper::getNewTensor(shape->getMeshWrapper()->getPrincipalMomentsOfInertia(),tr));
        // m is now the absolute inertia tensor centered at the COM
        m.copyToInterface(theDialog.tensor);
        C3Vector com(shape->getMeshWrapper()->getLocalInertiaFrame().X);
        com*=shape->getFullCumulativeTransformation(); // relative to the absolute frame!
        com.copyTo(theDialog.com);
        theDialog.applyToSelection=false;
        theDialog.refresh();
        if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
        {
            m.copyFromInterface(theDialog.tensor);
            C4Vector rot;
            C3Vector pmoment;
            CMeshWrapper::findPrincipalMomentOfInertia(m,rot,pmoment);
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            {
                CShape* a=App::currentWorld->sceneObjects->getShapeFromHandle(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                if (a!=nullptr)
                {
                    if ((a==shape)||theDialog.applyToSelection)
                    {
                        a->getMeshWrapper()->setPrincipalMomentsOfInertia(pmoment);
                        a->getMeshWrapper()->setLocalInertiaFrame(a->getFullCumulativeTransformation().getInverse()*C7Vector(rot,C3Vector(theDialog.com)));
                    }
                }
            }
            App::undoRedo_sceneChanged(""); // **************** UNDO THINGY ****************
        }
        refresh();
    }
}

void CQDlgShapeDyn::on_qqAutomaticToNonStatic_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SETTODYNAMICIFGETSPARENT_SHAPEDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
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
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount()-1;i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
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
        float density=(float)QInputDialog::getDouble(this,"Body density","Uniform density",1000.0,0.1,30000.0,1,&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=COMPUTE_MASSANDINERTIA_SHAPEDYNGUITRIGGEREDCMD;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            cmd.floatParams.push_back(density);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
