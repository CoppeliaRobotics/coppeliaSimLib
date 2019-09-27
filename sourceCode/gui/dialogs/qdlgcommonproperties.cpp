
#include "vrepMainHeader.h"
#include "qdlgcommonproperties.h"
#include "ui_qdlgcommonproperties.h"
#include "tt.h"
#include "gV.h"
#include "sceneObjectOperations.h"
#include "qdlgdetectableproperties.h"
#include "qdlgmodelproperties.h"
#include "qdlgassembly.h"
#include "qdlgscaling.h"
#include "v_repStrings.h"
#include "app.h"
#include "vMessageBox.h"

CQDlgCommonProperties::CQDlgCommonProperties(QWidget *parent) :
      CDlgEx(parent),
      ui(new Ui::CQDlgCommonProperties)
{
    _dlgType=GENERAL_PROPERTIES_DLG;
    ui->setupUi(this);
    inPlaceState=false;
    scalingState=0.5f;
    inMainRefreshRoutine=false;
}

CQDlgCommonProperties::~CQDlgCommonProperties()
{
    delete ui;
}

void CQDlgCommonProperties::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgCommonProperties::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();

    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();
    C3DObject* ls=App::ct->objCont->getLastSelection_object();
    bool objIsSelected=(ls!=nullptr);
    bool isDummy=false;
    bool isOctree=false;
    bool isPointcloud=false;
    bool isShape=false;
    bool isMirror=false;
    bool isSimpleShape=false;
    bool isPurePrimitive=false;
    bool isPath=false;
    bool isGraph=false;
    bool bigSel=App::ct->objCont->getSelSize()>=2;
    if (objIsSelected)
    {
        isMirror=(ls->getObjectType()==sim_object_mirror_type);
        isDummy=(ls->getObjectType()==sim_object_dummy_type);
        isOctree=(ls->getObjectType()==sim_object_octree_type);
        isPointcloud=(ls->getObjectType()==sim_object_pointcloud_type);
        isShape=(ls->getObjectType()==sim_object_shape_type);
        isPath=(ls->getObjectType()==sim_object_path_type);
        isGraph=(ls->getObjectType()==sim_object_graph_type);
        if (isShape)
        {
            isSimpleShape=!((CShape*)ls)->isCompound();
            isPurePrimitive=((CShape*)ls)->geomData->geomInfo->isPure();
        }
    }

    ui->qqExtensionString->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->qqSelectable->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->qqSelectInvisible->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->qqDepthInvisible->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->qqIgnoreForViewFitting->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->qqSelectBaseInstead->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->qqCannotBeDeleted->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->qqCannotBeDeletedDuringSimul->setEnabled(noEditModeNoSim&&objIsSelected&&((ls->getLocalObjectProperty()&sim_objectproperty_cannotdelete)==0));
    ui->qqDontShowInModelSelection->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->qqApplyGeneralProperties->setEnabled(noEditModeNoSim&&bigSel);
    ui->qqAssembling->setEnabled(noEditModeNoSim&&objIsSelected);

    ui->qqViewableObjects->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->qqViewableObjects->clear();

    if (ls!=nullptr)
    {
        ui->qqSelectable->setChecked((ls->getLocalObjectProperty()&sim_objectproperty_selectable)!=0);
        ui->qqSelectInvisible->setChecked((ls->getLocalObjectProperty()&sim_objectproperty_selectinvisible)!=0);
        ui->qqDepthInvisible->setChecked((ls->getLocalObjectProperty()&sim_objectproperty_depthinvisible)!=0);
        ui->qqIgnoreForViewFitting->setChecked(ls->getIgnoredByViewFitting());
        ui->qqSelectBaseInstead->setChecked((ls->getLocalObjectProperty()&sim_objectproperty_selectmodelbaseinstead)!=0);
        ui->qqDontShowInModelSelection->setChecked((ls->getLocalObjectProperty()&sim_objectproperty_dontshowasinsidemodel)!=0);
        ui->qqCannotBeDeleted->setChecked((ls->getLocalObjectProperty()&sim_objectproperty_cannotdelete)!=0);
        ui->qqCannotBeDeletedDuringSimul->setChecked((ls->getLocalObjectProperty()&(sim_objectproperty_cannotdeleteduringsim|sim_objectproperty_cannotdelete))!=0);
        ui->qqExtensionString->setText(ls->getExtensionString().c_str());
    }
    else
    {
        ui->qqSelectable->setChecked(false);
        ui->qqSelectInvisible->setChecked(false);
        ui->qqDepthInvisible->setChecked(false);
        ui->qqIgnoreForViewFitting->setChecked(false);
        ui->qqSelectBaseInstead->setChecked(false);
        ui->qqDontShowInModelSelection->setChecked(false);
        ui->qqCannotBeDeleted->setChecked(false);
        ui->qqCannotBeDeletedDuringSimul->setChecked(false);
        ui->qqExtensionString->setText("");
    }

    ui->a_1->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_2->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_3->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_4->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_5->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_6->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_7->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_8->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_9->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_10->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_11->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_12->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_13->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_14->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_15->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->a_16->setEnabled(noEditModeNoSim&&objIsSelected);
    ui->qqApplyLayers->setEnabled(noEditModeNoSim&&bigSel);
    if (ls!=nullptr)
    {
        ui->a_1->setChecked(ls->layer&1);
        ui->a_2->setChecked(ls->layer&2);
        ui->a_3->setChecked(ls->layer&4);
        ui->a_4->setChecked(ls->layer&8);
        ui->a_5->setChecked(ls->layer&16);
        ui->a_6->setChecked(ls->layer&32);
        ui->a_7->setChecked(ls->layer&64);
        ui->a_8->setChecked(ls->layer&128);
        ui->a_9->setChecked(ls->layer&256);
        ui->a_10->setChecked(ls->layer&512);
        ui->a_11->setChecked(ls->layer&1024);
        ui->a_12->setChecked(ls->layer&2048);
        ui->a_13->setChecked(ls->layer&4096);
        ui->a_14->setChecked(ls->layer&8192);
        ui->a_15->setChecked(ls->layer&16384);
        ui->a_16->setChecked(ls->layer&32768);
    }
    else
    {
        ui->a_1->setChecked(false);
        ui->a_2->setChecked(false);
        ui->a_3->setChecked(false);
        ui->a_4->setChecked(false);
        ui->a_5->setChecked(false);
        ui->a_6->setChecked(false);
        ui->a_7->setChecked(false);
        ui->a_8->setChecked(false);
        ui->a_9->setChecked(false);
        ui->a_10->setChecked(false);
        ui->a_11->setChecked(false);
        ui->a_12->setChecked(false);
        ui->a_13->setChecked(false);
        ui->a_14->setChecked(false);
        ui->a_15->setChecked(false);
        ui->a_16->setChecked(false);
    }

    if (ls!=nullptr)
    {
        ui->qqViewableObjects->addItem(strTranslate(IDSN_ALL_CAMERAS_AND_VISION_SENSORS),QVariant(-1));
        ui->qqViewableObjects->addItem(strTranslate(IDSN_NEVER_VISIBLE),QVariant(-2));

        std::vector<std::string> names;
        std::vector<int> ids;

        // Collections:
        for (int i=0;i<int(App::ct->collections->allCollections.size());i++)
        {
            CRegCollection* it2=App::ct->collections->allCollections[i];
            std::string name(tt::decorateString("[",strTranslate(IDSN_COLLECTION),"] "));
            name+=it2->getCollectionName();
            names.push_back(name);
            ids.push_back(it2->getCollectionID());
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            ui->qqViewableObjects->addItem(names[i].c_str(),QVariant(ids[i]));

        // Cameras:
        names.clear();
        ids.clear();
        for (int i=0;i<int(App::ct->objCont->cameraList.size());i++)
        {
            CCamera* it2=App::ct->objCont->getCamera(App::ct->objCont->cameraList[i]);
            std::string name(tt::decorateString("[",strTranslate(IDSN_CAMERA),"] "));
            name+=it2->getObjectName();
            names.push_back(name);
            ids.push_back(it2->getObjectHandle());
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            ui->qqViewableObjects->addItem(names[i].c_str(),QVariant(ids[i]));

        // Vision sensors:
        names.clear();
        ids.clear();
        for (int i=0;i<int(App::ct->objCont->visionSensorList.size());i++)
        {
            CVisionSensor* it2=App::ct->objCont->getVisionSensor(App::ct->objCont->visionSensorList[i]);
            std::string name(tt::decorateString("[",strTranslate(IDSN_VISION_SENSOR),"] "));
            name+=it2->getObjectName();
            names.push_back(name);
            ids.push_back(it2->getObjectHandle());
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            ui->qqViewableObjects->addItem(names[i].c_str(),QVariant(ids[i]));

        for (int i=0;i<ui->qqViewableObjects->count();i++)
        {
            if (ui->qqViewableObjects->itemData(i).toInt()==ls->getAuthorizedViewableObjects())
            {
                ui->qqViewableObjects->setCurrentIndex(i);
                break;
            }
        }

    }

    ui->qqModelBase->setEnabled(noEditModeNoSim&&objIsSelected);
    if (ls!=nullptr)
    {
        ui->qqModelBase->setChecked(ls->getModelBase());
        ui->qqEditModelProperties->setEnabled(ls->getModelBase());
    }
    else
    {
        ui->qqModelBase->setChecked(false);
        ui->qqEditModelProperties->setEnabled(false);
    }

    ui->qqApplySpecialProperties->setEnabled(bigSel&&noEditModeNoSim);//(isShape||isPath||isDummy||isGraph||isMirror)));
    if (ls!=nullptr)
    {
        //** Collidable
        ui->qqCollidable->setEnabled(ls->isPotentiallyCollidable()&&noEditModeNoSim);
        if (ls->isPotentiallyCollidable())
            ui->qqCollidable->setChecked((ls->getLocalObjectSpecialProperty()&sim_objectspecialproperty_collidable)!=0);
        else
            ui->qqCollidable->setChecked(false);
        //**

        //** Measurable
        ui->qqMeasurable->setEnabled(ls->isPotentiallyMeasurable()&&noEditModeNoSim);
        if (ls->isPotentiallyMeasurable())
            ui->qqMeasurable->setChecked((ls->getLocalObjectSpecialProperty()&sim_objectspecialproperty_measurable)!=0);
        else
            ui->qqMeasurable->setChecked(false);
        //**

        //** Renderable
        ui->qqRenderable->setEnabled(ls->isPotentiallyRenderable()&&noEditModeNoSim);
        if (ls->isPotentiallyRenderable())
            ui->qqRenderable->setChecked((ls->getLocalObjectSpecialProperty()&sim_objectspecialproperty_renderable)!=0);
        else
            ui->qqRenderable->setChecked(false);
        //**

        //** Cuttable
        if (App::userSettings->enableOldMillObjects)
        {
            ui->qqCuttable->setEnabled(ls->isPotentiallyCuttable()&&noEditModeNoSim);
            if (isSimpleShape)
                ui->qqCuttable->setChecked((ls->getLocalObjectSpecialProperty()&sim_objectspecialproperty_cuttable)!=0);
            else
                ui->qqCuttable->setChecked(false);
            if (isShape)
            {
                if (isSimpleShape&&(!isPurePrimitive))
                    ui->qqCuttable->setText(tr(IDS_CUTTABLE_PROP));
                else
                    ui->qqCuttable->setText(tr(IDS_CUTTABLE_BUT_NOT_SIMPLE_NON_PURE_SHAPE));
            }
            else
                ui->qqCuttable->setText(tr(IDS_CUTTABLE_PROP));
        }
        else
            ui->qqCuttable->setVisible(false);
        //**

        //** Detectable
        ui->qqDetectable->setEnabled(ls->isPotentiallyDetectable()&&noEditModeNoSim);
        ui->qqEditDetectableDetails->setEnabled(ls->isPotentiallyDetectable()&&noEditModeNoSim);
        if (ls->isPotentiallyDetectable())
        {
            if ((ls->getLocalObjectSpecialProperty()&sim_objectspecialproperty_detectable_all)==0)
                ui->qqDetectable->setCheckState(Qt::Unchecked);
            else
            {
                if ((ls->getLocalObjectSpecialProperty()&sim_objectspecialproperty_detectable_all)==sim_objectspecialproperty_detectable_all)
                    ui->qqDetectable->setCheckState(Qt::Checked);
                else
                    ui->qqDetectable->setCheckState(Qt::PartiallyChecked);
            }
        }
        else
        {
            ui->qqDetectable->setCheckState(Qt::Unchecked);
        }
        //**
    }
    else
    {
        ui->qqCollidable->setEnabled(false);
        ui->qqMeasurable->setEnabled(false);
        ui->qqRenderable->setEnabled(false);
        ui->qqCuttable->setEnabled(false);
        ui->qqDetectable->setEnabled(false);
        ui->qqEditDetectableDetails->setEnabled(false);
        ui->qqCollidable->setChecked(false);
        ui->qqMeasurable->setChecked(false);
        ui->qqRenderable->setChecked(false);
        ui->qqCuttable->setChecked(false);
        ui->qqCuttable->setText(tr(IDS_CUTTABLE_PROP));
        ui->qqDetectable->setChecked(false);
    }

    ui->qqUpdatable->setEnabled(objIsSelected&&noEditModeNoSim);
    ui->qqOpenScalingDialog->setEnabled(objIsSelected&&noEditModeNoSim);

    ui->qqSelfCollisionIndicator->setEnabled(objIsSelected&&noEditModeNoSim&&(isDummy||isShape));

    if (ls!=nullptr)
    {
        if (isDummy||isShape||isOctree)
            ui->qqSelfCollisionIndicator->setText(tt::getIString(false,ls->getCollectionSelfCollisionIndicator()).c_str());
        else
            ui->qqSelfCollisionIndicator->setText("");
        ui->qqUpdatable->setChecked((ls->getLocalObjectProperty()&sim_objectproperty_canupdatedna)!=0);
    }
    else
    {
        ui->qqSelfCollisionIndicator->setText("");
        ui->qqUpdatable->setChecked(false);
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgCommonProperties::on_qqSelectable_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SELECTABLE_COMMONPROPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqSelectBaseInstead_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SELECTBASEOFMODEL_COMMONPROPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqDontShowInModelSelection_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_DONGTSHOWINSIDEMODELSELECTION_COMMONPROPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqIgnoreForViewFitting_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_VIEWFITTINGIGNORED_COMMONPROPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqApplyGeneralProperties_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::ct->objCont->getSelSize()>=2)
        {
            C3DObject* last=App::ct->objCont->getLastSelection_object();
            SSimulationThreadCommand cmd;
            cmd.cmdId=APPLY_GENERALPROP_COMMONPROPGUITRIGGEREDCMD;
            cmd.intParams.push_back(last->getObjectHandle());
            for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
                cmd.intParams.push_back(App::ct->objCont->getSelID(i));
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_1_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^1;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_2_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^2;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_3_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^4;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_4_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^8;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_5_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^16;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_6_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^32;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_7_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^64;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_8_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^128;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_9_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^256;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_10_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^512;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_11_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^1024;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_12_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^2048;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_13_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^4096;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_14_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^8192;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_15_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^16384;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_a_16_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
        {
            int layer=it->layer^32768;
            App::appendSimulationThreadCommand(SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD,it->getObjectHandle(),layer);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}


void CQDlgCommonProperties::on_qqApplyLayers_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::ct->objCont->getSelSize()>=2)
        {
            C3DObject* last=App::ct->objCont->getLastSelection_object();
            SSimulationThreadCommand cmd;
            cmd.cmdId=APPLY_VISIBILITYPROP_COMMONPROPGUITRIGGEREDCMD;
            cmd.intParams.push_back(last->getObjectHandle());
            for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
                cmd.intParams.push_back(App::ct->objCont->getSelID(i));
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_qqModelBase_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_MODELBASE_COMMONPROPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqEditModelProperties_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::ct->objCont->getSelSize()>0)
        {
            C3DObject* it=App::ct->objCont->getLastSelection_object();
            if ((it!=nullptr)&&it->getModelBase())
            {
                CQDlgModelProperties theDialog(this);
                theDialog.modelBaseObject=it;
                theDialog.refresh();
                theDialog.makeDialogModal(); // things are modified/messages sent in the modal dlg
            }
        }
    }
}

void CQDlgCommonProperties::on_qqCollidable_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* ls=App::ct->objCont->getLastSelection_object();
        if ((ls!=nullptr)&&ls->isPotentiallyCollidable())
        {
            App::appendSimulationThreadCommand(TOGGLE_COLLIDABLE_COMMONPROPGUITRIGGEREDCMD,ls->getObjectHandle());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqMeasurable_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* ls=App::ct->objCont->getLastSelection_object();
        if ((ls!=nullptr)&&ls->isPotentiallyMeasurable())
        {
            App::appendSimulationThreadCommand(TOGGLE_MEASURABLE_COMMONPROPGUITRIGGEREDCMD,ls->getObjectHandle());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}


void CQDlgCommonProperties::on_qqRenderable_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* ls=App::ct->objCont->getLastSelection_object();
        if ((ls!=nullptr)&&ls->isPotentiallyRenderable())
        {
            App::appendSimulationThreadCommand(TOGGLE_RENDERABLE_COMMONPROPGUITRIGGEREDCMD,ls->getObjectHandle());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqCuttable_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* ls=App::ct->objCont->getLastSelection_object();
        if ((ls!=nullptr)&&ls->isPotentiallyCuttable())
        {
            App::appendSimulationThreadCommand(TOGGLE_CUTTABLE_COMMONPROPGUITRIGGEREDCMD,ls->getObjectHandle());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqDetectable_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* ls=App::ct->objCont->getLastSelection_object();
        if ((ls!=nullptr)&&ls->isPotentiallyDetectable())
        {
            App::appendSimulationThreadCommand(TOGGLE_DETECTABLE_COMMONPROPGUITRIGGEREDCMD,ls->getObjectHandle());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqEditDetectableDetails_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        C3DObject* last=App::ct->objCont->getLastSelection_object();
        if ((last!=nullptr)&&last->isPotentiallyDetectable())
        {
            CQDlgDetectableProperties dlg(this);
            dlg.objectProperties=last->getLocalObjectSpecialProperty();
            dlg.refresh();
            if (dlg.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
            {
                App::appendSimulationThreadCommand(SET_DETECTABLEITEMS_COMMONPROPGUITRIGGEREDCMD,last->getObjectHandle(),dlg.objectProperties);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            }
        }
    }
}

void CQDlgCommonProperties::on_qqApplySpecialProperties_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::ct->objCont->getSelSize()>=2)
        {
            C3DObject* last=App::ct->objCont->getLastSelection_object();
            SSimulationThreadCommand cmd;
            cmd.cmdId=APPLY_SPECIALPROP_COMMONPROPGUITRIGGEREDCMD;
            cmd.intParams.push_back(last->getObjectHandle());
            for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
                cmd.intParams.push_back(App::ct->objCont->getSelID(i));
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
    }
}


void CQDlgCommonProperties::on_qqOpenScalingDialog_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        if (App::ct->objCont->getSelSize()==0)
            return;
        CQDlgScaling theDialog(this);
        theDialog.refresh();
        theDialog.makeDialogModal();// undo points and scaling handled in the dialog routines
    }
}

void CQDlgCommonProperties::on_qqAssembling_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        if (App::ct->objCont->getSelSize()==0)
            return;
        CQDlgAssembly theDialog(this);
        theDialog.obj=App::ct->objCont->getLastSelection_object();
        theDialog.refresh();
        theDialog.makeDialogModal(); // things are modified in the dlg
    }
}

void CQDlgCommonProperties::on_qqUpdatable_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        C3DObject* ls=App::ct->objCont->getLastSelection_object();
        if (ls!=nullptr)
        {
            int p=ls->getLocalObjectProperty();
            if (p&sim_objectproperty_canupdatedna)
            {
                if (VMESSAGEBOX_REPLY_YES==App::uiThread->messageBox_warning(App::mainWindow,strTranslate("Updatable property"),strTranslate(IDSN_SURE_TO_DISABLE_UPDATABLE_WARNING),VMESSAGEBOX_YES_NO))
                    App::appendSimulationThreadCommand(TOGGLE_CANTRANSFERDNA_COMMONPROPGUITRIGGEREDCMD,ls->getObjectHandle());
            }
            else
                App::appendSimulationThreadCommand(TOGGLE_CANTRANSFERDNA_COMMONPROPGUITRIGGEREDCMD,ls->getObjectHandle());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqSelectInvisible_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SELECTINVISIBLE_COMMONPROPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqDepthInvisible_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_DEPTHMAPIGNORED_COMMONPROPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqSelfCollisionIndicator_editingFinished()
{
    if (!ui->qqSelfCollisionIndicator->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqSelfCollisionIndicator->text().toInt(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_SELFCOLLISIONINDICATOR_COMMONPROPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqViewableObjects_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_WRITE_DATA
        {
            int objID=ui->qqViewableObjects->itemData(ui->qqViewableObjects->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_CANBESEENBY_COMMONPROPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),objID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCommonProperties::on_qqExtensionString_editingFinished()
{
    if (!ui->qqExtensionString->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(SET_EXTENSIONSTRING_COMMONPROPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,0,0,ui->qqExtensionString->text().toStdString().c_str());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqCannotBeDeleted_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_CANNOTBEDELETED_COMMONPROPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCommonProperties::on_qqCannotBeDeletedDuringSimul_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_CANNOTBEDELETEDDURINGSIMULATION_COMMONPROPGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
