
#include "vrepMainHeader.h"
#include "qdlgcollisions.h"
#include "ui_qdlgcollisions.h"
#include "tt.h"
#include <QShortcut>
#include "qdlgentityvsentityselection.h"
#include "editboxdelegate.h"
#include "qdlgmaterial.h"
#include "app.h"

CQDlgCollisions::CQDlgCollisions(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgCollisions)
{
    _dlgType=COLLISION_DLG;
    ui->setupUi(this);
    inSelectionRoutine=false;
    QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcut,SIGNAL(activated()), this, SLOT(onDeletePressed()));
    QShortcut* shortcut2 = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
    connect(shortcut2,SIGNAL(activated()), this, SLOT(onDeletePressed()));
    CEditBoxDelegate* delegate=new CEditBoxDelegate();
    ui->qqCollisionList->setItemDelegate(delegate);
}

CQDlgCollisions::~CQDlgCollisions()
{
    delete ui;
}

void CQDlgCollisions::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(CALCULATION_DLG);
}

void CQDlgCollisions::dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    if ( (cmdIn!=nullptr)&&(cmdIn->intParams[0]==_dlgType) )
    {
        if (cmdIn->intParams[1]==0)
            selectObjectInList(cmdIn->intParams[2]);
    }
}

void CQDlgCollisions::refresh()
{
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();

    ui->qqEnableAll->setChecked(App::ct->mainSettings->collisionDetectionEnabled);

    if (!inSelectionRoutine)
    {
        int selectedObjectID=getSelectedObjectID();
        updateObjectsInList();
        selectObjectInList(selectedObjectID);
    }

    CRegCollision* it=App::ct->collisions->getObject(getSelectedObjectID());
    ui->qqComputeContour->setEnabled((it!=nullptr)&&it->canComputeCollisionContour()&&noEditModeNoSim);

    ui->qqAddNewObject->setEnabled(noEditModeNoSim);
    ui->qqCollisionColor->setEnabled(noEditModeNoSim);
    ui->qqColliderColorChanges->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqCollideeColorChanges->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqExplicitHandling->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqContourWidth->setEnabled((it!=nullptr)&&it->getExhaustiveDetection()&&noEditModeNoSim);
    ui->qqAdjustContourColor->setEnabled((it!=nullptr)&&it->getExhaustiveDetection()&&noEditModeNoSim);
    if (it!=nullptr)
    {
        ui->qqComputeContour->setChecked(it->getExhaustiveDetection());
        ui->qqColliderColorChanges->setChecked(it->getColliderChangesColor());
        ui->qqCollideeColorChanges->setChecked(it->getCollideeChangesColor());
        ui->qqExplicitHandling->setChecked(it->getExplicitHandling());
        ui->qqContourWidth->setText(tt::getIString(false,it->getContourWidth()).c_str());
    }
    else
    {
        ui->qqComputeContour->setChecked(false);
        ui->qqColliderColorChanges->setChecked(false);
        ui->qqCollideeColorChanges->setChecked(false);
        ui->qqExplicitHandling->setChecked(false);
        ui->qqContourWidth->setText("");
    }
    selectLineEdit(lineEditToSelect);
}

void CQDlgCollisions::updateObjectsInList()
{
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();
    ui->qqCollisionList->setEnabled(noEditModeNoSim);
    ui->qqCollisionList->clear();
    for (size_t i=0;i<App::ct->collisions->collisionObjects.size();i++)
    {
        CRegCollision* it=App::ct->collisions->collisionObjects[i];
        std::string tmp=it->getObjectPartnersName();
        int id=it->getObjectID();
        QListWidgetItem* itm=new QListWidgetItem(tmp.c_str());
        itm->setData(Qt::UserRole,QVariant(id));
        itm->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
        ui->qqCollisionList->addItem(itm);
    }
}

void CQDlgCollisions::on_qqAddNewObject_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgEntityVsEntitySelection theDialog(this);
        theDialog.initialize(0);
        if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
        {
            App::appendSimulationThreadCommand(ADD_NEW_COLLISIONGUITRIGGEREDCMD,theDialog.entity1,theDialog.entity2);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

int CQDlgCollisions::getSelectedObjectID()
{
    QList<QListWidgetItem*> sel=ui->qqCollisionList->selectedItems();
    if (sel.size()>0)
        return(sel.at(0)->data(Qt::UserRole).toInt());
    return(-1);
}

void CQDlgCollisions::selectObjectInList(int objectID)
{
    for (int i=0;i<ui->qqCollisionList->count();i++)
    {
        QListWidgetItem* it=ui->qqCollisionList->item(i);
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


void CQDlgCollisions::on_qqCollisionList_itemSelectionChanged()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        int objID=getSelectedObjectID();
        CRegCollision* coll=App::ct->collisions->getObject(objID);
        if (coll!=nullptr)
            ((CEditBoxDelegate*)ui->qqCollisionList->itemDelegate())->initialText=coll->getObjectName();
        else
            ((CEditBoxDelegate*)ui->qqCollisionList->itemDelegate())->initialText="";
        inSelectionRoutine=true;
        refresh();
        inSelectionRoutine=false;
    }
}

void CQDlgCollisions::on_qqCollisionList_itemChanged(QListWidgetItem *item)
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (item!=nullptr)
        {
            std::string newName(item->text().toStdString());
            CRegCollision* it=App::ct->collisions->getObject(item->data(Qt::UserRole).toInt());
            if ( (it!=nullptr)&&(newName!="") )
            {
                if (it->getObjectName()!=newName)
                {
                    tt::removeIllegalCharacters(newName,true);
                    if (App::ct->collisions->getObject(newName)==nullptr)
                    {
                        App::appendSimulationThreadCommand(SET_OBJECTNAME_COLLISIONGUITRIGGEREDCMD,it->getObjectID(),-1,0.0,0.0,newName.c_str());
                        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                    }
                }
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCollisions::onDeletePressed()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (focusWidget()==ui->qqCollisionList)
        {
            int objID=getSelectedObjectID();
            if (objID!=-1)
            {
                App::appendSimulationThreadCommand(DELETE_OBJECT_COLLISIONGUITRIGGEREDCMD,objID);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            }
        }
    }
}

void CQDlgCollisions::on_qqEnableAll_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_ENABLE_ALL_COLLISIONGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCollisions::on_qqCollisionColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_COLLISION,-1,-1,App::mainWindow);
    }
}

void CQDlgCollisions::on_qqExplicitHandling_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CRegCollision* it=App::ct->collisions->getObject(getSelectedObjectID());
        if (it!=nullptr)
        {
            App::appendSimulationThreadCommand(TOGGLE_EXPLICITHANDLING_COLLISIONGUITRIGGEREDCMD,it->getObjectID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCollisions::on_qqColliderColorChanges_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CRegCollision* it=App::ct->collisions->getObject(getSelectedObjectID());
        if (it!=nullptr)
        {
            App::appendSimulationThreadCommand(TOGGLE_COLLIDERCOLORCHANGES_COLLISIONGUITRIGGEREDCMD,it->getObjectID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCollisions::on_qqComputeContour_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CRegCollision* it=App::ct->collisions->getObject(getSelectedObjectID());
        if (it!=nullptr)
        {
            App::appendSimulationThreadCommand(TOGGLE_COLLISIONCONTOUR_COLLISIONGUITRIGGEREDCMD,it->getObjectID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCollisions::on_qqCollideeColorChanges_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CRegCollision* it=App::ct->collisions->getObject(getSelectedObjectID());
        if (it!=nullptr)
        {
            App::appendSimulationThreadCommand(TOGGLE_COLLIDEECOLORCHANGES_COLLISIONGUITRIGGEREDCMD,it->getObjectID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCollisions::on_qqAdjustContourColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_COLLISIONCONTOUR,getSelectedObjectID(),-1,App::mainWindow);
    }
}

void CQDlgCollisions::on_qqContourWidth_editingFinished()
{
    if (!ui->qqContourWidth->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CRegCollision* it=App::ct->collisions->getObject(getSelectedObjectID());
        if (it!=nullptr)
        {
            bool ok;
            int newVal=ui->qqContourWidth->text().toInt(&ok);
            if (ok)
            {
                App::appendSimulationThreadCommand(SET_CONTOURWIDTH_COLLISIONGUITRIGGEREDCMD,it->getObjectID(),newVal);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
