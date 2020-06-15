#include "qdlgentityvsentityselection.h"
#include "ui_qdlgentityvsentityselection.h"
#include "tt.h"
#include "simStrings.h"
#include "app.h"
#include "vMessageBox.h"

CQDlgEntityVsEntitySelection::CQDlgEntityVsEntitySelection(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgEntityVsEntitySelection)
{
    ui->setupUi(this);
}

CQDlgEntityVsEntitySelection::~CQDlgEntityVsEntitySelection()
{
    delete ui;
}

void CQDlgEntityVsEntitySelection::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgEntityVsEntitySelection::okEvent()
{
    if (checkSelectionValidity())
        defaultModalDialogEndRoutine(true);
}

void CQDlgEntityVsEntitySelection::initialize(int theMode)
{
    mode=theMode;
    if (theMode==0)
    { // collisions
        setWindowTitle(IDSN_COLLISION_OBJECT_SELECTION_OF_ENTITIES);
        fillComboWithCollisionEntities(ui->qqEntity1,false);
        fillComboWithCollisionEntities(ui->qqEntity2,true);
    }
    if ((theMode==1)||(theMode==2))
    { // distances
        if (theMode==1)
            setWindowTitle(IDSN_DISTANCE_OBJECT_SELECTION_OF_ENTITIES);
        if (theMode==2)
            setWindowTitle(IDSN_OBSTACLE_AVOIDANCE_SELECTION_OF_ENTITIES);
        fillComboWithDistanceEntities(ui->qqEntity1,false);
        fillComboWithDistanceEntities(ui->qqEntity2,true);
    }
}

void CQDlgEntityVsEntitySelection::on_qqOkCancel_accepted()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (checkSelectionValidity())
            defaultModalDialogEndRoutine(true);
    }
}

void CQDlgEntityVsEntitySelection::on_qqOkCancel_rejected()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        defaultModalDialogEndRoutine(false);
    }
}

void CQDlgEntityVsEntitySelection::fillComboWithCollisionEntities(QComboBox* combo,bool insertAllAtBeginning)
{
    combo->clear();
    if (insertAllAtBeginning)
        combo->addItem(IDS_ALL_OTHER_COLLIDABLE_OBJECTS_IN_SCENE,QVariant(-1));

    std::vector<std::string> names;
    std::vector<int> ids;

    // Now collections:
    for (size_t i=0;i<App::currentWorld->collections->getObjectCount();i++)
    {
        CCollection* it=App::currentWorld->collections->getObjectFromIndex(i);
        std::string name(tt::decorateString("[",IDSN_COLLECTION,"] "));
        name+=it->getCollectionName();
        names.push_back(name);
        ids.push_back(it->getCollectionHandle());
    }
    tt::orderStrings(names,ids);
    for (int i=0;i<int(names.size());i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));

    names.clear();
    ids.clear();

    // Now shapes:
    for (size_t i=0;i<App::currentWorld->sceneObjects->getShapeCount();i++)
    {
        CShape* it=App::currentWorld->sceneObjects->getShapeFromIndex(i);
        std::string name(tt::decorateString("[",IDSN_SHAPE,"] "));
        name+=it->getObjectName();
        names.push_back(name);
        ids.push_back(it->getObjectHandle());
    }
    tt::orderStrings(names,ids);
    for (size_t i=0;i<names.size();i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));

    names.clear();
    ids.clear();

    // Now Octrees:
    for (size_t i=0;i<App::currentWorld->sceneObjects->getOctreeCount();i++)
    {
        COctree* it=App::currentWorld->sceneObjects->getOctreeFromIndex(i);
        std::string name(tt::decorateString("[",IDSN_OCTREE,"] "));
        name+=it->getObjectName();
        names.push_back(name);
        ids.push_back(it->getObjectHandle());
    }
    tt::orderStrings(names,ids);
    for (size_t i=0;i<names.size();i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));

    names.clear();
    ids.clear();

    // Now Point clouds:
    for (size_t i=0;i<App::currentWorld->sceneObjects->getPointCloudCount();i++)
    {
        CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromIndex(i);
        std::string name(tt::decorateString("[",IDSN_POINTCLOUD,"] "));
        name+=it->getObjectName();
        names.push_back(name);
        ids.push_back(it->getObjectHandle());
    }
    tt::orderStrings(names,ids);
    for (size_t i=0;i<names.size();i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));

    names.clear();
    ids.clear();

    // Now dummies:
    for (size_t i=0;i<App::currentWorld->sceneObjects->getDummyCount();i++)
    {
        CDummy* it=App::currentWorld->sceneObjects->getDummyFromIndex(i);
        std::string name(tt::decorateString("[",IDSN_DUMMY,"] "));
        name+=it->getObjectName();
        names.push_back(name);
        ids.push_back(it->getObjectHandle());
    }
    tt::orderStrings(names,ids);
    for (size_t i=0;i<names.size();i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));

    names.clear();
    ids.clear();
}

void CQDlgEntityVsEntitySelection::fillComboWithDistanceEntities(QComboBox* combo,bool insertAllAtBeginning)
{
    combo->clear();
    if (insertAllAtBeginning)
        combo->addItem(IDS_ALL_OTHER_MEASURABLE_OBJECTS_IN_SCENE,QVariant(-1));

    std::vector<std::string> names;
    std::vector<int> ids;

    // Now collections:
    for (size_t i=0;i<App::currentWorld->collections->getObjectCount();i++)
    {
        CCollection* it=App::currentWorld->collections->getObjectFromIndex(i);
        std::string name(tt::decorateString("[",IDSN_COLLECTION,"] "));
        name+=it->getCollectionName();
        names.push_back(name);
        ids.push_back(it->getCollectionHandle());
    }
    tt::orderStrings(names,ids);
    for (int i=0;i<int(names.size());i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));

    names.clear();
    ids.clear();

    // Now shapes:
    for (size_t i=0;i<App::currentWorld->sceneObjects->getShapeCount();i++)
    {
        CShape* it=App::currentWorld->sceneObjects->getShapeFromIndex(i);
        std::string name(tt::decorateString("[",IDSN_SHAPE,"] "));
        name+=it->getObjectName();
        names.push_back(name);
        ids.push_back(it->getObjectHandle());
    }
    tt::orderStrings(names,ids);
    for (int i=0;i<int(names.size());i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));

    names.clear();
    ids.clear();

    // Now Octrees:
    for (size_t i=0;i<App::currentWorld->sceneObjects->getOctreeCount();i++)
    {
        COctree* it=App::currentWorld->sceneObjects->getOctreeFromIndex(i);
        std::string name(tt::decorateString("[",IDSN_OCTREE,"] "));
        name+=it->getObjectName();
        names.push_back(name);
        ids.push_back(it->getObjectHandle());
    }
    tt::orderStrings(names,ids);
    for (size_t i=0;i<names.size();i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));

    names.clear();
    ids.clear();

    // Now Point clouds:
    for (size_t i=0;i<App::currentWorld->sceneObjects->getPointCloudCount();i++)
    {
        CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromIndex(i);
        std::string name(tt::decorateString("[",IDSN_POINTCLOUD,"] "));
        name+=it->getObjectName();
        names.push_back(name);
        ids.push_back(it->getObjectHandle());
    }
    tt::orderStrings(names,ids);
    for (size_t i=0;i<names.size();i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));

    names.clear();
    ids.clear();

    // Now dummies:
    for (size_t i=0;i<App::currentWorld->sceneObjects->getDummyCount();i++)
    {
        CDummy* it=App::currentWorld->sceneObjects->getDummyFromIndex(i);
        std::string name(tt::decorateString("[",IDSN_DUMMY,"] "));
        name+=it->getObjectName();
        names.push_back(name);
        ids.push_back(it->getObjectHandle());
    }
    tt::orderStrings(names,ids);
    for (size_t i=0;i<names.size();i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));

    names.clear();
    ids.clear();
}

bool CQDlgEntityVsEntitySelection::checkSelectionValidity()
{
    entity1=ui->qqEntity1->itemData(ui->qqEntity1->currentIndex()).toInt();
    entity2=ui->qqEntity2->itemData(ui->qqEntity2->currentIndex()).toInt();
    if ((entity1==entity2)&&(entity1<=SIM_IDEND_SCENEOBJECT))
    { // error, we cannot check against itself if object
        App::uiThread->messageBox_warning(this,IDSN_COLLISION_DISTANCE_OBJECT,IDS_CANNOT_CHECK_OBJECT_AGAINST_ITSELF,VMESSAGEBOX_OKELI);
        return(false);
    }
    if (mode==0)
    {
        // We check if such an object already exists:
        for (size_t i=0;i<App::currentWorld->collisions->getObjectCount();i++)
        {
            if (App::currentWorld->collisions->getObjectFromIndex(i)->isSame(entity1,entity2))
            {
                App::uiThread->messageBox_warning(this,IDSN_COLLISION_DISTANCE_OBJECT,IDS_SIMILAR_OBJECT_ALREADY_EXISTS,VMESSAGEBOX_OKELI);
                return(false);
            }
        }

        // Now check if the combination is valid:
        bool invalidCombination=false;
        if ( (entity1<SIM_IDSTART_COLLECTION)&&(entity2<SIM_IDSTART_COLLECTION) )
        {
            int t1=App::currentWorld->sceneObjects->getObjectFromHandle(entity1)->getObjectType();
            int t2=sim_object_octree_type;
            if (entity2!=-1)
                t2=App::currentWorld->sceneObjects->getObjectFromHandle(entity2)->getObjectType();
            if (t1==sim_object_shape_type)
            {
                if ( (t2!=sim_object_shape_type)&&(t2!=sim_object_octree_type) )
                    invalidCombination=true;
            }
            if (t1==sim_object_octree_type)
            {
                if ( (t2!=sim_object_shape_type)&&(t2!=sim_object_octree_type)&&(t2!=sim_object_pointcloud_type)&&(t2!=sim_object_dummy_type) )
                    invalidCombination=true;
            }
            if ( (t1==sim_object_pointcloud_type)||(t1==sim_object_dummy_type) )
            {
                if (t2!=sim_object_octree_type)
                    invalidCombination=true;
            }
        }
        if (invalidCombination)
        {
            App::uiThread->messageBox_warning(this,IDSN_COLLISION_DISTANCE_OBJECT,IDS_INVALID_COLLISION_ITEM_COMBINATION,VMESSAGEBOX_OKELI);
            return(false);
        }

        bool displayWarning=false;
        if ((entity1<=SIM_IDEND_SCENEOBJECT)&&(entity1>=SIM_IDSTART_SCENEOBJECT))
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(entity1);
            if (it!=nullptr)
                displayWarning|=((it->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)==0);
        }
        if ((entity2<=SIM_IDEND_SCENEOBJECT)&&(entity2>=SIM_IDSTART_SCENEOBJECT))
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(entity2);
            if (it!=nullptr)
                displayWarning|=((it->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)==0);
        }
        if (displayWarning)
            App::uiThread->messageBox_warning(this,IDSN_COLLISION_OBJECT,IDS_OBJECTS_NOT_COLLIDABLE_WARNING,VMESSAGEBOX_OKELI);
    }
    if ((mode==1)||(mode==2))
    {
        if (mode==1)
        {
            // We check if such an object already exists:
            for (size_t i=0;i<App::currentWorld->distances->getObjectCount();i++)
            {
                if (App::currentWorld->distances->getObjectFromIndex(i)->isSame(entity1,entity2))
                {
                    App::uiThread->messageBox_warning(this,IDSN_COLLISION_DISTANCE_OBJECT,IDS_SIMILAR_OBJECT_ALREADY_EXISTS,VMESSAGEBOX_OKELI);
                    return(false);
                }
            }
        }


        // Now check if the combination is valid:
        bool invalidCombination=false;
        if ( (entity1<SIM_IDSTART_COLLECTION)&&(entity2<SIM_IDSTART_COLLECTION) )
        {
            int t1=App::currentWorld->sceneObjects->getObjectFromHandle(entity1)->getObjectType();
            int t2=sim_object_octree_type;
            if (entity2!=-1)
                t2=App::currentWorld->sceneObjects->getObjectFromHandle(entity2)->getObjectType();
            if ( (t1==sim_object_shape_type)||(t1==sim_object_octree_type)||(t1==sim_object_pointcloud_type)||(t1==sim_object_dummy_type) )
            {
                if ( (t2!=sim_object_shape_type)&&(t2!=sim_object_octree_type)&&(t2!=sim_object_pointcloud_type)&&(t2!=sim_object_dummy_type) )
                    invalidCombination=true;
            }
            else
                invalidCombination=true;
        }
        if (invalidCombination)
        {
            App::uiThread->messageBox_warning(this,IDSN_COLLISION_DISTANCE_OBJECT,IDS_INVALID_DISTANCE_ITEM_COMBINATION,VMESSAGEBOX_OKELI);
            return(false);
        }

        bool displayWarning=false;
        if ((entity1<=SIM_IDEND_SCENEOBJECT)&&(entity1>=SIM_IDSTART_SCENEOBJECT))
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(entity1);
            if (it!=nullptr)
                displayWarning|=((it->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0);
        }
        if ((entity2<=SIM_IDEND_SCENEOBJECT)&&(entity2>=SIM_IDSTART_SCENEOBJECT))
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(entity2);
            if (it!=nullptr)
                displayWarning|=((it->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0);
        }
        if (displayWarning)
        {
            if (mode==1)
                App::uiThread->messageBox_warning(this,IDSN_DISTANCE_OBJECT,IDS_OBJECTS_NOT_MEASURABLE_WARNING,VMESSAGEBOX_OKELI);
            if (mode==2)
                App::uiThread->messageBox_warning(this,IDSN_AVOIDANCE_OBJECT,IDS_OBJECTS_NOT_MEASURABLE_WARNING,VMESSAGEBOX_OKELI);
        }
    }
    return(true);
}
