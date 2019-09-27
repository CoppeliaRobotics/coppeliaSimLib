
#include "vrepMainHeader.h"
#include "qdlgpathshaping.h"
#include "ui_qdlgpathshaping.h"
#include "gV.h"
#include "tt.h"
#include "qdlgmaterial.h"
#include "app.h"
#include "v_repStrings.h"

bool CQDlgPathShaping::showWindow=false;

CQDlgPathShaping::CQDlgPathShaping(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgPathShaping)
{
    _dlgType=PATH_SHAPING_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgPathShaping::~CQDlgPathShaping()
{
    delete ui;
}

void CQDlgPathShaping::cancelEvent()
{ // no cancel event allowed
    showWindow=false;
    CDlgEx::cancelEvent();
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgPathShaping::refresh()
{
    inMainRefreshRoutine=true;
    //QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();

    int sectionType=0;
    CPath* it=App::ct->objCont->getLastSelection_path();
    bool en=false;
    if (it!=nullptr)
    {
        it=App::ct->objCont->getLastSelection_path();
        en=it->getShapingEnabled();
        sectionType=it->getShapingType();
    }

    ui->qqEnabled->setEnabled(noEditModeNoSim&&(it!=nullptr));

    ui->qqFollowOrientation->setEnabled(noEditModeNoSim&&(it!=nullptr)&&en);
//  ui->qqConvexHull->setEnabled(noEditModeNoSim&&(it!=nullptr)&&en);
    ui->qqAdjustColor->setEnabled(noEditModeNoSim&&(it!=nullptr)&&en);
    ui->qqGenerateShape->setEnabled(noEditModeNoSim&&(it!=nullptr)&&en);
    ui->qqMaxLength->setEnabled(noEditModeNoSim&&(it!=nullptr)&&en);

    ui->qqTypeCombo->setEnabled(noEditModeNoSim&&(it!=nullptr)&&en);
    ui->qqCyclic->setEnabled(noEditModeNoSim&&(it!=nullptr)&&en&&(sectionType==0));
    ui->qqScalingFactor->setEnabled(noEditModeNoSim&&(it!=nullptr)&&en);
    ui->qqCoordinates->setEnabled(noEditModeNoSim&&(it!=nullptr)&&en&&(sectionType==0));

    ui->qqEnabled->setChecked(noEditModeNoSim&&(it!=nullptr)&&it->getShapingEnabled());
    ui->qqFollowOrientation->setChecked((it!=nullptr)&&it->getShapingFollowFullOrientation());
//  ui->qqConvexHull->setChecked((it!=nullptr)&&it->getShapingThroughConvexHull());

    ui->qqTypeCombo->clear();
    ui->qqCyclic->setChecked((it!=nullptr)&&it->getShapingSectionClosed());

    if (it!=nullptr)
    {
        ui->qqMaxLength->setText(tt::getFString(false,it->getShapingElementMaxLength(),3).c_str());
        ui->qqScalingFactor->setText(tt::getFString(false,it->getShapingScaling(),3).c_str());

        ui->qqTypeCombo->addItem(strTranslate(IDS_SHAPING_CIRCLE),QVariant(1));
        ui->qqTypeCombo->addItem(strTranslate(IDS_SHAPING_SQUARE),QVariant(2));
        ui->qqTypeCombo->addItem(strTranslate(IDS_SHAPING_HORIZONTAL_SEGMENT),QVariant(3));
        ui->qqTypeCombo->addItem(strTranslate(IDS_SHAPING_VERTICAL_SEGMENT),QVariant(4));
        ui->qqTypeCombo->addItem(strTranslate(IDS_SHAPING_USER_DEFINED),QVariant(0));
        for (int i=0;i<ui->qqTypeCombo->count();i++)
        {
            if (ui->qqTypeCombo->itemData(i).toInt()==it->getShapingType())
            {
                ui->qqTypeCombo->setCurrentIndex(i);
                break;
            }
        }
        ui->qqCoordinates->clear();
        ui->qqCoordinates->setPlainText("");

        std::string tmp;
        for (int i=0;i<int(it->shapingCoordinates.size())/2;i++)
        {
            tmp+=tt::getFString(true,it->shapingCoordinates[2*i+0],3)+",";
            if (i<int(it->shapingCoordinates.size())/2-1)
            {
                tmp+=tt::getFString(true,it->shapingCoordinates[2*i+1],3)+",";
                tmp+=char(13);
                tmp+=char(10);
            }
            else
                tmp+=tt::getFString(true,it->shapingCoordinates[2*i+1],3);
        }
        ui->qqCoordinates->setPlainText(tmp.c_str());

    }
    else
    {
        ui->qqMaxLength->setText("");
        ui->qqScalingFactor->setText("");
        ui->qqCoordinates->setPlainText("");
        ui->qqCoordinates->clear();
    }

    inMainRefreshRoutine=false;
}

void CQDlgPathShaping::on_qqEnabled_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHAPINGENABLED_PATHSHAPINGGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPathShaping::on_qqFollowOrientation_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_FOLLOWORIENTATION_PATHSHAPINGGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
/*
void CQDlgPathShaping::on_qqConvexHull_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CPath* it=App::ct->objCont->getLastSelection_path();
        if (it!=nullptr)
        {
            if (!it->getShapingThroughConvexHull())
                App::appendSimulationThreadCommand(SHOW_PROGRESSDLGGUITRIGGEREDCMD,-1,-1,0.0,0.0,"Recomputing the path geometry...");
            App::appendSimulationThreadCommand(TOGGLE_CONVEXHULLS_PATHSHAPINGGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(HIDE_PROGRESSDLGGUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
*/
void CQDlgPathShaping::on_qqAdjustColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_PATH_SHAPING,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgPathShaping::on_qqGenerateShape_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(GENERATE_SHAPE_PATHSHAPINGGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPathShaping::on_qqMaxLength_editingFinished()
{
    if (!ui->qqMaxLength->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqMaxLength->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_ELEMENTMAXLENGTH_PATHSHAPINGGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPathShaping::on_qqTypeCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            App::appendSimulationThreadCommand(SET_TYPE_PATHSHAPINGGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),ui->qqTypeCombo->itemData(ui->qqTypeCombo->currentIndex()).toInt());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgPathShaping::on_qqCyclic_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_LASTCOORDLINKSTOFIRST_PATHSHAPINGGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPathShaping::on_qqScalingFactor_editingFinished()
{
    if (!ui->qqScalingFactor->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqScalingFactor->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_SCALINGFACTOR_PATHSHAPINGGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPathShaping::on_qqCoordinates_textChanged()
{
    if (!inMainRefreshRoutine)
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=SET_COORDINATES_PATHSHAPINGGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        std::string str(ui->qqCoordinates->toPlainText().toStdString());
        float tmp;
        while (_extractOneFloat(str,tmp))
            cmd.floatParams.push_back(tmp);
        if (cmd.floatParams.size()&1)
            cmd.floatParams.push_back(0.0f); // no odd number!
        App::appendSimulationThreadCommand(cmd);
        //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        //App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

bool CQDlgPathShaping::_extractOneFloat(std::string& txt,float& val)
{
    if (txt=="")
        return(false);
    std::string nb;
    bool brokenOut=false;
    for (int i=0;i<int(txt.length());i++)
    {
        if (txt[i]!=',')
        {
            if ((txt[i]!=10)&&(txt[i]!=13))
                nb+=txt[i];
        }
        else
        {
            txt.erase(txt.begin(),txt.begin()+i+1);
            brokenOut=true;
            break;
        }
    }
    if (!brokenOut)
        txt="";
    if (!tt::getValidFloat(nb,val))
        val=0.0f;
    return(true);
}
