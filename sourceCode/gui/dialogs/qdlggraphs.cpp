
#include "vrepMainHeader.h"
#include "qdlggraphs.h"
#include "ui_qdlggraphs.h"
#include "tt.h"
#include "gV.h"
#include "qdlgmaterial.h"
#include "qdlgdatastreamselection.h"
#include "graphingRoutines.h"
#include "editboxdelegate.h"
#include "qdlgcolor.h"
#include "qdlg2d3dgraphproperties.h"
#include "v_repStrings.h"
#include "app.h"
#include "vMessageBox.h"

CQDlgGraphs::CQDlgGraphs(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgGraphs)
{
    _dlgType=GRAPH_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
    inListSelectionRoutine=false;
    noListSelectionAllowed=false;
    delKeyShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(delKeyShortcut,SIGNAL(activated()), this, SLOT(onDeletePressed()));
    backspaceKeyShortcut = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
    connect(backspaceKeyShortcut,SIGNAL(activated()), this, SLOT(onDeletePressed()));
    CEditBoxDelegate* delegate=new CEditBoxDelegate();
    ui->qqRecordingList->setItemDelegate(delegate);
}

CQDlgGraphs::~CQDlgGraphs()
{
    delete ui;
}

void CQDlgGraphs::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgGraphs::dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    if ( (cmdIn!=nullptr)&&(cmdIn->intParams[0]==_dlgType) )
    {
        if (cmdIn->intParams[1]==0)
            selectObjectInList(cmdIn->intParams[2]);
    }
}

void CQDlgGraphs::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeAndNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();

    bool sel=App::ct->objCont->isLastSelectionAGraph();

    int streamId=-1;
    CGraph* it=nullptr;
    CGraphData* graphData=nullptr;
    if (sel)
    {
        it=App::ct->objCont->getLastSelection_graph();
        streamId=getSelectedObjectID();
        graphData=it->getGraphData(streamId);
    }

    ui->qqExplicitHandling->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqObjectSize->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqBufferIsCyclic->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqBufferSize->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqShowXYZPlanes->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqRemoveAllStatics->setEnabled(sel&&noEditModeAndNoSim);

    ui->qqAdjustBackgroundColor->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqAdjustGridColor->setEnabled(sel&&noEditModeAndNoSim);

    ui->qqAddNewDataStream->setEnabled(sel&&noEditModeAndNoSim);

    ui->qqRecordingList->setEnabled(sel&&noEditModeAndNoSim);

    ui->qqTransformationCombo->setEnabled(sel&&noEditModeAndNoSim&&(graphData!=nullptr));
    ui->qqTransformationCoeff->setEnabled(sel&&noEditModeAndNoSim&&(graphData!=nullptr));
    ui->qqTransformationOffset->setEnabled(sel&&noEditModeAndNoSim&&(graphData!=nullptr));
    ui->qqMovingAveragePeriod->setEnabled(sel&&noEditModeAndNoSim&&(graphData!=nullptr));

    ui->qqTimeGraphVisible->setEnabled(sel&&noEditModeAndNoSim&&(graphData!=nullptr));
    ui->qqShowLabel->setEnabled(sel&&noEditModeAndNoSim&&(graphData!=nullptr));
    ui->qqLinkPoints->setEnabled(sel&&noEditModeAndNoSim&&(graphData!=nullptr));
    ui->qqAdjustCurveColor->setEnabled(sel&&noEditModeAndNoSim&&(graphData!=nullptr));
    ui->qqDuplicateToStatic->setEnabled(sel&&noEditModeAndNoSim&&(graphData!=nullptr));

    ui->qqEditXYGraphs->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqEdit3DCurves->setEnabled(sel&&noEditModeAndNoSim);

    ui->qqExplicitHandling->setChecked(sel&&it->getExplicitHandling());
    ui->qqBufferIsCyclic->setChecked(sel&&it->getCyclic());
    ui->qqShowXYZPlanes->setChecked(sel&&it->xYZPlanesDisplay);

    ui->qqTimeGraphVisible->setChecked(sel&&(graphData!=nullptr)&&graphData->getVisible());
    ui->qqShowLabel->setChecked(sel&&(graphData!=nullptr)&&graphData->getLabel());
    ui->qqLinkPoints->setChecked(sel&&(graphData!=nullptr)&&graphData->getLinkPoints());

    ui->qqTransformationCombo->clear();

    if (!inListSelectionRoutine)
    {
        updateObjectsInList();
        selectObjectInList(streamId);
    }

    if (sel)
    {
        ui->qqObjectSize->setText(tt::getFString(false,it->getSize(),3).c_str());
        ui->qqBufferSize->setText(tt::getIString(false,it->getBufferSize()).c_str());

        ui->qqTransformationCombo->clear();
        ui->qqTransformationCombo->addItem(strTranslate(IDS_RAW),QVariant(DATA_STREAM_ORIGINAL));
        ui->qqTransformationCombo->addItem(strTranslate(IDS_DERIVATIVE),QVariant(DATA_STREAM_DERIVATIVE));
        ui->qqTransformationCombo->addItem(strTranslate(IDS_INTEGRAL),QVariant(DATA_STREAM_INTEGRAL));
        ui->qqTransformationCombo->addItem(strTranslate(IDS_CUMULATIVE),QVariant(DATA_STREAM_CUMULATIVE));

        if (graphData!=nullptr)
        {
            for (int i=0;i<ui->qqTransformationCombo->count();i++)
            {
                if (ui->qqTransformationCombo->itemData(i).toInt()==graphData->getDerivativeIntegralAndCumulative())
                {
                    ui->qqTransformationCombo->setCurrentIndex(i);
                    break;
                }
            }

            ui->qqTransformationCoeff->setText(tt::getEString(false,graphData->getZoomFactor(),3).c_str());
            ui->qqTransformationOffset->setText(tt::getEString(false,graphData->getAddCoeff(),3).c_str());
            ui->qqMovingAveragePeriod->setText(tt::getIString(false,graphData->getMovingAverageCount()).c_str());
        }
        else
        {
            ui->qqTransformationCoeff->setText("");
            ui->qqTransformationOffset->setText("");
            ui->qqMovingAveragePeriod->setText("");
        }
    }
    else
    {
        ui->qqObjectSize->setText("");
        ui->qqBufferSize->setText("");

        ui->qqTransformationCoeff->setText("");
        ui->qqTransformationOffset->setText("");
        ui->qqMovingAveragePeriod->setText("");
    }
    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}


void CQDlgGraphs::updateObjectsInList()
{
    noListSelectionAllowed=true;
    ui->qqRecordingList->clear();
    noListSelectionAllowed=false;
    if (!App::ct->objCont->isLastSelectionAGraph())
        return;
    CGraph* it=App::ct->objCont->getLastSelection_graph();
    noListSelectionAllowed=true;
    for (size_t i=0;i<it->daten.size();i++)
    {
        int dataType=it->daten[i]->getDataType();
        int dataObjectID=it->daten[i]->getDataObjectID();
        int theID=it->daten[i]->getIdentifier();
        std::string tmp=it->daten[i]->getName();
        tmp=tmp.append(" [");
        std::string tmp2=IDS_ERROR;
        CGraphingRoutines::loopThroughAllAndGetDataName(dataType,tmp2);
        tmp+=tmp2;
        tmp+=" (";
        tmp2=IDS_ERROR;
        CGraphingRoutines::loopThroughAllAndGetGraphObjectName(dataType,dataObjectID,tmp2);
        tmp+=tmp2;
        tmp+=")";
        tmp=tmp.append("]");
        QListWidgetItem* itm=new QListWidgetItem(tmp.c_str());
        itm->setData(Qt::UserRole,QVariant(theID));
        itm->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
        ui->qqRecordingList->addItem(itm);
    }
    noListSelectionAllowed=false;
}

int CQDlgGraphs::getSelectedObjectID()
{
    QList<QListWidgetItem*> sel=ui->qqRecordingList->selectedItems();
    if (sel.size()>0)
        return(sel.at(0)->data(Qt::UserRole).toInt());
    return(-1);
}

void CQDlgGraphs::selectObjectInList(int objectID)
{
    noListSelectionAllowed=true;
    for (int i=0;i<ui->qqRecordingList->count();i++)
    {
        QListWidgetItem* it=ui->qqRecordingList->item(i);
        if (it!=nullptr)
        {
            if (it->data(Qt::UserRole).toInt()==objectID)
            {
                it->setSelected(true);
                break;
            }
        }
    }
    noListSelectionAllowed=false;
}


void CQDlgGraphs::on_qqAddNewDataStream_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgDataStreamSelection theDialog(this);
        theDialog.refresh();
        if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
        {
            int currentDataType=theDialog.box1Id;
            int index=theDialog.box2Id;
            SSimulationThreadCommand cmd;
            cmd.cmdId=INSERT_DATASTREAM_GRAPHGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
            cmd.intParams.push_back(currentDataType);
            cmd.intParams.push_back(index);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgGraphs::onDeletePressed()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (focusWidget()==ui->qqRecordingList)
        {
            App::appendSimulationThreadCommand(REMOVE_DATASTREAM_GRAPHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),getSelectedObjectID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgGraphs::on_qqRecordingList_itemSelectionChanged()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!App::ct->objCont->isLastSelectionAGraph())
            return;
        CGraph* it=App::ct->objCont->getLastSelection_graph();
        int objID=getSelectedObjectID();
        CGraphData* grData=it->getGraphData(objID);
        if (grData!=nullptr)
            ((CEditBoxDelegate*)ui->qqRecordingList->itemDelegate())->initialText=grData->getName();
        else
            ((CEditBoxDelegate*)ui->qqRecordingList->itemDelegate())->initialText="";
        if (!noListSelectionAllowed)
        {
            inListSelectionRoutine=true;
            refresh();
            inListSelectionRoutine=false;
        }
    }
}

void CQDlgGraphs::on_qqRecordingList_itemChanged(QListWidgetItem *item)
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (item!=nullptr)
        {
            App::appendSimulationThreadCommand(RENAME_DATASTREAM_GRAPHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),item->data(Qt::UserRole).toInt(),0.0,0.0,item->text().toStdString().c_str());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgGraphs::on_qqExplicitHandling_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_EXPLICITHANDLING_GRAPHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgGraphs::on_qqBufferIsCyclic_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_BUFFERCYCLIC_GRAPHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgGraphs::on_qqShowXYZPlanes_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWXYZPLANES_GRAPHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgGraphs::on_qqObjectSize_editingFinished()
{
    if (!ui->qqObjectSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqObjectSize->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_OBJECTSIZE_GRAPHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgGraphs::on_qqBufferSize_editingFinished()
{
    if (!ui->qqBufferSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqBufferSize->text().toInt(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_BUFFERSIZE_GRAPHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgGraphs::on_qqRemoveAllStatics_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(REMOVE_ALLSTATICCURVES_GRAPHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgGraphs::on_qqAdjustBackgroundColor_clicked()
{
    CQDlgColor::displayDlg(COLOR_ID_GRAPH_BACKGROUND,App::ct->objCont->getLastSelectionID(),-1,sim_colorcomponent_ambient_diffuse,App::mainWindow);
}

void CQDlgGraphs::on_qqAdjustGridColor_clicked()
{
    CQDlgColor::displayDlg(COLOR_ID_GRAPH_GRID,App::ct->objCont->getLastSelectionID(),-1,sim_colorcomponent_ambient_diffuse,App::mainWindow);
}

void CQDlgGraphs::on_qqTransformationCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_VALUERAWSTATE_GRAPHGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intParams.push_back(ui->qqTransformationCombo->itemData(ui->qqTransformationCombo->currentIndex()).toInt());
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgGraphs::on_qqTransformationCoeff_editingFinished()
{
    if (!ui->qqTransformationCoeff->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqTransformationCoeff->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_VALUEMULTIPLIER_GRAPHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),getSelectedObjectID(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgGraphs::on_qqTransformationOffset_editingFinished()
{
    if (!ui->qqTransformationOffset->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqTransformationOffset->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_VALUEOFFSET_GRAPHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),getSelectedObjectID(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgGraphs::on_qqMovingAveragePeriod_editingFinished()
{
    if (!ui->qqMovingAveragePeriod->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqMovingAveragePeriod->text().toInt(&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_MOVINGAVERAGEPERIOD_GRAPHGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intParams.push_back(newVal);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgGraphs::on_qqTimeGraphVisible_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_TIMEGRAPHVISIBLE_GRAPHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),getSelectedObjectID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgGraphs::on_qqShowLabel_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_TIMEGRAPHSHOWLABEL_GRAPHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),getSelectedObjectID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgGraphs::on_qqLinkPoints_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_TIMEGRAPHLINKPOINTS_GRAPHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),getSelectedObjectID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgGraphs::on_qqAdjustCurveColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CGraph* it=App::ct->objCont->getLastSelection_graph();
        if (it!=nullptr)
        {
            CGraphData* grData=it->getGraphData(getSelectedObjectID());
            if (grData!=nullptr)
                CQDlgColor::displayDlg(COLOR_ID_GRAPH_TIMECURVE,App::ct->objCont->getLastSelectionID(),grData->getIdentifier(),sim_colorcomponent_ambient_diffuse,App::mainWindow);
        }
    }
}

void CQDlgGraphs::on_qqDuplicateToStatic_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(DUPLICATE_TOSTATIC_GRAPHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),getSelectedObjectID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgGraphs::on_qqEditXYGraphs_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CGraph* it=App::ct->objCont->getLastSelection_graph();
        if (it!=nullptr)
            CQDlg2D3DGraphProperties::display(it->getObjectHandle(),true,App::mainWindow);
    }
}

void CQDlgGraphs::on_qqEdit3DCurves_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CGraph* it=App::ct->objCont->getLastSelection_graph();
        if (it!=nullptr)
            CQDlg2D3DGraphProperties::display(it->getObjectHandle(),false,App::mainWindow);
    }
}
