#include <qdlgshapes.h>
#include <ui_qdlgshapes.h>
#include <tt.h>
#include <gV.h>
#include <qdlgmaterial.h>
#include <mesh.h>
#include <qdlgtextures.h>
#include <qdlggeometry.h>
#include <qdlgshapedyn.h>
#include <vFileDialog.h>
#include <app.h>
#include <imgLoaderSaver.h>
#include <simStrings.h>
#include <vMessageBox.h>

CQDlgShapes::CQDlgShapes(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgShapes)
{
    _dlgType=SHAPE_DLG;
    ui->setupUi(this);
}

CQDlgShapes::~CQDlgShapes()
{
    delete ui;
}

void CQDlgShapes::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgShapes::refresh()
{
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeAndNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped();

    bool sel=App::currentWorld->sceneObjects->isLastSelectionAShape();
    bool ssel=App::currentWorld->sceneObjects->isLastSelectionASimpleShape();
    int sc=(int)App::currentWorld->sceneObjects->getShapeCountInSelection();
    int ssc=(int)App::currentWorld->sceneObjects->getSimpleShapeCountInSelection();
    bool compoundShapeDisplay=(sel&&(!ssel));
    CShape* it=App::currentWorld->sceneObjects->getLastSelectionShape();

    ui->qqEditMultishape->setEnabled(compoundShapeDisplay&&noEditModeAndNoSim);
    ui->qqEditMultishape->setVisible(compoundShapeDisplay);
    ui->qqColorGroup->setVisible(!compoundShapeDisplay);
    ui->qqOtherPropGroup->setVisible(!compoundShapeDisplay);
    ui->qqApplyColors->setVisible(!compoundShapeDisplay);
    ui->qqApplyMain->setVisible(!compoundShapeDisplay);

    ui->qqShadingAngle->setEnabled(ssel&&noEditModeAndNoSim);
    ui->qqBackfaceCulling->setEnabled(ssel&&noEditModeAndNoSim);
    ui->qqShowEdges->setEnabled(ssel&&noEditModeAndNoSim);

    ui->qqHiddenBorder->setVisible(App::userSettings->showOldDlgs);
    ui->qqWireframe->setVisible(App::userSettings->showOldDlgs);
    ui->qqWireframe->setEnabled(ssel&&noEditModeAndNoSim);
    ui->qqHiddenBorder->setEnabled(ssel&&noEditModeAndNoSim);

    ui->qqApplyColors->setEnabled(ssel&&(ssc>1)&&noEditModeAndNoSim);
    ui->qqAdjustOutsideColor->setEnabled(ssel&&noEditModeAndNoSim);
    ui->qqInvertFaces->setEnabled(sel&&noEditModeAndNoSim);

    ui->qqGeometry->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqTexture->setEnabled(ssel&&noEditModeAndNoSim);
    ui->qqDirtTexture->setEnabled((sc>0)&&noEditModeAndNoSim);
    ui->qqClearTextures->setEnabled((sc>0)&&noEditModeAndNoSim);

    ui->qqApplyMain->setEnabled(sel&&(ssc>1)&&noEditModeAndNoSim);

    ui->qqEditDynamics->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqEditDynamics->setChecked(CQDlgShapeDyn::showDynamicWindow);


    if (ssel)
    {
        ui->qqShadingAngle->setText(tt::getAngleFString(false,it->getSingleMesh()->getShadingAngle(),1).c_str());
        ui->qqBackfaceCulling->setChecked(it->getSingleMesh()->getCulling());
        ui->qqWireframe->setChecked(it->getSingleMesh()->getWireframe_OLD());
        ui->qqShowEdges->setChecked(it->getSingleMesh()->getVisibleEdges());
        ui->qqHiddenBorder->setChecked(it->getSingleMesh()->getHideEdgeBorders_OLD());
    }
    else
    {
        ui->qqShadingAngle->setText("");
        ui->qqBackfaceCulling->setChecked(false);
        ui->qqWireframe->setChecked(false);
        ui->qqShowEdges->setChecked(false);
        ui->qqHiddenBorder->setChecked(false);
    }
    selectLineEdit(lineEditToSelect);
}

void CQDlgShapes::on_qqBackfaceCulling_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_BACKFACECULLING_SHAPEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqWireframe_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_WIREFRAME_SHAPEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqInvertFaces_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(INVERT_FACES_SHAPEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqShowEdges_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWEDGES_SHAPEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqShadingAngle_editingFinished()
{
    if (!ui->qqShadingAngle->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal=ui->qqShadingAngle->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_SHADINGANGLE_SHAPEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,gv::userToRad*newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqApplyMain_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_OTHERPROP_SHAPEGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount()-1;i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqEditDynamics_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgShapeDyn::showDynamicWindow=!CQDlgShapeDyn::showDynamicWindow;
        if (App::mainWindow->dlgCont->isVisible(SHAPE_DYN_DLG)!=CQDlgShapeDyn::showDynamicWindow)
            App::mainWindow->dlgCont->toggle(SHAPE_DYN_DLG);
    }
}

void CQDlgShapes::on_qqAdjustOutsideColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_SHAPE,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,App::mainWindow);
    }
}

void CQDlgShapes::on_qqApplyColors_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_VISUALPROP_SHAPEGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount()-1;i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqTexture_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgTextures::displayDlg(TEXTURE_ID_SIMPLE_SHAPE,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,App::mainWindow);
    }
}

void CQDlgShapes::on_qqGeometry_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgGeometry::display(App::currentWorld->sceneObjects->getLastSelectionHandle(),App::mainWindow);
    }
}

void CQDlgShapes::on_qqDirtTexture_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        std::string tst(App::folders->getTexturesPath());
        std::string filenameAndPath=App::uiThread->getOpenFileName(this,0,"Loading texture...",tst.c_str(),"",true,"Image files","tga","jpg","jpeg","png","gif","bmp","tiff");
        if (filenameAndPath.length()!=0)
        {
            if (VFile::doesFileExist(filenameAndPath.c_str()))
            {
                App::folders->setTexturesPath(App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());
                int resX,resY,n;
                unsigned char* data=CImageLoaderSaver::load(filenameAndPath.c_str(),&resX,&resY,&n,0);
                bool rgba=(n==4);
                if (n<3)
                {
                    delete[] data;
                    data=nullptr;
                }
                if (data==nullptr)
                    App::uiThread->messageBox_critical(App::mainWindow,"Texture",IDS_TEXTURE_FILE_COULD_NOT_BE_LOADED,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                else
                {
                    SSimulationThreadCommand cmd;
                    cmd.cmdId=SET_QUICKTEXTURES_SHAPEGUITRIGGEREDCMD;
                    for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                        cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                    cmd.uint8Params.assign(data,data+resX*resY*n);
                    delete[] data;
                    cmd.boolParams.push_back(rgba);
                    cmd.intParams.push_back(resX);
                    cmd.intParams.push_back(resY);
                    cmd.stringParams.push_back(App::folders->getNameFromFull(filenameAndPath.c_str()));
                    App::appendSimulationThreadCommand(cmd);
                    App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                }
            }
        }
    }
}

void CQDlgShapes::on_qqClearTextures_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=CLEAR_TEXTURES_SHAPEGUITRIGGEREDCMD;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqEditMultishape_clicked()
{
    SSimulationThreadCommand cmd;
    cmd.cmdId=SHAPE_EDIT_MODE_START_EMCMD;
    App::appendSimulationThreadCommand(cmd);
}

void CQDlgShapes::on_qqHiddenBorder_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_HIDEEDGEBORDERS_SHAPEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
