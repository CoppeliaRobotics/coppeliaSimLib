#include <qdlgshapes.h>
#include <ui_qdlgshapes.h>
#include <tt.h>
#include <utils.h>
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
#include <guiApp.h>

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
    GuiApp::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgShapes::refresh()
{
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeAndNoSim=(GuiApp::getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped();

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

    ui->qqWireframe->setVisible(App::userSettings->showOldDlgs);
    ui->qqWireframe->setEnabled(ssel&&noEditModeAndNoSim);

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
        ui->qqShadingAngle->setText(utils::getAngleString(false,it->getSingleMesh()->getShadingAngle()).c_str());
        ui->qqBackfaceCulling->setChecked(it->getSingleMesh()->getCulling());
        ui->qqWireframe->setChecked(it->getSingleMesh()->getWireframe_OLD());
        ui->qqShowEdges->setChecked(it->getSingleMesh()->getVisibleEdges());
    }
    else
    {
        ui->qqShadingAngle->setText("");
        ui->qqBackfaceCulling->setChecked(false);
        ui->qqWireframe->setChecked(false);
        ui->qqShowEdges->setChecked(false);
    }
    selectLineEdit(lineEditToSelect);
}

void CQDlgShapes::on_qqBackfaceCulling_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::appendSimulationThreadCommand(TOGGLE_BACKFACECULLING_SHAPEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqWireframe_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::appendSimulationThreadCommand(TOGGLE_WIREFRAME_SHAPEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqInvertFaces_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        GuiApp::appendSimulationThreadCommand(INVERT_FACES_SHAPEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqShowEdges_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::appendSimulationThreadCommand(TOGGLE_SHOWEDGES_SHAPEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqShadingAngle_editingFinished()
{
    if (!ui->qqShadingAngle->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal=ui->qqShadingAngle->text().toDouble(&ok);
        if (ok)
        {
            GuiApp::appendSimulationThreadCommand(SET_SHADINGANGLE_SHAPEGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,degToRad*newVal);
            GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
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
        GuiApp::appendSimulationThreadCommand(cmd);
        GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqEditDynamics_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgShapeDyn::showDynamicWindow=!CQDlgShapeDyn::showDynamicWindow;
        if (GuiApp::mainWindow->dlgCont->isVisible(SHAPE_DYN_DLG)!=CQDlgShapeDyn::showDynamicWindow)
            GuiApp::mainWindow->dlgCont->toggle(SHAPE_DYN_DLG);
    }
}

void CQDlgShapes::on_qqAdjustOutsideColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_SHAPE,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,GuiApp::mainWindow);
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
        GuiApp::appendSimulationThreadCommand(cmd);
        GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqTexture_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgTextures::displayDlg(TEXTURE_ID_SIMPLE_SHAPE,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,GuiApp::mainWindow);
    }
}

void CQDlgShapes::on_qqGeometry_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgGeometry::display(App::currentWorld->sceneObjects->getLastSelectionHandle(),GuiApp::mainWindow);
    }
}

void CQDlgShapes::on_qqDirtTexture_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        std::string tst(App::folders->getTexturesPath());
        std::string filenameAndPath=GuiApp::uiThread->getOpenFileName(this,0,"Loading texture...",tst.c_str(),"",true,"Image files","tga","jpg","jpeg","png","gif","bmp","tiff");
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
                    GuiApp::uiThread->messageBox_critical(GuiApp::mainWindow,"Texture",IDS_TEXTURE_FILE_COULD_NOT_BE_LOADED,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
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
                    GuiApp::appendSimulationThreadCommand(cmd);
                    GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
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
        GuiApp::appendSimulationThreadCommand(cmd);
        GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    }
}

void CQDlgShapes::on_qqEditMultishape_clicked()
{
    SSimulationThreadCommand cmd;
    cmd.cmdId=SHAPE_EDIT_MODE_START_EMCMD;
    GuiApp::appendSimulationThreadCommand(cmd);
}

