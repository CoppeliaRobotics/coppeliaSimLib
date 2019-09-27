
#include "vrepMainHeader.h"
#include "qdlgtextures.h"
#include "ui_qdlgtextures.h"
#include "tt.h"
#include "gV.h"
#include "oGL.h"
#include "imgLoaderSaver.h"
#include "geometric.h"
#include "qdlgtextureselection.h"
#include "qdlgtextureloadoptions.h"
#include "v_repStrings.h"
#include <boost/lexical_cast.hpp>
#include "vFileDialog.h"
#include "app.h"
#include "vMessageBox.h"

CQDlgTextures::CQDlgTextures(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgTextures)
{
    _dlgType=TEXTURE_DLG;
    ui->setupUi(this);
    if (App::mainWindow!=nullptr)
        App::mainWindow->dlgCont->close(TEXTURE_DLG);
    inMainRefreshRoutine=false;
}

CQDlgTextures::~CQDlgTextures()
{
    delete ui;
}

void CQDlgTextures::refresh()
{
    if (!isLinkedDataValid())
        return;
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool applyTexture3D;
    CGeometric* geom=nullptr;
    CTextureProperty* tp=App::getTexturePropertyPointerFromItem(_objType,_objID1,_objID2,nullptr,&applyTexture3D,nullptr,&geom);
    bool simStopped=App::ct->simulation->isSimulationStopped();
    bool usingFixedTextureCoordinates=false;
    bool foundTextureCoordinatesOnShape=false;
    bool forbidU=false;
    bool forbidV=false;

    if (applyTexture3D)
    {
        if (tp!=nullptr)
        {
            usingFixedTextureCoordinates=tp->getFixedCoordinates();
            forbidU=(tp->getTextureMapMode()==sim_texturemap_cylinder)||(tp->getTextureMapMode()==sim_texturemap_sphere);
            forbidV=(tp->getTextureMapMode()==sim_texturemap_sphere);
        }
        else
            foundTextureCoordinatesOnShape=(geom->textureCoords_notCopiedNorSerialized.size()!=0);
    }

    // Common part (plus select/remove, enabled later)
    ui->qqLoad->setEnabled((tp==nullptr)&&simStopped);
    ui->qqInterpolate->setEnabled((tp!=nullptr)&&simStopped);

    // 3D part only:
    ui->qqX->setEnabled((tp!=nullptr)&&simStopped&&applyTexture3D&&(!usingFixedTextureCoordinates));
    ui->qqY->setEnabled((tp!=nullptr)&&simStopped&&applyTexture3D&&(!usingFixedTextureCoordinates));
    ui->qqZ->setEnabled((tp!=nullptr)&&applyTexture3D&&(!usingFixedTextureCoordinates));
    ui->qqU->setEnabled((tp!=nullptr)&&simStopped&&applyTexture3D&&(!usingFixedTextureCoordinates)&&(!forbidU));
    ui->qqV->setEnabled((tp!=nullptr)&&simStopped&&applyTexture3D&&(!usingFixedTextureCoordinates)&&(!forbidV));
    ui->qqRepeatU->setEnabled((tp!=nullptr)&&simStopped&&applyTexture3D&&(!usingFixedTextureCoordinates)&&(!forbidU));
    ui->qqRepeatV->setEnabled((tp!=nullptr)&&simStopped&&applyTexture3D&&(!usingFixedTextureCoordinates)&&(!forbidV));
    ui->qqAlpha->setEnabled((tp!=nullptr)&&simStopped&&applyTexture3D&&(!usingFixedTextureCoordinates));
    ui->qqBeta->setEnabled((tp!=nullptr)&&simStopped&&applyTexture3D&&(!usingFixedTextureCoordinates));
    ui->qqGamma->setEnabled((tp!=nullptr)&&simStopped&&applyTexture3D&&(!usingFixedTextureCoordinates));
    ui->qqApplyMode->setEnabled((tp!=nullptr)&&simStopped&&applyTexture3D);
    ui->qqMapMode->setEnabled((tp!=nullptr)&&simStopped&&applyTexture3D);

    ui->qqApplyMode->clear();
    ui->qqMapMode->clear();

    if (tp!=nullptr)
    {
        C7Vector v(tp->getTextureRelativeConfig());
        C3Vector euler(v.Q.getEulerAngles());
        float scalingX,scalingY;
        tp->getTextureScaling(scalingX,scalingY);
        if (applyTexture3D)
        {
            if (usingFixedTextureCoordinates)
            {
                ui->qqX->setText("");
                ui->qqY->setText("");
                ui->qqZ->setText("");
                ui->qqAlpha->setText("");
                ui->qqBeta->setText("");
                ui->qqGamma->setText("");

                ui->qqU->setText("");
                ui->qqV->setText("");
                ui->qqMapMode->addItem(strTranslate(IDSN_MAP_COORD_IMPORTED),QVariant(-1));
            }
            else
            {
                ui->qqX->setText(tt::getFString(true,v.X(0),3).c_str());
                ui->qqY->setText(tt::getFString(true,v.X(1),3).c_str());
                ui->qqZ->setText(tt::getFString(true,v.X(2),3).c_str());
                ui->qqAlpha->setText(tt::getAngleFString(true,euler(0),2).c_str());
                ui->qqBeta->setText(tt::getAngleFString(true,euler(1),2).c_str());
                ui->qqGamma->setText(tt::getAngleFString(true,euler(2),2).c_str());

                ui->qqU->setText(tt::getFString(false,scalingX,2).c_str());
                ui->qqV->setText(tt::getFString(false,scalingY,2).c_str());
            }
            ui->qqRepeatU->setChecked(tp->getRepeatU());
            ui->qqRepeatV->setChecked(tp->getRepeatV());

            ui->qqMapMode->addItem(strTranslate(IDSN_MAP_COORD_PROJECTION),QVariant(sim_texturemap_plane));
            ui->qqMapMode->addItem(strTranslate(IDSN_MAP_COORD_CYLINDER),QVariant(sim_texturemap_cylinder));
            ui->qqMapMode->addItem(strTranslate(IDSN_MAP_COORD_SPHERE),QVariant(sim_texturemap_sphere));
            ui->qqMapMode->addItem(strTranslate(IDSN_MAP_COORD_CUBE),QVariant(sim_texturemap_cube));

            ui->qqApplyMode->addItem(strTranslate(IDSN_TEXTURE_APPLY_MODE_MODULATE),QVariant(0));
            ui->qqApplyMode->addItem(strTranslate(IDSN_TEXTURE_APPLY_MODE_DECAL),QVariant(1));
            ui->qqApplyMode->addItem(strTranslate(IDSN_TEXTURE_APPLY_MODE_ADD),QVariant(2));
// for now          ui->qqApplyMode->addItem(strTranslate(IDSN_TEXTURE_APPLY_MODE_BLEND),QVariant(3));
            for (int i=0;i<ui->qqMapMode->count();i++)
            {
                int mm=tp->getTextureMapMode();
                if (usingFixedTextureCoordinates)
                    mm=-1;
                if (ui->qqMapMode->itemData(i).toInt()==mm)
                {
                    ui->qqMapMode->setCurrentIndex(i);
                    break;
                }
            }
            for (int i=0;i<ui->qqApplyMode->count();i++)
            {
                int mm=tp->getApplyMode();
                if (ui->qqApplyMode->itemData(i).toInt()==mm)
                {
                    ui->qqApplyMode->setCurrentIndex(i);
                    break;
                }
            }
        }
        else
        { // non-3D here!
            ui->qqX->setText("");
            ui->qqY->setText("");
            ui->qqZ->setText("");
            ui->qqAlpha->setText("");
            ui->qqBeta->setText("");
            ui->qqGamma->setText("");

            ui->qqU->setText("");
            ui->qqV->setText("");

            ui->qqRepeatU->setChecked(false);
            ui->qqRepeatV->setChecked(false);
        }

        ui->qqInterpolate->setChecked(tp->getInterpolateColors());
        ui->qqApplyMode->setCurrentIndex(tp->getApplyMode());
        ui->qqRemoveSelect->setText(strTranslate(IDS_REMOVE_TEXTURE));
        ui->qqRemoveSelect->setEnabled(true);
        std::string textureName=strTranslate(IDS_TEXTURE_NAME_NONE);
        if ((tp->getTextureObjectID()>=SIM_IDSTART_TEXTURE)&&(tp->getTextureObjectID()<=SIM_IDEND_TEXTURE))
        { // we have a static texture
            CTextureObject* to=App::ct->textureCont->getObject(tp->getTextureObjectID());
            if (to!=nullptr)
            {
                textureName=to->getObjectName();
                int sx,sy;
                to->getTextureSize(sx,sy);
                textureName+=" [";
                textureName+=boost::lexical_cast<std::string>(sx)+"x"+boost::lexical_cast<std::string>(sy)+"] ";
                textureName+=tt::decorateString(" (",strTranslate(IDSN_STATIC_TEXTURE),")");
            }
        }
        else
        { // we have a dynamic texture
            CVisionSensor* rs=App::ct->objCont->getVisionSensor(tp->getTextureObjectID());
            if (rs!=nullptr)
            {
                textureName=rs->getObjectName();
                int s[2];
                rs->getRealResolution(s);
                textureName+=" [";
                textureName+=boost::lexical_cast<std::string>(s[0])+"x"+boost::lexical_cast<std::string>(s[1])+"] ";
                textureName+=tt::decorateString(" (",strTranslate(IDSN_DYNAMIC_TEXTURE),")");
            }
        }
        ui->qqTextureName->setText(textureName.c_str());

        if (foundTextureCoordinatesOnShape||usingFixedTextureCoordinates)
            ui->qqTextureCoordinates->setText(strTranslate(IDS_FROM_SHAPE_IMPORT));
        else
            ui->qqTextureCoordinates->setText(strTranslate(IDS_CALCULATED));
    }
    else
    {
        // Check if there are already existing textures:
        ui->qqRemoveSelect->setEnabled( (App::ct->textureCont->getObjectAtIndex(0)!=nullptr)||(App::ct->objCont->visionSensorList.size()!=0) );
        if (foundTextureCoordinatesOnShape)
            ui->qqTextureCoordinates->setText(strTranslate(IDS_FROM_SHAPE_IMPORT));
        else
            ui->qqTextureCoordinates->setText(strTranslate(IDS_TEXTURE_NAME_NONE)); // Actually just "none"

        ui->qqX->setText("");
        ui->qqY->setText("");
        ui->qqZ->setText("");
        ui->qqAlpha->setText("");
        ui->qqBeta->setText("");
        ui->qqGamma->setText("");
        ui->qqU->setText("");
        ui->qqV->setText("");
        ui->qqApplyMode->setCurrentIndex(1);
        ui->qqInterpolate->setChecked(false);
        ui->qqRepeatU->setChecked(false);
        ui->qqRepeatV->setChecked(false);
        ui->qqTextureName->setText(strTranslate(IDS_TEXTURE_NAME_NONE));
        ui->qqRemoveSelect->setText(strTranslate(IDS_SELECT_TEXTURE_FROM_EXISTING));
    }
    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

bool CQDlgTextures::needsDestruction()
{
    if (!isLinkedDataValid())
        return(true);
    return(CDlgEx::needsDestruction());
}

bool CQDlgTextures::isLinkedDataValid()
{
    if (!App::ct->simulation->isSimulationStopped())
        return(false);
    if (_objType==TEXTURE_ID_SIMPLE_SHAPE)
    {
        if (App::ct->objCont->getLastSelectionID()!=_objID1)
            return(false);
    }
    if (_objType==TEXTURE_ID_COMPOUND_SHAPE)
    {
        if (App::mainWindow->editModeContainer->getEditModeObjectID()!=_objID1)
            return(false);
        if (App::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex()!=_objID2)
            return(false);
    }
    if (_objType==TEXTURE_ID_OPENGL_GUI_BACKGROUND)
    {
        if (App::ct->buttonBlockContainer->getBlockInEdition()!=_objID1)
            return(false);
        if (App::ct->buttonBlockContainer->selectedButtons.size()>0)
            return(false);
    }
    if (_objType==TEXTURE_ID_OPENGL_GUI_BUTTON)
    {
        if (App::ct->buttonBlockContainer->getBlockInEdition()!=_objID1)
            return(false);
        CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(_objID1);
        if (itBlock==nullptr)
            return(false);
        if (App::ct->buttonBlockContainer->selectedButtons.size()<=0)
            return(false);
        int butt=App::ct->buttonBlockContainer->selectedButtons[App::ct->buttonBlockContainer->selectedButtons.size()-1];
        VPoint size;
        itBlock->getBlockSize(size);
        CSoftButton* itButton=itBlock->getButtonAtPos(butt%size.x,butt/size.x);
        if (itButton==nullptr)
            return(false);
        if (itButton->buttonID!=_objID2)
            return(false);
    }

    bool isValid;
    App::getTexturePropertyPointerFromItem(_objType,_objID1,_objID2,nullptr,nullptr,&isValid,nullptr);
    return(isValid);
}


void CQDlgTextures::displayDlg(int objType,int objID1,int objID2,QWidget* theParentWindow)
{
    if (App::mainWindow==nullptr)
        return;
    App::mainWindow->dlgCont->close(TEXTURE_DLG);
    if (App::mainWindow->dlgCont->openOrBringToFront(TEXTURE_DLG))
    {
        CQDlgTextures* tex=(CQDlgTextures*)App::mainWindow->dlgCont->getDialog(TEXTURE_DLG);
        if (tex!=nullptr)
            tex->_initializeDlg(objType,objID1,objID2);
    }
}

void CQDlgTextures::_initializeDlg(int objType,int objID1,int objID2)
{
    _objType=objType;
    _objID1=objID1;
    _objID2=objID2;
    std::string str;
    App::getTexturePropertyPointerFromItem(_objType,_objID1,_objID2,&str,nullptr,nullptr,nullptr);
    setWindowTitle(str.c_str());
    refresh();
}

void CQDlgTextures::_setTextureConfig(int index)
{
    QLineEdit* ww[6]={ui->qqX,ui->qqY,ui->qqZ,ui->qqAlpha,ui->qqBeta,ui->qqGamma};
    bool ok;
    float newVal=ww[index]->text().toFloat(&ok);
    if (ok)
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=SET_3DCONFIG_TEXTUREGUITRIGGEREDCMD;
        cmd.intParams.push_back(_objType);
        cmd.intParams.push_back(_objID1);
        cmd.intParams.push_back(_objID2);
        cmd.intParams.push_back(index);
        if (index>=3)
            newVal*=gv::userToRad;
        cmd.floatParams.push_back(newVal);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    }
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgTextures::_setTextureScaling(int index)
{
    QLineEdit* ww[2]={ui->qqU,ui->qqV};
    bool ok;
    float newVal=ww[index]->text().toFloat(&ok);
    if (ok)
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=SET_SCALING_TEXTUREGUITRIGGEREDCMD;
        cmd.intParams.push_back(_objType);
        cmd.intParams.push_back(_objID1);
        cmd.intParams.push_back(_objID2);
        cmd.intParams.push_back(index);
        cmd.floatParams.push_back(newVal);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    }
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgTextures::_setTextureBooleanProperty(int index)
{
    if (!isLinkedDataValid())
        return;
    SSimulationThreadCommand cmd;
    cmd.cmdId=TOGGLE_BOOLPROP_TEXTUREGUITRIGGEREDCMD;
    cmd.intParams.push_back(_objType);
    cmd.intParams.push_back(_objID1);
    cmd.intParams.push_back(_objID2);
    cmd.intParams.push_back(index);
    App::appendSimulationThreadCommand(cmd);
    App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgTextures::cancelEvent() // this was empty before VDialog wrap thing
{ // We just hide the dialog and destroy it at next rendering pass
    if (isModal()) // this condition and next line on 20/5/2013: on Linux the dlg couldn't be closed! Thanks to Ulrich Schwesinger
        defaultModalDialogEndRoutine(false);
    else
        CDlgEx::cancelEvent();
}

bool CQDlgTextures::doesInstanceSwitchRequireDestruction()
{
    return(true);
}

void CQDlgTextures::on_qqAlpha_editingFinished()
{
    if (!ui->qqAlpha->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        _setTextureConfig(3);
    }
}

void CQDlgTextures::on_qqBeta_editingFinished()
{
    if (!ui->qqBeta->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        _setTextureConfig(4);
    }
}

void CQDlgTextures::on_qqGamma_editingFinished()
{
    if (!ui->qqGamma->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        _setTextureConfig(5);
    }
}

void CQDlgTextures::on_qqX_editingFinished()
{
    if (!ui->qqX->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        _setTextureConfig(0);
    }
}

void CQDlgTextures::on_qqY_editingFinished()
{
    if (!ui->qqY->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        _setTextureConfig(1);
    }
}

void CQDlgTextures::on_qqZ_editingFinished()
{
    if (!ui->qqZ->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        _setTextureConfig(2);
    }
}

void CQDlgTextures::on_qqU_editingFinished()
{
    if (!ui->qqU->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        _setTextureScaling(0);
    }
}

void CQDlgTextures::on_qqV_editingFinished()
{
    if (!ui->qqV->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        _setTextureScaling(1);
    }
}

void CQDlgTextures::on_qqInterpolate_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        _setTextureBooleanProperty(0);
    }
}

void CQDlgTextures::on_qqRepeatU_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        _setTextureBooleanProperty(2);
    }
}

void CQDlgTextures::on_qqRepeatV_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        _setTextureBooleanProperty(3);
    }
}

void CQDlgTextures::on_qqRemoveSelect_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CTextureProperty* tp=App::getTexturePropertyPointerFromItem(_objType,_objID1,_objID2,nullptr,nullptr,nullptr,nullptr);
        int tObject=-1; // means remove
        if (tp==nullptr)
        { // add an existing texture
            CQDlgTextureSelection dlg(this);
            dlg.makeDialogModal();
            tObject=dlg.selectedTextureObject;
        }
        SSimulationThreadCommand cmd;
        cmd.cmdId=SELECT_REMOVE_TEXTUREGUITRIGGEREDCMD;
        cmd.intParams.push_back(_objType);
        cmd.intParams.push_back(_objID1);
        cmd.intParams.push_back(_objID2);
        cmd.intParams.push_back(tObject);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTextures::on_qqLoad_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        std::string tst(App::directories->textureDirectory);
        std::string filenameAndPath=App::uiThread->getOpenFileName(this,0,"Loading texture...",tst,"",true,"Image files","tga","jpg","jpeg","png","gif","bmp","tiff");
        if (filenameAndPath.length()!=0)
        {
            if (VFile::doesFileExist(filenameAndPath))
            {
                CQDlgTextureLoadOptions dlg(App::mainWindow);
                dlg.refresh();
                dlg.makeDialogModal();
                int scaleTo=0;
                if (dlg.scale)
                    scaleTo=dlg.scaleTo;
                App::appendSimulationThreadCommand(SET_CURRENTDIRECTORY_GUITRIGGEREDCMD,DIRECTORY_ID_TEXTURE,-1,0.0,0.0,App::directories->getPathFromFull(filenameAndPath).c_str());
                int resX,resY,n;
                unsigned char* data=CImageLoaderSaver::load(filenameAndPath.c_str(),&resX,&resY,&n,0,scaleTo);
                if ( (n<3)||(n>4) )
                {
                    delete[] data;
                    data=nullptr;
                }
                if (data==nullptr)
                    App::uiThread->messageBox_critical(App::mainWindow,strTranslate("Texture"),strTranslate(IDS_TEXTURE_FILE_COULD_NOT_BE_LOADED),VMESSAGEBOX_OKELI);
                else
                {
                    // Check if the resolution is a power of 2:
                    int oResX=resX;
                    int oResY=resY;
                    oResX&=(32768-1);
                    oResY&=(32768-1);
                    unsigned short tmp=32768;
                    while (tmp!=1)
                    {
                        if (oResX&tmp)
                            oResX&=tmp;
                        if (oResY&tmp)
                            oResY&=tmp;
                        tmp/=2;
                    }
                    if ((oResX!=resX)||(oResY!=resY))
                        App::uiThread->messageBox_warning(App::mainWindow,strTranslate("Texture"),strTranslate(IDS_TEXTURE_RESOLUTION_NOT_POWER_OF_TWO_WARNING),VMESSAGEBOX_OKELI);
                    SSimulationThreadCommand cmd;
                    cmd.cmdId=LOAD_ANDAPPLY_TEXTUREGUITRIGGEREDCMD;
                    cmd.intParams.push_back(_objType);
                    cmd.intParams.push_back(_objID1);
                    cmd.intParams.push_back(_objID2);
                    cmd.intParams.push_back(resX);
                    cmd.intParams.push_back(resY);
                    cmd.intParams.push_back(n);
                    cmd.uint8Params.assign(data,data+n*resX*resY);
                    cmd.stringParams.push_back(App::directories->getNameFromFull(filenameAndPath).c_str());
                    App::appendSimulationThreadCommand(cmd);
                    App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
                }
            }
        }
    }
}

void CQDlgTextures::on_qqMapMode_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_WRITE_DATA
        {
            int mode=ui->qqMapMode->itemData(ui->qqMapMode->currentIndex()).toInt();
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_MAPPINGMODE_TEXTUREGUITRIGGEREDCMD;
            cmd.intParams.push_back(_objType);
            cmd.intParams.push_back(_objID1);
            cmd.intParams.push_back(_objID2);
            cmd.intParams.push_back(mode);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgTextures::on_qqApplyMode_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            int mode=ui->qqApplyMode->itemData(ui->qqApplyMode->currentIndex()).toInt();
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_APPLYMODE_TEXTUREGUITRIGGEREDCMD;
            cmd.intParams.push_back(_objType);
            cmd.intParams.push_back(_objID1);
            cmd.intParams.push_back(_objID2);
            cmd.intParams.push_back(mode);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}
