
#include "vrepMainHeader.h"
#include "qdlgavirecorder.h"
#include "ui_qdlgavirecorder.h"
#include "tt.h"
#include "vFileDialog.h"
#include "v_repStrings.h"
#include "vVarious.h"
#include "app.h"
#include "funcDebug.h"

CQDlgAviRecorder::CQDlgAviRecorder(QWidget *parent) :
      CDlgEx(parent),
      ui(new Ui::CQDlgAviRecorder)
{
    _dlgType=AVI_RECORDER_DLG;
    ui->setupUi(this);
    inRefreshRoutine=false;
}

CQDlgAviRecorder::~CQDlgAviRecorder()
{
    delete ui;
}

void CQDlgAviRecorder::refresh()
{
    inRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();

    bool noEditMode=(App::getEditModeType()==NO_EDIT_MODE);
    bool noEditModeNoSim=noEditMode&&App::ct->simulation->isSimulationStopped();

    bool manualStarted=(App::mainWindow->simulationRecorder->getManualStart()&&App::mainWindow->simulationRecorder->getIsRecording());
    ui->recordNow->setEnabled((((!App::mainWindow->simulationRecorder->getIsRecording())&&(!App::mainWindow->simulationRecorder->getRecorderEnabled()))||manualStarted)&&noEditMode);
    if (!manualStarted)
        ui->recordNow->setText(IDS_RECORD_NOW);
    else
        ui->recordNow->setText(IDS_STOP_RECORDING);

    ui->launchAtSimulationStart->setEnabled(noEditModeNoSim&&(!App::mainWindow->simulationRecorder->getIsRecording()));
    ui->launchAtSimulationStart->setChecked(App::mainWindow->simulationRecorder->getRecorderEnabled());

    ui->recordDesktopInstead->setEnabled(noEditModeNoSim&&(!App::mainWindow->simulationRecorder->getIsRecording()));
    ui->recordDesktopInstead->setChecked(App::mainWindow->simulationRecorder->getDesktopRecording());

    ui->recordWindowInstead->setEnabled(noEditModeNoSim&&(!App::mainWindow->simulationRecorder->getIsRecording()));
    ui->recordWindowInstead->setChecked(!App::mainWindow->simulationRecorder->getDesktopRecording());

    ui->showCursor->setEnabled(noEditModeNoSim&&(!App::mainWindow->simulationRecorder->getIsRecording())&&(!App::mainWindow->simulationRecorder->getDesktopRecording()));
    ui->showCursor->setChecked(App::mainWindow->simulationRecorder->getShowCursor()&&(!App::mainWindow->simulationRecorder->getDesktopRecording()));

    ui->showButtons->setEnabled(noEditModeNoSim&&(!App::mainWindow->simulationRecorder->getIsRecording())&&App::mainWindow->simulationRecorder->getShowCursor()&&(!App::mainWindow->simulationRecorder->getDesktopRecording()));
    ui->showButtons->setChecked(App::mainWindow->simulationRecorder->getShowButtonStates()&&(!App::mainWindow->simulationRecorder->getDesktopRecording()));

    ui->selectLocation->setEnabled(noEditModeNoSim&&(!App::mainWindow->simulationRecorder->getIsRecording()));
    ui->displayedFramesVsRecordedFrame->setEnabled(noEditModeNoSim&&(!App::mainWindow->simulationRecorder->getIsRecording()));
    ui->displayedFramesVsRecordedFrame->setText(tt::getIString(false,App::mainWindow->simulationRecorder->getRecordEveryXRenderedFrame()).c_str());

    ui->autoFrameRate->setEnabled(noEditModeNoSim&&(!App::mainWindow->simulationRecorder->getIsRecording()));
    ui->autoFrameRate->setChecked(App::mainWindow->simulationRecorder->getAutoFrameRate());
    ui->frameRate->setEnabled(noEditModeNoSim&&(!App::mainWindow->simulationRecorder->getIsRecording())&&(!App::mainWindow->simulationRecorder->getAutoFrameRate()));
    ui->frameRate->setText(tt::getIString(false,App::mainWindow->simulationRecorder->getFrameRate()).c_str());

    ui->hideInfos->setEnabled(noEditModeNoSim&&(!App::mainWindow->simulationRecorder->getIsRecording())&&(!App::mainWindow->simulationRecorder->getDesktopRecording()));
    ui->hideInfos->setChecked(App::mainWindow->simulationRecorder->getHideInfoTextAndStatusBar()&&(!App::mainWindow->simulationRecorder->getDesktopRecording()));

    char userSet;
    std::string path=App::mainWindow->simulationRecorder->getPath(&userSet);
    if (userSet!=0)
        path+=".*";
    else
        path+="/recording_<date&time>.*";
    ui->fileLocation->setText(path.c_str());

    ui->qqOutputTypeCombo->clear();
    ui->qqOutputTypeCombo->setEnabled(noEditModeNoSim&&(!App::mainWindow->simulationRecorder->getIsRecording()));
    int cnt=0;
    std::string txt(App::mainWindow->simulationRecorder->getEncoderString(cnt));

    while (txt.length()!=0)
    {
        ui->qqOutputTypeCombo->addItem(txt.c_str(),QVariant(cnt));
        cnt++;
        txt=App::mainWindow->simulationRecorder->getEncoderString(cnt);
    }
    ui->qqOutputTypeCombo->setCurrentIndex(App::mainWindow->simulationRecorder->getEncoderIndex());

    /*
    int resX,resY;
    App::mainWindow->simulationRecorder->getRecordingSize(resX,resY);
    txt=tt::FNb(0,resX,false);
    txt+="x";
    txt+=tt::FNb(0,resY,false);
    ui->qqResolution->setText(txt.c_str());
*/
    selectLineEdit(lineEditToSelect);
    inRefreshRoutine=false;
}

void CQDlgAviRecorder::on_launchAtSimulationStart_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if ((!App::mainWindow->simulationRecorder->getIsRecording())&&App::ct->simulation->isSimulationStopped() )
        {
            if (App::ct->simulation->getThreadedRendering()||App::ct->simulation->getThreadedRenderingIfSimulationWasRunning())
                App::uiThread->messageBox_information(App::mainWindow,"Video Recorder","Cannot record while threaded rendering is enabled.",VMESSAGEBOX_OKELI);
            else
                App::mainWindow->simulationRecorder->setRecorderEnabled(!App::mainWindow->simulationRecorder->getRecorderEnabled());
        }
        refresh();
    }
}

void CQDlgAviRecorder::on_recordNow_clicked()
{
    FUNCTION_DEBUG;
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!App::mainWindow->simulationRecorder->getIsRecording())
        {
            if (App::ct->simulation->getThreadedRendering()||App::ct->simulation->getThreadedRenderingIfSimulationWasRunning())
                App::uiThread->messageBox_information(App::mainWindow,"Video Recorder","Cannot start recording while threaded rendering is enabled.",VMESSAGEBOX_OKELI);
            else
            {
                App::mainWindow->simulationRecorder->setRecorderEnabled(true);
                App::mainWindow->simulationRecorder->startRecording(true);
            }
        }
        else
        {
            if (App::mainWindow->simulationRecorder->getManualStart())
                App::mainWindow->simulationRecorder->stopRecording(true);
        }
        refresh();
    }
}

void CQDlgAviRecorder::on_showCursor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!App::mainWindow->simulationRecorder->getIsRecording())
            App::mainWindow->simulationRecorder->setShowCursor(!App::mainWindow->simulationRecorder->getShowCursor());
        refresh();
    }
}

void CQDlgAviRecorder::on_showButtons_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!App::mainWindow->simulationRecorder->getIsRecording())
            App::mainWindow->simulationRecorder->setShowButtonStates(!App::mainWindow->simulationRecorder->getShowButtonStates());
        refresh();
    }
}

void CQDlgAviRecorder::on_hideInfos_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!App::mainWindow->simulationRecorder->getIsRecording())
            App::mainWindow->simulationRecorder->setHideInfoTextAndStatusBar(!App::mainWindow->simulationRecorder->getHideInfoTextAndStatusBar());
        refresh();
    }
}

void CQDlgAviRecorder::on_selectLocation_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!App::mainWindow->simulationRecorder->getIsRecording())
        {
/*
            std::string initPath=App::directories->getPathFromFull(App::mainWindow->simulationRecorder->getFilenameAndPath());
            std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,strTranslate(IDSN_AVI_FILE_LOCATION),initPath,App::mainWindow->simulationRecorder->getFilenameAndPath(),false,"Various","*");
            if (filenameAndPath.length()!=0)
            {
                // Make sure we don't include the extension
                std::string _path=VVarious::splitPath_path(filenameAndPath);
                std::string _name=VVarious::splitPath_fileBase(filenameAndPath);
                filenameAndPath=_path+"/"+_name;
                App::mainWindow->simulationRecorder->setFilenameAndPath(filenameAndPath.c_str());
            }
*/
            std::string initPath=App::mainWindow->simulationRecorder->getPath(nullptr);
            std::string folder=App::uiThread->getOpenOrSaveFileName_api(sim_filedlg_type_folder,strTranslate(IDSN_AVI_FILE_LOCATION),initPath.c_str(),"","","");
            if (folder.length()!=0)
                App::mainWindow->simulationRecorder->setPath(folder.c_str());
        }
        refresh();
    }
}

void CQDlgAviRecorder::on_displayedFramesVsRecordedFrame_editingFinished()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!App::mainWindow->simulationRecorder->getIsRecording())
        {
            int newVal;
            bool ok;
            newVal=ui->displayedFramesVsRecordedFrame->text().toInt(&ok);
            if (ok)
                App::mainWindow->simulationRecorder->setRecordEveryXRenderedFrame(newVal);
        }
        refresh();
    }
}

void CQDlgAviRecorder::on_frameRate_editingFinished()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!App::mainWindow->simulationRecorder->getIsRecording())
        {
            int newVal;
            bool ok;
            newVal=ui->frameRate->text().toInt(&ok);
            if (ok)
                App::mainWindow->simulationRecorder->setFrameRate(newVal);
        }
        refresh();
    }
}

void CQDlgAviRecorder::on_qqOutputTypeCombo_currentIndexChanged(int index)
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!inRefreshRoutine)
        {
            App::mainWindow->simulationRecorder->setEncoderIndex(index);
            refresh();
        }
    }
}

void CQDlgAviRecorder::on_autoFrameRate_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!App::mainWindow->simulationRecorder->getIsRecording())
            App::mainWindow->simulationRecorder->setAutoFrameRate(!App::mainWindow->simulationRecorder->getAutoFrameRate());
        refresh();
    }
}

void CQDlgAviRecorder::on_recordDesktopInstead_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!App::mainWindow->simulationRecorder->getIsRecording())
            App::mainWindow->simulationRecorder->setDesktopRecording(true);
        refresh();
    }
}

void CQDlgAviRecorder::on_recordWindowInstead_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!App::mainWindow->simulationRecorder->getIsRecording())
            App::mainWindow->simulationRecorder->setDesktopRecording(false);
        refresh();
    }
}
