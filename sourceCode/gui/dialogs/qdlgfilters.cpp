
#include "vrepMainHeader.h"
#include "qdlgfilters.h"
#include "ui_qdlgfilters.h"
#include "app.h"
#include "tt.h"
#include "gV.h"
#include <QShortcut>
#include "qdlgvisionsensors.h"
#include "pluginContainer.h"
#include "v_repStrings.h"
#include "qdlgsimplefilter_3x3.h"
#include "qdlgsimplefilter_5x5.h"
#include "qdlgsimplefilter_binary.h"
#include "qdlgsimplefilter_blob.h"
#include "qdlgsimplefilter_circularCut.h"
#include "qdlgsimplefilter_colorSegmentation.h"
#include "qdlgsimplefilter_correlation.h"
#include "qdlgsimplefilter_edge.h"
#include "qdlgsimplefilter_intensityScale.h"
#include "qdlgsimplefilter_keepRemoveCols.h"
#include "qdlgsimplefilter_rectangularCut.h"
#include "qdlgsimplefilter_resize.h"
#include "qdlgsimplefilter_rotate.h"
#include "qdlgsimplefilter_scaleCols.h"
#include "qdlgsimplefilter_shift.h"
#include "qdlgsimplefilter_coordExtraction.h"
#include "qdlgsimplefilter_velodyne.h"
#include "qdlgsimplefilter_pixelChange.h"
#include "qdlgsimplefilter_uniformImage.h"

CQDlgFilters::CQDlgFilters(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgFilters)
{
    _dlgType=VISION_SENSOR_FILTER_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
    currentComboIndex=0;
    QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcut,SIGNAL(activated()), this, SLOT(onDeletePressed()));
    QShortcut* shortcut2 = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
    connect(shortcut2,SIGNAL(activated()), this, SLOT(onDeletePressed()));
}

CQDlgFilters::~CQDlgFilters()
{
    delete ui;
}

void CQDlgFilters::cancelEvent()
{ // no cancel event allowed
    CQDlgVisionSensors::showFilterWindow=false;
    CDlgEx::cancelEvent();
    App::setFullDialogRefreshFlag();
}

void CQDlgFilters::dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    if ( (cmdIn!=nullptr)&&(cmdIn->intParams[0]==_dlgType) )
    {
        if (cmdIn->intParams[1]==0)
            selectObjectInList(cmdIn->intParams[2]);
    }
}

void CQDlgFilters::refresh()
{
    inMainRefreshRoutine=true;
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();
    CVisionSensor* it=App::ct->objCont->getLastSelection_visionSensor();

    int selectedObjectID=getSelectedObjectID();
    updateObjectsInList();
    selectObjectInList(selectedObjectID);

    bool isSensor=App::ct->objCont->isLastSelectionAVisionSensor();
    bool manySensors=App::ct->objCont->getVisionSensorNumberInSelection()>1;

    ui->qqFilterList->setEnabled(isSensor&&noEditModeNoSim);
    ui->qqFilterCombo->setEnabled(isSensor&&noEditModeNoSim);
    ui->qqAddFilter->setEnabled(isSensor&&noEditModeNoSim);
    ui->qqApply->setEnabled(isSensor&&manySensors&&noEditModeNoSim);

    ui->qqFilterCombo->clear();

    if (it!=nullptr)
    {
        CComposedFilter* filters=it->getComposedFilter();
        int cnt=0;
        int tpe=CSimpleFilter::getAvailableFilter(cnt++);
        while (tpe!=-1)
        {
            ui->qqFilterCombo->addItem(CSimpleFilter::getSpecificFilterString(tpe).c_str(),QVariant(tpe));
            tpe=CSimpleFilter::getAvailableFilter(cnt++);
        }

        // Select current item:
        if (currentComboIndex>=ui->qqFilterCombo->count())
            currentComboIndex=0;
        ui->qqFilterCombo->setCurrentIndex(currentComboIndex);

        int objectIndexInList=getSelectedObjectID();
        if (objectIndexInList>=0)
            ui->qqComponentEnabled->setChecked(filters->getSimpleFilter(objectIndexInList)->getEnabled());
        else
            ui->qqComponentEnabled->setChecked(false);
        ui->qqUp->setEnabled(isSensor&&noEditModeNoSim&&(objectIndexInList>=0));
        ui->qqDown->setEnabled(isSensor&&noEditModeNoSim&&(objectIndexInList>=0));
        ui->qqComponentEnabled->setEnabled(isSensor&&noEditModeNoSim&&(objectIndexInList>=0));
    }
    else
    {
        ui->qqUp->setEnabled(false);
        ui->qqDown->setEnabled(false);
        ui->qqComponentEnabled->setEnabled(false);
        ui->qqComponentEnabled->setChecked(false);
    }

    inMainRefreshRoutine=false;
}

void CQDlgFilters::onDeletePressed()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        if (focusWidget()==ui->qqFilterList)
        {
            int objID=getSelectedObjectID();
            if (objID!=-1)
            {
                App::appendSimulationThreadCommand(DELETE_FILTER_VISIONSENSORFILTERGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),objID);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            }
        }
    }
}

void CQDlgFilters::on_qqFilterCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
        currentComboIndex=index;
}

void CQDlgFilters::updateObjectsInList()
{
    ui->qqFilterList->clear();

    if (!App::ct->objCont->isLastSelectionAVisionSensor())
        return;
    CVisionSensor* it=App::ct->objCont->getLastSelection_visionSensor();
    CComposedFilter* filters=it->getComposedFilter();

    for (int i=0;i<filters->getSimpleFilterCount();i++)
    {
        CSimpleFilter* sf=filters->getSimpleFilter(i);
        std::string tmp(sf->getFilterString().c_str());
        if (!sf->getEnabled())
        {
            tmp+=" ";
            tmp+=std::string(IDS_FILTER_DISABLED);
        }

        if (sf->canFilterBeEdited())
        {
            tmp+=" ";
            tmp+=tt::decorateString("(",strTranslate(IDSN_DOUBLE_CLICK_TO_EDIT),")");
        }

        QListWidgetItem* itm=new QListWidgetItem(tmp.c_str());
        itm->setData(Qt::UserRole,QVariant(i));
        itm->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->qqFilterList->addItem(itm);
    }
}

int CQDlgFilters::getSelectedObjectID()
{
    QList<QListWidgetItem*> sel=ui->qqFilterList->selectedItems();
    if (sel.size()>0)
        return(sel.at(0)->data(Qt::UserRole).toInt());
    return(-1);
}

void CQDlgFilters::selectObjectInList(int objectID)
{
    for (int i=0;i<ui->qqFilterList->count();i++)
    {
        QListWidgetItem* it=ui->qqFilterList->item(i);
        if (it!=nullptr)
        {
            if (it->data(Qt::UserRole).toInt()==objectID)
            {
                it->setSelected(true);
                break;
            }
        }
    }
}

void CQDlgFilters::on_qqFilterList_itemDoubleClicked(QListWidgetItem *item)
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        if (item!=nullptr)
            _editFilter(item->data(Qt::UserRole).toInt());
    }
}

bool CQDlgFilters::_editFilter(int index)
{
    bool retVal=false;
    CVisionSensor* it=App::ct->objCont->getVisionSensor(App::ct->objCont->getLastSelectionID());
    SSimulationThreadCommand cmd;
    cmd.cmdId=SET_FILTERPARAMS_VISIONSENSORFILTERGUITRIGGEREDCMD;
    if (it!=nullptr)
    {
        CComposedFilter* filters=it->getComposedFilter();
        CSimpleFilter* filter=filters->getSimpleFilter(index);
        if ((filter!=nullptr)&&(filter->canFilterBeEdited()))
        {
            int ft=filter->getFilterType();
            cmd.intParams.push_back(it->getObjectHandle());
            cmd.intParams.push_back(index);
            cmd.intParams.push_back(ft);
            if (ft==sim_filtercomponent_rotate)
            {
                CQDlgSimpleFilter_rotate theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_uniformimage)
            {
                CQDlgSimpleFilter_uniformImage theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_3x3filter)
            {
                CQDlgSimpleFilter_3x3 theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_5x5filter)
            {
                CQDlgSimpleFilter_5x5 theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_keeporremovecolors)
            {
                CQDlgSimpleFilter_keepRemoveCols theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_scaleandoffsetcolors)
            {
                CQDlgSimpleFilter_scaleCols theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_correlationwithbuffer1)
            {
                CQDlgSimpleFilter_correlation theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_shift)
            {
                CQDlgSimpleFilter_shift theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_imagetocoord)
            {
                CQDlgSimpleFilter_coordExtraction theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_velodyne)
            {
                CQDlgSimpleFilter_velodyne theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_pixelchange)
            {
                CQDlgSimpleFilter_pixelChange theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_circularcut)
            {
                CQDlgSimpleFilter_circularCut theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_colorsegmentation)
            {
                CQDlgSimpleFilter_colorSegmentation theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_rectangularcut)
            {
                CQDlgSimpleFilter_rectangularCut theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_intensityscale)
            {
                CQDlgSimpleFilter_intensityScale theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_binary)
            {
                CQDlgSimpleFilter_binary theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_edge)
            {
                CQDlgSimpleFilter_edge theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_blobextraction)
            {
                CQDlgSimpleFilter_blob theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft==sim_filtercomponent_resize)
            {
                CQDlgSimpleFilter_resize theDialog(this);
                theDialog.initializeDialogValues(filter);
                if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
                {
                    cmd.intParams.push_back(theDialog.appendDialogValues(cmd.uint8Params,cmd.intParams,cmd.floatParams));
                    retVal=true;
                }
            }
            if (ft>=sim_filtercomponent_customized)
            { // special here!
                std::vector<unsigned char> params;
                filter->getCustomFilterParameters(params);
                int header,id;
                filter->getCustomFilterInfo(header,id);
                int auxData[4]={header,id,static_cast<int>(params.size()),-1};
                int retVals[4]={-1,-1,-1,-1};
                void* callbackReturnVal=nullptr;
                if (params.size()==0)
                    callbackReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_imagefilter_adjustparams,auxData,nullptr,retVals);
                else
                    callbackReturnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_imagefilter_adjustparams,auxData,&params[0],retVals);
                if ( (retVals[0]>0)&&(callbackReturnVal!=nullptr) )
                {
                    for (int i=0;i<retVals[0];i++)
                        cmd.uint8Params.push_back(((unsigned char*)callbackReturnVal)[i]);
                    cmd.cmdId=SET_CUSTOMFILTERPARAMS_VISIONSENSORFILTERGUITRIGGEREDCMD;
                    retVal=true;
                }
                delete[] ((char*)callbackReturnVal);
            }
        }
    }
    if (retVal)
    {
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    }
    return(retVal);
}

void CQDlgFilters::on_qqAddFilter_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        int objID=ui->qqFilterCombo->itemData(ui->qqFilterCombo->currentIndex()).toInt();
        App::appendSimulationThreadCommand(ADD_FILTER_VISIONSENSORFILTERGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),objID);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgFilters::on_qqUp_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        int data=getSelectedObjectID();
        CVisionSensor* it=App::ct->objCont->getVisionSensor(App::ct->objCont->getLastSelectionID());
        if ( (it!=nullptr)&&(data>0) )
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=MOVE_FILTER_VISIONSENSORFILTERGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
            cmd.intParams.push_back(data);
            cmd.boolParams.push_back(true); // up
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            selectObjectInList(data-1); // in order to have the items still selected
        }
    }
}

void CQDlgFilters::on_qqDown_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        int data=getSelectedObjectID();
        CVisionSensor* it=App::ct->objCont->getVisionSensor(App::ct->objCont->getLastSelectionID());
        if ( (it!=nullptr)&&(data>=0) )
        {
            CComposedFilter* filters=it->getComposedFilter();
            if (data<filters->getSimpleFilterCount()-1)
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=MOVE_FILTER_VISIONSENSORFILTERGUITRIGGEREDCMD;
                cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
                cmd.intParams.push_back(data);
                cmd.boolParams.push_back(false); // down
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
                selectObjectInList(data+1); // in order to have the items still selected
            }
        }
    }
}

void CQDlgFilters::on_qqComponentEnabled_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        int data=getSelectedObjectID();
        App::appendSimulationThreadCommand(TOGGLE_FILTERENABLE_VISIONSENSORFILTERGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),data);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgFilters::on_qqFilterList_itemSelectionChanged()
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            refresh();
        }
    }
}

void CQDlgFilters::on_qqApply_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CVisionSensor* last=App::ct->objCont->getLastSelection_visionSensor();
        if (last!=nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=APPLY_FILTERS_VISIONSENSORFILTERGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
            for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
                cmd.intParams.push_back(App::ct->objCont->getSelID(i));
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
    }
}
