
#include "vrepMainHeader.h"
#include "qdlgui.h"
#include "ui_qdlgui.h"
#include "app.h"
#include "tt.h"
#include "gV.h"
#include "editboxdelegate.h"
#include "qdlgtextures.h"
#include "qdlgnewui.h"
#include "qdlguirolledup.h"
#include "v_repStrings.h"
#include <boost/lexical_cast.hpp>


CQDlgUi::CQDlgUi(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgUi)
{
    _dlgType=CUSTOM_UI_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgUi::~CQDlgUi()
{
    delete ui;
}

void CQDlgUi::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    // We close it by ending the button edit mode:
    App::mainWindow->editModeContainer->processCommand(ANY_EDIT_MODE_FINISH_AND_APPLY_CHANGES_EMCMD,nullptr);
}

void CQDlgUi::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    int uiID=App::ct->buttonBlockContainer->getBlockInEdition();
    CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(uiID);

    ui->qqCellCountH->setEnabled(it!=nullptr);
    ui->qqCellCountV->setEnabled(it!=nullptr);
    ui->qqCellSizeH->setEnabled(it!=nullptr);
    ui->qqCellSizeV->setEnabled(it!=nullptr);
    ui->qqUiPosH->setEnabled(it!=nullptr);
    ui->qqUiPosV->setEnabled(it!=nullptr);
    ui->qqUiBorderL->setEnabled(it!=nullptr);
    ui->qqUiBorderT->setEnabled(it!=nullptr);
    ui->qqVisible->setEnabled(it!=nullptr);
    ui->qqMoveable->setEnabled(it!=nullptr);
    ui->qqVisibleDuringSimulation->setEnabled(it!=nullptr);
    ui->qqVisibleWhenObjectSelected->setEnabled(it!=nullptr);
    ui->qqFixedWidthFont->setEnabled(it!=nullptr);
    ui->qqSelectObject->setEnabled(it!=nullptr);
    ui->qqPageCombo->setEnabled(it!=nullptr);
    ui->qqAssociatedObject->setEnabled(it!=nullptr);
    ui->qqSetTexture->setEnabled(it!=nullptr);
    ui->qqAdjustRolledUpSizes->setEnabled(it!=nullptr);

    ui->qqUiBorderL->setChecked((it!=nullptr)&&(it->getAttributes()&sim_ui_property_relativetoleftborder));
    ui->qqUiBorderT->setChecked((it!=nullptr)&&(it->getAttributes()&sim_ui_property_relativetotopborder));
    ui->qqVisible->setChecked((it!=nullptr)&&(it->getAttributes()&sim_ui_property_visible));
    ui->qqMoveable->setChecked((it!=nullptr)&&(it->getAttributes()&sim_ui_property_moveable));
    ui->qqVisibleDuringSimulation->setChecked((it!=nullptr)&&(it->getAttributes()&sim_ui_property_visibleduringsimulationonly));
    ui->qqVisibleWhenObjectSelected->setChecked((it!=nullptr)&&(it->getAttributes()&sim_ui_property_visiblewhenobjectselected));
    ui->qqFixedWidthFont->setChecked((it!=nullptr)&&(it->getAttributes()&sim_ui_property_fixedwidthfont));
    ui->qqSelectObject->setChecked((it!=nullptr)&&(it->getAttributes()&sim_ui_property_selectassociatedobject));

    ui->qqPageCombo->clear();
    ui->qqAssociatedObject->clear();

    if (it!=nullptr)
    {
        VPoint blockSize;
        it->getBlockSize(blockSize);
        ui->qqCellCountH->setText(tt::getIString(false,blockSize.x).c_str());
        ui->qqCellCountV->setText(tt::getIString(false,blockSize.y).c_str());
        VPoint buttonSize;
        it->getButtonSizeOriginal(buttonSize);
        ui->qqCellSizeH->setText(tt::getIString(false,buttonSize.x).c_str());
        ui->qqCellSizeV->setText(tt::getIString(false,buttonSize.y).c_str());
        VPoint blockPos;
        it->getBlockPositionRelative(blockPos);
        ui->qqUiPosH->setText(tt::getIString(false,blockPos.x).c_str());
        ui->qqUiPosV->setText(tt::getIString(false,blockPos.y).c_str());

        ui->qqPageCombo->addItem(strTranslate(IDS_ALL_PAGES),QVariant(-1));
        for (int i=0;i<8;i++)
        {
            std::string tmp(std::string(strTranslate(IDSN_PAGE_X))+" "+boost::lexical_cast<std::string>(i+1));
            ui->qqPageCombo->addItem(tmp.c_str(),QVariant(i));
        }
        for (int i=0;i<ui->qqPageCombo->count();i++)
        {
            if (ui->qqPageCombo->itemData(i).toInt()==it->getViewToAppearOn())
            {
                ui->qqPageCombo->setCurrentIndex(i);
                break;
            }
        }

        std::vector<std::string> names;
        std::vector<int> ids;
        ui->qqAssociatedObject->addItem(strTranslate(IDSN_NONE),QVariant(-1));
        for (size_t i=0;i<App::ct->objCont->objectList.size();i++)
        {
            C3DObject* it2=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
            names.push_back(it2->getObjectName());
            ids.push_back(it2->getObjectHandle());
        }
        tt::orderStrings(names,ids);
        for (size_t i=0;i<names.size();i++)
            ui->qqAssociatedObject->addItem(names[i].c_str(),QVariant(ids[i]));
        for (int i=0;i<ui->qqAssociatedObject->count();i++)
        {
            if (ui->qqAssociatedObject->itemData(i).toInt()==it->getObjectIDAttachedTo())
            {
                ui->qqAssociatedObject->setCurrentIndex(i);
                break;
            }
        }
    }
    else
    {
        ui->qqCellCountH->setText("");
        ui->qqCellCountV->setText("");
        ui->qqCellSizeH->setText("");
        ui->qqCellSizeV->setText("");
        ui->qqUiPosH->setText("");
        ui->qqUiPosV->setText("");
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgUi::on_qqAddNew_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgNewUi theDialog(this);
        theDialog.refresh();
        if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=ADD_UI_OPENGLUIBLOCKGUITRIGGEREDCMD;
            cmd.intParams.push_back(theDialog.xSize);
            cmd.intParams.push_back(theDialog.ySize);
            cmd.intParams.push_back(theDialog.menuBarAttributes);
            App::appendSimulationThreadCommand(cmd);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqCellCountH_editingFinished()
{
    if (!ui->qqCellCountH->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqCellCountH->text().toInt(&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_CELLCNT_OPENGLUIBLOCKGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->buttonBlockContainer->getBlockInEdition());
            cmd.intParams.push_back(newVal);
            cmd.boolParams.push_back(false);
            App::appendSimulationThreadCommand(cmd);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqCellCountV_editingFinished()
{
    if (!ui->qqCellCountV->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqCellCountV->text().toInt(&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_CELLCNT_OPENGLUIBLOCKGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->buttonBlockContainer->getBlockInEdition());
            cmd.intParams.push_back(newVal);
            cmd.boolParams.push_back(true);
            App::appendSimulationThreadCommand(cmd);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqCellSizeH_editingFinished()
{
    if (!ui->qqCellSizeH->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqCellSizeH->text().toInt(&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_CELLSIZE_OPENGLUIBLOCKGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->buttonBlockContainer->getBlockInEdition());
            cmd.intParams.push_back(newVal);
            cmd.boolParams.push_back(false);
            App::appendSimulationThreadCommand(cmd);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqCellSizeV_editingFinished()
{
    if (!ui->qqCellSizeV->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqCellSizeV->text().toInt(&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_CELLSIZE_OPENGLUIBLOCKGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->buttonBlockContainer->getBlockInEdition());
            cmd.intParams.push_back(newVal);
            cmd.boolParams.push_back(true);
            App::appendSimulationThreadCommand(cmd);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqUiPosH_editingFinished()
{
    if (!ui->qqUiPosH->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqUiPosH->text().toInt(&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_UIPOSITION_OPENGLUIBLOCKGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->buttonBlockContainer->getBlockInEdition());
            cmd.intParams.push_back(newVal);
            cmd.boolParams.push_back(false);
            App::appendSimulationThreadCommand(cmd);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqUiPosV_editingFinished()
{
    if (!ui->qqUiPosV->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqUiPosV->text().toInt(&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_UIPOSITION_OPENGLUIBLOCKGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->buttonBlockContainer->getBlockInEdition());
            cmd.intParams.push_back(newVal);
            cmd.boolParams.push_back(true);
            App::appendSimulationThreadCommand(cmd);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqUiBorderL_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(App::ct->buttonBlockContainer->getBlockInEdition());
        if (itBlock!=nullptr)
        {
            App::appendSimulationThreadCommand(SET_ATTRIBUTES_OPENGLUIBLOCKGUITRIGGEREDCMD,App::ct->buttonBlockContainer->getBlockInEdition(),itBlock->getAttributes()^sim_ui_property_relativetoleftborder);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqUiBorderT_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(App::ct->buttonBlockContainer->getBlockInEdition());
        if (itBlock!=nullptr)
        {
            App::appendSimulationThreadCommand(SET_ATTRIBUTES_OPENGLUIBLOCKGUITRIGGEREDCMD,App::ct->buttonBlockContainer->getBlockInEdition(),itBlock->getAttributes()^sim_ui_property_relativetotopborder);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqVisible_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(App::ct->buttonBlockContainer->getBlockInEdition());
        if (itBlock!=nullptr)
        {
            App::appendSimulationThreadCommand(SET_ATTRIBUTES_OPENGLUIBLOCKGUITRIGGEREDCMD,App::ct->buttonBlockContainer->getBlockInEdition(),itBlock->getAttributes()^sim_ui_property_visible);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqMoveable_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(App::ct->buttonBlockContainer->getBlockInEdition());
        if (itBlock!=nullptr)
        {
            App::appendSimulationThreadCommand(SET_ATTRIBUTES_OPENGLUIBLOCKGUITRIGGEREDCMD,App::ct->buttonBlockContainer->getBlockInEdition(),itBlock->getAttributes()^sim_ui_property_moveable);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqVisibleDuringSimulation_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(App::ct->buttonBlockContainer->getBlockInEdition());
        if (itBlock!=nullptr)
        {
            App::appendSimulationThreadCommand(SET_ATTRIBUTES_OPENGLUIBLOCKGUITRIGGEREDCMD,App::ct->buttonBlockContainer->getBlockInEdition(),itBlock->getAttributes()^sim_ui_property_visibleduringsimulationonly);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqFixedWidthFont_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(App::ct->buttonBlockContainer->getBlockInEdition());
        if (itBlock!=nullptr)
        {
            App::appendSimulationThreadCommand(SET_ATTRIBUTES_OPENGLUIBLOCKGUITRIGGEREDCMD,App::ct->buttonBlockContainer->getBlockInEdition(),itBlock->getAttributes()^sim_ui_property_fixedwidthfont);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqVisibleWhenObjectSelected_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(App::ct->buttonBlockContainer->getBlockInEdition());
        if (itBlock!=nullptr)
        {
            App::appendSimulationThreadCommand(SET_ATTRIBUTES_OPENGLUIBLOCKGUITRIGGEREDCMD,App::ct->buttonBlockContainer->getBlockInEdition(),itBlock->getAttributes()^sim_ui_property_visiblewhenobjectselected);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqSelectObject_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(App::ct->buttonBlockContainer->getBlockInEdition());
        if (itBlock!=nullptr)
        {
            App::appendSimulationThreadCommand(SET_ATTRIBUTES_OPENGLUIBLOCKGUITRIGGEREDCMD,App::ct->buttonBlockContainer->getBlockInEdition(),itBlock->getAttributes()^sim_ui_property_selectassociatedobject);
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgUi::on_qqPageCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            App::appendSimulationThreadCommand(SET_DISPLAYPAGE_OPENGLUIBLOCKGUITRIGGEREDCMD,App::ct->buttonBlockContainer->getBlockInEdition(),ui->qqPageCombo->itemData(ui->qqPageCombo->currentIndex()).toInt());
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgUi::on_qqAssociatedObject_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            App::appendSimulationThreadCommand(SET_ASSOCIATEDWITHOBJECT_OPENGLUIBLOCKGUITRIGGEREDCMD,App::ct->buttonBlockContainer->getBlockInEdition(),ui->qqAssociatedObject->itemData(ui->qqAssociatedObject->currentIndex()).toInt());
            //App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgUi::on_qqSetTexture_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgTextures::displayDlg(TEXTURE_ID_OPENGL_GUI_BACKGROUND,App::ct->buttonBlockContainer->getBlockInEdition(),-1,App::mainWindow);
    }
}

void CQDlgUi::on_qqAdjustRolledUpSizes_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgUiRolledUp theDialog(this);
        theDialog.itBlockID=App::ct->buttonBlockContainer->getBlockInEdition();
        theDialog.refresh();
        theDialog.makeDialogModal(); // messages posted in there
    }
}
