#include "qdlguserparameters.h"
#include "ui_qdlguserparameters.h"
#include <QShortcut>
#include "editboxdelegate.h"
#include "app.h"
#include "tt.h"

CQDlgUserParameters::CQDlgUserParameters(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgUserParameters)
{
    ui->setupUi(this);
    inSelectionRoutine=false;
    QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcut,SIGNAL(activated()), this, SLOT(onDeletePressed()));
    QShortcut* shortcut2 = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
    connect(shortcut2,SIGNAL(activated()), this, SLOT(onDeletePressed()));
    CEditBoxDelegate* delegate=new CEditBoxDelegate();
    ui->qqParameterList->setItemDelegate(delegate);
}

CQDlgUserParameters::~CQDlgUserParameters()
{
    delete ui;
}

void CQDlgUserParameters::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgUserParameters::okEvent()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgUserParameters::refresh()
{
    ui->qqAddNew->setEnabled(App::currentWorld->simulation->isSimulationStopped());
    ui->qqUp->setEnabled(App::currentWorld->simulation->isSimulationStopped());
    ui->qqDown->setEnabled(App::currentWorld->simulation->isSimulationStopped());
    int selectedObjectID=getSelectedObjectID();
    if (!inSelectionRoutine)
    {
        updateObjectsInList();
        selectObjectInList(selectedObjectID);
    }
    refreshPart2();
}

void CQDlgUserParameters::refreshPart2()
{
    bool noEditModeAndNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped();

    ui->qqAddNew->setEnabled(noEditModeAndNoSim);
    CUserParameters* it=object->getUserScriptParameterObject();
    int selectedObjectID=getSelectedObjectID();

    bool sel=(selectedObjectID>=0)&&(selectedObjectID<int(it->userParamEntries.size()));

    ui->qqValue->setEnabled(sel&&((it->userParamEntries[selectedObjectID].properties&4)==0));
    ui->qqUnit->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqPrivate->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqPersistent->setEnabled(sel&&noEditModeAndNoSim);

    if (sel)
    {
        if (it->userParamEntries[selectedObjectID].properties&4)
            ui->qqValue->setText("Contains embedded zeros");
        else
            ui->qqValue->setText(it->userParamEntries[selectedObjectID].value.c_str());
        ui->qqUnit->setText(it->userParamEntries[selectedObjectID].unit.c_str());
        ui->qqPrivate->setChecked(it->userParamEntries[selectedObjectID].properties&1);
        ui->qqPersistent->setChecked((it->userParamEntries[selectedObjectID].properties&2)!=0);
    }
    else
    {
        ui->qqValue->setText("");
        ui->qqUnit->setText("");
        ui->qqPrivate->setChecked(false);
        ui->qqPersistent->setChecked(false);
    }
}

void CQDlgUserParameters::on_qqAddNew_clicked()
{
    CUserParameters* p=object->getUserScriptParameterObject();
    std::string name("defaultVariableName");
    std::string dummy;
    while (p->getParameterValue(name.c_str(),dummy))
        name=tt::generateNewName_noHash(name);
    p->addParameterValue(name.c_str(),"defaultUnit","defaultValue",12);
    updateObjectsInList();
    selectObjectInList((int)p->userParamEntries.size()-1);
    refresh();
}

void CQDlgUserParameters::on_qqParameterList_itemChanged(QListWidgetItem *item)
{
    if (item!=nullptr)
    {
        std::string newName(item->text().toStdString());
        CUserParameters* it=object->getUserScriptParameterObject();
        int ind=item->data(Qt::UserRole).toInt();
        if ( (ind>=0)&&(ind<int(it->userParamEntries.size())) )
        {
            std::string dummy;
            if (!it->getParameterValue(newName.c_str(),dummy))
            {
                if (newName.length()!=0)
                    it->userParamEntries[ind].name=newName;
            }
        }
        refresh();
    }
}

void CQDlgUserParameters::on_qqValue_editingFinished()
{
    if (!ui->qqValue->isModified())
        return;
    CUserParameters* it=object->getUserScriptParameterObject();
    int _itemIndex=getSelectedObjectID();
    if ( (_itemIndex>=0)&&(_itemIndex<int(it->userParamEntries.size())) )
        it->userParamEntries[_itemIndex].value=ui->qqValue->text().toStdString();
    refreshPart2();
}

void CQDlgUserParameters::on_qqUnit_editingFinished()
{
    if (!ui->qqUnit->isModified())
        return;
    CUserParameters* it=object->getUserScriptParameterObject();
    int _itemIndex=getSelectedObjectID();
    if ( (_itemIndex>=0)&&(_itemIndex<int(it->userParamEntries.size())) )
        it->userParamEntries[_itemIndex].unit=ui->qqUnit->text().toStdString();
    refreshPart2();
}

void CQDlgUserParameters::on_qqPrivate_clicked()
{
    CUserParameters* it=object->getUserScriptParameterObject();
    int _itemIndex=getSelectedObjectID();
    if ( (_itemIndex>=0)&&(_itemIndex<int(it->userParamEntries.size())) )
        it->userParamEntries[_itemIndex].properties^=1;
    refreshPart2();
}

void CQDlgUserParameters::on_qqPersistent_clicked()
{
    CUserParameters* it=object->getUserScriptParameterObject();
    int _itemIndex=getSelectedObjectID();
    if ( (_itemIndex>=0)&&(_itemIndex<int(it->userParamEntries.size())) )
        it->userParamEntries[_itemIndex].properties^=2;
    refreshPart2();
}

void CQDlgUserParameters::on_qqClose_clicked(QAbstractButton *button)
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgUserParameters::onDeletePressed()
{
    if (focusWidget()==ui->qqParameterList)
    {
        int dataID=getSelectedObjectID();
        if (dataID!=-1)
        {
            CUserParameters* p=object->getUserScriptParameterObject();
            p->removeParameterValue(dataID);
            refresh();
        }
        return;
    }
}

void CQDlgUserParameters::updateObjectsInList()
{
    ui->qqParameterList->clear();
    CUserParameters* it=object->getUserScriptParameterObject();
    for (size_t i=0;i<it->userParamEntries.size();i++)
    {
        if ( ((it->userParamEntries[i].properties&1)==0)||App::currentWorld->simulation->isSimulationStopped() )
        {
            QListWidgetItem* itm=new QListWidgetItem(it->userParamEntries[i].name.c_str());
            itm->setData(Qt::UserRole,QVariant(int(i)));
            itm->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
            ui->qqParameterList->addItem(itm);
        }
    }
}

int CQDlgUserParameters::getSelectedObjectID()
{
    QList<QListWidgetItem*> sel=ui->qqParameterList->selectedItems();
    if (sel.size()>0)
        return(sel.at(0)->data(Qt::UserRole).toInt());
    return(-1);
}

void CQDlgUserParameters::selectObjectInList(int objectID)
{
    for (int i=0;i<ui->qqParameterList->count();i++)
    {
        QListWidgetItem* it=ui->qqParameterList->item(i);
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

void CQDlgUserParameters::on_qqParameterList_itemSelectionChanged()
{
    int objID=getSelectedObjectID();
    if ( (objID>=0)&&(objID<int(object->getUserScriptParameterObject()->userParamEntries.size())) )
        ((CEditBoxDelegate*)ui->qqParameterList->itemDelegate())->initialText=object->getUserScriptParameterObject()->userParamEntries[objID].name.c_str();
    else
        ((CEditBoxDelegate*)ui->qqParameterList->itemDelegate())->initialText="";
    inSelectionRoutine=true;
    refresh();
    inSelectionRoutine=false;
}

void CQDlgUserParameters::on_qqUp_clicked()
{
    CUserParameters* it=object->getUserScriptParameterObject();
    int _itemIndex=getSelectedObjectID();
    if ( (_itemIndex>0)&&(_itemIndex<int(it->userParamEntries.size())) )
    {
        it->moveItem(_itemIndex,_itemIndex-1);
        updateObjectsInList();
        selectObjectInList(_itemIndex-1);
        refresh();
    }
}

void CQDlgUserParameters::on_qqDown_clicked()
{
    CUserParameters* it=object->getUserScriptParameterObject();
    int _itemIndex=getSelectedObjectID();
    if ( (_itemIndex>=0)&&(_itemIndex<int(it->userParamEntries.size())-1) )
    {
        it->moveItem(_itemIndex,_itemIndex+1);
        updateObjectsInList();
        selectObjectInList(_itemIndex+1);
        refresh();
    }
}
