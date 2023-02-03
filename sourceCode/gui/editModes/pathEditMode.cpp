#include <pathEditMode.h>
#include <simConst.h>
#include <global.h>
#include <simStringTable.h>
#include <app.h>

CPathEditMode_old::CPathEditMode_old(CPath_old* path,CSceneObjectContainer* objCont)
{
    _editionPathCont=path->pathContainer->copyYourself();
    _path=path;
    _objCont=objCont;
}

CPathEditMode_old::~CPathEditMode_old()
{
}

void CPathEditMode_old::endEditMode(bool cancelChanges)
{ // return true means: select the edition object, otherwise it was erased
    // delete this object right after calling this function!

    for (int i=0;i<int(editBufferPathPointsCopy.size());i++)
        delete editBufferPathPointsCopy[i];
    editBufferPathPointsCopy.clear();

    if (!cancelChanges)
    {
        delete _path->pathContainer;
        _path->pathContainer=_editionPathCont;
    }
    else
        delete _editionPathCont;
    _editionPathCont=nullptr;
}

CPathCont_old* CPathEditMode_old::getEditModePathContainer_old()
{
    return(_editionPathCont);
}

CPath_old* CPathEditMode_old::getEditModePath_old()
{
    return(_path);
}

int CPathEditMode_old::getEditModeBufferSize()
{
    return(int(editModeBuffer.size()));
}

int CPathEditMode_old::getLastEditModeBufferValue()
{
    return(editModeBuffer[editModeBuffer.size()-1]);
}

int CPathEditMode_old::getEditModeBufferValue(int index)
{
    return(editModeBuffer[index]);
}

std::vector<int>* CPathEditMode_old::getEditModeBuffer()
{
    return(&editModeBuffer);
}

void CPathEditMode_old::deselectEditModeBuffer()
{
    editModeBuffer.clear();
}

void CPathEditMode_old::removeItemFromEditModeBuffer(int item)
{
    for (int i=0;i<int(editModeBuffer.size());i++)
    {
        if (editModeBuffer[i]==item)
        {
            editModeBuffer.erase(editModeBuffer.begin()+i);
            break;
        }
    }
}

bool CPathEditMode_old::isEditModeItemAValidItem(int item)
{
    if (item<0)
        return(false);
    return(true);
}

void CPathEditMode_old::xorAddItemToEditModeBuffer(int item)
{
    if (isEditModeItemAValidItem(item))
    {
        if (!alreadyInEditModeBuffer(item))
            addItemToEditModeBuffer(item);
        else
            removeItemFromEditModeBuffer(item);
    }
    else
        editModeBuffer.clear();
}

void CPathEditMode_old::addItemToEditModeBuffer(int item)
{
    if (isEditModeItemAValidItem(item))
    {
        if (!alreadyInEditModeBuffer(item))
            editModeBuffer.push_back(item);
    }
}

bool CPathEditMode_old::alreadyInEditModeBuffer(int item)
{
    for (int i=0;i<int(editModeBuffer.size());i++)
        if (editModeBuffer[i]==item)
            return(true);
    return(false);
}

int CPathEditMode_old::getBezierPathPointCount()
{
    return(_editionPathCont->getBezierPathPointCount());
}

CSimplePathPoint_old* CPathEditMode_old::getSimplePathPoint(int editModeBufferIndex)
{
    if (editModeBufferIndex<0)
        return(nullptr);
    if (editModeBufferIndex>=int(editModeBuffer.size()))
        return(nullptr);
    return(_editionPathCont->getSimplePathPoint(editModeBuffer[editModeBufferIndex]));
}

void CPathEditMode_old::_copySelection(std::vector<int>* selection)
{
    for (int i=0;i<int(editBufferPathPointsCopy.size());i++)
        delete editBufferPathPointsCopy[i];
    editBufferPathPointsCopy.clear();
    int maxIndex=-1;
    for (int i=0;i<int(selection->size());i++)
    {
        if (selection->at(i)>maxIndex)
            maxIndex=selection->at(i);
    }
    std::vector<bool> toCopy(maxIndex+1,false);
    for (int i=0;i<int(selection->size());i++)
        toCopy[selection->at(i)]=true;
    for (int i=0;i<int(toCopy.size());i++)
    {
        if (toCopy[i])
        {
            CSimplePathPoint_old* it=_editionPathCont->getSimplePathPoint(i);
            if (it!=nullptr)
                editBufferPathPointsCopy.push_back(it->copyYourself());
        }
    }
    deselectEditModeBuffer();
}

void CPathEditMode_old::_paste(int insertPosition)
{
    deselectEditModeBuffer();
    _editionPathCont->enableActualization(false);
    insertPosition++;
    for (int i=0;i<int(editBufferPathPointsCopy.size());i++)
    {
        _editionPathCont->insertSimplePathPoint(editBufferPathPointsCopy[i]->copyYourself(),insertPosition+i);
        addItemToEditModeBuffer(insertPosition+i);
    }
    _editionPathCont->enableActualization(true);
    _editionPathCont->actualizePath();
}

void CPathEditMode_old::_insertNewPoint(int insertPosition)
{
    deselectEditModeBuffer();
    if (insertPosition==-1)
        insertPosition=0;
    CSimplePathPoint_old* it=_editionPathCont->getSimplePathPoint(insertPosition);
    if (it==nullptr)
    {
        it=new CSimplePathPoint_old();
        C7Vector tr(it->getTransformation());
        tr.X(2)=0.1;
        it->setTransformation(tr,_editionPathCont->getAttributes());
    }
    else
    {
        it=it->copyYourself();
        insertPosition++;
    }
    _editionPathCont->insertSimplePathPoint(it,insertPosition);
    addItemToEditModeBuffer(insertPosition);
    _editionPathCont->actualizePath();
}

void CPathEditMode_old::_cutSelection(std::vector<int>* selection)
{
    _copySelection(selection);
    _deleteSelection(selection);
}

void CPathEditMode_old::_deleteSelection(std::vector<int>* selection)
{
    int maxIndex=-1;
    for (int i=0;i<int(selection->size());i++)
    {
        if (selection->at(i)>maxIndex)
            maxIndex=selection->at(i);
    }
    std::vector<bool> toDelete(maxIndex+1,false);
    for (int i=0;i<int(selection->size());i++)
        toDelete[selection->at(i)]=true;
    _editionPathCont->enableActualization(false);
    for (int i=int(toDelete.size())-1;i>=0;i--)
    {
        if (toDelete[i])
            _editionPathCont->removeSimplePathPoint(i);
    }
    _editionPathCont->enableActualization(true);
    _editionPathCont->actualizePath();
    deselectEditModeBuffer();
}

void CPathEditMode_old::_keepXAxisAndAlignZAxis(std::vector<int>* selection)
{
    if (selection->size()>1)
    {
        C7Vector ctm(_path->getCumulativeTransformation());
        CSimplePathPoint_old* last=_editionPathCont->getSimplePathPoint((*selection)[selection->size()-1]);
        C3Vector zVect(last->getTransformation().getMatrix().M.axis[2]);
        C3X3Matrix posRot,negRot;
        posRot.buildXRotation(piValue/1800.0);
        negRot.buildXRotation(-piValue/1800.0);
        for (int i=0;i<int(selection->size()-1);i++)
        {
            CSimplePathPoint_old* it=_editionPathCont->getSimplePathPoint((*selection)[i]);
            C4X4Matrix m=it->getTransformation().getMatrix();
            double d=(m.M.axis[2]-zVect).getLength();
            bool positiveRot=true;
            for (int j=0;j<4000;j++)
            {
                if (positiveRot)
                    m.M=m.M*posRot;
                else
                    m.M=m.M*negRot;
                double dp=(m.M.axis[2]-zVect).getLength();
                if (dp>d)
                    positiveRot=!positiveRot;
                d=dp;
            }
            it->setTransformation(m.getTransformation(),_editionPathCont->getAttributes());
        }
        _editionPathCont->actualizePath();
    }
}

void CPathEditMode_old::_generatePath()
{
    if (_path!=nullptr)
    {
        CPath_old* newPath=(CPath_old*)_path->copyYourself();
        newPath->pathContainer->enableActualization(false);
        newPath->pathContainer->removeAllSimplePathPoints();
        int i=0;
        CBezierPathPoint_old* it=_editionPathCont->getBezierPathPoint(i++);
        while (it!=nullptr)
        {
            CSimplePathPoint_old* bp=new CSimplePathPoint_old();
            bp->setTransformation(it->getTransformation(),_editionPathCont->getAttributes());
            newPath->pathContainer->addSimplePathPoint(bp);
            it=_editionPathCont->getBezierPathPoint(i++);
        }
        newPath->pathContainer->enableActualization(true);
        newPath->pathContainer->actualizePath();
        _objCont->addObjectToScene(newPath,false,true);
    }
}

void CPathEditMode_old::addMenu(VMenu* menu,CSceneObject* viewableObject)
{
    int selSize=getEditModeBufferSize();
    int buffSize=(int)editBufferPathPointsCopy.size();
    menu->appendMenuItem(selSize>0,false,PATH_EDIT_MODE_OLD_PATH_POINT_COPY_EMCMD,IDS_COPY_SELECTED_PATH_POINTS_MENU_ITEM);
    if (selSize>=1)
        menu->appendMenuItem((buffSize>0)&&(selSize==1),false,PATH_EDIT_MODE_OLD_PASTE_PATH_POINT_EMCMD,IDS_PASTE_PATH_POINTS_MENU_ITEM);
    else
        menu->appendMenuItem(buffSize>0,false,PATH_EDIT_MODE_OLD_PASTE_PATH_POINT_EMCMD,IDS_PASTE_PATH_POINTS_AT_BEGINNING_MENU_ITEM);
    menu->appendMenuItem(selSize>0,false,PATH_EDIT_MODE_OLD_DELETE_PATH_POINT_EMCMD,IDS_DELETE_SELECTED_PATH_POINTS_MENU_ITEM);
    menu->appendMenuItem(selSize>0,false,PATH_EDIT_MODE_OLD_PATH_POINT_CUT_EMCMD,IDS_CUT_SELECTED_PATH_POINTS_MENU_ITEM);

    menu->appendMenuSeparator();

    if (selSize>=1)
        menu->appendMenuItem(selSize==1,false,PATH_EDIT_MODE_OLD_INSERT_NEW_PATH_POINT_EMCMD,IDS_INSERT_NEW_PATH_POINT_MENU_ITEM);
    else
        menu->appendMenuItem(true,false,PATH_EDIT_MODE_OLD_INSERT_NEW_PATH_POINT_EMCMD,IDS_INSERT_NEW_PATH_POINT_AT_BEGINNING_MENU_ITEM);
    menu->appendMenuItem((viewableObject!=nullptr)&&(viewableObject->getObjectType()==sim_object_camera_type),false,PATH_EDIT_MODE_OLD_APPEND_NEW_PATH_POINT_FROM_CAMERA_EMCMD,IDS_APPEND_NEW_PATH_POINT_FROM_CAMERA_MENU_ITEM);
    menu->appendMenuItem(_editionPathCont->getBezierPathPointCount()>1,false,PATH_EDIT_MODE_OLD_MAKE_PATH_FROM_BEZIER_EMCMD,IDS_MAKE_PATH_FROM_BEZIER_PATH_MENU_ITEM);
    menu->appendMenuItem(selSize>0,false,PATH_EDIT_MODE_OLD_MAKE_DUMMIES_FROM_PATH_CTRL_POINTS_EMCMD,IDS_MAKE_DUMMIES_FROM_PATH_CTRL_POINTS_MENU_ITEM);
    menu->appendMenuItem(selSize>0,false,PATH_EDIT_MODE_OLD_INVERSE_ORDER_OF_SELECTED_PATH_CTRL_POINTS_EMCMD,IDS_INVERSE_ORDER_OF_SELECTED_PATH_CTRL_POINTS_MENU_ITEM);

    menu->appendMenuSeparator();

    menu->appendMenuItem(true,false,PATH_EDIT_MODE_OLD_SELECT_ALL_PATH_POINTS_EMCMD,IDSN_SELECT_ALL_MENU_ITEM);

    menu->appendMenuSeparator();

    menu->appendMenuItem(true,false,ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD,IDS_LEAVE_EDIT_MODE_AND_CANCEL_CHANGES_MENU_ITEM);
    menu->appendMenuItem(true,false,ANY_EDIT_MODE_FINISH_AND_APPLY_CHANGES_EMCMD,IDS_LEAVE_EDIT_MODE_AND_APPLY_CHANGES_MENU_ITEM);
}

bool CPathEditMode_old::processCommand(int commandID,CSceneObject* viewableObject)
{   // Return value is true if the command was successful
    bool retVal=true;
    if (commandID==PATH_EDIT_MODE_OLD_SELECT_ALL_PATH_POINTS_EMCMD)
    {
        for (int i=0;i<int(_editionPathCont->getSimplePathPointCount());i++)
            addItemToEditModeBuffer(i);
        return(retVal);
    }

    if (commandID==PATH_EDIT_MODE_OLD_PATH_POINT_COPY_EMCMD)
    {
        std::vector<int> sel;
        for (int i=0;i<getEditModeBufferSize();i++)
            sel.push_back(editModeBuffer[i]);
        _copySelection(&sel);
        return(retVal);
    }

    if (commandID==PATH_EDIT_MODE_OLD_PASTE_PATH_POINT_EMCMD)
    {
        std::vector<int> sel;
        for (int i=0;i<getEditModeBufferSize();i++)
            sel.push_back(editModeBuffer[i]);
        if (sel.size()<2)
        {
            if (sel.size()==0)
                _paste(-1);
            else
                _paste(sel[0]);
        }
        else
            retVal=false;
        return(retVal);
    }

    if (commandID==PATH_EDIT_MODE_OLD_INSERT_NEW_PATH_POINT_EMCMD)
    {
        std::vector<int> sel;
        for (int i=0;i<getEditModeBufferSize();i++)
            sel.push_back(editModeBuffer[i]);
        if (sel.size()<2)
        {
            if (sel.size()==0)
                _insertNewPoint(-1);
            else
                _insertNewPoint(sel[0]);
        }
        else
            retVal=false;
        return(retVal);
    }

    if (commandID==PATH_EDIT_MODE_OLD_APPEND_NEW_PATH_POINT_FROM_CAMERA_EMCMD)
    {
        deselectEditModeBuffer();
        CSimplePathPoint_old* it=new CSimplePathPoint_old();
        C7Vector pathInv(_path->getFullCumulativeTransformation().getInverse());
        it->setTransformation(pathInv*viewableObject->getCumulativeTransformation(),_editionPathCont->getAttributes());
        _editionPathCont->insertSimplePathPoint(it,_editionPathCont->getSimplePathPointCount());
        addItemToEditModeBuffer(_editionPathCont->getSimplePathPointCount()-1);
        _editionPathCont->actualizePath();
        return(retVal);
    }

    if (commandID==PATH_EDIT_MODE_OLD_DELETE_PATH_POINT_EMCMD)
    {
        std::vector<int> sel;
        for (int i=0;i<getEditModeBufferSize();i++)
            sel.push_back(editModeBuffer[i]);
        _deleteSelection(&sel);
        deselectEditModeBuffer();
        return(sel.size()>0);
    }

    if (commandID==PATH_EDIT_MODE_OLD_PATH_POINT_CUT_EMCMD)
    {
        std::vector<int> sel;
        for (int i=0;i<getEditModeBufferSize();i++)
            sel.push_back(editModeBuffer[i]);
        _cutSelection(&sel);
        deselectEditModeBuffer();
        return(sel.size()>0);
    }

    if (commandID==PATH_EDIT_MODE_OLD_KEEP_ALL_X_AND_ALIGN_Z_FOR_PATH_POINTS_EMCMD)
    {
            std::vector<int> sel;
            for (int i=0;i<getEditModeBufferSize();i++)
                sel.push_back(editModeBuffer[i]);
            if ( (sel.size()>0)&&(_editionPathCont->getBezierPathPointCount()>=2) )
                _keepXAxisAndAlignZAxis(&sel);
            else
                retVal=false;

        return(retVal);
    }

    if (commandID==PATH_EDIT_MODE_OLD_INVERSE_ORDER_OF_SELECTED_PATH_CTRL_POINTS_EMCMD)
    {
        std::vector<int> sel;
        for (int i=0;i<getEditModeBufferSize();i++)
            sel.push_back(editModeBuffer[i]);
        retVal=_editionPathCont->invertSimplePathPointOrder(sel);
        deselectEditModeBuffer();
        return(retVal);
    }

    if (commandID==PATH_EDIT_MODE_OLD_MAKE_PATH_FROM_BEZIER_EMCMD)
    {
        _generatePath();
        return(retVal);
    }

    return(false);
}

void CPathEditMode_old::makeDummies()
{
    std::vector<int> sel;
    for (int i=0;i<getEditModeBufferSize();i++)
        sel.push_back(editModeBuffer[i]);
    for (size_t i=0;i<sel.size();i++)
    {
        CSimplePathPoint_old* it=_editionPathCont->getSimplePathPoint(sel[i]);
        SSimulationThreadCommand cmd;
        cmd.cmdId=PATHEDIT_MAKEDUMMY_GUITRIGGEREDCMD;
        cmd.stringParams.push_back("ExtractedDummy");
        cmd.floatParams.push_back(0.02); // so that we can see them over the ctrl pts!
        cmd.transfParams.push_back(_path->getFullCumulativeTransformation()*it->getTransformation());
        App::appendSimulationThreadCommand(cmd);
    }
    deselectEditModeBuffer();
}
