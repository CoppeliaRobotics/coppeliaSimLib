
#include "vrepMainHeader.h"
#include "qdlgtextureselection.h"
#include "ui_qdlgtextureselection.h"
#include "app.h"
#include "tt.h"
#include "v_repStrings.h"
#include <boost/lexical_cast.hpp>

CQDlgTextureSelection::CQDlgTextureSelection(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgTextureSelection)
{
    ui->setupUi(this);
    selectedTextureObject=-1;
    refresh();
}

CQDlgTextureSelection::~CQDlgTextureSelection()
{
    delete ui;
}

void CQDlgTextureSelection::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgTextureSelection::okEvent()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgTextureSelection::refresh()
{
    ui->qqTextureList->clear();
    int itemCount=0;
    while (App::ct->textureCont->getObjectAtIndex(itemCount)!=nullptr)
    {
        CTextureObject* it=App::ct->textureCont->getObjectAtIndex(itemCount);
        std::string txt(it->getObjectName());
        int sx,sy;
        it->getTextureSize(sx,sy);
        txt+=" [";
        txt+=boost::lexical_cast<std::string>(sx)+"x"+boost::lexical_cast<std::string>(sy)+"] ";
        txt+=tt::decorateString(" (",strTranslate(IDSN_STATIC_TEXTURE),")");
        QListWidgetItem* itm=new QListWidgetItem(txt.c_str());
        itm->setData(Qt::UserRole,QVariant(it->getObjectID()));
        itm->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->qqTextureList->addItem(itm);
        itemCount++;
    }
    for (int i=0;i<int(App::ct->objCont->visionSensorList.size());i++)
    {
        CVisionSensor* rs=App::ct->objCont->getVisionSensor(App::ct->objCont->visionSensorList[i]);
        std::string txt(rs->getObjectName());
        int s[2];
        rs->getRealResolution(s);
        txt+=" [";
        txt+=boost::lexical_cast<std::string>(s[0])+"x"+boost::lexical_cast<std::string>(s[1])+"] ";
        txt+=tt::decorateString(" (",strTranslate(IDSN_DYNAMIC_TEXTURE),")");
        QListWidgetItem* itm=new QListWidgetItem(txt.c_str());
        itm->setData(Qt::UserRole,QVariant(rs->getObjectHandle()));
        itm->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->qqTextureList->addItem(itm);
        itemCount++;
    }
}

void CQDlgTextureSelection::on_qqOkCancel_accepted()
{
    QList<QListWidgetItem*> sel=ui->qqTextureList->selectedItems();
    if (sel.size()>0)
        selectedTextureObject=sel.at(0)->data(Qt::UserRole).toInt();
    else
        selectedTextureObject=-1;
    defaultModalDialogEndRoutine(true);
}

void CQDlgTextureSelection::on_qqOkCancel_rejected()
{
    selectedTextureObject=-1;
    defaultModalDialogEndRoutine(false);
}
