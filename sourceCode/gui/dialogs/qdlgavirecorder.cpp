#include <qdlgavirecorder.h>
#include <ui_qdlgavirecorder.h>
#include <tt.h>
#include <utils.h>
#include <vFileDialog.h>
#include <simStrings.h>
#include <vVarious.h>
#include <app.h>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

CQDlgAviRecorder::CQDlgAviRecorder(QWidget* parent)
    : CDlgEx(parent), ui(new Ui::CQDlgAviRecorder)
{
    _dlgType = AVI_RECORDER_DLG;
    ui->setupUi(this);
    inRefreshRoutine = false;
}

CQDlgAviRecorder::~CQDlgAviRecorder()
{
    delete ui;
}

void CQDlgAviRecorder::refresh()
{
    inRefreshRoutine = true;
    QLineEdit* lineEditToSelect = getSelectedLineEdit();

    bool noEditMode = (GuiApp::getEditModeType() == NO_EDIT_MODE);
    bool noEditModeNoSim = noEditMode && App::currentWorld->simulation->isSimulationStopped();

    bool manualStarted = (GuiApp::mainWindow->simulationRecorder->getManualStart() &&
                          GuiApp::mainWindow->simulationRecorder->getIsRecording());
    ui->recordNow->setEnabled((((!GuiApp::mainWindow->simulationRecorder->getIsRecording()) &&
                                (!GuiApp::mainWindow->simulationRecorder->getRecorderEnabled())) ||
                               manualStarted) &&
                              noEditMode);
    if (!manualStarted)
        ui->recordNow->setText(IDS_RECORD_NOW);
    else
        ui->recordNow->setText(IDS_STOP_RECORDING);

    ui->launchAtSimulationStart->setEnabled(noEditModeNoSim &&
                                            (!GuiApp::mainWindow->simulationRecorder->getIsRecording()));
    ui->launchAtSimulationStart->setChecked(GuiApp::mainWindow->simulationRecorder->getRecorderEnabled());

    ui->recordDesktopInstead->setEnabled(noEditModeNoSim &&
                                         (!GuiApp::mainWindow->simulationRecorder->getIsRecording()));
    ui->recordDesktopInstead->setChecked(GuiApp::mainWindow->simulationRecorder->getDesktopRecording());

    ui->recordWindowInstead->setEnabled(noEditModeNoSim && (!GuiApp::mainWindow->simulationRecorder->getIsRecording()));
    ui->recordWindowInstead->setChecked(!GuiApp::mainWindow->simulationRecorder->getDesktopRecording());

    ui->showCursor->setEnabled(noEditModeNoSim && (!GuiApp::mainWindow->simulationRecorder->getIsRecording()) &&
                               (!GuiApp::mainWindow->simulationRecorder->getDesktopRecording()));
    ui->showCursor->setChecked(GuiApp::mainWindow->simulationRecorder->getShowCursor() &&
                               (!GuiApp::mainWindow->simulationRecorder->getDesktopRecording()));

    ui->showButtons->setEnabled(noEditModeNoSim && (!GuiApp::mainWindow->simulationRecorder->getIsRecording()) &&
                                GuiApp::mainWindow->simulationRecorder->getShowCursor() &&
                                (!GuiApp::mainWindow->simulationRecorder->getDesktopRecording()));
    ui->showButtons->setChecked(GuiApp::mainWindow->simulationRecorder->getShowButtonStates() &&
                                (!GuiApp::mainWindow->simulationRecorder->getDesktopRecording()));

    ui->selectLocation->setEnabled(noEditModeNoSim && (!GuiApp::mainWindow->simulationRecorder->getIsRecording()));
    ui->displayedFramesVsRecordedFrame->setEnabled(noEditModeNoSim &&
                                                   (!GuiApp::mainWindow->simulationRecorder->getIsRecording()));
    ui->displayedFramesVsRecordedFrame->setText(
        utils::getIntString(false, GuiApp::mainWindow->simulationRecorder->getRecordEveryXRenderedFrame()).c_str());

    ui->autoFrameRate->setEnabled(noEditModeNoSim && (!GuiApp::mainWindow->simulationRecorder->getIsRecording()));
    ui->autoFrameRate->setChecked(GuiApp::mainWindow->simulationRecorder->getAutoFrameRate());
    ui->frameRate->setEnabled(noEditModeNoSim && (!GuiApp::mainWindow->simulationRecorder->getIsRecording()) &&
                              (!GuiApp::mainWindow->simulationRecorder->getAutoFrameRate()));
    ui->frameRate->setText(utils::getIntString(false, GuiApp::mainWindow->simulationRecorder->getFrameRate()).c_str());

    ui->hideInfos->setEnabled(noEditModeNoSim && (!GuiApp::mainWindow->simulationRecorder->getIsRecording()) &&
                              (!GuiApp::mainWindow->simulationRecorder->getDesktopRecording()));
    ui->hideInfos->setChecked(GuiApp::mainWindow->simulationRecorder->getHideInfoTextAndStatusBar() &&
                              (!GuiApp::mainWindow->simulationRecorder->getDesktopRecording()));

    char userSet;
    std::string path = GuiApp::mainWindow->simulationRecorder->getPath(&userSet);
    if (userSet != 0)
        path += ".*";
    else
        path += "/recording_<date&time>.*";
    ui->fileLocation->setText(path.c_str());

    ui->qqOutputTypeCombo->clear();
    ui->qqOutputTypeCombo->setEnabled(noEditModeNoSim && (!GuiApp::mainWindow->simulationRecorder->getIsRecording()));
    int cnt = 0;
    std::string txt(GuiApp::mainWindow->simulationRecorder->getEncoderString(cnt));

    while (txt.length() != 0)
    {
        ui->qqOutputTypeCombo->addItem(txt.c_str(), QVariant(cnt));
        cnt++;
        txt = GuiApp::mainWindow->simulationRecorder->getEncoderString(cnt);
    }
    ui->qqOutputTypeCombo->setCurrentIndex(GuiApp::mainWindow->simulationRecorder->getEncoderIndex());

    selectLineEdit(lineEditToSelect);
    inRefreshRoutine = false;
}

void CQDlgAviRecorder::on_launchAtSimulationStart_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if ((!GuiApp::mainWindow->simulationRecorder->getIsRecording()) &&
            App::currentWorld->simulation->isSimulationStopped())
            GuiApp::mainWindow->simulationRecorder->setRecorderEnabled(
                !GuiApp::mainWindow->simulationRecorder->getRecorderEnabled());
        refresh();
    }
}

void CQDlgAviRecorder::on_recordNow_clicked()
{
    TRACE_INTERNAL;
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!GuiApp::mainWindow->simulationRecorder->getIsRecording())
        {
            GuiApp::mainWindow->simulationRecorder->setRecorderEnabled(true);
            GuiApp::mainWindow->simulationRecorder->startRecording(true);
        }
        else
        {
            if (GuiApp::mainWindow->simulationRecorder->getManualStart())
                GuiApp::mainWindow->simulationRecorder->stopRecording(true);
        }
        refresh();
    }
}

void CQDlgAviRecorder::on_showCursor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!GuiApp::mainWindow->simulationRecorder->getIsRecording())
            GuiApp::mainWindow->simulationRecorder->setShowCursor(
                !GuiApp::mainWindow->simulationRecorder->getShowCursor());
        refresh();
    }
}

void CQDlgAviRecorder::on_showButtons_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!GuiApp::mainWindow->simulationRecorder->getIsRecording())
            GuiApp::mainWindow->simulationRecorder->setShowButtonStates(
                !GuiApp::mainWindow->simulationRecorder->getShowButtonStates());
        refresh();
    }
}

void CQDlgAviRecorder::on_hideInfos_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!GuiApp::mainWindow->simulationRecorder->getIsRecording())
            GuiApp::mainWindow->simulationRecorder->setHideInfoTextAndStatusBar(
                !GuiApp::mainWindow->simulationRecorder->getHideInfoTextAndStatusBar());
        refresh();
    }
}

void CQDlgAviRecorder::on_selectLocation_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!GuiApp::mainWindow->simulationRecorder->getIsRecording())
        {
            /*
                        std::string
               initPath=App::directories->getPathFromFull(GuiApp::mainWindow->simulationRecorder->getFilenameAndPath());
                        std::string
               filenameAndPath=GuiApp::uiThread->getSaveFileName(GuiApp::mainWindow,0,IDSN_AVI_FILE_LOCATION,initPath,GuiApp::mainWindow->simulationRecorder->getFilenameAndPath(),false,"Various","*");
                        if (filenameAndPath.length()!=0)
                        {
                            // Make sure we don't include the extension
                            std::string _path=VVarious::splitPath_path(filenameAndPath);
                            std::string _name=VVarious::splitPath_fileBase(filenameAndPath);
                            filenameAndPath=_path+"/"+_name;
                            GuiApp::mainWindow->simulationRecorder->setFilenameAndPath(filenameAndPath.c_str());
                        }
            */
            std::string initPath = GuiApp::mainWindow->simulationRecorder->getPath(nullptr);
            std::string folder = GuiApp::uiThread->getOpenOrSaveFileName_api(
                sim_filedlg_type_folder, IDSN_AVI_FILE_LOCATION, initPath.c_str(), "", "", "");
            if (folder.length() != 0)
                GuiApp::mainWindow->simulationRecorder->setPath(folder.c_str());
        }
        refresh();
    }
}

void CQDlgAviRecorder::on_displayedFramesVsRecordedFrame_editingFinished()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!GuiApp::mainWindow->simulationRecorder->getIsRecording())
        {
            int newVal;
            bool ok;
            newVal = (int)GuiApp::getEvalInt(ui->displayedFramesVsRecordedFrame->text().toStdString().c_str(), &ok);
            if (ok)
                GuiApp::mainWindow->simulationRecorder->setRecordEveryXRenderedFrame(newVal);
        }
        refresh();
    }
}

void CQDlgAviRecorder::on_frameRate_editingFinished()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!GuiApp::mainWindow->simulationRecorder->getIsRecording())
        {
            int newVal;
            bool ok;
            newVal = (int)GuiApp::getEvalInt(ui->frameRate->text().toStdString().c_str(), &ok);
            if (ok)
                GuiApp::mainWindow->simulationRecorder->setFrameRate(newVal);
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
            GuiApp::mainWindow->simulationRecorder->setEncoderIndex(index);
            refresh();
        }
    }
}

void CQDlgAviRecorder::on_autoFrameRate_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!GuiApp::mainWindow->simulationRecorder->getIsRecording())
            GuiApp::mainWindow->simulationRecorder->setAutoFrameRate(
                !GuiApp::mainWindow->simulationRecorder->getAutoFrameRate());
        refresh();
    }
}

void CQDlgAviRecorder::on_recordDesktopInstead_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!GuiApp::mainWindow->simulationRecorder->getIsRecording())
            GuiApp::mainWindow->simulationRecorder->setDesktopRecording(true);
        refresh();
    }
}

void CQDlgAviRecorder::on_recordWindowInstead_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!GuiApp::mainWindow->simulationRecorder->getIsRecording())
            GuiApp::mainWindow->simulationRecorder->setDesktopRecording(false);
        refresh();
    }
}
