
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "sceneHierarchyWidget.h"
#include "sceneHierarchyItemDelegate.h"
#include "app.h"
#include <QHeaderView>
#include "vDateTime.h"

bool _inRefreshRoutine=false;

CSceneHierarchyWidget::CSceneHierarchyWidget() : QTreeWidget()
{
    setItemDelegate(new CSceneHierarchyItemDelegate);
    QStringList headerLabels;
    headerLabels.push_back("Scene hierarchy");
    headerLabels.push_back("");
    headerLabels.push_back("");
    headerLabels.push_back("");
    setHeaderLabels(headerLabels);
//    setColumnCount(4);
//    setColumnWidth(0,200);
 //   header()->resizeSection(1,16);
    header()->setStretchLastSection(false);
    header()->setMovable(false);
    header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1,QHeaderView::QHeaderView::Fixed);
    header()->setSectionResizeMode(2,QHeaderView::QHeaderView::Fixed);
    header()->setSectionResizeMode(3,QHeaderView::QHeaderView::Fixed);
 //   header()->xxx(2);
    setColumnWidth(1,16);
    setColumnWidth(2,16);
    setColumnWidth(3,16);
    setAnimated(false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    std::string st="QTreeView::branch:has-siblings:!adjoins-item{border-image: url(:/variousImageFiles/iline.png) 0;}";
    st+="QTreeView::branch:has-siblings:adjoins-item{border-image: url(:/variousImageFiles/tline.png) 0;}";
    st+="QTreeView::branch:!has-children:!has-siblings:adjoins-item{border-image: url(:/variousImageFiles/lline.png) 0;}";
    st+="QTreeView::branch:has-children:!has-siblings:closed,QTreeView::branch:closed:has-children:has-siblings{border-image: none;image: url(:/variousImageFiles/plus.png);}";
    st+="QTreeView::branch:open:has-children:!has-siblings,QTreeView::branch:open:has-children:has-siblings{border-image: none;image: url(:/variousImageFiles/minus.png);}";
    st+="QTreeView::branch:selected{background: palette(base);}";
    st+="QTreeView::item:selected {background-color: #3d9dfe; color: #f1f1f1}";
    setStyleSheet(st.c_str());
    connect(this,SIGNAL(itemCollapsed(QTreeWidgetItem*)),this,SLOT(onItemCollapsed(QTreeWidgetItem*)));
    connect(this,SIGNAL(itemExpanded(QTreeWidgetItem*)),this,SLOT(onItemExpanded(QTreeWidgetItem*)));
    connect(this,SIGNAL(itemSelectionChanged()),this,SLOT(onItemSelectionChanged()));
    connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(onItemDoubleClicked(QTreeWidgetItem*,int)));
    setIconSize(QSize(20,16));
    setHeaderHidden(false);
    header()->setStretchLastSection(false);
    setExpandsOnDoubleClick(false);
 //   setIndentation(16);
}

void CSceneHierarchyWidget::rebuild()
{
    setUpdatesEnabled(false);
    int st=VDateTime::getTimeInMs();
    printf("rebuild\n");
    _inRefreshRoutine=true;

    static int cnt=0;
    if (cnt==0)
    {

        removeAll();
        for (size_t i=0;i<App::ct->objCont->orphanList.size();i++)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->orphanList[i]);
            QTreeWidgetItem* itm=_buildObjectWithHierarchy(it);
            insertTopLevelItem((int)i,itm);
        }
    }
    else
    {
        // 1. Remove all items linked to disappeared objects:
        std::map<int,QTreeWidgetItem*> items;
        for (std::map<int,QTreeWidgetItem*>::iterator it=_allTreeItems.begin();it!=_allTreeItems.end();it++)
        {
            int id=it->first;
            QTreeWidgetItem* item=it->second;
            items[id]=item;
        }
        _allTreeItems.clear();

        for (std::map<int,QTreeWidgetItem*>::iterator it=items.begin();it!=items.end();it++)
        {
            int id=it->first;
            QTreeWidgetItem* item=it->second;
            if (App::ct->objCont->getObjectFromHandle(id)!=nullptr)
                _allTreeItems[it->first]=item;
            else
            {
                QTreeWidgetItem* previousParent=item->parent();
                QList<QTreeWidgetItem*> children=item->takeChildren();
                if (children.size()>0)
                {
                    if (previousParent==nullptr)
                        addTopLevelItems(children);
                    else
                        previousParent->addChildren(children);
                }
                delete item;
            }
        }
        items.clear();

        // 2. Add items from new objects, from base to tip, and attach them directly:
        std::vector<C3DObject*> toExplore;
        for (size_t i=0;i<App::ct->objCont->orphanList.size();i++)
        {
            C3DObject* obj=App::ct->objCont->getObjectFromHandle(App::ct->objCont->orphanList[i]);
            toExplore.push_back(obj);
        }
        while (toExplore.size()>0)
        {
            C3DObject* obj=toExplore[0];
            toExplore.erase(toExplore.begin());
            for (size_t i=0;i<obj->childList.size();i++)
            {
                C3DObject* child=obj->childList[i];
                toExplore.push_back(child);
            }
            int id=obj->getObjectHandle();
            std::map<int,QTreeWidgetItem*>::iterator it=_allTreeItems.find(id);
            if (it==_allTreeItems.end())
            { // that object is new
                QTreeWidgetItem* item=_buildObject(obj);
                _allTreeItems[id]=item;
                C3DObject* parent=obj->getParentObject();
                if (parent==nullptr)
                    addTopLevelItem(item);
                else
                {
                    std::map<int,QTreeWidgetItem*>::iterator treeParent=_allTreeItems.find(parent->getObjectHandle());
                    treeParent->second->addChild(item);
                }
                _allTreeItems[id]=item;
            }
        }

        // 3. Adjust tree item parents from tip to base:
        std::vector<QTreeWidgetItem*> theLeafItems;
        for (std::map<int,QTreeWidgetItem*>::iterator it=_allTreeItems.begin();it!=_allTreeItems.end();it++)
        {
            QTreeWidgetItem* item=it->second;
            if (item->childCount()==0)
                theLeafItems.push_back(item);
        }
        std::map<int,bool> handledLeaves;
        while (theLeafItems.size()>0)
        {
            QTreeWidgetItem* item=theLeafItems[0];
            theLeafItems.erase(theLeafItems.begin());
            int id=item->data(0,Qt::UserRole+1).toInt();
            std::map<int,bool>::iterator present=handledLeaves.find(id);
            if (present==handledLeaves.end())
            { // ok, item not yet handled
                handledLeaves[id]=true;
                C3DObject* obj=App::ct->objCont->getObjectFromHandle(id);
                C3DObject* parentObj=obj->getParentObject();
                QTreeWidgetItem* parentItem=item->parent();
                if ( (parentObj!=nullptr)||(parentItem!=nullptr) )
                {
                    if (parentItem!=nullptr)
                        theLeafItems.push_back(parentItem);
                    if (parentObj==nullptr)
                    { // object has no parent, but item has parent
                        parentItem->removeChild(item);
                        addTopLevelItem(item);
                    }
                    else
                    { // object has a parent
                        if (parentItem==nullptr)
                        { // item has no parent
                            int ind=indexOfTopLevelItem(item);
                            takeTopLevelItem(ind);
                            std::map<int,QTreeWidgetItem*>::iterator newParentItem=_allTreeItems.find(parentObj->getObjectHandle());
                            newParentItem->second->addChild(item);
                        }
                        else
                        { // item has a parent
                            int parentItemStoredId=parentItem->data(0,Qt::UserRole+1).toInt();
                            if (parentItemStoredId!=parentObj->getObjectHandle())
                            { // the parents are different!
                                parentItem->removeChild(item);
                                std::map<int,QTreeWidgetItem*>::iterator newParentItem=_allTreeItems.find(parentObj->getObjectHandle());
                                newParentItem->second->addChild(item);
                            }
                        }
                    }
                }
            }
        }
    }

    cnt++;
    if (cnt>100)
        cnt=0;


    _inRefreshRoutine=false;
    printf("Time: %i\n",VDateTime::getTimeDiffInMs(st));
    setUpdatesEnabled(true);
}

void CSceneHierarchyWidget::refresh()
{
    setUpdatesEnabled(false);
    printf("Refresh\n");
    int st=VDateTime::getTimeInMs();
    _inRefreshRoutine=true;
    for (std::map<int,QTreeWidgetItem*>::iterator it=_allTreeItems.begin();it!=_allTreeItems.end();it++)
    {
        C3DObject* obj=App::ct->objCont->getObjectFromHandle(it->first);
        if (obj!=nullptr)
        {
            int data=0;
            if (App::ct->objCont->isObjectSelected(it->first))
            {
                it->second->setSelected(App::ct->objCont->isObjectSelected(it->first));
                data|=1;
                if (App::ct->objCont->getLastSelectionID()==it->first)
                    data|=2;
            }
            else
            {
                it->second->setSelected(false);
                switch (obj->getHierarchyColorIndex())
                {
                    case 0: it->second->setBackgroundColor(0, QColor(255,218,218)); break;
                    case 1: it->second->setBackgroundColor(0, QColor(204,255,204)); break;
                    case 2: it->second->setBackgroundColor(0, QColor(218,218,255)); break;
                    default: it->second->setBackgroundColor(0, QColor(255,255,255)); break;
                }
            }
            if ( ((obj->layer&App::ct->mainSettings->getActiveLayers())==0) || obj->isObjectPartOfInvisibleModel() )
                data|=4;
            if (obj->getModelBase())
                data|=8;
            if ( ((obj->getLocalObjectProperty()&sim_objectproperty_collapsed)==0)&&(obj->childList.size()>0) )
            {
                data|=16;
                it->second->setExpanded(true);
            }
            else
                it->second->setExpanded(false);
            it->second->setData(0,Qt::UserRole,QVariant(data));
            it->second->setData(1,Qt::UserRole,QVariant(data));
            it->second->setData(2,Qt::UserRole,QVariant(data));
            it->second->setData(3,Qt::UserRole,QVariant(data));
        }
    }
    printf("Time: %i\n",VDateTime::getTimeDiffInMs(st));
    _inRefreshRoutine=false;
    setUpdatesEnabled(true);
}

CSceneHierarchyWidget::~CSceneHierarchyWidget()
{
    removeAll();
}

void CSceneHierarchyWidget::removeAll()
{
    clear();
    _allTreeItems.clear();
}

QTreeWidgetItem* CSceneHierarchyWidget::_buildObject(C3DObject* it)
{
    QTreeWidgetItem* item=new QTreeWidgetItem((QTreeWidget*)0,QStringList(it->getObjectName().c_str()));
    _allTreeItems[it->getObjectHandle()]=item;
    item->setIcon(0,*(new QIcon(":/toolbarFiles/cameraShift.png")));
//    QPixmap pix(16,16);
//    QIcon* icn=new QIcon(pix);
//    item->setIcon(2,*icn);
//    item->setIcon(2,*(new QIcon(":/toolbarFiles/camera.png")));
    item->setData(0,Qt::UserRole,QVariant(0));
    item->setData(0,Qt::UserRole+1,QVariant(it->getObjectHandle()));
    item->setData(1,Qt::UserRole,QVariant(0));
    item->setData(2,Qt::UserRole,QVariant(0));
    item->setData(3,Qt::UserRole,QVariant(0));
    return(item);
}

QTreeWidgetItem* CSceneHierarchyWidget::_buildObjectWithHierarchy(C3DObject* it)
{
    QTreeWidgetItem* item=_buildObject(it);
    _allTreeItems[it->getObjectHandle()]=item;

    for (size_t i=0;i<it->childList.size();i++)
    {
        QTreeWidgetItem* child=_buildObjectWithHierarchy(it->childList[i]);
        item->insertChild((int)i,child);
    }
    return(item);
}

void CSceneHierarchyWidget::onItemCollapsed(QTreeWidgetItem* item)
{
    if (!_inRefreshRoutine)
    {
        printf("onItemCollapsed\n");
        SSimulationThreadCommand cmd;
        cmd.cmdId=TOGGLE_EXPAND_COLLAPSE_HIERARCHY_OBJECT_CMD;
        cmd.intParams.push_back(item->data(0,Qt::UserRole+1).toInt());
        App::appendSimulationThreadCommand(cmd);
    }
}

void CSceneHierarchyWidget::onItemExpanded(QTreeWidgetItem* item)
{
    if (!_inRefreshRoutine)
    {
        printf("onItemExpanded\n");
        SSimulationThreadCommand cmd;
        cmd.cmdId=TOGGLE_EXPAND_COLLAPSE_HIERARCHY_OBJECT_CMD;
        cmd.intParams.push_back(item->data(0,Qt::UserRole+1).toInt());
        App::appendSimulationThreadCommand(cmd);
    }
}

void CSceneHierarchyWidget::onItemSelectionChanged()
{
    if (!_inRefreshRoutine)
    {
        printf("onItemSelectionChanged\n");
        SSimulationThreadCommand cmd;
        cmd.cmdId=ADD_OR_REMOVE_TO_FROM_OBJECT_SELECTION_CMD;
        for (std::map<int,QTreeWidgetItem*>::iterator it=_allTreeItems.begin();it!=_allTreeItems.end();it++)
        {
            QTreeWidgetItem* item=it->second;
            cmd.intParams.push_back(it->first);
            cmd.boolParams.push_back(item->isSelected());
        }
        App::appendSimulationThreadCommand(cmd);
    }
}

void CSceneHierarchyWidget::mouseDoubleClickEvent(QMouseEvent* aEvent)
{
    _doubleClickedColumn=-1;
    QTreeWidget::mouseDoubleClickEvent(aEvent);
    const QPoint clickedPosition=aEvent->pos();
    QTreeWidgetItem* item=itemAt(clickedPosition);
    if (item!=nullptr)
    {
        int data=item->data(0,Qt::UserRole).toInt();
        int objHandle=item->data(0,Qt::UserRole+1).toInt();
        if (_doubleClickedColumn==0)
        {
            const QRect itemRectangle=visualItemRect(item);
            if ((data&8)>0)
            { // we have a model
                QRect modelIconRectangle;
                modelIconRectangle.setTopLeft(itemRectangle.topLeft());
                modelIconRectangle.setWidth(CSceneHierarchyItemDelegate::decoSize.width());
                modelIconRectangle.setHeight(CSceneHierarchyItemDelegate::decoSize.height());
                if (modelIconRectangle.contains(clickedPosition))
                {
                    SSimulationThreadCommand cmd;
                    cmd.cmdId=OPEN_MODAL_MODEL_PROPERTIES_CMD;
                    cmd.intParams.push_back(objHandle);
                    App::appendSimulationThreadCommand(cmd);
                    return;
                }
            }

            QRect objectIconRectangle;
            objectIconRectangle.setTopLeft(itemRectangle.topLeft()+QPoint(CSceneHierarchyItemDelegate::decoSize.width()+TREEITEMDELEGATE_ICONSPACING,0));
            objectIconRectangle.setWidth(CSceneHierarchyItemDelegate::decoSize.width());
            objectIconRectangle.setHeight(CSceneHierarchyItemDelegate::decoSize.height());
            if (objectIconRectangle.contains(clickedPosition))
            {
                printf("Object Icon clicked\n");
            }

            QRect textRectangle;
            textRectangle.setTopLeft(itemRectangle.topLeft()+QPoint(2*(CSceneHierarchyItemDelegate::decoSize.width()+TREEITEMDELEGATE_ICONSPACING),0));
            textRectangle.setWidth(itemRectangle.width()-(2*(CSceneHierarchyItemDelegate::decoSize.width()+TREEITEMDELEGATE_ICONSPACING)));
            textRectangle.setHeight(itemRectangle.height());
            if (textRectangle.contains(clickedPosition))
            {
                printf("Text clicked\n");
            }
        }
        if (_doubleClickedColumn==1)
        {
            SSimulationThreadCommand cmd;
            //cmd.cmdId=OPEN_MODAL_CUSTOMIZATION_SCRIPT_EDITOR_CMD;
            cmd.intParams.push_back(objHandle);
            App::appendSimulationThreadCommand(cmd);
            return;
        }
        if (_doubleClickedColumn==2)
        {
            SSimulationThreadCommand cmd;
            //cmd.cmdId=OPEN_CHILD_SCRIPT_EDITOR_CMD;
            cmd.intParams.push_back(objHandle);
            App::appendSimulationThreadCommand(cmd);
            return;
        }
        if (_doubleClickedColumn==3)
        {
            SSimulationThreadCommand cmd;
            //cmd.cmdId=OPEN_JOINT_CALLBACK_SCRIPT_EDITOR_CMD;
            cmd.intParams.push_back(objHandle);
            App::appendSimulationThreadCommand(cmd);
            return;
        }
   }
}

void CSceneHierarchyWidget::onItemDoubleClicked(QTreeWidgetItem* item,int column)
{
    _doubleClickedColumn=column;
}
