
#include "vrepMainHeader.h"
#include "qdlgikelements.h"
#include "ui_qdlgikelements.h"
#include "gV.h"
#include "tt.h"
#include <QShortcut>
#include "app.h"
#include "v_repStrings.h"

bool CQDlgIkElements::_invalid=false;
int CQDlgIkElements::_ikGroupHandle=-1;

CQDlgIkElements::CQDlgIkElements(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgIkElements)
{
    _dlgType=IKELEMENT_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
    inListSelectionRoutine=false;
    noListSelectionAllowed=false;
    QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcut,SIGNAL(activated()), this, SLOT(onDeletePressed()));
    QShortcut* shortcut2 = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
    connect(shortcut2,SIGNAL(activated()), this, SLOT(onDeletePressed()));
    _ikGroupHandle=-1;
    if (App::mainWindow!=nullptr)
        App::mainWindow->dlgCont->close(IKELEMENT_DLG);
}

CQDlgIkElements::~CQDlgIkElements()
{
    delete ui;
}

void CQDlgIkElements::cancelEvent()
{ // We just hide the dialog and destroy it at next rendering pass
    _ikGroupHandle=-1;
    CDlgEx::cancelEvent();
}

void CQDlgIkElements::updateObjectsInList()
{
    noListSelectionAllowed=true;
    ui->qqList->clear();
    CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(_ikGroupHandle);
    if (ikGroup!=nullptr)
    {
        for (size_t i=0;i<ikGroup->ikElements.size();i++)
        {
            int tooltipID=ikGroup->ikElements[i]->getTooltip();
            int elementID=ikGroup->ikElements[i]->getObjectID();
            CDummy* theTooltip=App::ct->objCont->getDummy(tooltipID);
            QListWidgetItem* itm=new QListWidgetItem(theTooltip->getObjectName().c_str());
            itm->setData(Qt::UserRole,QVariant(elementID));
            itm->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            ui->qqList->addItem(itm);
        }
    }
    noListSelectionAllowed=false;
}

int CQDlgIkElements::getSelectedObjectID()
{
    QList<QListWidgetItem*> sel=ui->qqList->selectedItems();
    if (sel.size()>0)
        return(sel.at(0)->data(Qt::UserRole).toInt());
    return(-1);
}

void CQDlgIkElements::selectObjectInList(int objectID)
{
    noListSelectionAllowed=true;
    for (int i=0;i<ui->qqList->count();i++)
    {
        QListWidgetItem* it=ui->qqList->item(i);
        if (it!=nullptr)
        {
            if (it->data(Qt::UserRole).toInt()==objectID)
            {
                it->setSelected(true);
                break;
            }
        }
    }
    noListSelectionAllowed=false;
}

bool CQDlgIkElements::needsDestruction()
{
    if (!isLinkedDataValid())
        return(true);
    return(CDlgEx::needsDestruction());
}

void CQDlgIkElements::_initialize(int ikGroupHandle)
{
    _ikGroupHandle=ikGroupHandle;
    CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(_ikGroupHandle);
    if (ikGroup!=nullptr)
    {
        std::string titleText("IK Group (");
        titleText+=ikGroup->getObjectName();
        titleText+=")";
        setWindowTitle(titleText.c_str());
    }
    _invalid=false;
    refresh();
}

bool CQDlgIkElements::isLinkedDataValid()
{
    if (!App::ct->simulation->isSimulationStopped())
        return(false);
    if (App::getEditModeType()!=NO_EDIT_MODE)
        return(false);
    if (App::ct->ikGroups->getIkGroup(_ikGroupHandle)!=nullptr)
        return(!_invalid);
    return(false);
}

void CQDlgIkElements::display(int ikGroupHandle,QWidget* theParentWindow)
{
    if (App::mainWindow==nullptr)
        return;
    App::mainWindow->dlgCont->close(IKELEMENT_DLG);
    if (App::mainWindow->dlgCont->openOrBringToFront(IKELEMENT_DLG))
    {
        CQDlgIkElements* dlg=(CQDlgIkElements*)App::mainWindow->dlgCont->getDialog(IKELEMENT_DLG);
        if (dlg!=nullptr)
            dlg->_initialize(ikGroupHandle);
    }
}

bool CQDlgIkElements::doesInstanceSwitchRequireDestruction()
{
    return(true);
}

void CQDlgIkElements::dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    if ( (cmdIn!=nullptr)&&(cmdIn->intParams[0]==_dlgType) )
    {
        if (cmdIn->intParams[1]==0)
        {
            selectObjectInList(cmdIn->intParams[2]);
            refresh();
        }
    }
}

void CQDlgIkElements::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();
    if (!isLinkedDataValid())
        return;
    CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(_ikGroupHandle);
    int elementID=getSelectedObjectID();
    CikEl* it=ikGroup->getIkElement(elementID);

    if (!inListSelectionRoutine)
    {
        updateObjectsInList();
        selectObjectInList(elementID);
    }

    ui->qqActive->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqBaseCombo->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqX->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqY->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqZ->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqAlphaBeta->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqGamma->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqRelativeCombo->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqPrecisionLinear->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqPrecisionAngular->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqWeightLinear->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqWeightAngular->setEnabled((it!=nullptr)&&noEditModeNoSim);

    ui->qqActive->setChecked((it!=nullptr)&&it->getActive());
    ui->qqBaseCombo->clear();
    ui->qqX->setChecked((it!=nullptr)&&(it->getConstraints()&sim_ik_x_constraint));
    ui->qqY->setChecked((it!=nullptr)&&(it->getConstraints()&sim_ik_y_constraint));
    ui->qqZ->setChecked((it!=nullptr)&&(it->getConstraints()&sim_ik_z_constraint));
    ui->qqAlphaBeta->setChecked((it!=nullptr)&&(it->getConstraints()&sim_ik_alpha_beta_constraint));
    ui->qqGamma->setChecked((it!=nullptr)&&(it->getConstraints()&sim_ik_gamma_constraint));
    ui->qqRelativeCombo->clear();

    std::vector<std::string> names;
    std::vector<int> ids;

    ui->qqTipCombo->clear();
    for (size_t i=0;i<App::ct->objCont->dummyList.size();i++)
    {
        CDummy* it2=App::ct->objCont->getDummy(App::ct->objCont->dummyList[i]);
        names.push_back(it2->getObjectName());
        ids.push_back(it2->getObjectHandle());
    }
    tt::orderStrings(names,ids);
    for (size_t i=0;i<names.size();i++)
        ui->qqTipCombo->addItem(names[i].c_str(),QVariant(ids[i]));

    if (it!=nullptr)
    {
        CDummy* tip=App::ct->objCont->getDummy(it->getTooltip());
        if (tip==nullptr)
            ui->qqTargetString->setText("");
        else
        {
            CDummy* target=App::ct->objCont->getDummy(tip->getLinkedDummyID());
            if (target==nullptr)
                ui->qqTargetString->setText(strTranslate(IDS_WARNING_TIP_DUMMY_NOT_LINKED));
            else
            {
                if (tip->getLinkType()!=sim_dummy_linktype_ik_tip_target)
                    ui->qqTargetString->setText(strTranslate(IDS_WARNING_WRONG_DUMMY_LINK_TYPE));
                else
                    ui->qqTargetString->setText(target->getObjectName().c_str());
            }
        }

        CDummy* tooltip=App::ct->objCont->getDummy(it->getTooltip());
        ui->qqBaseCombo->addItem(strTranslate(IDSN_WORLD),QVariant(-1));
        names.clear();
        ids.clear();
        for (size_t i=0;i<App::ct->objCont->objectList.size();i++)
        {
            C3DObject* it2=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
            if (tooltip->isObjectParentedWith(it2))
            {
                names.push_back(it2->getObjectName());
                ids.push_back(it2->getObjectHandle());
            }
        }
        tt::orderStrings(names,ids);
        for (size_t i=0;i<names.size();i++)
            ui->qqBaseCombo->addItem(names[i].c_str(),QVariant(ids[i]));
        for (int i=0;i<ui->qqBaseCombo->count();i++)
        {
            if (ui->qqBaseCombo->itemData(i).toInt()==it->getBase())
            {
                ui->qqBaseCombo->setCurrentIndex(i);
                break;
            }
        }

        names.clear();
        ids.clear();
        ui->qqRelativeCombo->addItem(strTranslate(IDSN_SAME_AS_BASE),QVariant(-1));
        for (size_t i=0;i<App::ct->objCont->objectList.size();i++)
        {
            C3DObject* it2=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
            if ( (it2->getObjectType()==sim_object_dummy_type)&&(it2!=tooltip) )
            {
                names.push_back(it2->getObjectName());
                ids.push_back(it2->getObjectHandle());
            }
        }
        tt::orderStrings(names,ids);
        for (size_t i=0;i<names.size();i++)
            ui->qqRelativeCombo->addItem(names[i].c_str(),QVariant(ids[i]));
        for (int i=0;i<ui->qqRelativeCombo->count();i++)
        {
            if (ui->qqRelativeCombo->itemData(i).toInt()==it->getAlternativeBaseForConstraints())
            {
                ui->qqRelativeCombo->setCurrentIndex(i);
                break;
            }
        }

        ui->qqPrecisionLinear->setText(tt::getEString(false,it->getMinLinearPrecision(),2).c_str());
        ui->qqPrecisionAngular->setText(tt::getAngleEString(false,it->getMinAngularPrecision(),2).c_str());
        ui->qqWeightLinear->setText(tt::getFString(false,it->getPositionWeight(),2).c_str());
        ui->qqWeightAngular->setText(tt::getFString(false,it->getOrientationWeight(),2).c_str());
    }
    else
    {
        ui->qqTargetString->setText("");
        ui->qqPrecisionLinear->setText("");
        ui->qqPrecisionAngular->setText("");
        ui->qqWeightLinear->setText("");
        ui->qqWeightAngular->setText("");
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgIkElements::onDeletePressed()
{
    if (focusWidget()==ui->qqList)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            App::appendSimulationThreadCommand(REMOVE_ELEMENT_IKELEMENTGUITRIGGEREDCMD,_ikGroupHandle,getSelectedObjectID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgIkElements::on_qqAddNewElement_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(ADD_ELEMENT_IKELEMENTGUITRIGGEREDCMD,_ikGroupHandle,ui->qqTipCombo->itemData(ui->qqTipCombo->currentIndex()).toInt());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIkElements::on_qqList_itemSelectionChanged()
{
    if (!noListSelectionAllowed)
    {
        inListSelectionRoutine=true;
        refresh();
        inListSelectionRoutine=false;
    }
}

void CQDlgIkElements::on_qqActive_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_ACTIVE_IKELEMENTGUITRIGGEREDCMD,_ikGroupHandle,getSelectedObjectID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIkElements::on_qqBaseCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_BASE_IKELEMENTGUITRIGGEREDCMD;
            cmd.intParams.push_back(_ikGroupHandle);
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intParams.push_back(ui->qqBaseCombo->itemData(ui->qqBaseCombo->currentIndex()).toInt());
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgIkElements::on_qqX_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=TOGGLE_CONSTRAINT_IKELEMENTGUITRIGGEREDCMD;
        cmd.intParams.push_back(_ikGroupHandle);
        cmd.intParams.push_back(getSelectedObjectID());
        cmd.intParams.push_back(sim_ik_x_constraint);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIkElements::on_qqY_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=TOGGLE_CONSTRAINT_IKELEMENTGUITRIGGEREDCMD;
        cmd.intParams.push_back(_ikGroupHandle);
        cmd.intParams.push_back(getSelectedObjectID());
        cmd.intParams.push_back(sim_ik_y_constraint);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIkElements::on_qqZ_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=TOGGLE_CONSTRAINT_IKELEMENTGUITRIGGEREDCMD;
        cmd.intParams.push_back(_ikGroupHandle);
        cmd.intParams.push_back(getSelectedObjectID());
        cmd.intParams.push_back(sim_ik_z_constraint);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIkElements::on_qqAlphaBeta_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=TOGGLE_CONSTRAINT_IKELEMENTGUITRIGGEREDCMD;
        cmd.intParams.push_back(_ikGroupHandle);
        cmd.intParams.push_back(getSelectedObjectID());
        cmd.intParams.push_back(sim_ik_alpha_beta_constraint);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIkElements::on_qqGamma_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=TOGGLE_CONSTRAINT_IKELEMENTGUITRIGGEREDCMD;
        cmd.intParams.push_back(_ikGroupHandle);
        cmd.intParams.push_back(getSelectedObjectID());
        cmd.intParams.push_back(sim_ik_gamma_constraint);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIkElements::on_qqRelativeCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_REFERENCEFRAME_IKELEMENTGUITRIGGEREDCMD;
            cmd.intParams.push_back(_ikGroupHandle);
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intParams.push_back(ui->qqRelativeCombo->itemData(ui->qqRelativeCombo->currentIndex()).toInt());
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgIkElements::on_qqPrecisionLinear_editingFinished()
{
    if (!ui->qqPrecisionLinear->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqPrecisionLinear->text().toFloat(&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_PRECISION_IKELEMENTGUITRIGGEREDCMD;
            cmd.intParams.push_back(_ikGroupHandle);
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intParams.push_back(0); // linear
            cmd.floatParams.push_back(newVal);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIkElements::on_qqPrecisionAngular_editingFinished()
{
    if (!ui->qqPrecisionAngular->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqPrecisionAngular->text().toFloat(&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_PRECISION_IKELEMENTGUITRIGGEREDCMD;
            cmd.intParams.push_back(_ikGroupHandle);
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intParams.push_back(1); // angular
            cmd.floatParams.push_back(newVal*gv::userToRad);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIkElements::on_qqWeightLinear_editingFinished()
{
    if (!ui->qqWeightLinear->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqWeightLinear->text().toFloat(&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_WEIGHT_IKELEMENTGUITRIGGEREDCMD;
            cmd.intParams.push_back(_ikGroupHandle);
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intParams.push_back(0); // linear
            cmd.floatParams.push_back(newVal);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIkElements::on_qqWeightAngular_editingFinished()
{
    if (!ui->qqWeightAngular->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqWeightAngular->text().toFloat(&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_WEIGHT_IKELEMENTGUITRIGGEREDCMD;
            cmd.intParams.push_back(_ikGroupHandle);
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intParams.push_back(1); // angular
            cmd.floatParams.push_back(newVal);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
