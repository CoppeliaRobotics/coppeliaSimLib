
#include "vrepMainHeader.h"
#include "qdlgrotation.h"
#include "ui_qdlgrotation.h"
#include "gV.h"
#include "tt.h"
#include "app.h"
#include "v_repStrings.h"

int CQDlgRotation::coordMode=0; //0=abs,1=rel to parent
float CQDlgRotation::rotAngles[3]={0.0f,0.0f,0.0f};
int CQDlgRotation::transfMode=0; //0=abs,1=rel to parent,2=rel to self
int CQDlgRotation::currentTab=0; //0=mouse rot., 1=orient., 2=rotation

int CQDlgRotation::manipulationModePermission;
int CQDlgRotation::manipulationRotationRelativeTo;
float CQDlgRotation::manipulationRotationStepSize;
bool CQDlgRotation::objectRotationSettingsLocked;

CQDlgRotation::CQDlgRotation(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgRotation)
{
    ui->setupUi(this);
    lastLastSelectionID=-1;
}

CQDlgRotation::~CQDlgRotation()
{
    delete ui;
}

void CQDlgRotation::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgRotation::refresh()
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
        _enableTransformationPart(sel,sel,true);
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
            ui->qqCoordAlpha->setText(tt::getAngleEString(true,euler(0),4).c_str());
            ui->qqCoordBeta->setText(tt::getAngleEString(true,euler(1),4).c_str());
            ui->qqCoordGamma->setText(tt::getAngleEString(true,euler(2),4).c_str());
            ui->qqCoordWorld->setChecked(coordMode==0);
            ui->qqCoordParent->setChecked(coordMode==1);
            // Transformation part:
            _setValuesTransformationPart(true);
            // Manip part:
            manipulationModePermission=object->getObjectManipulationModePermissions();
            manipulationRotationRelativeTo=object->getObjectManipulationRotationRelativeTo();
            manipulationRotationStepSize=object->getNonDefaultRotationStepSize();
            objectRotationSettingsLocked=object->getObjectRotationSettingsLocked();
        }
        else
        {
            _setDefaultValuesCoordinatePart(true);
            _setDefaultValuesTransformationPart(true);
        }

        // Mouse manip part:
        ui->qqOrWorld->setEnabled((object!=nullptr)&&(!objectRotationSettingsLocked));
        ui->qqOrParent->setEnabled((object!=nullptr)&&(!objectRotationSettingsLocked));
        ui->qqOrOwn->setEnabled((object!=nullptr)&&(!objectRotationSettingsLocked));
        ui->qqOrA->setEnabled((object!=nullptr)&&(!objectRotationSettingsLocked));
        ui->qqOrB->setEnabled((object!=nullptr)&&(!objectRotationSettingsLocked));
        ui->qqOrG->setEnabled((object!=nullptr)&&(!objectRotationSettingsLocked));
        ui->qqOrCombo->setEnabled((object!=nullptr)&&(!objectRotationSettingsLocked));

        ui->qqOrWorld->setChecked((object!=nullptr)&&(manipulationRotationRelativeTo==0));
        ui->qqOrParent->setChecked((object!=nullptr)&&(manipulationRotationRelativeTo==1));
        ui->qqOrOwn->setChecked((object!=nullptr)&&(manipulationRotationRelativeTo==2));

        ui->qqOrA->setChecked((object!=nullptr)&&(manipulationModePermission&0x008));
        ui->qqOrB->setChecked((object!=nullptr)&&(manipulationModePermission&0x010));
        ui->qqOrG->setChecked((object!=nullptr)&&(manipulationModePermission&0x020));

        ui->qqOrCombo->clear();
        if (object!=nullptr)
        {
            ui->qqOrCombo->addItem(IDS_MANIP_NONE,QVariant(-1));
            ui->qqOrCombo->addItem(IDS_DEFAULT,QVariant(0));
            ui->qqOrCombo->addItem(tt::getAngleFString(false,1.0f*degToRad_f,1).c_str(),QVariant(1000));
            ui->qqOrCombo->addItem(tt::getAngleFString(false,2.0f*degToRad_f,1).c_str(),QVariant(2000));
            ui->qqOrCombo->addItem(tt::getAngleFString(false,5.0f*degToRad_f,1).c_str(),QVariant(5000));
            ui->qqOrCombo->addItem(tt::getAngleFString(false,10.0f*degToRad_f,1).c_str(),QVariant(10000));
            ui->qqOrCombo->addItem(tt::getAngleFString(false,15.0f*degToRad_f,1).c_str(),QVariant(15000));
            ui->qqOrCombo->addItem(tt::getAngleFString(false,30.0f*degToRad_f,1).c_str(),QVariant(30000));
            ui->qqOrCombo->addItem(tt::getAngleFString(false,45.0f*degToRad_f,1).c_str(),QVariant(45000));

            if (App::ct->simulation->isSimulationStopped())
            {
                if (object->getObjectRotationDisabledDuringNonSimulation())
                    _selectItemOfCombobox(ui->qqOrCombo,-1);
                else
                    _selectItemOfCombobox(ui->qqOrCombo,int((manipulationRotationStepSize*radToDeg_f+0.0005f)*1000.0f));
            }
            else
            {
                if (object->getObjectRotationDisabledDuringSimulation())
                    _selectItemOfCombobox(ui->qqOrCombo,-1);
                else
                    _selectItemOfCombobox(ui->qqOrCombo,int((manipulationRotationStepSize*radToDeg_f+0.0005f)*1000.0f));
            }
        }
    }
    else
    { // We are in an edit mode
        // Mouse manip part:
        ui->qqOrWorld->setEnabled(false);
        ui->qqOrParent->setEnabled(false);
        ui->qqOrOwn->setEnabled(false);
        ui->qqOrA->setEnabled(false);
        ui->qqOrB->setEnabled(false);
        ui->qqOrG->setEnabled(false);
        ui->qqOrCombo->setEnabled(false);
        ui->qqOrWorld->setChecked(false);
        ui->qqOrParent->setChecked(false);
        ui->qqOrOwn->setChecked(false);
        ui->qqOrA->setChecked(false);
        ui->qqOrB->setChecked(false);
        ui->qqOrG->setChecked(false);
        ui->qqOrCombo->clear();


        if (editMode&(BUTTON_EDIT_MODE|TRIANGLE_EDIT_MODE|EDGE_EDIT_MODE))
        {
            _enableCoordinatePart(false,false,true);
            _enableTransformationPart(false,false,true);
            _setDefaultValuesCoordinatePart(true);
            _setDefaultValuesTransformationPart(true);
        }
        else
        { // Vertex or path edit mode
            if (editMode&VERTEX_EDIT_MODE)
            {
                bool sel=(App::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
                if (transfMode==2)
                    transfMode=1;

                _enableCoordinatePart(false,false,true);
                _setDefaultValuesCoordinatePart(true);
                _enableTransformationPart(sel,sel,false);
                ui->qqTransfWorld->setEnabled(sel);
                ui->qqTransfParent->setEnabled(sel);
                ui->qqTransfOwn->setEnabled(false);
                CShape* shape=App::mainWindow->editModeContainer->getEditModeShape();
                if (sel&&(shape!=nullptr))
                {
                    // Coordinate part:
                    int ind=App::mainWindow->editModeContainer->getLastEditModeBufferValue();
                    C3Vector pos(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind));
                    if (coordMode==0)
                        pos=shape->getCumulativeTransformation()*pos;
                    ui->qqCoordWorld->setChecked(coordMode==0);
                    ui->qqCoordParent->setChecked(coordMode==1);
                    // Transformation part:
                    _setValuesTransformationPart(true);
                }
                else
                    _setDefaultValuesTransformationPart(true);
            }
            if (editMode&PATH_EDIT_MODE)
            {
                bool sel=(App::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
                bool bigSel=(App::mainWindow->editModeContainer->getEditModeBufferSize()>1);

                _enableCoordinatePart(sel,bigSel,true);
                _enableTransformationPart(sel,sel,true);
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
                        ui->qqCoordAlpha->setText(tt::getAngleEString(true,euler(0),4).c_str());
                        ui->qqCoordBeta->setText(tt::getAngleEString(true,euler(1),4).c_str());
                        ui->qqCoordGamma->setText(tt::getAngleEString(true,euler(2),4).c_str());
                        ui->qqCoordWorld->setChecked(coordMode==0);
                        ui->qqCoordParent->setChecked(coordMode==1);
                        // Transformation part:
                        _setValuesTransformationPart(true);
                    }
                    else
                        _setDefaultValuesTransformationPart(true);
                }
                else
                {
                    _setDefaultValuesCoordinatePart(true);
                    _setDefaultValuesTransformationPart(true);
                }
            }
        }
    }
    selectLineEdit(lineEditToSelect);
}

void CQDlgRotation::_enableCoordinatePart(bool enableState,bool enableButtons,bool alsoRadioButtons)
{
    ui->qqCoordAlpha->setEnabled(enableState);
    ui->qqCoordBeta->setEnabled(enableState);
    ui->qqCoordGamma->setEnabled(enableState);
    ui->qqCoordApplyOr->setEnabled(enableButtons);
    if (alsoRadioButtons)
    {
        ui->qqCoordWorld->setEnabled(enableState);
        ui->qqCoordParent->setEnabled(enableState);
    }
}

void CQDlgRotation::_enableTransformationPart(bool enableState,bool enableButtons,bool alsoRadioButtons)
{
    ui->qqTransfAlpha->setEnabled(enableState);
    ui->qqTransfBeta->setEnabled(enableState);
    ui->qqTransfGamma->setEnabled(enableState);
    ui->qqTransfApplyOr->setEnabled(enableButtons);
    ui->qqTransfApplyOrX->setEnabled(enableButtons);
    ui->qqTransfApplyOrY->setEnabled(enableButtons);
    ui->qqTransfApplyOrZ->setEnabled(enableButtons);
    if (alsoRadioButtons)
    {
        ui->qqTransfWorld->setEnabled(enableState);
        ui->qqTransfParent->setEnabled(enableState);
        ui->qqTransfOwn->setEnabled(enableState);
    }
}

void CQDlgRotation::_setDefaultValuesCoordinatePart(bool alsoRadioButtons)
{
    ui->qqCoordAlpha->setText("");
    ui->qqCoordBeta->setText("");
    ui->qqCoordGamma->setText("");
    if (alsoRadioButtons)
    {
        ui->qqCoordWorld->setChecked(false);
        ui->qqCoordParent->setChecked(false);
    }
}

void CQDlgRotation::_setDefaultValuesTransformationPart(bool alsoRadioButtons)
{
    ui->qqTransfAlpha->setText("");
    ui->qqTransfBeta->setText("");
    ui->qqTransfGamma->setText("");
    if (alsoRadioButtons)
    {
        ui->qqTransfWorld->setChecked(false);
        ui->qqTransfParent->setChecked(false);
        ui->qqTransfOwn->setChecked(false);
    }
}

void CQDlgRotation::_setValuesTransformationPart(bool alsoRadioButtons)
{
    ui->qqTransfAlpha->setText(tt::getAngleEString(true,rotAngles[0],4).c_str());
    ui->qqTransfBeta->setText(tt::getAngleEString(true,rotAngles[1],4).c_str());
    ui->qqTransfGamma->setText(tt::getAngleEString(true,rotAngles[2],4).c_str());
    if (alsoRadioButtons)
    {
        ui->qqTransfWorld->setChecked(transfMode==0);
        ui->qqTransfParent->setChecked(transfMode==1);
        ui->qqTransfOwn->setChecked(transfMode==2);
    }
}

bool CQDlgRotation::_setCoord_userUnit(float newValueInUserUnit,int index)
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

C7Vector CQDlgRotation::_getNewTransf(const C7Vector& transf,float newValueInUserUnit,int index)
{
    C7Vector retVal(transf);
    C3Vector euler(retVal.Q.getEulerAngles());
    euler(index)=newValueInUserUnit*gv::userToRad;
    retVal.Q.setEulerAngles(euler(0),euler(1),euler(2));
    return(retVal);
}

bool CQDlgRotation::_applyCoord()
{
    bool retVal=false;
    int editMode=App::getEditModeType();
    C3DObject* object=App::ct->objCont->getLastSelection_object();
    int objSelSize=App::ct->objCont->getSelSize();
    int editObjSelSize=App::mainWindow->editModeContainer->getEditModeBufferSize();
    if ( (editMode==NO_EDIT_MODE)&&(object!=nullptr)&&(objSelSize>1) )
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_OR_ORIENTATIONROTATIONGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        cmd.intParams.push_back(coordMode);
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
                    trIt.Q=tr.Q;
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
                trIt.Q=tr.Q;
                if (coordMode==0)
                    trIt=shape->getCumulativeTransformationPart1().getInverse()*trIt;
                App::mainWindow->editModeContainer->getShapeEditMode()->setEditionVertex(ind,trIt.X);
            }
        }
        retVal=true;
    }
    return(retVal);
}

bool CQDlgRotation::_applyTransformation(int axis)
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
            TX[0]=rotAngles[0];
            TX[1]=rotAngles[1];
            TX[2]=rotAngles[2];
        }
        else
            TX[axis]=rotAngles[axis];
        SSimulationThreadCommand cmd;
        cmd.cmdId=ROTATE_SELECTION_ORIENTATIONROTATIONGUITRIGGEREDCMD;
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        cmd.intParams.push_back(transfMode);
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
                if (transfMode==0)
                    tr=path->getCumulativeTransformationPart1()*tr;
                _transform(tr,transfMode==2,axis);
                if (transfMode==0)
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
                if (transfMode==0)
                    tr=shape->getCumulativeTransformationPart1()*tr;
                _transform(tr,transfMode==2,axis);
                if (transfMode==0)
                    tr=shape->getCumulativeTransformationPart1().getInverse()*tr;
                App::mainWindow->editModeContainer->getShapeEditMode()->setEditionVertex(ind,tr.X);
            }
        }
        retVal=true;
    }
    return(retVal);
}

void CQDlgRotation::_transform(C7Vector& tr,bool self,int axis)
{ // axis: 0-2, or -1 for all axes
    float TX[3]={0.0,0.0,0.0};
    if (axis==-1)
    {
        TX[0]=rotAngles[0];
        TX[1]=rotAngles[1];
        TX[2]=rotAngles[2];
    }
    else
        TX[axis]=rotAngles[axis];
    C7Vector m;
    m.setIdentity();
    m.Q.setEulerAngles(TX[0],TX[1],TX[2]);
    if (self)
        tr=tr*m;
    else
        tr=m*tr;
}


void CQDlgRotation::on_qqCoordWorld_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        coordMode=0;
        refresh();
    }
}

void CQDlgRotation::on_qqCoordParent_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        coordMode=1;
        refresh();
    }
}

void CQDlgRotation::on_qqCoordAlpha_editingFinished()
{
    if (!ui->qqCoordAlpha->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqCoordAlpha->text().toFloat(&ok);
        if (ok)
        {
            if (_setCoord_userUnit(newVal,0))
                refresh();
        }
    }
}

void CQDlgRotation::on_qqCoordBeta_editingFinished()
{
    if (!ui->qqCoordBeta->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqCoordBeta->text().toFloat(&ok);
        if (ok)
        {
            if (_setCoord_userUnit(newVal,1))
                refresh();
        }
    }
}

void CQDlgRotation::on_qqCoordGamma_editingFinished()
{
    if (!ui->qqCoordGamma->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqCoordGamma->text().toFloat(&ok);
        if (ok)
        {
            if (_setCoord_userUnit(newVal,2))
                refresh();
        }
    }
}

void CQDlgRotation::on_qqCoordApplyOr_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyCoord())
            refresh();
    }
}

void CQDlgRotation::on_qqTransfWorld_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        transfMode=0;
        refresh();
    }
}

void CQDlgRotation::on_qqTransfParent_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        transfMode=1;
        refresh();
    }
}

void CQDlgRotation::on_qqTransfOwn_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        transfMode=2;
        refresh();
    }
}

void CQDlgRotation::on_qqTransfAlpha_editingFinished()
{
    if (!ui->qqTransfAlpha->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqTransfAlpha->text().toFloat(&ok);
        if (ok)
        {
            newVal*=gv::userToRad;
            newVal=fmod(newVal,piValTimes2);
            rotAngles[0]=newVal;
        }
        refresh();
    }
}

void CQDlgRotation::on_qqTransfBeta_editingFinished()
{
    if (!ui->qqTransfBeta->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqTransfBeta->text().toFloat(&ok);
        if (ok)
        {
            newVal*=gv::userToRad;
            newVal=fmod(newVal,piValTimes2);
            rotAngles[1]=newVal;
        }
        refresh();
    }
}

void CQDlgRotation::on_qqTransfGamma_editingFinished()
{
    if (!ui->qqTransfGamma->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqTransfGamma->text().toFloat(&ok);
        if (ok)
        {
            newVal*=gv::userToRad;
            newVal=fmod(newVal,piValTimes2);
            rotAngles[2]=newVal;
        }
        refresh();
    }
}

void CQDlgRotation::on_qqTransfApplyOr_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyTransformation(-1))
            refresh();
    }
}


void CQDlgRotation::on_qqTransfApplyOrX_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyTransformation(0))
            refresh();
    }
}

void CQDlgRotation::on_qqTransfApplyOrY_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyTransformation(1))
            refresh();
    }
}

void CQDlgRotation::on_qqTransfApplyOrZ_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyTransformation(2))
            refresh();
    }
}

void CQDlgRotation::on_tabWidget_currentChanged(int index)
{
    currentTab=index;
}

void CQDlgRotation::on_qqOrWorld_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_ORRELATIVETO_OBJECTMANIPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),0);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgRotation::on_qqOrParent_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_ORRELATIVETO_OBJECTMANIPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),1);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgRotation::on_qqOrOwn_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_ORRELATIVETO_OBJECTMANIPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),2);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgRotation::on_qqOrA_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* object=App::ct->objCont->getLastSelection_object();
        if (object!=nullptr)
        {
            int permission=object->getObjectManipulationModePermissions();
            permission=(permission&0x07)|0x08;
            App::appendSimulationThreadCommand(SET_PERMISSIONS_OBJECTMANIPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),permission);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgRotation::on_qqOrB_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* object=App::ct->objCont->getLastSelection_object();
        if (object!=nullptr)
        {
            int permission=object->getObjectManipulationModePermissions();
            permission=(permission&0x07)|0x10;
            App::appendSimulationThreadCommand(SET_PERMISSIONS_OBJECTMANIPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),permission);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgRotation::on_qqOrG_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* object=App::ct->objCont->getLastSelection_object();
        if (object!=nullptr)
        {
            int permission=object->getObjectManipulationModePermissions();
            permission=(permission&0x07)|0x20;
            App::appendSimulationThreadCommand(SET_PERMISSIONS_OBJECTMANIPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),permission);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgRotation::on_qqOrCombo_activated(int index)
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_ORSTEPSIZE_OBJECTMANIPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,float(ui->qqOrCombo->itemData(index).toInt())*degToRad_f/1000.0f);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgRotation::_selectItemOfCombobox(QComboBox* theBox,int itemData)
{
    theBox->setCurrentIndex(_getIndexOfComboboxItemWithData(theBox,itemData));
}

int CQDlgRotation::_getIndexOfComboboxItemWithData(QComboBox* theBox,int itemData)
{
    for (int i=0;i<theBox->count();i++)
    {
        if (theBox->itemData(i).toInt()==itemData)
            return(i);
    }
    return(-1);
}
