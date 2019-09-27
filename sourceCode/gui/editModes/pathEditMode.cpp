
#include "pathEditMode.h"
#include "v_repConst.h"
#include "global.h"
#include "v_repStringTable.h"
#include "app.h"

CPathEditMode::CPathEditMode(CPath* path,CObjCont* objCont)
{
    _editionPathCont=path->pathContainer->copyYourself();
    _path=path;
    _objCont=objCont;
}

CPathEditMode::~CPathEditMode()
{
}

void CPathEditMode::endEditMode(bool cancelChanges)
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

CPathCont* CPathEditMode::getEditModePathContainer()
{
    return(_editionPathCont);
}

CPath* CPathEditMode::getEditModePath()
{
    return(_path);
}

int CPathEditMode::getEditModeBufferSize()
{
    return(int(editModeBuffer.size()));
}

int CPathEditMode::getLastEditModeBufferValue()
{
    return(editModeBuffer[editModeBuffer.size()-1]);
}

int CPathEditMode::getEditModeBufferValue(int index)
{
    return(editModeBuffer[index]);
}

std::vector<int>* CPathEditMode::getEditModeBuffer()
{
    return(&editModeBuffer);
}

void CPathEditMode::deselectEditModeBuffer()
{
    editModeBuffer.clear();
}

void CPathEditMode::removeItemFromEditModeBuffer(int item)
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

bool CPathEditMode::isEditModeItemAValidItem(int item)
{
    if (item<0)
        return(false);
    return(true);
}

void CPathEditMode::xorAddItemToEditModeBuffer(int item)
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

void CPathEditMode::addItemToEditModeBuffer(int item)
{
    if (isEditModeItemAValidItem(item))
    {
        if (!alreadyInEditModeBuffer(item))
            editModeBuffer.push_back(item);
    }
}

bool CPathEditMode::alreadyInEditModeBuffer(int item)
{
    for (int i=0;i<int(editModeBuffer.size());i++)
        if (editModeBuffer[i]==item)
            return(true);
    return(false);
}

int CPathEditMode::getBezierPathPointCount()
{
    return(_editionPathCont->getBezierPathPointCount());
}

CSimplePathPoint* CPathEditMode::getSimplePathPoint(int editModeBufferIndex)
{
    if (editModeBufferIndex<0)
        return(nullptr);
    if (editModeBufferIndex>=int(editModeBuffer.size()))
        return(nullptr);
    return(_editionPathCont->getSimplePathPoint(editModeBuffer[editModeBufferIndex]));
}

void CPathEditMode::_copySelection(std::vector<int>* selection)
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
            CSimplePathPoint* it=_editionPathCont->getSimplePathPoint(i);
            if (it!=nullptr)
                editBufferPathPointsCopy.push_back(it->copyYourself());
        }
    }
    deselectEditModeBuffer();
}

void CPathEditMode::_paste(int insertPosition)
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

void CPathEditMode::_insertNewPoint(int insertPosition)
{
    deselectEditModeBuffer();
    if (insertPosition==-1)
        insertPosition=0;
    CSimplePathPoint* it=_editionPathCont->getSimplePathPoint(insertPosition);
    if (it==nullptr)
    {
        it=new CSimplePathPoint();
        C7Vector tr(it->getTransformation());
        tr.X(2)=0.1f;
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

void CPathEditMode::_cutSelection(std::vector<int>* selection)
{
    _copySelection(selection);
    _deleteSelection(selection);
}

void CPathEditMode::_deleteSelection(std::vector<int>* selection)
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

void CPathEditMode::_keepXAxisAndAlignZAxis(std::vector<int>* selection)
{
    if (selection->size()>1)
    {
        C7Vector ctm(_path->getCumulativeTransformationPart1());
        CSimplePathPoint* last=_editionPathCont->getSimplePathPoint((*selection)[selection->size()-1]);
        C3Vector zVect(last->getTransformation().getMatrix().M.axis[2]);
        C3X3Matrix posRot,negRot;
        posRot.buildXRotation(piValue_f/1800.0f);
        negRot.buildXRotation(-piValue_f/1800.0f);
        for (int i=0;i<int(selection->size()-1);i++)
        {
            CSimplePathPoint* it=_editionPathCont->getSimplePathPoint((*selection)[i]);
            C4X4Matrix m=it->getTransformation().getMatrix();
            float d=(m.M.axis[2]-zVect).getLength();
            bool positiveRot=true;
            for (int j=0;j<4000;j++)
            {
                if (positiveRot)
                    m.M=m.M*posRot;
                else
                    m.M=m.M*negRot;
                float dp=(m.M.axis[2]-zVect).getLength();
                if (dp>d)
                    positiveRot=!positiveRot;
                d=dp;
            }
            it->setTransformation(m.getTransformation(),_editionPathCont->getAttributes());
        }
        _editionPathCont->actualizePath();
    }
}

void CPathEditMode::_generatePath()
{
    if (_path!=nullptr)
    {
        CPath* newPath=(CPath*)_path->copyYourself();
        newPath->pathContainer->enableActualization(false);
        newPath->pathContainer->removeAllSimplePathPoints();
        int i=0;
        CBezierPathPoint* it=_editionPathCont->getBezierPathPoint(i++);
        while (it!=nullptr)
        {
            CSimplePathPoint* bp=new CSimplePathPoint();
            bp->setTransformation(it->getTransformation(),_editionPathCont->getAttributes());
            newPath->pathContainer->addSimplePathPoint(bp);
            it=_editionPathCont->getBezierPathPoint(i++);
        }
        newPath->pathContainer->enableActualization(true);
        newPath->pathContainer->actualizePath();
        _objCont->addObjectToScene(newPath,false,true);
    }
}

void CPathEditMode::addMenu(VMenu* menu,C3DObject* viewableObject)
{
    int selSize=getEditModeBufferSize();
    int buffSize=(int)editBufferPathPointsCopy.size();
    menu->appendMenuItem(selSize>0,false,PATH_EDIT_MODE_PATH_POINT_COPY_EMCMD,IDS_COPY_SELECTED_PATH_POINTS_MENU_ITEM);
    if (selSize>=1)
        menu->appendMenuItem((buffSize>0)&&(selSize==1),false,PATH_EDIT_MODE_PASTE_PATH_POINT_EMCMD,IDS_PASTE_PATH_POINTS_MENU_ITEM);
    else
        menu->appendMenuItem(buffSize>0,false,PATH_EDIT_MODE_PASTE_PATH_POINT_EMCMD,IDS_PASTE_PATH_POINTS_AT_BEGINNING_MENU_ITEM);
    menu->appendMenuItem(selSize>0,false,PATH_EDIT_MODE_DELETE_PATH_POINT_EMCMD,IDS_DELETE_SELECTED_PATH_POINTS_MENU_ITEM);
    menu->appendMenuItem(selSize>0,false,PATH_EDIT_MODE_PATH_POINT_CUT_EMCMD,IDS_CUT_SELECTED_PATH_POINTS_MENU_ITEM);

    menu->appendMenuSeparator();

    if (selSize>=1)
        menu->appendMenuItem(selSize==1,false,PATH_EDIT_MODE_INSERT_NEW_PATH_POINT_EMCMD,IDS_INSERT_NEW_PATH_POINT_MENU_ITEM);
    else
        menu->appendMenuItem(true,false,PATH_EDIT_MODE_INSERT_NEW_PATH_POINT_EMCMD,IDS_INSERT_NEW_PATH_POINT_AT_BEGINNING_MENU_ITEM);
    menu->appendMenuItem((viewableObject!=nullptr)&&(viewableObject->getObjectType()==sim_object_camera_type),false,PATH_EDIT_MODE_APPEND_NEW_PATH_POINT_FROM_CAMERA_EMCMD,IDS_APPEND_NEW_PATH_POINT_FROM_CAMERA_MENU_ITEM);
    menu->appendMenuItem(_editionPathCont->getBezierPathPointCount()>1,false,PATH_EDIT_MODE_MAKE_PATH_FROM_BEZIER_EMCMD,IDS_MAKE_PATH_FROM_BEZIER_PATH_MENU_ITEM);
    menu->appendMenuItem(selSize>0,false,PATH_EDIT_MODE_MAKE_DUMMIES_FROM_PATH_CTRL_POINTS_EMCMD,IDS_MAKE_DUMMIES_FROM_PATH_CTRL_POINTS_MENU_ITEM);
    menu->appendMenuItem(selSize>0,false,PATH_EDIT_MODE_INVERSE_ORDER_OF_SELECTED_PATH_CTRL_POINTS_EMCMD,IDS_INVERSE_ORDER_OF_SELECTED_PATH_CTRL_POINTS_MENU_ITEM);

    menu->appendMenuSeparator();

    menu->appendMenuItem(true,false,PATH_EDIT_MODE_SELECT_ALL_PATH_POINTS_EMCMD,IDSN_SELECT_ALL_MENU_ITEM);

    menu->appendMenuSeparator();

    menu->appendMenuItem(true,false,ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD,IDS_LEAVE_EDIT_MODE_AND_CANCEL_CHANGES_MENU_ITEM);
    menu->appendMenuItem(true,false,ANY_EDIT_MODE_FINISH_AND_APPLY_CHANGES_EMCMD,IDS_LEAVE_EDIT_MODE_AND_APPLY_CHANGES_MENU_ITEM);
}

bool CPathEditMode::processCommand(int commandID,C3DObject* viewableObject)
{   // Return value is true if the command was successful
    bool retVal=true;
    if (commandID==PATH_EDIT_MODE_SELECT_ALL_PATH_POINTS_EMCMD)
    {
        for (int i=0;i<int(_editionPathCont->getSimplePathPointCount());i++)
            addItemToEditModeBuffer(i);
        return(retVal);
    }

    if (commandID==PATH_EDIT_MODE_PATH_POINT_COPY_EMCMD)
    {
        std::vector<int> sel;
        for (int i=0;i<getEditModeBufferSize();i++)
            sel.push_back(editModeBuffer[i]);
        _copySelection(&sel);
        return(retVal);
    }

    if (commandID==PATH_EDIT_MODE_PASTE_PATH_POINT_EMCMD)
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

    if (commandID==PATH_EDIT_MODE_INSERT_NEW_PATH_POINT_EMCMD)
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

    if (commandID==PATH_EDIT_MODE_APPEND_NEW_PATH_POINT_FROM_CAMERA_EMCMD)
    {
        deselectEditModeBuffer();
        CSimplePathPoint* it=new CSimplePathPoint();
        C7Vector pathInv(_path->getCumulativeTransformation().getInverse());
        it->setTransformation(pathInv*viewableObject->getCumulativeTransformationPart1(),_editionPathCont->getAttributes());
        _editionPathCont->insertSimplePathPoint(it,_editionPathCont->getSimplePathPointCount());
        addItemToEditModeBuffer(_editionPathCont->getSimplePathPointCount()-1);
        _editionPathCont->actualizePath();
        return(retVal);
    }

    if (commandID==PATH_EDIT_MODE_DELETE_PATH_POINT_EMCMD)
    {
        std::vector<int> sel;
        for (int i=0;i<getEditModeBufferSize();i++)
            sel.push_back(editModeBuffer[i]);
        _deleteSelection(&sel);
        deselectEditModeBuffer();
        return(sel.size()>0);
    }

    if (commandID==PATH_EDIT_MODE_PATH_POINT_CUT_EMCMD)
    {
        std::vector<int> sel;
        for (int i=0;i<getEditModeBufferSize();i++)
            sel.push_back(editModeBuffer[i]);
        _cutSelection(&sel);
        deselectEditModeBuffer();
        return(sel.size()>0);
    }

    if (commandID==PATH_EDIT_MODE_KEEP_ALL_X_AND_ALIGN_Z_FOR_PATH_POINTS_EMCMD)
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

    if (commandID==PATH_EDIT_MODE_INVERSE_ORDER_OF_SELECTED_PATH_CTRL_POINTS_EMCMD)
    {
        std::vector<int> sel;
        for (int i=0;i<getEditModeBufferSize();i++)
            sel.push_back(editModeBuffer[i]);
        retVal=_editionPathCont->invertSimplePathPointOrder(sel);
        deselectEditModeBuffer();
        return(retVal);
    }

    if (commandID==PATH_EDIT_MODE_MAKE_PATH_FROM_BEZIER_EMCMD)
    {
        _generatePath();
        return(retVal);
    }

    return(false);
}

void CPathEditMode::makeDummies()
{
    std::vector<int> sel;
    for (int i=0;i<getEditModeBufferSize();i++)
        sel.push_back(editModeBuffer[i]);
    for (size_t i=0;i<sel.size();i++)
    {
        CSimplePathPoint* it=_editionPathCont->getSimplePathPoint(sel[i]);
        SSimulationThreadCommand cmd;
        cmd.cmdId=PATHEDIT_MAKEDUMMY_GUITRIGGEREDCMD;
        cmd.stringParams.push_back("ExtractedDummy");
        cmd.floatParams.push_back(0.02f); // so that we can see them over the ctrl pts!
        cmd.transfParams.push_back(_path->getCumulativeTransformation()*it->getTransformation());
        App::appendSimulationThreadCommand(cmd);
    }
    deselectEditModeBuffer();
}
