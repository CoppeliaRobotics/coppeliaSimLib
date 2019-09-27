
#include "vrepMainHeader.h"
#include "qdlgscriptparameters.h"
#include "ui_qdlgscriptparameters.h"
#include <QShortcut>
#include "editboxdelegate.h"
#include "app.h"
#include "tt.h"

CQDlgScriptParameters::CQDlgScriptParameters(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgScriptParameters)
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

CQDlgScriptParameters::~CQDlgScriptParameters()
{
    delete ui;
}

void CQDlgScriptParameters::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgScriptParameters::okEvent()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgScriptParameters::refresh()
{
    ui->qqAddNew->setEnabled(App::ct->simulation->isSimulationStopped());
    ui->qqUp->setEnabled(App::ct->simulation->isSimulationStopped());
    ui->qqDown->setEnabled(App::ct->simulation->isSimulationStopped());
    int selectedObjectID=getSelectedObjectID();
    if (!inSelectionRoutine)
    {
        updateObjectsInList();
        selectObjectInList(selectedObjectID);
    }
    refreshPart2();
}

void CQDlgScriptParameters::refreshPart2()
{
    bool noEditModeAndNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();

    ui->qqAddNew->setEnabled(noEditModeAndNoSim);
    CLuaScriptParameters* it=script->getScriptParametersObject();
    int selectedObjectID=getSelectedObjectID();

    bool sel=(selectedObjectID>=0)&&(selectedObjectID<int(it->scriptParamEntries.size()));

    ui->qqValue->setEnabled(sel&&((it->scriptParamEntries[selectedObjectID].properties&4)==0));
    ui->qqUnit->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqPrivate->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqPersistent->setEnabled(sel&&noEditModeAndNoSim);

    if (sel)
    {
        if (it->scriptParamEntries[selectedObjectID].properties&4)
            ui->qqValue->setText("Contains embedded zeros");
        else
            ui->qqValue->setText(it->scriptParamEntries[selectedObjectID].value.c_str());
        ui->qqUnit->setText(it->scriptParamEntries[selectedObjectID].unit.c_str());
        ui->qqPrivate->setChecked(it->scriptParamEntries[selectedObjectID].properties&1);
        ui->qqPersistent->setChecked((it->scriptParamEntries[selectedObjectID].properties&2)!=0);
    }
    else
    {
        ui->qqValue->setText("");
        ui->qqUnit->setText("");
        ui->qqPrivate->setChecked(false);
        ui->qqPersistent->setChecked(false);
    }
}

void CQDlgScriptParameters::on_qqAddNew_clicked()
{
    CLuaScriptParameters* p=script->getScriptParametersObject();
    std::string name("defaultVariableName");
    std::string dummy;
    while (p->getParameterValue(name.c_str(),dummy))
        name=tt::generateNewName_noDash(name);
    script->getScriptParametersObject()->addParameterValue(name.c_str(),"defaultUnit","defaultValue",12);
    updateObjectsInList();
    selectObjectInList((int)p->scriptParamEntries.size()-1);
    refresh();
}

void CQDlgScriptParameters::on_qqParameterList_itemChanged(QListWidgetItem *item)
{
    if (item!=nullptr)
    {
        std::string newName(item->text().toStdString());
        CLuaScriptParameters* it=script->getScriptParametersObject();
        int ind=item->data(Qt::UserRole).toInt();
        if ( (ind>=0)&&(ind<int(it->scriptParamEntries.size())) )
        {
            std::string dummy;
            if (!it->getParameterValue(newName.c_str(),dummy))
            {
                if (newName.length()!=0)
                    it->scriptParamEntries[ind].name=newName;
            }
        }
        refresh();
    }
}

void CQDlgScriptParameters::on_qqValue_editingFinished()
{
    if (!ui->qqValue->isModified())
        return;
    CLuaScriptParameters* it=script->getScriptParametersObject();
    int _itemIndex=getSelectedObjectID();
    if ( (_itemIndex>=0)&&(_itemIndex<int(it->scriptParamEntries.size())) )
        it->scriptParamEntries[_itemIndex].value=ui->qqValue->text().toStdString();
    refreshPart2();
}

void CQDlgScriptParameters::on_qqUnit_editingFinished()
{
    if (!ui->qqUnit->isModified())
        return;
    CLuaScriptParameters* it=script->getScriptParametersObject();
    int _itemIndex=getSelectedObjectID();
    if ( (_itemIndex>=0)&&(_itemIndex<int(it->scriptParamEntries.size())) )
        it->scriptParamEntries[_itemIndex].unit=ui->qqUnit->text().toStdString();
    refreshPart2();
}

void CQDlgScriptParameters::on_qqPrivate_clicked()
{
    CLuaScriptParameters* it=script->getScriptParametersObject();
    int _itemIndex=getSelectedObjectID();
    if ( (_itemIndex>=0)&&(_itemIndex<int(it->scriptParamEntries.size())) )
        it->scriptParamEntries[_itemIndex].properties^=1;
    refreshPart2();
}

void CQDlgScriptParameters::on_qqPersistent_clicked()
{
    CLuaScriptParameters* it=script->getScriptParametersObject();
    int _itemIndex=getSelectedObjectID();
    if ( (_itemIndex>=0)&&(_itemIndex<int(it->scriptParamEntries.size())) )
        it->scriptParamEntries[_itemIndex].properties^=2;
    refreshPart2();
}

void CQDlgScriptParameters::on_qqClose_clicked(QAbstractButton *button)
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgScriptParameters::onDeletePressed()
{
    if (focusWidget()==ui->qqParameterList)
    {
        int dataID=getSelectedObjectID();
        if (dataID!=-1)
        {
            CLuaScriptParameters* p=script->getScriptParametersObject();
            p->removeParameterValue(dataID);
            refresh();
        }
        return;
    }
}

void CQDlgScriptParameters::updateObjectsInList()
{
    ui->qqParameterList->clear();
    CLuaScriptParameters* it=script->getScriptParametersObject();
    for (size_t i=0;i<it->scriptParamEntries.size();i++)
    {
        if ( ((it->scriptParamEntries[i].properties&1)==0)||App::ct->simulation->isSimulationStopped() )
        {
            QListWidgetItem* itm=new QListWidgetItem(it->scriptParamEntries[i].name.c_str());
            itm->setData(Qt::UserRole,QVariant(int(i)));
            itm->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
            ui->qqParameterList->addItem(itm);
        }
    }
}

int CQDlgScriptParameters::getSelectedObjectID()
{
    QList<QListWidgetItem*> sel=ui->qqParameterList->selectedItems();
    if (sel.size()>0)
        return(sel.at(0)->data(Qt::UserRole).toInt());
    return(-1);
}

void CQDlgScriptParameters::selectObjectInList(int objectID)
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

void CQDlgScriptParameters::on_qqParameterList_itemSelectionChanged()
{
    int objID=getSelectedObjectID();
    if ( (objID>=0)&&(objID<int(script->getScriptParametersObject()->scriptParamEntries.size())) )
        ((CEditBoxDelegate*)ui->qqParameterList->itemDelegate())->initialText=script->getScriptParametersObject()->scriptParamEntries[objID].name.c_str();
    else
        ((CEditBoxDelegate*)ui->qqParameterList->itemDelegate())->initialText="";
    inSelectionRoutine=true;
    refresh();
    inSelectionRoutine=false;
}

void CQDlgScriptParameters::on_qqUp_clicked()
{
    CLuaScriptParameters* it=script->getScriptParametersObject();
    int _itemIndex=getSelectedObjectID();
    if ( (_itemIndex>0)&&(_itemIndex<int(it->scriptParamEntries.size())) )
    {
        it->moveItem(_itemIndex,_itemIndex-1);
        updateObjectsInList();
        selectObjectInList(_itemIndex-1);
        refresh();
    }
}

void CQDlgScriptParameters::on_qqDown_clicked()
{
    CLuaScriptParameters* it=script->getScriptParametersObject();
    int _itemIndex=getSelectedObjectID();
    if ( (_itemIndex>=0)&&(_itemIndex<int(it->scriptParamEntries.size())-1) )
    {
        it->moveItem(_itemIndex,_itemIndex+1);
        updateObjectsInList();
        selectObjectInList(_itemIndex+1);
        refresh();
    }
}
