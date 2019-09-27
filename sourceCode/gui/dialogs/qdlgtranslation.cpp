
#include "vrepMainHeader.h"
#include "qdlgtranslation.h"
#include "ui_qdlgtranslation.h"
#include "gV.h"
#include "tt.h"
#include "app.h"
#include "v_repStrings.h"

int CQDlgTranslation::coordMode=0; //0=abs,1=rel to parent
float CQDlgTranslation::translationValues[3]={0.0f,0.0f,0.0f};
float CQDlgTranslation::scalingValues[3]={1.0f,1.0f,1.0f};
int CQDlgTranslation::translateMode=0; //0=abs,1=rel to parent,2=rel to self
int CQDlgTranslation::scaleMode=0; //0=abs,1=rel to parent
int CQDlgTranslation::currentTab=0; //0=mouse transl., 1=pos, 2=transl., 3=scaling

int CQDlgTranslation::manipulationModePermission;
int CQDlgTranslation::manipulationTranslationRelativeTo;
float CQDlgTranslation::manipulationTranslationStepSize;
bool CQDlgTranslation::objectTranslationSettingsLocked;


CQDlgTranslation::CQDlgTranslation(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgTranslation)
{
    ui->setupUi(this);
    lastLastSelectionID=-1;
}

CQDlgTranslation::~CQDlgTranslation()
{
    delete ui;
}

void CQDlgTranslation::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgTranslation::refresh()
{
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    int editMode=App::getEditModeType();
    int lastSelID=App::ct->objCont->getLastSelectionID();
    lastLastSelectionID=lastSelID;

    ui->tabWidget->setCurrentIndex(currentTab);

    if (editMode==NO_EDIT_MODE)
    {
        bool sel=(App::ct->objCont->getSelSize()!=0);
        bool bigSel=(App::ct->objCont->getSelSize()>1);
        _enableCoordinatePart(sel,bigSel,true);
        _enableTranslationPart(sel,sel,true);
        _enableScalingPart(sel&&(scaleMode!=2),sel&&(scaleMode!=2),true);
        C3DObject* object=App::ct->objCont->getLastSelection_object();
        if (sel&&(object!=nullptr))
        {
            // Coordinate part:
            C3Vector euler,pos;
            if (coordMode==0)
            {
                euler=object->getCumulativeTransformationPart1().Q.getEulerAngles();
                pos=object->getCumulativeTransformationPart1().X;
            }
            else
            {
                euler=object->getLocalTransformationPart1().Q.getEulerAngles();
                pos=object->getLocalTransformationPart1().X;
            }
            ui->qqCoordX->setText(tt::getEString(true,pos(0),4).c_str());
            ui->qqCoordY->setText(tt::getEString(true,pos(1),4).c_str());
            ui->qqCoordZ->setText(tt::getEString(true,pos(2),4).c_str());
            ui->qqCoordWorld->setChecked(coordMode==0);
            ui->qqCoordParent->setChecked(coordMode==1);
            // Transformation part:
            _setValuesTranslationPart(true);
            _setValuesScalingPart(true);
            // Mouse manip part:
            manipulationModePermission=object->getObjectManipulationModePermissions();
            manipulationTranslationRelativeTo=object->getObjectManipulationTranslationRelativeTo();
            manipulationTranslationStepSize=object->getNonDefaultTranslationStepSize();
            objectTranslationSettingsLocked=object->getObjectTranslationSettingsLocked();
        }
        else
        {
            _setDefaultValuesCoordinatePart(true);
            _setDefaultValuesTranslationPart(true);
            _setDefaultValuesScalingPart(true);
        }

        // mouse manip part:
        ui->qqPosWorld->setEnabled((object!=nullptr)&&(!objectTranslationSettingsLocked));
        ui->qqPosParent->setEnabled((object!=nullptr)&&(!objectTranslationSettingsLocked));
        ui->qqPosOwn->setEnabled((object!=nullptr)&&(!objectTranslationSettingsLocked));
        ui->qqPosX->setEnabled((object!=nullptr)&&(!objectTranslationSettingsLocked));
        ui->qqPosY->setEnabled((object!=nullptr)&&(!objectTranslationSettingsLocked));
        ui->qqPosZ->setEnabled((object!=nullptr)&&(!objectTranslationSettingsLocked));
        ui->qqPosCombo->setEnabled((object!=nullptr)&&(!objectTranslationSettingsLocked));

        ui->qqPosWorld->setChecked((object!=nullptr)&&(manipulationTranslationRelativeTo==0));
        ui->qqPosParent->setChecked((object!=nullptr)&&(manipulationTranslationRelativeTo==1));
        ui->qqPosOwn->setChecked((object!=nullptr)&&(manipulationTranslationRelativeTo==2));

        ui->qqPosX->setChecked((object!=nullptr)&&(manipulationModePermission&0x01));
        ui->qqPosY->setChecked((object!=nullptr)&&(manipulationModePermission&0x02));
        ui->qqPosZ->setChecked((object!=nullptr)&&(manipulationModePermission&0x04));

        ui->qqPosCombo->clear();
        if (object!=nullptr)
        {
            ui->qqPosCombo->addItem(IDS_MANIP_NONE,QVariant(-1));
            ui->qqPosCombo->addItem(IDS_DEFAULT,QVariant(0));

            ui->qqPosCombo->addItem(tt::getFString(false,0.001f,3).c_str(),QVariant(1));
            ui->qqPosCombo->addItem(tt::getFString(false,0.002f,3).c_str(),QVariant(2));
            ui->qqPosCombo->addItem(tt::getFString(false,0.005f,3).c_str(),QVariant(5));
            ui->qqPosCombo->addItem(tt::getFString(false,0.01f,3).c_str(),QVariant(10));
            ui->qqPosCombo->addItem(tt::getFString(false,0.025f,3).c_str(),QVariant(25));
            ui->qqPosCombo->addItem(tt::getFString(false,0.05f,3).c_str(),QVariant(50));
            ui->qqPosCombo->addItem(tt::getFString(false,0.1f,3).c_str(),QVariant(100));
            ui->qqPosCombo->addItem(tt::getFString(false,0.25f,3).c_str(),QVariant(250));
            ui->qqPosCombo->addItem(tt::getFString(false,0.5f,3).c_str(),QVariant(500));

            if (App::ct->simulation->isSimulationStopped())
            {
                if (object->getObjectTranslationDisabledDuringNonSimulation())
                    _selectItemOfCombobox(ui->qqPosCombo,-1);
                else
                    _selectItemOfCombobox(ui->qqPosCombo,int((manipulationTranslationStepSize+0.0005f)*1000.0f));

            }
            else
            {
                if (object->getObjectTranslationDisabledDuringSimulation())
                    _selectItemOfCombobox(ui->qqPosCombo,-1);
                else
                    _selectItemOfCombobox(ui->qqPosCombo,int((manipulationTranslationStepSize+0.0005f)*1000.0f));
            }
        }

    }
    else
    { // We are in an edit mode

        // mouse manip part:
       // ui->qqDisabledWhenRunning->setEnabled(false);
       // ui->qqDisabledWhenNotRunning->setEnabled(false);
        ui->qqPosWorld->setEnabled(false);
        ui->qqPosParent->setEnabled(false);
        ui->qqPosOwn->setEnabled(false);
        ui->qqPosX->setEnabled(false);
        ui->qqPosY->setEnabled(false);
        ui->qqPosZ->setEnabled(false);
        ui->qqPosCombo->setEnabled(false);
        ui->qqPosWorld->setChecked(false);
        ui->qqPosParent->setChecked(false);
        ui->qqPosOwn->setChecked(false);
        ui->qqPosX->setChecked(false);
        ui->qqPosY->setChecked(false);
        ui->qqPosZ->setChecked(false);
        ui->qqPosCombo->clear();

        if (editMode&(BUTTON_EDIT_MODE|TRIANGLE_EDIT_MODE|EDGE_EDIT_MODE))
        {
            _enableCoordinatePart(false,false,true);
            _enableTranslationPart(false,false,true);
            _enableScalingPart(false,false,true);
            _setDefaultValuesCoordinatePart(true);
            _setDefaultValuesTranslationPart(true);
            _setDefaultValuesScalingPart(true);
        }
        else
        { // Vertex or path edit mode
            if (editMode&VERTEX_EDIT_MODE)
            {
                bool sel=(App::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
                bool bigSel=(App::mainWindow->editModeContainer->getEditModeBufferSize()>1);
                if (translateMode==2)
                    translateMode=1;

                _setDefaultValuesCoordinatePart(true);
                _enableCoordinatePart(sel,bigSel,true);
                _enableTranslationPart(sel,sel,false);
                _enableScalingPart(sel&&(scaleMode!=2),sel&&(scaleMode!=2),false);
                ui->qqTransfWorld->setEnabled(sel);
                ui->qqTransfParent->setEnabled(sel);
                ui->qqTransfOwn->setEnabled(false);
                ui->qqScaleWorld->setEnabled(sel);
                ui->qqScaleParent->setEnabled(sel);
                CShape* shape=App::mainWindow->editModeContainer->getEditModeShape();
                if (sel&&(shape!=nullptr))
                {
                    // Coordinate part:
                    int ind=App::mainWindow->editModeContainer->getLastEditModeBufferValue();
                    C3Vector pos(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind));
                    if (coordMode==0)
                        pos=shape->getCumulativeTransformation()*pos;
                    ui->qqCoordX->setText(tt::getEString(true,pos(0),4).c_str());
                    ui->qqCoordY->setText(tt::getEString(true,pos(1),4).c_str());
                    ui->qqCoordZ->setText(tt::getEString(true,pos(2),4).c_str());
                    ui->qqCoordWorld->setChecked(coordMode==0);
                    ui->qqCoordParent->setChecked(coordMode==1);
                    // Transformation part:
                    _setValuesTranslationPart(true);
                    _setValuesScalingPart(true);
                }
                else
                {
                    _setDefaultValuesCoordinatePart(false);
                    _setDefaultValuesTranslationPart(true);
                    _setDefaultValuesScalingPart(true);
                }
            }
            if (editMode&PATH_EDIT_MODE)
            {
                bool sel=(App::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
                bool bigSel=(App::mainWindow->editModeContainer->getEditModeBufferSize()>1);

                _enableCoordinatePart(sel,bigSel,true);
                _enableTranslationPart(sel,sel,true);
                _enableScalingPart(sel&&(scaleMode!=2),sel&&(scaleMode!=2),true);
                if (sel)
                {
                    CPath* path=App::mainWindow->editModeContainer->getEditModePath();
                    int ind=App::mainWindow->editModeContainer->getLastEditModeBufferValue();
                    CSimplePathPoint* pp=App::mainWindow->editModeContainer->getEditModePathContainer()->getSimplePathPoint(ind);
                    if (sel&&(path!=nullptr)&&(pp!=nullptr))
                    {
                        // Coordinate part:
                        C7Vector tr(pp->getTransformation());
                        if (coordMode==0)
                            tr=path->getCumulativeTransformation()*tr;
                        C3Vector euler(tr.Q.getEulerAngles());
                        ui->qqCoordX->setText(tt::getEString(true,tr.X(0),4).c_str());
                        ui->qqCoordY->setText(tt::getEString(true,tr.X(1),4).c_str());
                        ui->qqCoordZ->setText(tt::getEString(true,tr.X(2),4).c_str());
                        ui->qqCoordWorld->setChecked(coordMode==0);
                        ui->qqCoordParent->setChecked(coordMode==1);
                        // Transformation part:
                        _setValuesTranslationPart(true);
                        _setValuesScalingPart(true);
                    }
                    else
                    {
                        _setDefaultValuesCoordinatePart(false);
                        _setDefaultValuesTranslationPart(true);
                        _setDefaultValuesScalingPart(true);
                    }
                }
                else
                {
                    _setDefaultValuesCoordinatePart(true);
                    _setDefaultValuesTranslationPart(true);
                    _setDefaultValuesScalingPart(true);
                }
            }
        }
    }
    selectLineEdit(lineEditToSelect);
}

void CQDlgTranslation::_enableCoordinatePart(bool enableState,bool enableButtons,bool alsoRadioButtons)
{
    ui->qqCoordX->setEnabled(enableState);
    ui->qqCoordY->setEnabled(enableState);
    ui->qqCoordZ->setEnabled(enableState);
    ui->qqCoordApplyPos->setEnabled(enableButtons);
    ui->qqCoordApplyX->setEnabled(enableButtons);
    ui->qqCoordApplyY->setEnabled(enableButtons);
    ui->qqCoordApplyZ->setEnabled(enableButtons);
    if (alsoRadioButtons)
    {
        ui->qqCoordWorld->setEnabled(enableState);
        ui->qqCoordParent->setEnabled(enableState);
    }
}

void CQDlgTranslation::_enableTranslationPart(bool enableState,bool enableButtons,bool alsoRadioButtons)
{
    ui->qqTransfX->setEnabled(enableState);
    ui->qqTransfY->setEnabled(enableState);
    ui->qqTransfZ->setEnabled(enableState);
    ui->qqTransfApplyPos->setEnabled(enableButtons);
    ui->qqTransfApplyPosX->setEnabled(enableButtons);
    ui->qqTransfApplyPosY->setEnabled(enableButtons);
    ui->qqTransfApplyPosZ->setEnabled(enableButtons);
    if (alsoRadioButtons)
    {
        ui->qqTransfWorld->setEnabled(enableState);
        ui->qqTransfParent->setEnabled(enableState);
        ui->qqTransfOwn->setEnabled(enableState);
    }
}

void CQDlgTranslation::_enableScalingPart(bool enableState,bool enableButtons,bool alsoRadioButtons)
{
    ui->qqTransfSX->setEnabled(enableState);
    ui->qqTransfSY->setEnabled(enableState);
    ui->qqTransfSZ->setEnabled(enableState);
    ui->qqTransfApplyScale->setEnabled(enableButtons);
    ui->qqTransfApplyScaleX->setEnabled(enableButtons);
    ui->qqTransfApplyScaleY->setEnabled(enableButtons);
    ui->qqTransfApplyScaleZ->setEnabled(enableButtons);
    if (alsoRadioButtons)
    {
        ui->qqScaleWorld->setEnabled(enableState);
        ui->qqScaleParent->setEnabled(enableState);
    }
}

void CQDlgTranslation::_setDefaultValuesCoordinatePart(bool alsoRadioButtons)
{
    ui->qqCoordX->setText("");
    ui->qqCoordY->setText("");
    ui->qqCoordZ->setText("");
    if (alsoRadioButtons)
    {
        ui->qqCoordWorld->setChecked(false);
        ui->qqCoordParent->setChecked(false);
    }
}

void CQDlgTranslation::_setDefaultValuesTranslationPart(bool alsoRadioButtons)
{
    ui->qqTransfX->setText("");
    ui->qqTransfY->setText("");
    ui->qqTransfZ->setText("");
    if (alsoRadioButtons)
    {
        ui->qqTransfWorld->setChecked(false);
        ui->qqTransfParent->setChecked(false);
        ui->qqTransfOwn->setChecked(false);
    }
}

void CQDlgTranslation::_setDefaultValuesScalingPart(bool alsoRadioButtons)
{
    ui->qqTransfSX->setText("");
    ui->qqTransfSY->setText("");
    ui->qqTransfSZ->setText("");
    if (alsoRadioButtons)
    {
        ui->qqScaleWorld->setChecked(false);
        ui->qqScaleParent->setChecked(false);
    }
}

void CQDlgTranslation::_setValuesTranslationPart(bool alsoRadioButtons)
{
    ui->qqTransfX->setText(tt::getEString(true,translationValues[0],4).c_str());
    ui->qqTransfY->setText(tt::getEString(true,translationValues[1],4).c_str());
    ui->qqTransfZ->setText(tt::getEString(true,translationValues[2],4).c_str());
    if (alsoRadioButtons)
    {
        ui->qqTransfWorld->setChecked(translateMode==0);
        ui->qqTransfParent->setChecked(translateMode==1);
        ui->qqTransfOwn->setChecked(translateMode==2);
    }
}

void CQDlgTranslation::_setValuesScalingPart(bool alsoRadioButtons)
{
    ui->qqTransfSX->setText(tt::getEString(true,scalingValues[0],3).c_str());
    ui->qqTransfSY->setText(tt::getEString(true,scalingValues[1],3).c_str());
    ui->qqTransfSZ->setText(tt::getEString(true,scalingValues[2],3).c_str());
    if (alsoRadioButtons)
    {
        ui->qqScaleWorld->setChecked(scaleMode==0);
        ui->qqScaleParent->setChecked(scaleMode==1);
    }
}

bool CQDlgTranslation::_setCoord_userUnit(float newValueInUserUnit,int index)
{
    bool retVal=false;
    int editMode=App::getEditModeType();
    C3DObject* object=App::ct->objCont->getLastSelection_object();
    if ( (editMode==NO_EDIT_MODE)&&(object!=nullptr) )
    {
        C7Vector tr;
        if (coordMode==0)
            tr=object->getCumulativeTransformationPart1();
        else
            tr=object->getLocalTransformationPart1();
        tr=_getNewTransf(tr,newValueInUserUnit,index);

        SSimulationThreadCommand cmd;
        cmd.cmdId=SET_TRANSF_POSITIONTRANSLATIONGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        cmd.intParams.push_back(coordMode);
        cmd.transfParams.push_back(tr);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
    if ( (editMode&PATH_EDIT_MODE)&&(App::mainWindow->editModeContainer->getEditModeBufferSize()!=0)&&(App::mainWindow->editModeContainer->getEditModePathContainer()!=nullptr) )
    {
        CPathCont* pathCont=App::mainWindow->editModeContainer->getEditModePathContainer();
        int ind=App::mainWindow->editModeContainer->getLastEditModeBufferValue();
        CSimplePathPoint* pp=pathCont->getSimplePathPoint(ind);
        CPath* path=App::mainWindow->editModeContainer->getEditModePath();
        if ( (pp!=nullptr)&&(path!=nullptr) )
        {
            C7Vector tr(pp->getTransformation());
            if (coordMode==0)
                tr=path->getCumulativeTransformationPart1()*tr;
            tr=_getNewTransf(tr,newValueInUserUnit,index);
            if (coordMode==0)
                pp->setTransformation(path->getCumulativeTransformation().getInverse()*tr,pathCont->getAttributes());
            else
                pp->setTransformation(tr,pathCont->getAttributes());
            pathCont->actualizePath();
        }
        retVal=true;
    }
    if ( (editMode&VERTEX_EDIT_MODE)&&(App::mainWindow->editModeContainer->getEditModeBufferSize()!=0) )
    {
        int ind=App::mainWindow->editModeContainer->getLastEditModeBufferValue();
        C3Vector v(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind));
        CShape* shape=App::mainWindow->editModeContainer->getEditModeShape();
        if (shape!=nullptr)
        {
            C7Vector tr;
            tr.setIdentity();
            tr.X=v;
            if (coordMode==0)
                tr=shape->getCumulativeTransformationPart1()*tr;
            tr=_getNewTransf(tr,newValueInUserUnit,index);
            if (coordMode==0)
                tr=shape->getCumulativeTransformation().getInverse()*tr;
            App::mainWindow->editModeContainer->getShapeEditMode()->setEditionVertex(ind,tr.X);
        }
        retVal=true;
    }
    return(retVal);
}

C7Vector CQDlgTranslation::_getNewTransf(const C7Vector& transf,float newValueInUserUnit,int index)
{
    C7Vector retVal(transf);
    retVal.X(index)=newValueInUserUnit;
    return(retVal);
}

bool CQDlgTranslation::_applyCoord(int mask)
{
    bool retVal=false;
    int editMode=App::getEditModeType();
    C3DObject* object=App::ct->objCont->getLastSelection_object();
    int objSelSize=App::ct->objCont->getSelSize();
    int editObjSelSize=App::mainWindow->editModeContainer->getEditModeBufferSize();
    if ( (editMode==NO_EDIT_MODE)&&(object!=nullptr)&&(objSelSize>1) )
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_POS_POSITIONTRANSLATIONGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        cmd.intParams.push_back(coordMode);
        cmd.intParams.push_back(mask);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
    if ( (editMode&PATH_EDIT_MODE)&&(editObjSelSize>1)&&(App::mainWindow->editModeContainer->getEditModePathContainer()!=nullptr) )
    {
        CPathCont* pathCont=App::mainWindow->editModeContainer->getEditModePathContainer();
        int ind=App::mainWindow->editModeContainer->getLastEditModeBufferValue();
        CSimplePathPoint* pp=pathCont->getSimplePathPoint(ind);
        CPath* path=App::mainWindow->editModeContainer->getEditModePath();
        if ( (pp!=nullptr)&&(path!=nullptr) )
        {
            C7Vector tr(pp->getTransformation());
            if (coordMode==0)
                tr=path->getCumulativeTransformationPart1()*tr;
            for (int i=0;i<editObjSelSize-1;i++)
            {
                CSimplePathPoint* ppIt=App::mainWindow->editModeContainer->getPathEditMode()->getSimplePathPoint(i);
                if (ppIt!=nullptr)
                {
                    C7Vector trIt(ppIt->getTransformation());
                    if (coordMode==0)
                        trIt=path->getCumulativeTransformationPart1()*trIt;
                    _copyTransf(tr,trIt,mask);
                    if (coordMode==0)
                        trIt=path->getCumulativeTransformationPart1().getInverse()*trIt;
                    ppIt->setTransformation(trIt,pathCont->getAttributes());
                }
            }
            pathCont->actualizePath();
        }
        retVal=true;
    }
    if ( (editMode&VERTEX_EDIT_MODE)&&(editObjSelSize>1) )
    {
        int ind=App::mainWindow->editModeContainer->getLastEditModeBufferValue();
        C3Vector v(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind));
        CShape* shape=App::mainWindow->editModeContainer->getEditModeShape();
        if (shape!=nullptr)
        {
            C7Vector tr;
            tr.setIdentity();
            tr.X=v;
            if (coordMode==0)
                tr=shape->getCumulativeTransformationPart1()*tr;
            for (int i=0;i<editObjSelSize-1;i++)
            {
                ind=App::mainWindow->editModeContainer->getEditModeBufferValue(i);
                C7Vector trIt;
                trIt.setIdentity();
                trIt.X=App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind);
                if (coordMode==0)
                    trIt=shape->getCumulativeTransformationPart1()*trIt;
                _copyTransf(tr,trIt,mask);
                if (coordMode==0)
                    trIt=shape->getCumulativeTransformationPart1().getInverse()*trIt;
                App::mainWindow->editModeContainer->getShapeEditMode()->setEditionVertex(ind,trIt.X);
            }
        }
        retVal=true;
    }
    return(retVal);
}

void CQDlgTranslation::_copyTransf(const C7Vector& tr,C7Vector& trIt,int mask)
{
    if (mask&1)
        trIt.X(0)=tr.X(0);
    if (mask&2)
        trIt.X(1)=tr.X(1);
    if (mask&4)
        trIt.X(2)=tr.X(2);
}

bool CQDlgTranslation::_applyTranslation(int axis)
{ // axis: 0-2, or -1 for all axes
    bool retVal=false;
    int editMode=App::getEditModeType();
    int objSelSize=App::ct->objCont->getSelSize();
    int editObjSelSize=App::mainWindow->editModeContainer->getEditModeBufferSize();
    if ( (editMode==NO_EDIT_MODE)&&(objSelSize>0) )
    {
        float TX[3]={0.0,0.0,0.0};
        if (axis==-1)
        {
            TX[0]=translationValues[0];
            TX[1]=translationValues[1];
            TX[2]=translationValues[2];
        }
        else
            TX[axis]=translationValues[axis];
        SSimulationThreadCommand cmd;
        cmd.cmdId=TRANSLATESCALE_SELECTION_POSITIONTRANSLATIONGUITRIGGEREDCMD;
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        cmd.intParams.push_back(translateMode);
        cmd.intParams.push_back(1);
        cmd.floatParams.push_back(1.0);
        cmd.floatParams.push_back(1.0);
        cmd.floatParams.push_back(1.0);
        cmd.floatParams.push_back(TX[0]);
        cmd.floatParams.push_back(TX[1]);
        cmd.floatParams.push_back(TX[2]);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
    if ( (editMode&PATH_EDIT_MODE)&&(editObjSelSize>0)&&(App::mainWindow->editModeContainer->getEditModePathContainer()!=nullptr) )
    {
        CPathCont* pathCont=App::mainWindow->editModeContainer->getEditModePathContainer();
        CPath* path=App::mainWindow->editModeContainer->getEditModePath();
        for (int i=0;i<editObjSelSize;i++)
        {
            CSimplePathPoint* pp=App::mainWindow->editModeContainer->getPathEditMode()->getSimplePathPoint(i);
            if ( (pp!=nullptr)&&(path!=nullptr) )
            {
                C7Vector tr(pp->getTransformation());
                if (translateMode==0)
                    tr=path->getCumulativeTransformationPart1()*tr;
                _transformTranslation(tr,translateMode==2,axis);
                if (translateMode==0)
                    tr=path->getCumulativeTransformationPart1().getInverse()*tr;
                pp->setTransformation(tr,pathCont->getAttributes());
            }
        }
        pathCont->actualizePath();
        retVal=true;
    }
    if ( (editMode&VERTEX_EDIT_MODE)&&(editObjSelSize>0) )
    {
        CShape* shape=App::mainWindow->editModeContainer->getEditModeShape();
        if (shape!=nullptr)
        {
            for (int i=0;i<editObjSelSize;i++)
            {
                C7Vector tr;
                tr.setIdentity();
                int ind=App::mainWindow->editModeContainer->getEditModeBufferValue(i);
                tr.X=App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind);
                if (translateMode==0)
                    tr=shape->getCumulativeTransformationPart1()*tr;
                _transformTranslation(tr,translateMode==2,axis);
                if (translateMode==0)
                    tr=shape->getCumulativeTransformationPart1().getInverse()*tr;
                App::mainWindow->editModeContainer->getShapeEditMode()->setEditionVertex(ind,tr.X);
            }
        }
        retVal=true;
    }
    return(retVal);
}

bool CQDlgTranslation::_applyScaling(int axis)
{ // axis: 0-2, or -1 for all axes
    bool retVal=false;
    int editMode=App::getEditModeType();
    int objSelSize=App::ct->objCont->getSelSize();
    int editObjSelSize=App::mainWindow->editModeContainer->getEditModeBufferSize();
    if ( (editMode==NO_EDIT_MODE)&&(objSelSize>0) )
    {
        float TX[3]={1.0,1.0,1.0};
        if (axis==-1)
        {
            TX[0]=scalingValues[0];
            TX[1]=scalingValues[1];
            TX[2]=scalingValues[2];
        }
        else
            TX[axis]=scalingValues[axis];
        SSimulationThreadCommand cmd;
        cmd.cmdId=TRANSLATESCALE_SELECTION_POSITIONTRANSLATIONGUITRIGGEREDCMD;
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        cmd.intParams.push_back(scaleMode);
        cmd.intParams.push_back(2);
        cmd.floatParams.push_back(TX[0]);
        cmd.floatParams.push_back(TX[1]);
        cmd.floatParams.push_back(TX[2]);
        cmd.floatParams.push_back(0.0);
        cmd.floatParams.push_back(0.0);
        cmd.floatParams.push_back(0.0);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
    if ( (editMode&PATH_EDIT_MODE)&&(editObjSelSize>0)&&(App::mainWindow->editModeContainer->getEditModePathContainer()!=nullptr) )
    {
        CPathCont* pathCont=App::mainWindow->editModeContainer->getEditModePathContainer();
        CPath* path=App::mainWindow->editModeContainer->getEditModePath();
        for (int i=0;i<editObjSelSize;i++)
        {
            CSimplePathPoint* pp=App::mainWindow->editModeContainer->getPathEditMode()->getSimplePathPoint(i);
            if ( (pp!=nullptr)&&(path!=nullptr) )
            {
                C7Vector tr(pp->getTransformation());
                if (scaleMode==0)
                    tr=path->getCumulativeTransformationPart1()*tr;
                _transformScaling(tr,axis);
                if (scaleMode==0)
                    tr=path->getCumulativeTransformationPart1().getInverse()*tr;
                pp->setTransformation(tr,pathCont->getAttributes());
            }
        }
        pathCont->actualizePath();
        retVal=true;
    }
    if ( (editMode&VERTEX_EDIT_MODE)&&(editObjSelSize>0) )
    {
        CShape* shape=App::mainWindow->editModeContainer->getEditModeShape();
        if (shape!=nullptr)
        {
            for (int i=0;i<editObjSelSize;i++)
            {
                C7Vector tr;
                tr.setIdentity();
                int ind=App::mainWindow->editModeContainer->getEditModeBufferValue(i);
                tr.X=App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind);
                if (scaleMode==0)
                    tr=shape->getCumulativeTransformationPart1()*tr;
                _transformScaling(tr,axis);
                if (scaleMode==0)
                    tr=shape->getCumulativeTransformationPart1().getInverse()*tr;
                App::mainWindow->editModeContainer->getShapeEditMode()->setEditionVertex(ind,tr.X);
            }
        }
        retVal=true;
    }
    return(retVal);
}

void CQDlgTranslation::_transformTranslation(C7Vector& tr,bool self,int axis)
{
    float TX[3]={0.0,0.0,0.0};
    if (axis==-1)
    {
        TX[0]=translationValues[0];
        TX[1]=translationValues[1];
        TX[2]=translationValues[2];
    }
    else
        TX[axis]=translationValues[axis];
    C7Vector m;
    m.setIdentity();
    m.X.set(TX);
    if (self)
        tr=tr*m;
    else
        tr=m*tr;
}

void CQDlgTranslation::_transformScaling(C7Vector& tr,int axis)
{
    float TX[3]={1.0,1.0,1.0};
    if (axis==-1)
    {
        TX[0]=scalingValues[0];
        TX[1]=scalingValues[1];
        TX[2]=scalingValues[2];
    }
    else
        TX[axis]=scalingValues[axis];
    tr.X(0)=tr.X(0)*TX[0];
    tr.X(1)=tr.X(1)*TX[1];
    tr.X(2)=tr.X(2)*TX[2];
}


void CQDlgTranslation::on_qqCoordWorld_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        coordMode=0;
        refresh();
    }
}

void CQDlgTranslation::on_qqCoordParent_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        coordMode=1;
        refresh();
    }
}

void CQDlgTranslation::on_qqCoordX_editingFinished()
{
    if (!ui->qqCoordX->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqCoordX->text().toFloat(&ok);
        if (ok)
        {
            if (_setCoord_userUnit(newVal,0))
                refresh();
        }
    }
}

void CQDlgTranslation::on_qqCoordY_editingFinished()
{
    if (!ui->qqCoordY->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqCoordY->text().toFloat(&ok);
        if (ok)
        {
            if (_setCoord_userUnit(newVal,1))
                refresh();
        }
    }
}

void CQDlgTranslation::on_qqCoordZ_editingFinished()
{
    if (!ui->qqCoordZ->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqCoordZ->text().toFloat(&ok);
        if (ok)
        {
            if (_setCoord_userUnit(newVal,2))
                refresh();
        }
    }
}

void CQDlgTranslation::on_qqCoordApplyPos_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyCoord(1+2+4))
            refresh();
    }
}

void CQDlgTranslation::on_qqCoordApplyX_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyCoord(1))
            refresh();
    }
}

void CQDlgTranslation::on_qqCoordApplyY_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyCoord(2))
            refresh();
    }
}

void CQDlgTranslation::on_qqCoordApplyZ_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyCoord(4))
            refresh();
    }
}

void CQDlgTranslation::on_qqTransfWorld_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        translateMode=0;
        refresh();
    }
}

void CQDlgTranslation::on_qqTransfParent_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        translateMode=1;
        refresh();
    }
}

void CQDlgTranslation::on_qqTransfOwn_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        translateMode=2;
        refresh();
    }
}


void CQDlgTranslation::on_qqTransfX_editingFinished()
{
    if (!ui->qqTransfX->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqTransfX->text().toFloat(&ok);
        if (ok)
        {
            tt::limitValue(-1000000.0f,+1000000.0f,newVal);
            translationValues[0]=newVal;
        }
        refresh();
    }
}

void CQDlgTranslation::on_qqTransfY_editingFinished()
{
    if (!ui->qqTransfY->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqTransfY->text().toFloat(&ok);
        if (ok)
        {
            tt::limitValue(-1000000.0f,+1000000.0f,newVal);
            translationValues[1]=newVal;
        }
        refresh();
    }
}

void CQDlgTranslation::on_qqTransfZ_editingFinished()
{
    if (!ui->qqTransfZ->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqTransfZ->text().toFloat(&ok);
        if (ok)
        {
            tt::limitValue(-1000000.0f,+1000000.0f,newVal);
            translationValues[2]=newVal;
        }
        refresh();
    }
}

void CQDlgTranslation::on_qqTransfApplyPos_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyTranslation(-1))
            refresh();
    }
}

void CQDlgTranslation::on_qqTransfSX_editingFinished()
{
    if (!ui->qqTransfSX->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqTransfSX->text().toFloat(&ok);
        if (ok)
        {
            tt::limitValue(-1000000.0f,1000000.0f,newVal);
            scalingValues[0]=newVal;
        }
        refresh();
    }
}

void CQDlgTranslation::on_qqTransfSY_editingFinished()
{
    if (!ui->qqTransfSY->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqTransfSY->text().toFloat(&ok);
        if (ok)
        {
            tt::limitValue(-1000000.0f,1000000.0f,newVal);
            scalingValues[1]=newVal;
        }
        refresh();
    }
}

void CQDlgTranslation::on_qqTransfSZ_editingFinished()
{
    if (!ui->qqTransfSZ->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqTransfSZ->text().toFloat(&ok);
        if (ok)
        {
            tt::limitValue(-1000000.0f,1000000.0f,newVal);
            scalingValues[2]=newVal;
        }
        refresh();
    }
}

void CQDlgTranslation::on_qqTransfApplyScale_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyScaling(-1))
            refresh();
    }
}






void CQDlgTranslation::on_qqScaleWorld_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        scaleMode=0;
        refresh();
    }
}

void CQDlgTranslation::on_qqScaleParent_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        scaleMode=1;
        refresh();
    }
}

void CQDlgTranslation::on_qqTransfApplyScaleX_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyScaling(0))
            refresh();
    }
}

void CQDlgTranslation::on_qqTransfApplyScaleY_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyScaling(1))
            refresh();
    }
}

void CQDlgTranslation::on_qqTransfApplyScaleZ_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyScaling(2))
            refresh();
    }
}

void CQDlgTranslation::on_qqTransfApplyPosX_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyTranslation(0))
            refresh();
    }
}

void CQDlgTranslation::on_qqTransfApplyPosY_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyTranslation(1))
            refresh();
    }
}

void CQDlgTranslation::on_qqTransfApplyPosZ_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyTranslation(2))
            refresh();
    }
}

void CQDlgTranslation::on_tabWidget_currentChanged(int index)
{
    currentTab=index;
}

void CQDlgTranslation::on_qqPosWorld_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_POSRELATIVETO_OBJECTMANIPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),0);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTranslation::on_qqPosParent_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_POSRELATIVETO_OBJECTMANIPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),1);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTranslation::on_qqPosOwn_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_POSRELATIVETO_OBJECTMANIPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),2);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTranslation::on_qqPosX_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* object=App::ct->objCont->getLastSelection_object();
        if (object!=nullptr)
        {
            int permission=object->getObjectManipulationModePermissions();
            permission=permission^0x01;
            int low=permission&0x07;
            int high=permission&56;
            if (low==0x07)
                low=3;
            if (low==0)
                low=1;
            permission=low+high;
            App::appendSimulationThreadCommand(SET_PERMISSIONS_OBJECTMANIPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),permission);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTranslation::on_qqPosY_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* object=App::ct->objCont->getLastSelection_object();
        if (object!=nullptr)
        {
            int permission=object->getObjectManipulationModePermissions();
            permission=permission^0x02;
            int low=permission&0x07;
            int high=permission&56;
            if (low==0x07)
                low=3;
            if (low==0)
                low=2;
            permission=low+high;
            App::appendSimulationThreadCommand(SET_PERMISSIONS_OBJECTMANIPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),permission);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTranslation::on_qqPosZ_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* object=App::ct->objCont->getLastSelection_object();
        if (object!=nullptr)
        {
            int permission=object->getObjectManipulationModePermissions();
            permission=permission^0x04;
            int low=permission&0x07;
            int high=permission&56;
            if (low==0x07)
                low=6;
            if (low==0)
                low=4;
            permission=low+high;
            App::appendSimulationThreadCommand(SET_PERMISSIONS_OBJECTMANIPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),permission);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTranslation::on_qqPosCombo_activated(int index)
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_POSSTEPSIZE_OBJECTMANIPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,float(ui->qqPosCombo->itemData(index).toInt())/1000.0f);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTranslation::_selectItemOfCombobox(QComboBox* theBox,int itemData)
{
    theBox->setCurrentIndex(_getIndexOfComboboxItemWithData(theBox,itemData));
}

int CQDlgTranslation::_getIndexOfComboboxItemWithData(QComboBox* theBox,int itemData)
{
    for (int i=0;i<theBox->count();i++)
    {
        if (theBox->itemData(i).toInt()==itemData)
            return(i);
    }
    return(-1);
}
