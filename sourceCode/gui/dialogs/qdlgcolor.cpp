#include "qdlgcolor.h"
#include "ui_qdlgcolor.h"
#include "gV.h"
#include "tt.h"
#include "app.h"

CQDlgColor::CQDlgColor(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgColor)
{
    _dlgType=COLOR_DLG;
    ui->setupUi(this);
    inRefreshPart=false;
    _validityCheck1=App::currentWorld->sceneObjects->getLastSelectionHandle();
    _validityCheck2=App::currentWorld->sceneObjects->getSelectionCount();
}

CQDlgColor::~CQDlgColor()
{
    delete ui;
}

void CQDlgColor::refresh()
{
    inRefreshPart=true;
    if (!isModal())
    {
        _getRGBFromItem();
        computeHSLValues();
    }

    QImage img(70,240,QImage::Format_RGB32);
    QRgb value=qRgb(int(redState*255.5),int(greenState*255.5),int(blueState*255.5));
    for (int i=0;i<240;i++)
    {
        for (int j=0;j<70;j++)
            img.setPixel(j,i,value);
    }
    QPixmap pimg;
    pimg.convertFromImage(img);
    ui->qqImage->setPixmap(pimg);

    ui->qqRedValue->setText(tt::getFString(false,redState,2).c_str());
    ui->qqGreenValue->setText(tt::getFString(false,greenState,2).c_str());
    ui->qqBlueValue->setText(tt::getFString(false,blueState,2).c_str());
    ui->qqHueValue->setText(tt::getFString(false,hueState,2).c_str());
    ui->qqSaturationValue->setText(tt::getFString(false,saturationState,2).c_str());
    ui->qqLuminosityValue->setText(tt::getFString(false,luminosityState,2).c_str());

    ui->qqRedSlider->setSliderPosition(int(redState*100.5));
    ui->qqGreenSlider->setSliderPosition(int(greenState*100.5));
    ui->qqBlueSlider->setSliderPosition(int(blueState*100.5));
    ui->qqHueSlider->setSliderPosition(int(hueState*100.5));
    ui->qqSaturationSlider->setSliderPosition(int(saturationState*100.5));
    ui->qqLuminositySlider->setSliderPosition(int(luminosityState*100.5));

    inRefreshPart=false;
}

bool CQDlgColor::needsDestruction()
{
    if (!isLinkedDataValid())
        return(true);
    return(CDlgEx::needsDestruction());
}

void CQDlgColor::initializationEvent()
{
    CDlgEx::initializationEvent();
    computeHSLValues();
    refresh();
}

void CQDlgColor::displayDlg(int objType,int objID1,int objID2,int colComponent,QWidget* theParentWindow,bool doNotCloseMaterialDlg,bool doNotCloseLightMaterialDlg,bool appendColorComponentInName)
{
    if (App::mainWindow==nullptr)
        return;
    if (!doNotCloseMaterialDlg)
        App::mainWindow->dlgCont->close(MATERIAL_DLG);
    if (!doNotCloseLightMaterialDlg)
        App::mainWindow->dlgCont->close(LIGHTMATERIAL_DLG);
    App::mainWindow->dlgCont->close(COLOR_DLG);
    if (App::mainWindow->dlgCont->openOrBringToFront(COLOR_DLG))
    {
        CQDlgColor* it=(CQDlgColor*)App::mainWindow->dlgCont->getDialog(COLOR_DLG);
        if (it!=nullptr)
            it->initializeDlg(objType,objID1,objID2,colComponent,appendColorComponentInName);
    }
}

void CQDlgColor::displayDlgModal(int objType,int objID1,int objID2,int colComponent,QWidget* theParentWindow,bool doNotCloseMaterialDlg,bool doNotCloseLightMaterialDlg,bool appendColorComponentInName)
{
    if (App::mainWindow==nullptr)
        return;
    if (!doNotCloseMaterialDlg)
        App::mainWindow->dlgCont->close(MATERIAL_DLG);
    if (!doNotCloseLightMaterialDlg)
        App::mainWindow->dlgCont->close(LIGHTMATERIAL_DLG);
    App::mainWindow->dlgCont->close(COLOR_DLG);
    CQDlgColor it(theParentWindow);
    it.initializeDlg(objType,objID1,objID2,colComponent,appendColorComponentInName);
    it.makeDialogModal();
}

void CQDlgColor::initializeDlg(int objType,int objID1,int objID2,int colComponent,bool appendColorComponentInName)
{
    _objType=objType;
    _objID1=objID1;
    _objID2=objID2;
    _colComponent=colComponent;
    std::string str;
    App::getRGBPointerFromItem(_objType,_objID1,_objID2,_colComponent,&str);
    if (appendColorComponentInName)
    {
        if (_colComponent==sim_colorcomponent_ambient_diffuse)
            str+=" (ambient/diffuse)";
        if (_colComponent==sim_colorcomponent_diffuse)
            str+=" (diffuse)";
        if (_colComponent==sim_colorcomponent_specular)
            str+=" (specular)";
        if (_colComponent==sim_colorcomponent_emission)
            str+=" (emission)";
        if (_colComponent==sim_colorcomponent_auxiliary)
            str+=" (aux. channel)";
    }
    setWindowTitle(str.c_str());
    refresh();
}


bool CQDlgColor::isLinkedDataValid()
{
    if (_validityCheck1!=App::currentWorld->sceneObjects->getLastSelectionHandle())
        return(false);
    if (_validityCheck2!=App::currentWorld->sceneObjects->getSelectionCount())
        return(false);
    if (!App::currentWorld->simulation->isSimulationStopped())
        return(false);
    return(App::getRGBPointerFromItem(_objType,_objID1,_objID2,_colComponent,nullptr)!=nullptr);
}

void CQDlgColor::computeRGBValues()
{
    float hsl[3]={hueState,saturationState,luminosityState};
    float rgb[3];
    tt::hslToRgb(hsl,rgb);
    redState=rgb[0];
    greenState=rgb[1];
    blueState=rgb[2];
}

void CQDlgColor::computeHSLValues()
{
    float rgb[3]={redState,greenState,blueState};
    float hsl[3];
    tt::rgbToHsl(rgb,hsl);
    hueState=hsl[0];
    saturationState=hsl[1];
    luminosityState=hsl[2];
}

void CQDlgColor::_getRGBFromItem()
{
    const float* col=App::getRGBPointerFromItem(_objType,_objID1,_objID2,_colComponent,nullptr);
    if (col!=nullptr)
    {
        redState=col[0];
        greenState=col[1];
        blueState=col[2];
    }
}

void CQDlgColor::_setRGBToItem()
{
    float* col=App::getRGBPointerFromItem(_objType,_objID1,_objID2,_colComponent,nullptr);
    if (col!=nullptr)
    {
        col[0]=redState;
        col[1]=greenState;
        col[2]=blueState;
    }
}


bool CQDlgColor::doesInstanceSwitchRequireDestruction()
{
    return(true);
}

void CQDlgColor::cancelEvent() // this was empty before VDialog wrap thing
{ // We just hide the dialog and destroy it at next rendering pass
    if (isModal()) // this condition and next line on 25/1/2013: on Linux the dlg couldn't be closed!
        defaultModalDialogEndRoutine(false);
    else
        CDlgEx::cancelEvent();
}

void CQDlgColor::on_qqRedValue_editingFinished()
{
    if (inRefreshPart||(!ui->qqRedValue->isModified()))
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqRedValue->text().toFloat(&ok);
        if (ok)
        {
            tt::limitValue(0.0,1.0,newVal);
            redState=newVal;
            computeHSLValues();
            _setRGBToItem(); // so that we have first a local change (the server side change takes longer.. not fluid)
            if (!isModal())
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_ITEMRGB_COLORGUITRIGGEREDCMD;
                cmd.intParams.push_back(_objType);
                cmd.intParams.push_back(_objID1);
                cmd.intParams.push_back(_objID2);
                cmd.intParams.push_back(_colComponent);
                cmd.floatParams.push_back(redState);
                cmd.floatParams.push_back(greenState);
                cmd.floatParams.push_back(blueState);
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
        }
        refresh();
    }
}

void CQDlgColor::on_qqGreenValue_editingFinished()
{
    if (inRefreshPart||(!ui->qqGreenValue->isModified()))
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqGreenValue->text().toFloat(&ok);
        if (ok)
        {
            tt::limitValue(0.0,1.0,newVal);
            greenState=newVal;
            computeHSLValues();
            _setRGBToItem(); // so that we have first a local change (the server side change takes longer.. not fluid)
            if (!isModal())
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_ITEMRGB_COLORGUITRIGGEREDCMD;
                cmd.intParams.push_back(_objType);
                cmd.intParams.push_back(_objID1);
                cmd.intParams.push_back(_objID2);
                cmd.intParams.push_back(_colComponent);
                cmd.floatParams.push_back(redState);
                cmd.floatParams.push_back(greenState);
                cmd.floatParams.push_back(blueState);
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
        }
        refresh();
    }
}

void CQDlgColor::on_qqBlueValue_editingFinished()
{
    if (inRefreshPart||(!ui->qqBlueValue->isModified()))
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqBlueValue->text().toFloat(&ok);
        if (ok)
        {
            tt::limitValue(0.0,1.0,newVal);
            blueState=newVal;
            computeHSLValues();
            _setRGBToItem(); // so that we have first a local change (the server side change takes longer.. not fluid)
            if (!isModal())
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_ITEMRGB_COLORGUITRIGGEREDCMD;
                cmd.intParams.push_back(_objType);
                cmd.intParams.push_back(_objID1);
                cmd.intParams.push_back(_objID2);
                cmd.intParams.push_back(_colComponent);
                cmd.floatParams.push_back(redState);
                cmd.floatParams.push_back(greenState);
                cmd.floatParams.push_back(blueState);
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
        }
        refresh();
    }
}

void CQDlgColor::on_qqHueValue_editingFinished()
{
    if (inRefreshPart||(!ui->qqHueValue->isModified()))
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqHueValue->text().toFloat(&ok);
        if (ok)
        {
            tt::limitValue(0.0,1.0,newVal);
            hueState=newVal;
            computeRGBValues();
            _setRGBToItem(); // so that we have first a local change (the server side change takes longer.. not fluid)
            if (!isModal())
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_ITEMRGB_COLORGUITRIGGEREDCMD;
                cmd.intParams.push_back(_objType);
                cmd.intParams.push_back(_objID1);
                cmd.intParams.push_back(_objID2);
                cmd.intParams.push_back(_colComponent);
                cmd.floatParams.push_back(redState);
                cmd.floatParams.push_back(greenState);
                cmd.floatParams.push_back(blueState);
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
        }
        refresh();
    }
}

void CQDlgColor::on_qqSaturationValue_editingFinished()
{
    if (inRefreshPart||(!ui->qqSaturationValue->isModified()))
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqSaturationValue->text().toFloat(&ok);
        if (ok)
        {
            tt::limitValue(0.0,1.0,newVal);
            saturationState=newVal;
            computeRGBValues();
            _setRGBToItem(); // so that we have first a local change (the server side change takes longer.. not fluid)
            if (!isModal())
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_ITEMRGB_COLORGUITRIGGEREDCMD;
                cmd.intParams.push_back(_objType);
                cmd.intParams.push_back(_objID1);
                cmd.intParams.push_back(_objID2);
                cmd.intParams.push_back(_colComponent);
                cmd.floatParams.push_back(redState);
                cmd.floatParams.push_back(greenState);
                cmd.floatParams.push_back(blueState);
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
        }
        refresh();
    }
}

void CQDlgColor::on_qqLuminosityValue_editingFinished()
{
    if (inRefreshPart||(!ui->qqLuminosityValue->isModified()))
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqLuminosityValue->text().toFloat(&ok);
        if (ok)
        {
            tt::limitValue(0.0,1.0,newVal);
            luminosityState=newVal;
            computeRGBValues();
            _setRGBToItem(); // so that we have first a local change (the server side change takes longer.. not fluid)
            if (!isModal())
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_ITEMRGB_COLORGUITRIGGEREDCMD;
                cmd.intParams.push_back(_objType);
                cmd.intParams.push_back(_objID1);
                cmd.intParams.push_back(_objID2);
                cmd.intParams.push_back(_colComponent);
                cmd.floatParams.push_back(redState);
                cmd.floatParams.push_back(greenState);
                cmd.floatParams.push_back(blueState);
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
        }
        refresh();
    }
}

void CQDlgColor::on_qqRedSlider_sliderMoved(int position)
{
    if (inRefreshPart)
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        redState=float(position)/100.0;
        computeHSLValues();
        _setRGBToItem(); // so that we have first a local change (the server side change takes longer.. not fluid)
        if (!isModal())
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ITEMRGB_COLORGUITRIGGEREDCMD;
            cmd.intParams.push_back(_objType);
            cmd.intParams.push_back(_objID1);
            cmd.intParams.push_back(_objID2);
            cmd.intParams.push_back(_colComponent);
            cmd.floatParams.push_back(redState);
            cmd.floatParams.push_back(greenState);
            cmd.floatParams.push_back(blueState);
            App::appendSimulationThreadCommand(cmd);
        }
        refresh();
    }
}

void CQDlgColor::on_qqGreenSlider_sliderMoved(int position)
{
    if (inRefreshPart)
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        greenState=float(position)/100.0;
        computeHSLValues();
        _setRGBToItem(); // so that we have first a local change (the server side change takes longer.. not fluid)
        if (!isModal())
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ITEMRGB_COLORGUITRIGGEREDCMD;
            cmd.intParams.push_back(_objType);
            cmd.intParams.push_back(_objID1);
            cmd.intParams.push_back(_objID2);
            cmd.intParams.push_back(_colComponent);
            cmd.floatParams.push_back(redState);
            cmd.floatParams.push_back(greenState);
            cmd.floatParams.push_back(blueState);
            App::appendSimulationThreadCommand(cmd);
        }
        refresh();
    }
}

void CQDlgColor::on_qqBlueSlider_sliderMoved(int position)
{
    if (inRefreshPart)
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        blueState=float(position)/100.0;
        computeHSLValues();
        _setRGBToItem(); // so that we have first a local change (the server side change takes longer.. not fluid)
        if (!isModal())
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ITEMRGB_COLORGUITRIGGEREDCMD;
            cmd.intParams.push_back(_objType);
            cmd.intParams.push_back(_objID1);
            cmd.intParams.push_back(_objID2);
            cmd.intParams.push_back(_colComponent);
            cmd.floatParams.push_back(redState);
            cmd.floatParams.push_back(greenState);
            cmd.floatParams.push_back(blueState);
            App::appendSimulationThreadCommand(cmd);
        }
        refresh();
    }
}

void CQDlgColor::on_qqHueSlider_sliderMoved(int position)
{
    if (inRefreshPart)
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        hueState=float(position)/100.0;
        computeRGBValues();
        _setRGBToItem(); // so that we have first a local change (the server side change takes longer.. not fluid)
        if (!isModal())
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ITEMRGB_COLORGUITRIGGEREDCMD;
            cmd.intParams.push_back(_objType);
            cmd.intParams.push_back(_objID1);
            cmd.intParams.push_back(_objID2);
            cmd.intParams.push_back(_colComponent);
            cmd.floatParams.push_back(redState);
            cmd.floatParams.push_back(greenState);
            cmd.floatParams.push_back(blueState);
            App::appendSimulationThreadCommand(cmd);
        }
        refresh();
    }
}

void CQDlgColor::on_qqSaturationSlider_sliderMoved(int position)
{
    if (inRefreshPart)
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        saturationState=float(position)/100.0;
        computeRGBValues();
        _setRGBToItem(); // so that we have first a local change (the server side change takes longer.. not fluid)
        if (!isModal())
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ITEMRGB_COLORGUITRIGGEREDCMD;
            cmd.intParams.push_back(_objType);
            cmd.intParams.push_back(_objID1);
            cmd.intParams.push_back(_objID2);
            cmd.intParams.push_back(_colComponent);
            cmd.floatParams.push_back(redState);
            cmd.floatParams.push_back(greenState);
            cmd.floatParams.push_back(blueState);
            App::appendSimulationThreadCommand(cmd);
        }
        refresh();
    }
}

void CQDlgColor::on_qqLuminositySlider_sliderMoved(int position)
{
    if (inRefreshPart)
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        luminosityState=float(position)/100.0;
        computeRGBValues();
        _setRGBToItem(); // so that we have first a local change (the server side change takes longer.. not fluid)
        if (!isModal())
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ITEMRGB_COLORGUITRIGGEREDCMD;
            cmd.intParams.push_back(_objType);
            cmd.intParams.push_back(_objID1);
            cmd.intParams.push_back(_objID2);
            cmd.intParams.push_back(_colComponent);
            cmd.floatParams.push_back(redState);
            cmd.floatParams.push_back(greenState);
            cmd.floatParams.push_back(blueState);
            App::appendSimulationThreadCommand(cmd);
        }
        refresh();
    }
}


void CQDlgColor::on_qqRedSlider_sliderReleased()
{
    if (!isModal())
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
}

void CQDlgColor::on_qqGreenSlider_sliderReleased()
{
    if (!isModal())
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
}

void CQDlgColor::on_qqBlueSlider_sliderReleased()
{
    if (!isModal())
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
}

void CQDlgColor::on_qqHueSlider_sliderReleased()
{
    if (!isModal())
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
}

void CQDlgColor::on_qqSaturationSlider_sliderReleased()
{
    if (!isModal())
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
}

void CQDlgColor::on_qqLuminositySlider_sliderReleased()
{
    if (!isModal())
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
}
