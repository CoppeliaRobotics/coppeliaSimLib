
#include "vrepMainHeader.h"
#include "qdlgdependencyequation.h"
#include "ui_qdlgdependencyequation.h"
#include "gV.h"
#include "tt.h"
#include "app.h"
#include "v_repStrings.h"

CQDlgDependencyEquation::CQDlgDependencyEquation(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgDependencyEquation)
{
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgDependencyEquation::~CQDlgDependencyEquation()
{
    delete ui;
}

void CQDlgDependencyEquation::cancelEvent()
{ // no cancel event allowed
    //defaultModalDialogEndRoutine(false);
}

void CQDlgDependencyEquation::okEvent()
{ // no ok event allowed
    //defaultModalDialogEndRoutine(false);
}

void CQDlgDependencyEquation::refresh()
{ // dlg is modal
    inMainRefreshRoutine=true;
    bool dependencyPartActive=false;
    CJoint* it=App::ct->objCont->getLastSelection_joint();
    if (it!=nullptr)
        dependencyPartActive=((it->getJointMode()==sim_jointmode_dependent)||(it->getJointMode()==sim_jointmode_reserved_previously_ikdependent));

    ui->qqOffset->setEnabled(dependencyPartActive&&(it->getDependencyJointID()!=-1));
    ui->qqCoeff->setEnabled(dependencyPartActive&&(it->getDependencyJointID()!=-1));
    ui->qqCombo->setEnabled(dependencyPartActive);
    ui->qqCombo->clear();

    if (it!=nullptr)
    {
        ui->qqOffset->setText(tt::getEString(true,it->getDependencyJointFact(),3).c_str());
        ui->qqCoeff->setText(tt::getEString(true,it->getDependencyJointCoeff(),3).c_str());

        ui->qqCombo->addItem(strTranslate(IDSN_NONE),QVariant(-1));

        std::vector<std::string> names;
        std::vector<int> ids;

        // Joints:
        for (size_t i=0;i<App::ct->objCont->jointList.size();i++)
        {
            CJoint* it2=App::ct->objCont->getJoint(App::ct->objCont->jointList[i]);
            if ((it2!=it)&&(it2->getJointType()!=sim_joint_spherical_subtype))
            {
                std::string name(tt::decorateString("[",strTranslate(IDSN_JOINT),"] "));
                name+=it2->getObjectName();
                names.push_back(name);
                ids.push_back(it2->getObjectHandle());
            }
        }
        tt::orderStrings(names,ids);
        for (size_t i=0;i<names.size();i++)
            ui->qqCombo->addItem(names[i].c_str(),QVariant(ids[i]));

        // Select current item:
        for (int i=0;i<ui->qqCombo->count();i++)
        {
            if (ui->qqCombo->itemData(i).toInt()==it->getDependencyJointID())
            {
                ui->qqCombo->setCurrentIndex(i);
                break;
            }
        }
    }
    else
    {
        ui->qqOffset->setText("");
        ui->qqCoeff->setText("");
    }

    inMainRefreshRoutine=false;
}

void CQDlgDependencyEquation::on_qqClose_clicked(QAbstractButton *button)
{
    App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    defaultModalDialogEndRoutine(true);
}

void CQDlgDependencyEquation::on_qqOffset_editingFinished()
{
    if (!ui->qqOffset->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        bool ok;
        float newVal=ui->qqOffset->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            it->setDependencyJointOffset(newVal); // we also modify the ui resources (dlg is modal)
            App::appendSimulationThreadCommand(SET_OFFFSET_JOINTDEPENDENCYGUITRIGGEREDCMD,it->getObjectHandle(),-1,newVal);
            // scene change announcement at the end of this modal dlg
        }
        refresh();
    }
}

void CQDlgDependencyEquation::on_qqCoeff_editingFinished()
{
    if (!ui->qqCoeff->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        bool ok;
        float newVal=ui->qqCoeff->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            it->setDependencyJointCoeff(newVal); // we also modify the ui resources (dlg is modal)
            App::appendSimulationThreadCommand(SET_MULTFACT_JOINTDEPENDENCYGUITRIGGEREDCMD,it->getObjectHandle(),-1,newVal);
            // scene change announcement at the end of this modal dlg
        }
        refresh();
    }
}

void CQDlgDependencyEquation::on_qqCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            CJoint* it=App::ct->objCont->getLastSelection_joint();
            int objID=ui->qqCombo->itemData(ui->qqCombo->currentIndex()).toInt();
            if (it!=nullptr)
            {
                it->setDependencyJointID(objID); // we also modify the ui resources (dlg is modal)
                App::appendSimulationThreadCommand(SET_OTHERJOINT_JOINTDEPENDENCYGUITRIGGEREDCMD,it->getObjectHandle(),objID);
                // scene change announcement at the end of this modal dlg
            }
            refresh();
        }
    }
}
