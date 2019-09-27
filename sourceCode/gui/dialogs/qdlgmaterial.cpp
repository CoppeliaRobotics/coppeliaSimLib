
#include "vrepMainHeader.h"
#include "qdlgmaterial.h"
#include "ui_qdlgmaterial.h"
#include "gV.h"
#include "tt.h"
#include "app.h"
#include "qdlgcolorpulsation.h"

CQDlgMaterial::CQDlgMaterial(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgMaterial)
{
    _dlgType=MATERIAL_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
    _lastSelectedObjectID=App::ct->objCont->getLastSelectionID();
    _objectSelectionSize=App::ct->objCont->getSelSize();

    if (App::mainWindow!=nullptr)
        App::mainWindow->dlgCont->close(COLOR_DLG);
}

CQDlgMaterial::~CQDlgMaterial()
{
    delete ui;
    if (App::mainWindow!=nullptr)
        App::mainWindow->dlgCont->close(COLOR_DLG);
}

void CQDlgMaterial::refresh()
{
    inMainRefreshRoutine=true;
    int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
    CVisualParam* vc=App::getVisualParamPointerFromItem(_objType,_objID1,_objID2,nullptr,&allowedParts);
    bool simStopped=App::ct->simulation->isSimulationStopped();
    ui->qqAmbientAdjust->setEnabled(simStopped&&(allowedParts&1));
    ui->qqSpecularAdjust->setEnabled(simStopped&&(allowedParts&4));
    ui->qqEmissiveAdjust->setEnabled(simStopped&&(allowedParts&8));
    ui->qqAuxiliaryAdjust->setEnabled(simStopped&&(allowedParts&16));
    ui->qqPulsationAdjust->setEnabled(simStopped&&(allowedParts&32));
    ui->qqShininess->setEnabled(simStopped&&(allowedParts&64));
    ui->qqOpacityEnable->setEnabled(simStopped&&(allowedParts&128));
    ui->qqColorName->setEnabled(simStopped&&(allowedParts&256));
    ui->qqExtensionString->setEnabled(simStopped&&(allowedParts&512));

    if (allowedParts&64)
        ui->qqShininess->setText(tt::getIString(false,vc->shininess).c_str());
    else
        ui->qqShininess->setText("");
    if (allowedParts&128)
    {
        ui->qqOpacityEnable->setChecked(vc->translucid);
        ui->qqOpacity->setEnabled(vc->translucid);
        ui->qqOpacity->setText(tt::getFString(false,vc->transparencyFactor,2).c_str());
    }
    else
    {
        ui->qqOpacityEnable->setChecked(0);
        ui->qqOpacity->setEnabled(false);
        ui->qqOpacity->setText("");
    }

    if (allowedParts&256)
        ui->qqColorName->setText(vc->colorName.c_str());
    else
        ui->qqColorName->setText("");

    if (allowedParts&512)
        ui->qqExtensionString->setText(vc->extensionString.c_str());
    else
        ui->qqExtensionString->setText("");

    inMainRefreshRoutine=false;
}

bool CQDlgMaterial::needsDestruction()
{
    if (!isLinkedDataValid())
        return(true);
    return(CDlgEx::needsDestruction());
}

bool CQDlgMaterial::isLinkedDataValid()
{
    if (_lastSelectedObjectID!=App::ct->objCont->getLastSelectionID())
        return(false);
    if (_objectSelectionSize!=App::ct->objCont->getSelSize())
        return(false);
    if (!App::ct->simulation->isSimulationStopped())
        return(false);
    return(App::getVisualParamPointerFromItem(_objType,_objID1,_objID2,nullptr,nullptr)!=nullptr);
}

void CQDlgMaterial::displayMaterialDlg(int objType,int objID1,int objID2,QWidget* theParentWindow)
{
    if (App::mainWindow==nullptr)
        return;
    App::mainWindow->dlgCont->close(MATERIAL_DLG);
    App::mainWindow->dlgCont->close(COLOR_DLG);
    if (App::mainWindow->dlgCont->openOrBringToFront(MATERIAL_DLG))
    {
        CQDlgMaterial* mat=(CQDlgMaterial*)App::mainWindow->dlgCont->getDialog(MATERIAL_DLG);
        if (mat!=nullptr)
            mat->_initializeDlg(objType,objID1,objID2);
    }
}

void CQDlgMaterial::displayMaterialDlgModal(int objType,int objID1,int objID2,QWidget* theParentWindow)
{
    if (App::mainWindow==nullptr)
        return;
    App::mainWindow->dlgCont->close(MATERIAL_DLG);
    App::mainWindow->dlgCont->close(COLOR_DLG);

    CQDlgMaterial it(theParentWindow);
    it._initializeDlg(objType,objID1,objID2);
    it.makeDialogModal();
}

void CQDlgMaterial::_initializeDlg(int objType,int objID1,int objID2)
{
    _objType=objType;
    _objID1=objID1;
    _objID2=objID2;
    std::string str;
    App::getRGBPointerFromItem(_objType,_objID1,_objID2,-1,&str);
    setWindowTitle(str.c_str());
    refresh();
}

void CQDlgMaterial::_adjustCol(int colComponent)
{
    if (!isModal())
        CQDlgColor::displayDlg(_objType,_objID1,_objID2,colComponent,App::mainWindow,true,false,true);
    else
    {
        CQDlgColor::displayDlgModal(_objType,_objID1,_objID2,colComponent,this,true,false,true);
        float* col=App::getRGBPointerFromItem(_objType,_objID1,_objID2,colComponent,nullptr);
        if (col!=nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ITEMRGB_COLORGUITRIGGEREDCMD;
            cmd.intParams.push_back(_objType);
            cmd.intParams.push_back(_objID1);
            cmd.intParams.push_back(_objID2);
            cmd.intParams.push_back(colComponent);
            cmd.floatParams.push_back(col[0]);
            cmd.floatParams.push_back(col[1]);
            cmd.floatParams.push_back(col[2]);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgMaterial::cancelEvent()
{ // We just hide the dialog and destroy it at next rendering pass
    if (((QDialog*)this)->isModal()) // this condition and next line on 31/3/2013: on Linux the dlg couldn't be closed!
        defaultModalDialogEndRoutine(false);
    else
        CDlgEx::cancelEvent();
}

bool CQDlgMaterial::doesInstanceSwitchRequireDestruction()
{
    return(true);
}

void CQDlgMaterial::on_qqAmbientAdjust_clicked()
{
    _adjustCol(sim_colorcomponent_ambient_diffuse);
}

void CQDlgMaterial::on_qqSpecularAdjust_clicked()
{
    _adjustCol(sim_colorcomponent_specular);
}

void CQDlgMaterial::on_qqEmissiveAdjust_clicked()
{
    _adjustCol(sim_colorcomponent_emission);
}

void CQDlgMaterial::on_qqPulsationAdjust_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (isLinkedDataValid())
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            CVisualParam* it=App::getVisualParamPointerFromItem(_objType,_objID1,_objID2,nullptr,&allowedParts);
            if (allowedParts&32)
            {
                CQDlgColorPulsation theDialog(this);
                theDialog.pulsationEnabled=it->flash;
                theDialog.pulsationRealTime=!it->useSimulationTime;
                theDialog.pulsationFrequency=it->flashFrequency;
                theDialog.pulsationPhase=it->flashPhase;
                theDialog.pulsationRatio=it->flashRatio;
                theDialog.refresh();
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    SSimulationThreadCommand cmd;
                    cmd.cmdId=SET_PULSATIONPARAMS_MATERIALGUITRIGGEREDCMD;
                    cmd.intParams.push_back(_objType);
                    cmd.intParams.push_back(_objID1);
                    cmd.intParams.push_back(_objID2);
                    cmd.boolParams.push_back(theDialog.pulsationEnabled);
                    cmd.boolParams.push_back(!theDialog.pulsationRealTime);
                    cmd.floatParams.push_back(theDialog.pulsationFrequency);
                    cmd.floatParams.push_back(theDialog.pulsationPhase);
                    cmd.floatParams.push_back(theDialog.pulsationRatio);
                    App::appendSimulationThreadCommand(cmd);
                    App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                }
                App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            }
        }
    }
}

void CQDlgMaterial::on_qqShininess_editingFinished()
{
    if (!ui->qqShininess->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (isLinkedDataValid())
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            App::getVisualParamPointerFromItem(_objType,_objID1,_objID2,nullptr,&allowedParts);
            bool ok;
            int newVal=ui->qqShininess->text().toInt(&ok);
            if (ok&&(allowedParts&64))
            {
                int s=tt::getLimitedInt(0,128,newVal);
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_SHININESS_MATERIALGUITRIGGEREDCMD;
                cmd.intParams.push_back(_objType);
                cmd.intParams.push_back(_objID1);
                cmd.intParams.push_back(_objID2);
                cmd.intParams.push_back(s);
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgMaterial::on_qqOpacityEnable_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (isLinkedDataValid())
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            App::getVisualParamPointerFromItem(_objType,_objID1,_objID2,nullptr,&allowedParts);
            if (allowedParts&128)
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=TOGGLE_TRANSPARENCY_MATERIALGUITRIGGEREDCMD;
                cmd.intParams.push_back(_objType);
                cmd.intParams.push_back(_objID1);
                cmd.intParams.push_back(_objID2);
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgMaterial::on_qqOpacity_editingFinished()
{
    if (!ui->qqOpacity->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (isLinkedDataValid())
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            App::getVisualParamPointerFromItem(_objType,_objID1,_objID2,nullptr,&allowedParts);
            bool ok;
            float newVal=ui->qqOpacity->text().toFloat(&ok);
            if (ok&&(allowedParts&128))
            {
                float s=tt::getLimitedFloat(0.0f,1.0f,newVal);
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_TRANSPARENCYFACT_MATERIALGUITRIGGEREDCMD;
                cmd.intParams.push_back(_objType);
                cmd.intParams.push_back(_objID1);
                cmd.intParams.push_back(_objID2);
                cmd.floatParams.push_back(s);
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgMaterial::on_qqColorName_editingFinished()
{
    if (!ui->qqColorName->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (isLinkedDataValid())
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            App::getVisualParamPointerFromItem(_objType,_objID1,_objID2,nullptr,&allowedParts);
            if (allowedParts&256)
            {
                std::string nm(ui->qqColorName->text().toStdString());
                tt::removeIllegalCharacters(nm,false);
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_NAME_MATERIALGUITRIGGEREDCMD;
                cmd.intParams.push_back(_objType);
                cmd.intParams.push_back(_objID1);
                cmd.intParams.push_back(_objID2);
                cmd.stringParams.push_back(nm.c_str());
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}


void CQDlgMaterial::on_qqAuxiliaryAdjust_clicked()
{
    _adjustCol(sim_colorcomponent_auxiliary);
}

void CQDlgMaterial::on_qqExtensionString_editingFinished()
{
    if (!ui->qqExtensionString->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (isLinkedDataValid())
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            App::getVisualParamPointerFromItem(_objType,_objID1,_objID2,nullptr,&allowedParts);
            if (allowedParts&512)
            {
                std::string nm(ui->qqExtensionString->text().toStdString());
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_EXTSTRING_MATERIALGUITRIGGEREDCMD;
                cmd.intParams.push_back(_objType);
                cmd.intParams.push_back(_objID1);
                cmd.intParams.push_back(_objID2);
                cmd.stringParams.push_back(nm.c_str());
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}
