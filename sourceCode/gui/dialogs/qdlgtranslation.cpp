#include <qdlgtranslation.h>
#include <ui_qdlgtranslation.h>
#include <tt.h>
#include <utils.h>
#include <app.h>
#include <simStrings.h>
#include <guiApp.h>

int CQDlgTranslation::coordMode = 0; // 0=abs,1=rel to parent
double CQDlgTranslation::translationValues[3] = {0.0, 0.0, 0.0};
double CQDlgTranslation::scalingValues[3] = {1.0, 1.0, 1.0};
int CQDlgTranslation::translateMode = 0; // 0=abs,1=rel to parent,2=rel to self
int CQDlgTranslation::scaleMode = 0;     // 0=abs,1=rel to parent
int CQDlgTranslation::currentTab = 0;    // 0=mouse transl., 1=pos, 2=transl., 3=scaling

int CQDlgTranslation::manipulationModePermission;
int CQDlgTranslation::manipulationTranslationRelativeTo;
double CQDlgTranslation::manipulationTranslationStepSize;
bool CQDlgTranslation::objectTranslationSettingsLocked;

CQDlgTranslation::CQDlgTranslation(QWidget* parent)
    : CDlgEx(parent), ui(new Ui::CQDlgTranslation)
{
    ui->setupUi(this);
    lastLastSelectionID = -1;
}

CQDlgTranslation::~CQDlgTranslation()
{
    delete ui;
}

void CQDlgTranslation::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    GuiApp::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgTranslation::refresh()
{
    QLineEdit* lineEditToSelect = getSelectedLineEdit();
    int editMode = GuiApp::getEditModeType();
    int lastSelID = App::currentWorld->sceneObjects->getLastSelectionHandle();
    lastLastSelectionID = lastSelID;

    ui->tabWidget->setCurrentIndex(currentTab);

    if (editMode == NO_EDIT_MODE)
    {
        bool sel = (App::currentWorld->sceneObjects->getSelectionCount() != 0);
        bool bigSel = (App::currentWorld->sceneObjects->getSelectionCount() > 1);
        _enableCoordinatePart(sel, bigSel, true);
        _enableTranslationPart(sel, sel, true);
        _enableScalingPart(sel && (scaleMode != 2), sel && (scaleMode != 2), true);
        CSceneObject* object = App::currentWorld->sceneObjects->getLastSelectionObject();
        if (sel && (object != nullptr))
        {
            // Coordinate part:
            C3Vector euler, pos;
            if (coordMode == 0)
            {
                euler = object->getCumulativeTransformation().Q.getEulerAngles();
                pos = object->getCumulativeTransformation().X;
            }
            else
            {
                euler = object->getLocalTransformation().Q.getEulerAngles();
                pos = object->getLocalTransformation().X;
            }
            ui->qqCoordX->setText(utils::getPosString(true, pos(0)).c_str());
            ui->qqCoordY->setText(utils::getPosString(true, pos(1)).c_str());
            ui->qqCoordZ->setText(utils::getPosString(true, pos(2)).c_str());
            ui->qqCoordWorld->setChecked(coordMode == 0);
            ui->qqCoordParent->setChecked(coordMode == 1);
            // Transformation part:
            _setValuesTranslationPart(true);
            _setValuesScalingPart(true);
            // Mouse manip part:
            manipulationModePermission = object->getObjectMovementPreferredAxes();
            manipulationTranslationRelativeTo = object->getObjectMovementRelativity(0);
            manipulationTranslationStepSize = object->getObjectMovementStepSize(0);
            objectTranslationSettingsLocked = (object->getObjectMovementOptions() & (16 + 64 + 128 + 256)) != 0;
        }
        else
        {
            _setDefaultValuesCoordinatePart(true);
            _setDefaultValuesTranslationPart(true);
            _setDefaultValuesScalingPart(true);
        }

        // mouse manip part:
        ui->qqPosWorld->setEnabled((object != nullptr) && (!objectTranslationSettingsLocked));
        ui->qqPosParent->setEnabled((object != nullptr) && (!objectTranslationSettingsLocked));
        ui->qqPosParent->setVisible((object != nullptr) &&
                                    (manipulationTranslationRelativeTo == 1)); // this mode is deprecated
        ui->qqPosOwn->setEnabled((object != nullptr) && (!objectTranslationSettingsLocked));
        ui->qqPosX->setEnabled((object != nullptr) && (!objectTranslationSettingsLocked));
        ui->qqPosY->setEnabled((object != nullptr) && (!objectTranslationSettingsLocked));
        ui->qqPosZ->setEnabled((object != nullptr) && (!objectTranslationSettingsLocked));
        ui->qqPosCombo->setEnabled((object != nullptr) && (!objectTranslationSettingsLocked));

        ui->qqPosWorld->setChecked((object != nullptr) && (manipulationTranslationRelativeTo == 0));
        ui->qqPosParent->setChecked((object != nullptr) && (manipulationTranslationRelativeTo == 1));
        ui->qqPosOwn->setChecked((object != nullptr) && (manipulationTranslationRelativeTo == 2));

        ui->qqPosX->setChecked((object != nullptr) && (manipulationModePermission & 0x01));
        ui->qqPosY->setChecked((object != nullptr) && (manipulationModePermission & 0x02));
        ui->qqPosZ->setChecked((object != nullptr) && (manipulationModePermission & 0x04));

        ui->qqPosCombo->clear();
        if (object != nullptr)
        {
            ui->qqPosCombo->addItem(IDS_MANIP_NONE, QVariant(-1));
            ui->qqPosCombo->addItem(IDS_DEFAULT, QVariant(0));

            ui->qqPosCombo->addItem(utils::getSizeString(false, 0.001).c_str(), QVariant(1));
            ui->qqPosCombo->addItem(utils::getSizeString(false, 0.002).c_str(), QVariant(2));
            ui->qqPosCombo->addItem(utils::getSizeString(false, 0.005).c_str(), QVariant(5));
            ui->qqPosCombo->addItem(utils::getSizeString(false, 0.01).c_str(), QVariant(10));
            ui->qqPosCombo->addItem(utils::getSizeString(false, 0.025).c_str(), QVariant(25));
            ui->qqPosCombo->addItem(utils::getSizeString(false, 0.05).c_str(), QVariant(50));
            ui->qqPosCombo->addItem(utils::getSizeString(false, 0.1).c_str(), QVariant(100));
            ui->qqPosCombo->addItem(utils::getSizeString(false, 0.25).c_str(), QVariant(250));
            ui->qqPosCombo->addItem(utils::getSizeString(false, 0.5).c_str(), QVariant(500));

            if (App::currentWorld->simulation->isSimulationStopped())
            {
                if (object->getObjectMovementOptions() & 1)
                    _selectItemOfCombobox(ui->qqPosCombo, -1);
                else
                    _selectItemOfCombobox(ui->qqPosCombo, int((manipulationTranslationStepSize + 0.0005) * 1000.0));
            }
            else
            {
                if (object->getObjectMovementOptions() & 2)
                    _selectItemOfCombobox(ui->qqPosCombo, -1);
                else
                    _selectItemOfCombobox(ui->qqPosCombo, int((manipulationTranslationStepSize + 0.0005) * 1000.0));
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
        ui->qqPosParent->setVisible(false); // this mode is deprecated
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

        if (editMode & (TRIANGLE_EDIT_MODE | EDGE_EDIT_MODE))
        {
            _enableCoordinatePart(false, false, true);
            _enableTranslationPart(false, false, true);
            _enableScalingPart(false, false, true);
            _setDefaultValuesCoordinatePart(true);
            _setDefaultValuesTranslationPart(true);
            _setDefaultValuesScalingPart(true);
        }
        else
        { // Vertex or path edit mode
            if (editMode & VERTEX_EDIT_MODE)
            {
                bool sel = (GuiApp::mainWindow->editModeContainer->getEditModeBufferSize() != 0);
                bool bigSel = (GuiApp::mainWindow->editModeContainer->getEditModeBufferSize() > 1);
                if (translateMode == 2)
                    translateMode = 1;

                _setDefaultValuesCoordinatePart(true);
                _enableCoordinatePart(sel, bigSel, true);
                _enableTranslationPart(sel, sel, false);
                _enableScalingPart(sel && (scaleMode != 2), sel && (scaleMode != 2), false);
                ui->qqTransfWorld->setEnabled(sel);
                ui->qqTransfParent->setEnabled(sel);
                ui->qqTransfOwn->setEnabled(false);
                ui->qqScaleWorld->setEnabled(sel);
                ui->qqScaleParent->setEnabled(sel);
                CShape* shape = GuiApp::mainWindow->editModeContainer->getEditModeShape();
                if (sel && (shape != nullptr))
                {
                    // Coordinate part:
                    int ind = GuiApp::mainWindow->editModeContainer->getLastEditModeBufferValue();
                    C3Vector pos(GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind));
                    if (coordMode == 0)
                        pos = shape->getFullCumulativeTransformation() * pos;
                    ui->qqCoordX->setText(utils::getPosString(true, pos(0)).c_str());
                    ui->qqCoordY->setText(utils::getPosString(true, pos(1)).c_str());
                    ui->qqCoordZ->setText(utils::getPosString(true, pos(2)).c_str());
                    ui->qqCoordWorld->setChecked(coordMode == 0);
                    ui->qqCoordParent->setChecked(coordMode == 1);
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
            if (editMode & PATH_EDIT_MODE_OLD)
            {
                bool sel = (GuiApp::mainWindow->editModeContainer->getEditModeBufferSize() != 0);
                bool bigSel = (GuiApp::mainWindow->editModeContainer->getEditModeBufferSize() > 1);

                _enableCoordinatePart(sel, bigSel, true);
                _enableTranslationPart(sel, sel, true);
                _enableScalingPart(sel && (scaleMode != 2), sel && (scaleMode != 2), true);
                if (sel)
                {
                    CPath_old* path = GuiApp::mainWindow->editModeContainer->getEditModePath_old();
                    int ind = GuiApp::mainWindow->editModeContainer->getLastEditModeBufferValue();
                    CSimplePathPoint_old* pp =
                        GuiApp::mainWindow->editModeContainer->getEditModePathContainer_old()->getSimplePathPoint(ind);
                    if (sel && (path != nullptr) && (pp != nullptr))
                    {
                        // Coordinate part:
                        C7Vector tr(pp->getTransformation());
                        if (coordMode == 0)
                            tr = path->getFullCumulativeTransformation() * tr;
                        C3Vector euler(tr.Q.getEulerAngles());
                        ui->qqCoordX->setText(utils::getPosString(true, tr.X(0)).c_str());
                        ui->qqCoordY->setText(utils::getPosString(true, tr.X(1)).c_str());
                        ui->qqCoordZ->setText(utils::getPosString(true, tr.X(2)).c_str());
                        ui->qqCoordWorld->setChecked(coordMode == 0);
                        ui->qqCoordParent->setChecked(coordMode == 1);
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

void CQDlgTranslation::_enableCoordinatePart(bool enableState, bool enableButtons, bool alsoRadioButtons)
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

void CQDlgTranslation::_enableTranslationPart(bool enableState, bool enableButtons, bool alsoRadioButtons)
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

void CQDlgTranslation::_enableScalingPart(bool enableState, bool enableButtons, bool alsoRadioButtons)
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
    ui->qqTransfX->setText(utils::getPosString(true, translationValues[0]).c_str());
    ui->qqTransfY->setText(utils::getPosString(true, translationValues[1]).c_str());
    ui->qqTransfZ->setText(utils::getPosString(true, translationValues[2]).c_str());
    if (alsoRadioButtons)
    {
        ui->qqTransfWorld->setChecked(translateMode == 0);
        ui->qqTransfParent->setChecked(translateMode == 1);
        ui->qqTransfOwn->setChecked(translateMode == 2);
    }
}

void CQDlgTranslation::_setValuesScalingPart(bool alsoRadioButtons)
{
    ui->qqTransfSX->setText(utils::getMultString(true, scalingValues[0]).c_str());
    ui->qqTransfSY->setText(utils::getMultString(true, scalingValues[1]).c_str());
    ui->qqTransfSZ->setText(utils::getMultString(true, scalingValues[2]).c_str());
    if (alsoRadioButtons)
    {
        ui->qqScaleWorld->setChecked(scaleMode == 0);
        ui->qqScaleParent->setChecked(scaleMode == 1);
    }
}

bool CQDlgTranslation::_setCoord_userUnit(double newValueInUserUnit, int index)
{
    bool retVal = false;
    int editMode = GuiApp::getEditModeType();
    CSceneObject* object = App::currentWorld->sceneObjects->getLastSelectionObject();
    if ((editMode == NO_EDIT_MODE) && (object != nullptr))
    {
        C7Vector tr;
        if (coordMode == 0)
            tr = object->getCumulativeTransformation();
        else
            tr = object->getLocalTransformation();
        tr = _getNewTransf(tr, newValueInUserUnit, index);

        SSimulationThreadCommand cmd;
        cmd.cmdId = SET_TRANSF_POSITIONTRANSLATIONGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        cmd.intParams.push_back(coordMode);
        cmd.transfParams.push_back(tr);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
    if ((editMode & PATH_EDIT_MODE_OLD) && (GuiApp::mainWindow->editModeContainer->getEditModeBufferSize() != 0) &&
        (GuiApp::mainWindow->editModeContainer->getEditModePathContainer_old() != nullptr))
    {
        CPathCont_old* pathCont = GuiApp::mainWindow->editModeContainer->getEditModePathContainer_old();
        int ind = GuiApp::mainWindow->editModeContainer->getLastEditModeBufferValue();
        CSimplePathPoint_old* pp = pathCont->getSimplePathPoint(ind);
        CPath_old* path = GuiApp::mainWindow->editModeContainer->getEditModePath_old();
        if ((pp != nullptr) && (path != nullptr))
        {
            C7Vector tr(pp->getTransformation());
            if (coordMode == 0)
                tr = path->getCumulativeTransformation() * tr;
            tr = _getNewTransf(tr, newValueInUserUnit, index);
            if (coordMode == 0)
                pp->setTransformation(path->getFullCumulativeTransformation().getInverse() * tr,
                                      pathCont->getAttributes());
            else
                pp->setTransformation(tr, pathCont->getAttributes());
            pathCont->actualizePath();
        }
        retVal = true;
    }
    if ((editMode & VERTEX_EDIT_MODE) && (GuiApp::mainWindow->editModeContainer->getEditModeBufferSize() != 0))
    {
        int ind = GuiApp::mainWindow->editModeContainer->getLastEditModeBufferValue();
        C3Vector v(GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind));
        CShape* shape = GuiApp::mainWindow->editModeContainer->getEditModeShape();
        if (shape != nullptr)
        {
            C7Vector tr;
            tr.setIdentity();
            tr.X = v;
            if (coordMode == 0)
                tr = shape->getCumulativeTransformation() * tr;
            tr = _getNewTransf(tr, newValueInUserUnit, index);
            if (coordMode == 0)
                tr = shape->getFullCumulativeTransformation().getInverse() * tr;
            GuiApp::mainWindow->editModeContainer->getShapeEditMode()->setEditionVertex(ind, tr.X);
        }
        retVal = true;
    }
    return (retVal);
}

C7Vector CQDlgTranslation::_getNewTransf(const C7Vector& transf, double newValueInUserUnit, int index)
{
    C7Vector retVal(transf);
    retVal.X(index) = newValueInUserUnit;
    return (retVal);
}

bool CQDlgTranslation::_applyCoord(int mask)
{
    bool retVal = false;
    int editMode = GuiApp::getEditModeType();
    CSceneObject* object = App::currentWorld->sceneObjects->getLastSelectionObject();
    size_t objSelSize = App::currentWorld->sceneObjects->getSelectionCount();
    int editObjSelSize = GuiApp::mainWindow->editModeContainer->getEditModeBufferSize();
    if ((editMode == NO_EDIT_MODE) && (object != nullptr) && (objSelSize > 1))
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId = APPLY_POS_POSITIONTRANSLATIONGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount() - 1; i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        cmd.intParams.push_back(coordMode);
        cmd.intParams.push_back(mask);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
    if ((editMode & PATH_EDIT_MODE_OLD) && (editObjSelSize > 1) &&
        (GuiApp::mainWindow->editModeContainer->getEditModePathContainer_old() != nullptr))
    {
        CPathCont_old* pathCont = GuiApp::mainWindow->editModeContainer->getEditModePathContainer_old();
        int ind = GuiApp::mainWindow->editModeContainer->getLastEditModeBufferValue();
        CSimplePathPoint_old* pp = pathCont->getSimplePathPoint(ind);
        CPath_old* path = GuiApp::mainWindow->editModeContainer->getEditModePath_old();
        if ((pp != nullptr) && (path != nullptr))
        {
            C7Vector tr(pp->getTransformation());
            if (coordMode == 0)
                tr = path->getCumulativeTransformation() * tr;
            for (int i = 0; i < editObjSelSize - 1; i++)
            {
                CSimplePathPoint_old* ppIt =
                    GuiApp::mainWindow->editModeContainer->getPathEditMode()->getSimplePathPoint(i);
                if (ppIt != nullptr)
                {
                    C7Vector trIt(ppIt->getTransformation());
                    if (coordMode == 0)
                        trIt = path->getCumulativeTransformation() * trIt;
                    _copyTransf(tr, trIt, mask);
                    if (coordMode == 0)
                        trIt = path->getCumulativeTransformation().getInverse() * trIt;
                    ppIt->setTransformation(trIt, pathCont->getAttributes());
                }
            }
            pathCont->actualizePath();
        }
        retVal = true;
    }
    if ((editMode & VERTEX_EDIT_MODE) && (editObjSelSize > 1))
    {
        int ind = GuiApp::mainWindow->editModeContainer->getLastEditModeBufferValue();
        C3Vector v(GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind));
        CShape* shape = GuiApp::mainWindow->editModeContainer->getEditModeShape();
        if (shape != nullptr)
        {
            C7Vector tr;
            tr.setIdentity();
            tr.X = v;
            if (coordMode == 0)
                tr = shape->getCumulativeTransformation() * tr;
            for (int i = 0; i < editObjSelSize - 1; i++)
            {
                ind = GuiApp::mainWindow->editModeContainer->getEditModeBufferValue(i);
                C7Vector trIt;
                trIt.setIdentity();
                trIt.X = GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind);
                if (coordMode == 0)
                    trIt = shape->getCumulativeTransformation() * trIt;
                _copyTransf(tr, trIt, mask);
                if (coordMode == 0)
                    trIt = shape->getCumulativeTransformation().getInverse() * trIt;
                GuiApp::mainWindow->editModeContainer->getShapeEditMode()->setEditionVertex(ind, trIt.X);
            }
        }
        retVal = true;
    }
    return (retVal);
}

void CQDlgTranslation::_copyTransf(const C7Vector& tr, C7Vector& trIt, int mask)
{
    if (mask & 1)
        trIt.X(0) = tr.X(0);
    if (mask & 2)
        trIt.X(1) = tr.X(1);
    if (mask & 4)
        trIt.X(2) = tr.X(2);
}

bool CQDlgTranslation::_applyTranslation(int axis)
{ // axis: 0-2, or -1 for all axes
    bool retVal = false;
    int editMode = GuiApp::getEditModeType();
    int objSelSize = (int)App::currentWorld->sceneObjects->getSelectionCount();
    int editObjSelSize = GuiApp::mainWindow->editModeContainer->getEditModeBufferSize();
    if ((editMode == NO_EDIT_MODE) && (objSelSize > 0))
    {
        double TX[3] = {0.0, 0.0, 0.0};
        if (axis == -1)
        {
            TX[0] = translationValues[0];
            TX[1] = translationValues[1];
            TX[2] = translationValues[2];
        }
        else
            TX[axis] = translationValues[axis];
        SSimulationThreadCommand cmd;
        cmd.cmdId = TRANSLATESCALE_SELECTION_POSITIONTRANSLATIONGUITRIGGEREDCMD;
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        cmd.intParams.push_back(translateMode);
        cmd.intParams.push_back(1);
        cmd.doubleParams.push_back(1.0);
        cmd.doubleParams.push_back(1.0);
        cmd.doubleParams.push_back(1.0);
        cmd.doubleParams.push_back(TX[0]);
        cmd.doubleParams.push_back(TX[1]);
        cmd.doubleParams.push_back(TX[2]);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
    if ((editMode & PATH_EDIT_MODE_OLD) && (editObjSelSize > 0) &&
        (GuiApp::mainWindow->editModeContainer->getEditModePathContainer_old() != nullptr))
    {
        CPathCont_old* pathCont = GuiApp::mainWindow->editModeContainer->getEditModePathContainer_old();
        CPath_old* path = GuiApp::mainWindow->editModeContainer->getEditModePath_old();
        for (int i = 0; i < editObjSelSize; i++)
        {
            CSimplePathPoint_old* pp = GuiApp::mainWindow->editModeContainer->getPathEditMode()->getSimplePathPoint(i);
            if ((pp != nullptr) && (path != nullptr))
            {
                C7Vector tr(pp->getTransformation());
                if (translateMode == 0)
                    tr = path->getCumulativeTransformation() * tr;
                _transformTranslation(tr, translateMode == 2, axis);
                if (translateMode == 0)
                    tr = path->getCumulativeTransformation().getInverse() * tr;
                pp->setTransformation(tr, pathCont->getAttributes());
            }
        }
        pathCont->actualizePath();
        retVal = true;
    }
    if ((editMode & VERTEX_EDIT_MODE) && (editObjSelSize > 0))
    {
        CShape* shape = GuiApp::mainWindow->editModeContainer->getEditModeShape();
        if (shape != nullptr)
        {
            for (int i = 0; i < editObjSelSize; i++)
            {
                C7Vector tr;
                tr.setIdentity();
                int ind = GuiApp::mainWindow->editModeContainer->getEditModeBufferValue(i);
                tr.X = GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind);
                if (translateMode == 0)
                    tr = shape->getCumulativeTransformation() * tr;
                _transformTranslation(tr, translateMode == 2, axis);
                if (translateMode == 0)
                    tr = shape->getCumulativeTransformation().getInverse() * tr;
                GuiApp::mainWindow->editModeContainer->getShapeEditMode()->setEditionVertex(ind, tr.X);
            }
        }
        retVal = true;
    }
    return (retVal);
}

bool CQDlgTranslation::_applyScaling(int axis)
{ // axis: 0-2, or -1 for all axes
    bool retVal = false;
    int editMode = GuiApp::getEditModeType();
    size_t objSelSize = App::currentWorld->sceneObjects->getSelectionCount();
    int editObjSelSize = GuiApp::mainWindow->editModeContainer->getEditModeBufferSize();
    if ((editMode == NO_EDIT_MODE) && (objSelSize > 0))
    {
        double TX[3] = {1.0, 1.0, 1.0};
        if (axis == -1)
        {
            TX[0] = scalingValues[0];
            TX[1] = scalingValues[1];
            TX[2] = scalingValues[2];
        }
        else
            TX[axis] = scalingValues[axis];
        SSimulationThreadCommand cmd;
        cmd.cmdId = TRANSLATESCALE_SELECTION_POSITIONTRANSLATIONGUITRIGGEREDCMD;
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        cmd.intParams.push_back(scaleMode);
        cmd.intParams.push_back(2);
        cmd.doubleParams.push_back(TX[0]);
        cmd.doubleParams.push_back(TX[1]);
        cmd.doubleParams.push_back(TX[2]);
        cmd.doubleParams.push_back(0.0);
        cmd.doubleParams.push_back(0.0);
        cmd.doubleParams.push_back(0.0);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
    if ((editMode & PATH_EDIT_MODE_OLD) && (editObjSelSize > 0) &&
        (GuiApp::mainWindow->editModeContainer->getEditModePathContainer_old() != nullptr))
    {
        CPathCont_old* pathCont = GuiApp::mainWindow->editModeContainer->getEditModePathContainer_old();
        CPath_old* path = GuiApp::mainWindow->editModeContainer->getEditModePath_old();
        for (int i = 0; i < editObjSelSize; i++)
        {
            CSimplePathPoint_old* pp = GuiApp::mainWindow->editModeContainer->getPathEditMode()->getSimplePathPoint(i);
            if ((pp != nullptr) && (path != nullptr))
            {
                C7Vector tr(pp->getTransformation());
                if (scaleMode == 0)
                    tr = path->getCumulativeTransformation() * tr;
                _transformScaling(tr, axis);
                if (scaleMode == 0)
                    tr = path->getCumulativeTransformation().getInverse() * tr;
                pp->setTransformation(tr, pathCont->getAttributes());
            }
        }
        pathCont->actualizePath();
        retVal = true;
    }
    if ((editMode & VERTEX_EDIT_MODE) && (editObjSelSize > 0))
    {
        CShape* shape = GuiApp::mainWindow->editModeContainer->getEditModeShape();
        if (shape != nullptr)
        {
            for (int i = 0; i < editObjSelSize; i++)
            {
                C7Vector tr;
                tr.setIdentity();
                int ind = GuiApp::mainWindow->editModeContainer->getEditModeBufferValue(i);
                tr.X = GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind);
                if (scaleMode == 0)
                    tr = shape->getCumulativeTransformation() * tr;
                _transformScaling(tr, axis);
                if (scaleMode == 0)
                    tr = shape->getCumulativeTransformation().getInverse() * tr;
                GuiApp::mainWindow->editModeContainer->getShapeEditMode()->setEditionVertex(ind, tr.X);
            }
        }
        retVal = true;
    }
    return (retVal);
}

void CQDlgTranslation::_transformTranslation(C7Vector& tr, bool self, int axis)
{
    double TX[3] = {0.0, 0.0, 0.0};
    if (axis == -1)
    {
        TX[0] = translationValues[0];
        TX[1] = translationValues[1];
        TX[2] = translationValues[2];
    }
    else
        TX[axis] = translationValues[axis];
    C7Vector m;
    m.setIdentity();
    m.X.setData(TX);
    if (self)
        tr = tr * m;
    else
        tr = m * tr;
}

void CQDlgTranslation::_transformScaling(C7Vector& tr, int axis)
{
    double TX[3] = {1.0, 1.0, 1.0};
    if (axis == -1)
    {
        TX[0] = scalingValues[0];
        TX[1] = scalingValues[1];
        TX[2] = scalingValues[2];
    }
    else
        TX[axis] = scalingValues[axis];
    tr.X(0) = tr.X(0) * TX[0];
    tr.X(1) = tr.X(1) * TX[1];
    tr.X(2) = tr.X(2) * TX[2];
}

void CQDlgTranslation::on_qqCoordWorld_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        coordMode = 0;
        refresh();
    }
}

void CQDlgTranslation::on_qqCoordParent_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        coordMode = 1;
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
        double newVal = GuiApp::getEvalDouble(ui->qqCoordX->text().toStdString().c_str(), &ok);
        if (ok)
            _setCoord_userUnit(newVal, 0);
        refresh();
    }
}

void CQDlgTranslation::on_qqCoordY_editingFinished()
{
    if (!ui->qqCoordY->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqCoordY->text().toStdString().c_str(), &ok);
        if (ok)
            _setCoord_userUnit(newVal, 1);
        refresh();
    }
}

void CQDlgTranslation::on_qqCoordZ_editingFinished()
{
    if (!ui->qqCoordZ->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqCoordZ->text().toStdString().c_str(), &ok);
        if (ok)
            _setCoord_userUnit(newVal, 2);
        refresh();
    }
}

void CQDlgTranslation::on_qqCoordApplyPos_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (_applyCoord(1 + 2 + 4))
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
        translateMode = 0;
        refresh();
    }
}

void CQDlgTranslation::on_qqTransfParent_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        translateMode = 1;
        refresh();
    }
}

void CQDlgTranslation::on_qqTransfOwn_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        translateMode = 2;
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
        double newVal = GuiApp::getEvalDouble(ui->qqTransfX->text().toStdString().c_str(), &ok);
        if (ok)
        {
            tt::limitValue(-1000000.0, +1000000.0, newVal);
            translationValues[0] = newVal;
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
        double newVal = GuiApp::getEvalDouble(ui->qqTransfY->text().toStdString().c_str(), &ok);
        if (ok)
        {
            tt::limitValue(-1000000.0, +1000000.0, newVal);
            translationValues[1] = newVal;
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
        double newVal = GuiApp::getEvalDouble(ui->qqTransfZ->text().toStdString().c_str(), &ok);
        if (ok)
        {
            tt::limitValue(-1000000.0, +1000000.0, newVal);
            translationValues[2] = newVal;
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
        double newVal = GuiApp::getEvalDouble(ui->qqTransfSX->text().toStdString().c_str(), &ok);
        if (ok)
        {
            tt::limitValue(-1000000.0, 1000000.0, newVal);
            scalingValues[0] = newVal;
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
        double newVal = GuiApp::getEvalDouble(ui->qqTransfSY->text().toStdString().c_str(), &ok);
        if (ok)
        {
            tt::limitValue(-1000000.0, 1000000.0, newVal);
            scalingValues[1] = newVal;
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
        double newVal = GuiApp::getEvalDouble(ui->qqTransfSZ->text().toStdString().c_str(), &ok);
        if (ok)
        {
            tt::limitValue(-1000000.0, 1000000.0, newVal);
            scalingValues[2] = newVal;
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
        scaleMode = 0;
        refresh();
    }
}

void CQDlgTranslation::on_qqScaleParent_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        scaleMode = 1;
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
    currentTab = index;
}

void CQDlgTranslation::on_qqPosWorld_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_POSRELATIVETO_OBJECTMANIPGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle(), 0);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTranslation::on_qqPosParent_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_POSRELATIVETO_OBJECTMANIPGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle(), 1);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTranslation::on_qqPosOwn_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_POSRELATIVETO_OBJECTMANIPGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle(), 2);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTranslation::on_qqPosX_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        CSceneObject* object = App::currentWorld->sceneObjects->getLastSelectionObject();
        if (object != nullptr)
        {
            int permission = object->getObjectMovementPreferredAxes();
            permission = permission ^ 0x01;
            int low = permission & 0x07;
            int high = permission & 56;
            if (low == 0x07)
                low = 3;
            if (low == 0)
                low = 1;
            permission = low + high;
            App::appendSimulationThreadCommand(SET_PERMISSIONS_OBJECTMANIPGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), permission);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTranslation::on_qqPosY_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        CSceneObject* object = App::currentWorld->sceneObjects->getLastSelectionObject();
        if (object != nullptr)
        {
            int permission = object->getObjectMovementPreferredAxes();
            permission = permission ^ 0x02;
            int low = permission & 0x07;
            int high = permission & 56;
            if (low == 0x07)
                low = 3;
            if (low == 0)
                low = 2;
            permission = low + high;
            App::appendSimulationThreadCommand(SET_PERMISSIONS_OBJECTMANIPGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), permission);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTranslation::on_qqPosZ_clicked()
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        CSceneObject* object = App::currentWorld->sceneObjects->getLastSelectionObject();
        if (object != nullptr)
        {
            int permission = object->getObjectMovementPreferredAxes();
            permission = permission ^ 0x04;
            int low = permission & 0x07;
            int high = permission & 56;
            if (low == 0x07)
                low = 6;
            if (low == 0)
                low = 4;
            permission = low + high;
            App::appendSimulationThreadCommand(SET_PERMISSIONS_OBJECTMANIPGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), permission);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTranslation::on_qqPosCombo_activated(int index)
{ // mouse manip
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_POSSTEPSIZE_OBJECTMANIPGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle(), -1,
                                           double(ui->qqPosCombo->itemData(index).toInt()) / 1000.0);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTranslation::_selectItemOfCombobox(QComboBox* theBox, int itemData)
{
    theBox->setCurrentIndex(_getIndexOfComboboxItemWithData(theBox, itemData));
}

int CQDlgTranslation::_getIndexOfComboboxItemWithData(QComboBox* theBox, int itemData)
{
    for (int i = 0; i < theBox->count(); i++)
    {
        if (theBox->itemData(i).toInt() == itemData)
            return (i);
    }
    return (-1);
}
