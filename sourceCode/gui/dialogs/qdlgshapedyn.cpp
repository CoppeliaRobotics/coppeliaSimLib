#include <qdlgshapedyn.h>
#include <ui_qdlgshapedyn.h>
#include <tt.h>
#include <utils.h>
#include <simStrings.h>
#include <app.h>
#include <QInputDialog>
#include <vMessageBox.h>
#include <guiApp.h>

bool CQDlgShapeDyn::showDynamicWindow=false;
bool CQDlgShapeDyn::masslessInertia=true;

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
    GuiApp::setFullDialogRefreshFlag();
}

void CQDlgShapeDyn::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeAndNoSim=(GuiApp::getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped();
    bool sel=App::currentWorld->sceneObjects->isLastSelectionAShape();
    int sc=(int)App::currentWorld->sceneObjects->getShapeCountInSelection();
    bool notHeightfield=true;
    bool lastSelIsNotStatic=false;
    bool lastSelIsConvex=false;
    CShape* it=App::currentWorld->sceneObjects->getLastSelectionShape();
    if (sel)
    {
        notHeightfield=(it->getMesh()->getPurePrimitiveType()!=sim_primitiveshape_heightfield);
        lastSelIsNotStatic=!it->getStatic();
        lastSelIsConvex=it->getMesh()->isConvex();
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
    ui->qqMass->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqI00->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqI01->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqI02->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqI10->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqI11->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqI12->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqI20->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqI21->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqI22->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqMassless->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqPX->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqPY->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqPZ->setEnabled(sel&&lastSelIsNotStatic&&noEditModeAndNoSim);
    ui->qqApplyMassAndInertiaProperties->setEnabled((sc>1)&&sel&&noEditModeAndNoSim);

    ui->qqDynamic->setChecked(sel&&lastSelIsNotStatic);
    ui->qqAutomaticToNonStatic->setChecked(sel&&it->getSetAutomaticallyToNonStaticIfGetsParent());
    ui->qqMassless->setChecked(sel&&masslessInertia);
    ui->qqSleepModeStart->setChecked(sel&&it->getStartInDynamicSleeping());

    if (masslessInertia)
        ui->qqInertiaMatrixTitle->setTitle("Inertia matrix [m^2]");
    else
        ui->qqInertiaMatrixTitle->setTitle("Inertia matrix [kg*m^2]");
    if (sel)
    {
        double mass=it->getMesh()->getMass();
        ui->qqMass->setText(utils::getMassString(mass).c_str());
        C3X3Matrix m(it->getMesh()->getInertia());
        if (!masslessInertia)
            m*=mass;
        ui->qqI00->setText(utils::getTensorString(masslessInertia,m(0,0)).c_str());
        ui->qqI01->setText(utils::getTensorString(masslessInertia,m(0,1)).c_str());
        ui->qqI02->setText(utils::getTensorString(masslessInertia,m(0,2)).c_str());
        ui->qqI10->setText(utils::getTensorString(masslessInertia,m(1,0)).c_str());
        ui->qqI11->setText(utils::getTensorString(masslessInertia,m(1,1)).c_str());
        ui->qqI12->setText(utils::getTensorString(masslessInertia,m(1,2)).c_str());
        ui->qqI20->setText(utils::getTensorString(masslessInertia,m(2,0)).c_str());
        ui->qqI21->setText(utils::getTensorString(masslessInertia,m(2,1)).c_str());
        ui->qqI22->setText(utils::getTensorString(masslessInertia,m(2,2)).c_str());
        C3Vector com(it->getMesh()->getCOM());
        ui->qqPX->setText(utils::getPosString(true,com(0)).c_str());
        ui->qqPY->setText(utils::getPosString(true,com(1)).c_str());
        ui->qqPZ->setText(utils::getPosString(true,com(2)).c_str());
        ui->qqInfo->setText(it->getMesh()->getInertiaErrorString().c_str());
        ui->qqInfo->setStyleSheet("QLabel { color : red; }");
    }
    else
    {
        ui->qqInfo->setText("");
        ui->qqMass->setText("");
        ui->qqI00->setText("");
        ui->qqI01->setText("");
        ui->qqI02->setText("");
        ui->qqI10->setText("");
        ui->qqI11->setText("");
        ui->qqI12->setText("");
        ui->qqI20->setText("");
        ui->qqI21->setText("");
        ui->qqI22->setText("");
        ui->qqPX->setText("");
        ui->qqPY->setText("");
        ui->qqPZ->setText("");
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
            if ((!it->getRespondable())&&(!it->getMesh()->isPure())&&(!it->getMesh()->isConvex()))
                GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow,"Shape",IDS_MAKING_NON_PURE_CONCAVE_SHAPE_RESPONDABLE_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
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
        double newVal=GuiApp::getEvalDouble(ui->qqMass->text().toStdString().c_str(), &ok);
        CShape* shape=App::currentWorld->sceneObjects->getLastSelectionShape();
        if (ok&&(shape!=nullptr))
        {
            App::appendSimulationThreadCommand(SET_MASS_SHAPEDYNGUITRIGGEREDCMD,shape->getObjectHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapeDyn::on_qqI00_editingFinished()
{
    _inertiaChanged(0,0,ui->qqI00);
}

void CQDlgShapeDyn::on_qqI01_editingFinished()
{
    _inertiaChanged(0,1,ui->qqI01);
}

void CQDlgShapeDyn::on_qqI02_editingFinished()
{
    _inertiaChanged(0,2,ui->qqI02);
}

void CQDlgShapeDyn::on_qqI10_editingFinished()
{
    _inertiaChanged(1,0,ui->qqI10);
}

void CQDlgShapeDyn::on_qqI11_editingFinished()
{
    _inertiaChanged(1,1,ui->qqI11);
}

void CQDlgShapeDyn::on_qqI12_editingFinished()
{
    _inertiaChanged(1,2,ui->qqI12);
}

void CQDlgShapeDyn::on_qqI20_editingFinished()
{
    _inertiaChanged(2,0,ui->qqI20);
}

void CQDlgShapeDyn::on_qqI21_editingFinished()
{
    _inertiaChanged(2,1,ui->qqI21);
}

void CQDlgShapeDyn::on_qqI22_editingFinished()
{
    _inertiaChanged(2,2,ui->qqI22);
}

void CQDlgShapeDyn::_inertiaChanged(size_t row,size_t col,QLineEdit* ct)
{
    if (!ct->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CShape* shape=App::currentWorld->sceneObjects->getLastSelectionShape();
        bool ok;
        double newVal=GuiApp::getEvalDouble(ct->text().toStdString().c_str(), &ok);
        if (ok&&(shape!=nullptr))
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_INERTIAMATRIX_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(shape->getObjectHandle());
            cmd.intParams.push_back(row);
            cmd.intParams.push_back(col);
            cmd.doubleParams.push_back(newVal);
            cmd.boolParams.push_back(masslessInertia);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
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
        double newVal=GuiApp::getEvalDouble(ui->qqPX->text().toStdString().c_str(), &ok);
        if (ok&&(shape!=nullptr))
        {
            C3Vector com(shape->getMesh()->getCOM());
            com(0)=newVal;
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_COMMATRIX_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(shape->getObjectHandle());
            cmd.posParams.push_back(com);
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
        double newVal=GuiApp::getEvalDouble(ui->qqPY->text().toStdString().c_str(), &ok);
        if (ok&&(shape!=nullptr))
        {
            C3Vector com(shape->getMesh()->getCOM());
            com(1)=newVal;
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_COMMATRIX_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(shape->getObjectHandle());
            cmd.posParams.push_back(com);
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
        double newVal=GuiApp::getEvalDouble(ui->qqPZ->text().toStdString().c_str(), &ok);
        if (ok&&(shape!=nullptr))
        {
            C3Vector com(shape->getMesh()->getCOM());
            com(2)=newVal;
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_COMMATRIX_SHAPEDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(shape->getObjectHandle());
            cmd.posParams.push_back(com);
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

void CQDlgShapeDyn::on_qqMassless_clicked()
{
    masslessInertia=!masslessInertia;
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
