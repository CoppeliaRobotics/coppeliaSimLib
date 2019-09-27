
// This file requires some serious refactoring!!

#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "hierarchyElement.h"
#include "oGL.h"
#include "imgLoaderSaver.h"
#include "app.h"
#include "tt.h"
#include "geometric.h"
#include <boost/lexical_cast.hpp>

const int CONST_VAL_6=6;

CHierarchyElement::CHierarchyElement(int theObjectID)
{
    children.clear();
    objectID=theObjectID;
}

CHierarchyElement::~CHierarchyElement()
{
    for (int i=0;i<int(children.size());i++)
        delete children[i];
    children.clear();
}

void CHierarchyElement::setSceneName(const std::string& sn)
{
    if (sn=="")
        _sceneName="new scene";
    else
        _sceneName=sn;
}

CHierarchyElement* CHierarchyElement::getElementLinkedWithObject(int objID)
{
    if (objectID==objID)
        return(this);
    CHierarchyElement* retVal=nullptr;
    for (int i=0;i<int(children.size());i++)
    {
        retVal=children[i]->getElementLinkedWithObject(objID);
        if (retVal!=nullptr)
            return(retVal);
    }
    return(nullptr);
}

int CHierarchyElement::addYourChildren()
{ // returns the total number of children
    int retVal=0;
    if (App::getEditModeType()==NO_EDIT_MODE)
    {
        std::vector<int> objIDs;
        std::vector<std::string> objNames;
        if ((objectID<0)&&isLocalWorld())
        { // this is the world!
            for (size_t i=0;i<App::ct->objCont->objectList.size();i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
#ifdef KEYWORD__NOT_DEFINED_FORMELY_BR
                if ((it->getParent()==nullptr)&&it->getModelBase()&&(!it->isObjectPartOfInvisibleModel()))
#else
                if (it->getParentObject()==nullptr)
#endif
                {
                    objIDs.push_back(it->getObjectHandle());
                    objNames.push_back(tt::getLowerUpperCaseString(it->getObjectName(),false));
                }
            }
            tt::orderStrings(objNames,objIDs);
            for (size_t i=0;i<objIDs.size();i++)
            {
                retVal++;
                CHierarchyElement* aKid=new CHierarchyElement(objIDs[i]);
                retVal+=aKid->addYourChildren();
                children.push_back(aKid);
            }
        }
        else
        { // this is a 3DObject!
            C3DObject* it=App::ct->objCont->getObjectFromHandle(objectID);
            if (it!=nullptr)
            {
#ifdef KEYWORD__NOT_DEFINED_FORMELY_BR
                std::vector<C3DObject*> firstModelRelatives;
                retVal+=it->getFirstModelRelatives(firstModelRelatives,true);
                for (size_t i=0;i<firstModelRelatives.size();i++)
                {
                    C3DObject* modl=firstModelRelatives[i];
                    objIDs.push_back(modl->getID());
                    objNames.push_back(tt::getLowerUpperCaseString(modl->getName(),false));
                }
                tt::orderStrings(objNames,objIDs);
                for (size_t i=0;i<objIDs.size();i++)
                {
                    CHierarchyElement* aKid=new CHierarchyElement(objIDs[i]);
                    retVal+=aKid->addYourChildren();
                    children.push_back(aKid);
                }
#else
                retVal+=int(it->childList.size());
                for (int i=0;i<int(it->childList.size());i++)
                {
                    if (it->childList[i]!=nullptr)
                    {
                        objIDs.push_back(it->childList[i]->getObjectHandle());
                        objNames.push_back(tt::getLowerUpperCaseString(it->childList[i]->getObjectName(),false));
                    }
                }
                tt::orderStrings(objNames,objIDs);
                for (int i=0;i<int(objIDs.size());i++)
                {
                    CHierarchyElement* aKid=new CHierarchyElement(objIDs[i]);
                    retVal+=aKid->addYourChildren();
                    children.push_back(aKid);
                }
#endif
            }
        }
    }
    // Nothing needed for the various edit modes (for now!)
    return(retVal);
}

int CHierarchyElement::getLinkedObjectID()
{
    return(objectID);
}

#ifdef KEYWORD__NOT_DEFINED_FORMELY_BR
void CHierarchyElement::renderElement_3DObject(CHierarchy* hier,int labelEditObjectID,bool& bright,bool dontDisplay,
        int renderingSize[2],int textPos[2],
        int indentNb,std::vector<int>* vertLines,int minRenderedPos[2],int maxRenderedPos[2],bool forDragAndDrop/*=false*/,int transparentForTreeObjects/*=-1*/,int dropID/*=-1*/,int worldClick/*=-9999*/)
{ // transparentForTreeObjects==-1: normal, transparentForTreeObjects==-2: transparent, otherwise transparent only for objID=transparentForTreeObjects
    const unsigned char horizontalShift=13*App::sc;
    float transparencyFactor=0.0f;
    if ((transparentForTreeObjects>=0)&&(transparentForTreeObjects==objectID))
        transparentForTreeObjects=-2; // from here on, everything is transparent
    if (transparentForTreeObjects==-2)
        transparencyFactor=0.875f;

    bool isOtherWorld=((objectID<0)&&(!isLocalWorld()));

    if (!forDragAndDrop)
    {
        bright=!bright;
        if (textPos[0]<minRenderedPos[0])
            minRenderedPos[0]=textPos[0];
        if (textPos[1]<minRenderedPos[1])
            minRenderedPos[1]=textPos[1];
        if (textPos[0]>maxRenderedPos[0])
            maxRenderedPos[0]=textPos[0];
        if (textPos[1]>maxRenderedPos[1])
            maxRenderedPos[1]=textPos[1];

        if (textPos[1]<-HIERARCHY_INTER_LINE_SPACE*App::sc)
            dontDisplay=true; // We don't display what is outside of the view!
    }

    bool textInside=(textPos[1]<renderingSize[1]+HIERARCHY_INTER_LINE_SPACE*App::sc);
    C3DObject* it=App::ct->objCont->getObject(objectID);
    std::string theText;
    if (it!=nullptr)
        theText=it->getName();
    else
    {
        theText+=_sceneName;
        theText+=tt::decorateString(" (scene ",tt::FNb(-objectID),")");
    }


    bool inSelection=false;
    float dummyCol[3]={0.0f,0.0f,0.0f};
    float* bgCol=dummyCol;
    if (!forDragAndDrop)
    {
        bool hasAColor=false;
        if (dropID!=-9999)
        {
            if ((transparentForTreeObjects!=-2)&&(dropID==objectID))
            {
                bgCol=ogl::HIERARCHY_DROP_LOCATION_COLOR;
                hasAColor=true;
            }
        }
        else
        {
            if (App::ct->objCont->isObjectInSelection(objectID)&&textInside&&(!dontDisplay))
            {
                if (App::ct->objCont->getLastSelection_object()==it)
                    bgCol=ogl::HIERARCHY_AND_BROWSER_LAST_SELECTION_COLOR;
                else
                {
                    if (App::ct->collections->isObjectInMarkedCollection(objectID))
                    {
                        if (bright)
                            bgCol=ogl::HIERARCHY_COLLECTION_SELECTION_COLOR_BRIGHT;
                        else
                            bgCol=ogl::HIERARCHY_COLLECTION_SELECTION_COLOR_DARK;
                    }
                    else
                    {
                        if (bright)
                            bgCol=ogl::HIERARCHY_NOT_LAST_SELECTION_COLOR_BRIGHT;
                        else
                            bgCol=ogl::HIERARCHY_NOT_LAST_SELECTION_COLOR_DARK;
                    }
                }
                inSelection=true;
                hasAColor=true;
            }
        }
        if (textInside&&(!dontDisplay)&&(!hasAColor))
        {
            if (it==nullptr)
            { // world
                if (worldClick==objectID)
                    bgCol=ogl::HIERARCHY_WORLD_CLICK_COLOR;
                else
                {
                    if (isLocalWorld())
                    {
                        if (!bright)
                            bgCol=ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_DARK;
                        else
                            bgCol=ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT;
                    }
                    else
                        bgCol=ogl::HIERARCHY_UNACTIVE_WORLD_COLOR;
                }
            }
            else
            {
                int colIndex=it->getHierarchyColorIndex();
                if (colIndex==-1)
                {
                    if (!bright)
                        bgCol=ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_DARK;
                    else
                        bgCol=ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT;
                }
                else
                {
                    if (colIndex==0)
                    {
                        if (!bright)
                            bgCol=ogl::HIERARCHY_NO_SELECTION_RED_DARK;
                        else
                            bgCol=ogl::HIERARCHY_NO_SELECTION_RED_BRIGHT;
                    }
                    if (colIndex==1)
                    {
                        if (!bright)
                            bgCol=ogl::HIERARCHY_NO_SELECTION_GREEN_DARK;
                        else
                            bgCol=ogl::HIERARCHY_NO_SELECTION_GREEN_BRIGHT;
                    }
                    if (colIndex==2)
                    {
                        if (!bright)
                            bgCol=ogl::HIERARCHY_NO_SELECTION_BLUE_DARK;
                        else
                            bgCol=ogl::HIERARCHY_NO_SELECTION_BLUE_BRIGHT;
                    }
                }
            }
        }

        if (!forDragAndDrop)
        {
            ogl::setMaterialColor(sim_colorcomponent_emission,bgCol);
            glBegin(GL_QUADS);
            glVertex3i(0,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
            glVertex3i(renderingSize[0],textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
            glVertex3i(renderingSize[0],textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
            glVertex3i(0,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
            glEnd();
        }


        if (textInside&&(!dontDisplay))
        {
            hier->objectPosition.push_back(textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
            hier->objectPosition.push_back(objectID);
        }
    }
    else
    { // for drag and drop only:
        inSelection=App::ct->objCont->isObjectInSelection(objectID);
    }

    int off=2;
    if (it!=nullptr)
    {
        if (it->countFirstModelRelatives(true)!=0)
        {
            if (it->getLocalObjectProperty()&sim_objectproperty_collapsed)
            {
                int picture=PLUS_SIGN_TREE_PICTURE;
                if (dontDisplay||(!textInside))
                    picture=NO_TREE_PICTURE;
                _drawIcon_brm(hier,textPos[0]+off-5*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,it,picture,true,transparencyFactor,forDragAndDrop);
            }
            else
            {
                int picture=MINUS_SIGN_TREE_PICTURE;
                if (dontDisplay||(!textInside))
                    picture=NO_TREE_PICTURE;
                _drawIcon_brm(hier,textPos[0]+off-5*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,it,picture,true,transparencyFactor,forDragAndDrop);
            }
        }

        off=off+_drawIcon_brm(hier,textPos[0]+off+8*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,it,-1,!dontDisplay,transparencyFactor,forDragAndDrop);
    }

    if (objectID<0)
    { // For the world!
        off=off+_drawIcon_brm(hier,textPos[0]+off-15*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,it,objectID,!dontDisplay,transparencyFactor,forDragAndDrop);
        off-=24/App::sc; // tweaking
    }

    int textEndPosition=textPos[0]+off+ogl::getTextLengthInPixels(theText);
    if ( (labelEditObjectID==objectID)&&(objectID>=0) )
        textEndPosition=textPos[0]+off+ogl::getTextLengthInPixels(hier->editionText)+16*App::sc;

    if (!forDragAndDrop)
    {
        if (objectID>=0)
        { // world text can never be edited anyways
            hier->textPosition.push_back(textPos[0]);
            hier->textPosition.push_back(textPos[1]);
            hier->textPosition.push_back(textPos[0]+off);
            hier->textPosition.push_back(textEndPosition);
            hier->textPosition.push_back(textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
            hier->textPosition.push_back(objectID);
        }
    }

    if (textInside&&(!dontDisplay))
    {
        float* tc;
//        if (inSelection)
            tc=ogl::HIERARCHY_AND_BROWSER_TEXT_COLOR_VISIBLE;
//        else
//        {
//            if ( (it!=nullptr)&&( ((it->layer&App::ct->mainSettings->getActiveLayers())==0)||it->isObjectPartOfInvisibleModel() ) )
//                tc=ogl::HIERARCHY_AND_BROWSER_TEXT_COLOR_INVISIBLE;
//            else
//                tc=ogl::HIERARCHY_AND_BROWSER_TEXT_COLOR_VISIBLE;
//        }
        if (transparentForTreeObjects==-2)
            ogl::setTextColor((tc[0]+7.0f*bgCol[0])*0.125f,(tc[1]+7.0f*bgCol[1])*0.125f,(tc[2]+7.0f*bgCol[2])*0.125f);
        else
        {
            if ((it==nullptr)&&(!isLocalWorld()))
                ogl::setTextColor((tc[0]+3.0f*bgCol[0])*0.25f,(tc[1]+3.0f*bgCol[1])*0.25f,(tc[2]+3.0f*bgCol[2])*0.25f);
            else
                ogl::setTextColor(tc);
        }

        if ( (labelEditObjectID!=objectID)||(objectID==-1) )
            ogl::drawText(textPos[0]+off,textPos[1],0,theText,false);
        else
            hier->drawEditionLabel(textPos[0]+off,textPos[1]);
        /*
        // Following is for vertical lines that originates from one of this object's parent
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_AND_BROWSER_LINE_COLOR);
        if (!forDragAndDrop)
        {
            for (int i=0;i<int(vertLines->size());i++)
            {
                if (vertLines->at(i)<indentNb-1)
                    ogl::drawSingle2dLine_i(textPos[0]+3+horizontalShift/2-horizontalShift*(indentNb-vertLines->at(i)),textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,textPos[0]+3+horizontalShift/2-horizontalShift*(indentNb-vertLines->at(i)),textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc);
            }
        }
       // */
    }

    if (isOtherWorld)
    {
        textPos[1]=textPos[1]-HIERARCHY_INTER_LINE_SPACE*App::sc;
        return;
    }

    int lineLastPos=textEndPosition;

    // Show the script and script parameter icons:
    int tPosX=lineLastPos;
    int tPosY=textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc;
    int localOffset=16;
    lineLastPos+=localOffset;

    if (lineLastPos>maxRenderedPos[0])
        maxRenderedPos[0]=lineLastPos;

    if (!forDragAndDrop)
    {
        hier->lineLastPosition.push_back(lineLastPos);
        hier->lineLastPosition.push_back(textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
        if (it!=nullptr)
            hier->lineLastPosition.push_back(it->getID());
        else
            hier->lineLastPosition.push_back(-1);
    }
/*
    if ((!dontDisplay)&&(!forDragAndDrop)&&(it!=nullptr))
    { // Following for the line element(s) just left of scene object icons:
        if (true) //it->getModelBase())
        {
            ogl::drawSingle2dLine_i(textPos[0]+16*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,textPos[0]+20*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
            if ((it->childList.size()!=0)&&((it->getLocalObjectProperty()&sim_objectproperty_collapsed)==0))
                ogl::drawSingle2dLine_i(textPos[0]+10*App::sc,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-CONST_VAL_6)*App::sc,textPos[0]+10*App::sc,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_INTER_LINE_SPACE)*App::sc);
        }
    }
//*/
    textPos[1]=textPos[1]-HIERARCHY_INTER_LINE_SPACE*App::sc;


    if ( ( (it!=nullptr)&&((it->getLocalObjectProperty()&sim_objectproperty_collapsed)==0) )||(objectID<0) )
    {
        int xPosCopy=textPos[0];
        textPos[0]=textPos[0]+horizontalShift;
        int indentCopy=indentNb+1;

        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_AND_BROWSER_LINE_COLOR);

        std::vector<CHierarchyElement*> el;
        std::vector<int> elNb;

        for (int i=0;i<int(children.size());i++)
        { // This is needed to order the elements from least sub-elements to most sub-elements
            int els=children[i]->getNumberOfElements();
            if (App::userSettings->orderHierarchyAlphabetically)
                els=1;
            int j;
            for (j=0;j<int(elNb.size());j++)
            {
                if (els<elNb[j])
                    break;
            }
            elNb.insert(elNb.begin()+j,els);
            el.insert(el.begin()+j,children[i]);
        }

        for (int i=0;i<int(el.size());i++)
        {
            if (i!=int(el.size())-1)
            { // Vertical line going through (a T turned counter-clockwise)
 //               vertLines->push_back(indentNb);
 //               int txtYTmp=textPos[1];
                el[i]->renderElement_3DObject(hier,labelEditObjectID,bright,dontDisplay,
                                            renderingSize,textPos,indentCopy,
                                            vertLines,minRenderedPos,maxRenderedPos,forDragAndDrop,transparentForTreeObjects,dropID);
 /*
                vertLines->erase(vertLines->end()-1);
                if ((!dontDisplay)&&(!forDragAndDrop))
                {
                    C3DObject* childIt=App::ct->objCont->getObject(el[i]->getLinkedObjectID());
                    if (childIt->childList.size()==0)
                    {
                        ogl::drawSingle2dLine_i(xPosCopy+10*App::sc,txtYTmp+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,xPosCopy+10*App::sc,txtYTmp+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc);
                        ogl::drawSingle2dLine_i(xPosCopy+10*App::sc,txtYTmp+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,xPosCopy+17*App::sc,txtYTmp+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
                    }
                }
                */
            }
            else
            { // Vertical line stopping in the middle (L)
//                int txtYTmp=textPos[1];
                el[i]->renderElement_3DObject(hier,labelEditObjectID,bright,dontDisplay,
                                            renderingSize,textPos,indentCopy,
                                            vertLines,minRenderedPos,maxRenderedPos,forDragAndDrop,transparentForTreeObjects,dropID);
/*
                if ((!dontDisplay)&&(!forDragAndDrop))
                {
                    C3DObject* childIt=App::ct->objCont->getObject(el[i]->getLinkedObjectID());
                    if (childIt->childList.size()==0)
                    {
                        ogl::drawSingle2dLine_i(xPosCopy+10*App::sc,txtYTmp+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,xPosCopy+10*App::sc,txtYTmp+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
                        ogl::drawSingle2dLine_i(xPosCopy+10*App::sc,txtYTmp+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,xPosCopy+17*App::sc,txtYTmp+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
                    }
                }
                */
            }
        }

        textPos[0]=xPosCopy;
    }
}
#else
void CHierarchyElement::renderElement_3DObject(CHierarchy* hier,int labelEditObjectID,bool& bright,bool dontDisplay,
        int renderingSize[2],int textPos[2],
        int indentNb,std::vector<int>* vertLines,int minRenderedPos[2],int maxRenderedPos[2],bool forDragAndDrop/*=false*/,int transparentForTreeObjects/*=-1*/,int dropID/*=-1*/,int worldClick/*=-9999*/)
{ // transparentForTreeObjects==-1: normal, transparentForTreeObjects==-2: transparent, otherwise transparent only for objID=transparentForTreeObjects
    const unsigned char horizontalShift=13*App::sc;
    float transparencyFactor=0.0f;
    if ((transparentForTreeObjects>=0)&&(transparentForTreeObjects==objectID))
        transparentForTreeObjects=-2; // from here on, everything is transparent
    if (transparentForTreeObjects==-2)
        transparencyFactor=0.875f;

    bool isOtherWorld=((objectID<0)&&(!isLocalWorld()));

    if (!forDragAndDrop)
    {
        bright=!bright;
        if (textPos[0]<minRenderedPos[0])
            minRenderedPos[0]=textPos[0];
        if (textPos[1]<minRenderedPos[1])
            minRenderedPos[1]=textPos[1];
        if (textPos[0]>maxRenderedPos[0])
            maxRenderedPos[0]=textPos[0];
        if (textPos[1]>maxRenderedPos[1])
            maxRenderedPos[1]=textPos[1];

        if (textPos[1]<-HIERARCHY_INTER_LINE_SPACE*App::sc)
            dontDisplay=true; // We don't display what is outside of the view!
    }

    bool textInside=(textPos[1]<renderingSize[1]+HIERARCHY_INTER_LINE_SPACE*App::sc);
    C3DObject* it=App::ct->objCont->getObjectFromHandle(objectID);
    std::string theText;
    if (it!=nullptr)
        theText=it->getObjectName();
    else
    {
        theText+=_sceneName;
        theText+=tt::decorateString(" (scene ",tt::FNb(-objectID),")");
    }


    bool inSelection=false;
    float dummyCol[3]={0.0f,0.0f,0.0f};
    float* bgCol=dummyCol;
    if (!forDragAndDrop)
    {
        bool hasAColor=false;
        if (dropID!=-9999)
        {
            if ((transparentForTreeObjects!=-2)&&(dropID==objectID))
            {
                bgCol=ogl::HIERARCHY_DROP_LOCATION_COLOR;
                hasAColor=true;
            }
        }
        else
        {
            if (App::ct->objCont->isObjectInSelection(objectID)&&textInside&&(!dontDisplay))
            {
                if (App::ct->objCont->getLastSelection_object()==it)
                    bgCol=ogl::HIERARCHY_AND_BROWSER_LAST_SELECTION_COLOR;
                else
                {
                    if (App::ct->collections->isObjectInMarkedCollection(objectID))
                    {
                        if (bright)
                            bgCol=ogl::HIERARCHY_COLLECTION_SELECTION_COLOR_BRIGHT;
                        else
                            bgCol=ogl::HIERARCHY_COLLECTION_SELECTION_COLOR_DARK;
                    }
                    else
                    {
                        if (bright)
                            bgCol=ogl::HIERARCHY_NOT_LAST_SELECTION_COLOR_BRIGHT;
                        else
                            bgCol=ogl::HIERARCHY_NOT_LAST_SELECTION_COLOR_DARK;
                    }
                }
                inSelection=true;
                hasAColor=true;
            }
        }
        if (textInside&&(!dontDisplay)&&(!hasAColor))
        {
            if (it==nullptr)
            { // world
                if (worldClick==objectID)
                    bgCol=ogl::HIERARCHY_WORLD_CLICK_COLOR;
                else
                {
                    if (isLocalWorld())
                    {
                        if (!bright)
                            bgCol=ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_DARK;
                        else
                            bgCol=ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT;
                    }
                    else
                        bgCol=ogl::HIERARCHY_UNACTIVE_WORLD_COLOR;
                }
            }
            else
            {
                int colIndex=it->getHierarchyColorIndex();
                if (colIndex==-1)
                {
                    if (!bright)
                        bgCol=ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_DARK;
                    else
                        bgCol=ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT;
                }
                else
                {
                    if (colIndex==0)
                    {
                        if (!bright)
                            bgCol=ogl::HIERARCHY_NO_SELECTION_RED_DARK;
                        else
                            bgCol=ogl::HIERARCHY_NO_SELECTION_RED_BRIGHT;
                    }
                    if (colIndex==1)
                    {
                        if (!bright)
                            bgCol=ogl::HIERARCHY_NO_SELECTION_GREEN_DARK;
                        else
                            bgCol=ogl::HIERARCHY_NO_SELECTION_GREEN_BRIGHT;
                    }
                    if (colIndex==2)
                    {
                        if (!bright)
                            bgCol=ogl::HIERARCHY_NO_SELECTION_BLUE_DARK;
                        else
                            bgCol=ogl::HIERARCHY_NO_SELECTION_BLUE_BRIGHT;
                    }
                }
            }
        }

        if (!forDragAndDrop)
        {
            ogl::setMaterialColor(sim_colorcomponent_emission,bgCol);
            glBegin(GL_QUADS);
            glVertex3i(0,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
            glVertex3i(renderingSize[0],textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
            glVertex3i(renderingSize[0],textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
            glVertex3i(0,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
            glEnd();
        }


        if (textInside&&(!dontDisplay))
        {
            hier->objectPosition.push_back(textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
            hier->objectPosition.push_back(objectID);
        }
    }
    else
    { // for drag and drop only:
        inSelection=App::ct->objCont->isObjectInSelection(objectID);
    }

    int off=2;
    if (it!=nullptr)
    {
        if (it->childList.size()!=0)
        {
            if (it->getLocalObjectProperty()&sim_objectproperty_collapsed)
            {
                int picture=PLUS_SIGN_TREE_PICTURE;
                if (dontDisplay||(!textInside))
                    picture=NO_TREE_PICTURE;
                _drawIcon_3DObject(hier,textPos[0]+off-5*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,it,picture,true,transparencyFactor,forDragAndDrop);
            }
            else
            {
                int picture=MINUS_SIGN_TREE_PICTURE;
                if (dontDisplay||(!textInside))
                    picture=NO_TREE_PICTURE;
                _drawIcon_3DObject(hier,textPos[0]+off-5*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,it,picture,true,transparencyFactor,forDragAndDrop);
            }
        }

        off=off+_drawIcon_3DObject(hier,textPos[0]+off+8*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,it,-1,!dontDisplay,transparencyFactor,forDragAndDrop);
    }

    if (objectID<0)
    { // For the world!
        off=off+_drawIcon_3DObject(hier,textPos[0]+off-15*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,it,objectID,!dontDisplay,transparencyFactor,forDragAndDrop);
        off-=24/App::sc; // tweaking
    }

    int textEndPosition=textPos[0]+off+ogl::getTextLengthInPixels(theText);
    if ( (labelEditObjectID==objectID)&&(objectID>=0) )
        textEndPosition=textPos[0]+off+ogl::getTextLengthInPixels(hier->editionText)+16*App::sc;

    if (!forDragAndDrop)
    {
        if (objectID>=0)
        { // world text can never be edited anyways
            hier->textPosition.push_back(textPos[0]);
            hier->textPosition.push_back(textPos[1]);
            hier->textPosition.push_back(textPos[0]+off);
            hier->textPosition.push_back(textEndPosition);
            hier->textPosition.push_back(textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
            hier->textPosition.push_back(objectID);
        }
    }

    if (textInside&&(!dontDisplay))
    {
        float* tc;
        if (inSelection)
            tc=ogl::HIERARCHY_AND_BROWSER_TEXT_COLOR_VISIBLE;
        else
        {
            if ( (it!=nullptr)&&( ((it->layer&App::ct->mainSettings->getActiveLayers())==0)||it->isObjectPartOfInvisibleModel() ) )
                tc=ogl::HIERARCHY_AND_BROWSER_TEXT_COLOR_INVISIBLE;
            else
                tc=ogl::HIERARCHY_AND_BROWSER_TEXT_COLOR_VISIBLE;
        }
        if (transparentForTreeObjects==-2)
            ogl::setTextColor((tc[0]+7.0f*bgCol[0])*0.125f,(tc[1]+7.0f*bgCol[1])*0.125f,(tc[2]+7.0f*bgCol[2])*0.125f);
        else
        {
            if ((it==nullptr)&&(!isLocalWorld()))
                ogl::setTextColor((tc[0]+3.0f*bgCol[0])*0.25f,(tc[1]+3.0f*bgCol[1])*0.25f,(tc[2]+3.0f*bgCol[2])*0.25f);
            else
                ogl::setTextColor(tc);
        }

        if ( (labelEditObjectID!=objectID)||(objectID==-1) )
            ogl::drawText(textPos[0]+off,textPos[1],0,theText,false);
        else
            hier->drawEditionLabel(textPos[0]+off,textPos[1]);
        // Following is for vertical lines that originates from one of this object's parent
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_AND_BROWSER_LINE_COLOR);
        if (!forDragAndDrop)
        {
            for (int i=0;i<int(vertLines->size());i++)
            {
                if (vertLines->at(i)<indentNb-1)
                    ogl::drawSingle2dLine_i(textPos[0]+3+horizontalShift/2-horizontalShift*(indentNb-vertLines->at(i)),textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,textPos[0]+3+horizontalShift/2-horizontalShift*(indentNb-vertLines->at(i)),textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc);
            }
        }
    }

    if (isOtherWorld)
    {
        textPos[1]=textPos[1]-HIERARCHY_INTER_LINE_SPACE*App::sc;
        return;
    }

    int lineLastPos=textEndPosition;

    // Show the script and script parameter icons:
    int tPosX=lineLastPos;
    int tPosY=textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc;
    int localOffset=16;
    if (it!=nullptr)
    {
        // Child scripts:
        CLuaScriptObject* script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(it->getObjectHandle());
        if (script!=nullptr)
        {
            if (!dontDisplay)
            {
                if (script->getThreadedExecution())
                {
                    if (script->getScriptIsDisabled()||((it->getCumulativeModelProperty()&sim_modelproperty_scripts_inactive)!=0))
                        App::ct->globalGuiTextureCont->startTextureDisplay(SCRIPTDISABLED_THREADED_PICTURE);
                    else
                        App::ct->globalGuiTextureCont->startTextureDisplay(SCRIPT_THREADED_PICTURE);
                }
                else
                {
                    if (script->getScriptIsDisabled()||((it->getCumulativeModelProperty()&sim_modelproperty_scripts_inactive)!=0))
                        App::ct->globalGuiTextureCont->startTextureDisplay(SCRIPTDISABLED_PICTURE);
                    else
                        App::ct->globalGuiTextureCont->startTextureDisplay(SCRIPT_PICTURE);
                }
                _drawTexturedIcon(tPosX+localOffset,tPosY,HIERARCHY_ICON_WIDTH*App::sc,HIERARCHY_ICON_HEIGHT*App::sc,transparencyFactor);
            }
            if (!forDragAndDrop)
            {
                hier->scriptIconPosition.push_back(tPosX+localOffset);
                hier->scriptIconPosition.push_back(tPosY);
                hier->scriptIconPosition.push_back(script->getScriptID());
            }
            localOffset+=HIERARCHY_ICON_WIDTH*App::sc;
            if (!dontDisplay)
            {
                
                if (script->getScriptParametersObject()->scriptParamEntries.size()!=0)
                    App::ct->globalGuiTextureCont->startTextureDisplay(SCRIPT_PARAMETERS_ACTIVE_PICTURE);
                else
                    App::ct->globalGuiTextureCont->startTextureDisplay(SCRIPT_PARAMETERS_PICTURE);
                _drawTexturedIcon(tPosX+localOffset,tPosY,HIERARCHY_ICON_WIDTH*App::sc,HIERARCHY_ICON_HEIGHT*App::sc,transparencyFactor);
            }
            if (!forDragAndDrop)
            {
                hier->scriptParametersIconPosition.push_back(tPosX+localOffset);
                hier->scriptParametersIconPosition.push_back(tPosY);
                hier->scriptParametersIconPosition.push_back(script->getScriptID());
            }
            localOffset+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*App::sc;
        }

        // Customization scripts:
        script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(it->getObjectHandle());
        if (script!=nullptr)
        {
            if (!dontDisplay)
            {
                if (script->hasCustomizationScripAnyChanceToGetExecuted(false,false))
                    App::ct->globalGuiTextureCont->startTextureDisplay(CUSTOMIZATIONSCRIPT_PICTURE);
                else
                    App::ct->globalGuiTextureCont->startTextureDisplay(CUSTOMIZATIONSCRIPTDISABLED_PICTURE);
                _drawTexturedIcon(tPosX+localOffset,tPosY,HIERARCHY_ICON_WIDTH*App::sc,HIERARCHY_ICON_HEIGHT*App::sc,transparencyFactor);
            }
            if (!forDragAndDrop)
            {
                hier->scriptIconPosition.push_back(tPosX+localOffset);
                hier->scriptIconPosition.push_back(tPosY);
                hier->scriptIconPosition.push_back(script->getScriptID());
            }
            localOffset+=HIERARCHY_ICON_WIDTH*App::sc;
        }
    }
    else
    { // This is for the main script (pseudo object "world"):
        CLuaScriptObject* script=App::ct->luaScriptContainer->getMainScript();
        if (script!=nullptr)
        {
            if (!dontDisplay)
            {
                if (script->isDefaultMainScript())
                    App::ct->globalGuiTextureCont->startTextureDisplay(MAIN_SCRIPT_PICTURE);
                else
                    App::ct->globalGuiTextureCont->startTextureDisplay(MAIN_SCRIPT_CUSTOMIZED_PICTURE);
                _drawTexturedIcon(tPosX+localOffset,tPosY,HIERARCHY_ICON_WIDTH*App::sc,HIERARCHY_ICON_HEIGHT*App::sc,transparencyFactor);
            }
            if (!forDragAndDrop)
            {
                hier->scriptIconPosition.push_back(tPosX+localOffset);
                hier->scriptIconPosition.push_back(tPosY);
                hier->scriptIconPosition.push_back(script->getScriptID());
            }
            localOffset+=HIERARCHY_ICON_WIDTH*App::sc;
            if (!dontDisplay)
            {
                
                if (script->getScriptParametersObject()->scriptParamEntries.size()!=0)
                    App::ct->globalGuiTextureCont->startTextureDisplay(SCRIPT_PARAMETERS_ACTIVE_PICTURE);
                else
                    App::ct->globalGuiTextureCont->startTextureDisplay(SCRIPT_PARAMETERS_PICTURE);
                _drawTexturedIcon(tPosX+localOffset,tPosY,HIERARCHY_ICON_WIDTH*App::sc,HIERARCHY_ICON_HEIGHT*App::sc,transparencyFactor);
            }
            if (!forDragAndDrop)
            {
                hier->scriptParametersIconPosition.push_back(tPosX+localOffset);
                hier->scriptParametersIconPosition.push_back(tPosY);
                hier->scriptParametersIconPosition.push_back(script->getScriptID());
            }
            localOffset+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*App::sc;
        }
    }
    lineLastPos+=localOffset;

    if (lineLastPos>maxRenderedPos[0])
        maxRenderedPos[0]=lineLastPos;


    // Show the dynamic icons:
    if ((it!=nullptr)&&(!forDragAndDrop))
    { 
        int simIconCode=it->getDynamicSimulationIconCode();
        if (simIconCode!=sim_dynamicsimicon_none)
        {
            int ic;
            if (simIconCode==sim_dynamicsimicon_objectisdynamicallysimulated)
                ic=DYNAMICS_PICTURE;
            if (simIconCode==sim_dynamicsimicon_objectisnotdynamicallyenabled)
                ic=WARNING_PICTURE;
            lineLastPos+=16*App::sc+_drawIcon_3DObject(hier,lineLastPos+24*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,it,ic,!dontDisplay,transparencyFactor,forDragAndDrop);
            int mmm=SIM_MAX(lineLastPos,textEndPosition);
//          if (textEndPosition+lineLastPos>maxRenderedPos[0])
//              maxRenderedPos[0]=textEndPosition+lineLastPos;
            if (mmm>maxRenderedPos[0])
                maxRenderedPos[0]=mmm;
        }
    }

    if (!forDragAndDrop)
    {
        hier->lineLastPosition.push_back(lineLastPos);
        hier->lineLastPosition.push_back(textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
        if (it!=nullptr)
            hier->lineLastPosition.push_back(it->getObjectHandle());
        else
            hier->lineLastPosition.push_back(-1);
    }



    if ((!dontDisplay)&&(!forDragAndDrop)&&(it!=nullptr))
    { // Following for the line element(s) just left of scene object icons:
        if (it->getModelBase())
        {
            ogl::drawSingle2dLine_i(textPos[0]+16*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,textPos[0]+20*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
            if ((it->childList.size()!=0)&&((it->getLocalObjectProperty()&sim_objectproperty_collapsed)==0))
                ogl::drawSingle2dLine_i(textPos[0]+10*App::sc,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-CONST_VAL_6)*App::sc,textPos[0]+10*App::sc,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_INTER_LINE_SPACE)*App::sc);
        }
        else
        {
            ogl::drawSingle2dLine_i(textPos[0]+4*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,textPos[0]+20*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
            if ((it->childList.size()!=0)&&((it->getLocalObjectProperty()&sim_objectproperty_collapsed)==0))
                ogl::drawSingle2dLine_i(textPos[0]+10*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,textPos[0]+10*App::sc,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_INTER_LINE_SPACE)*App::sc);
        }
    }

    textPos[1]=textPos[1]-HIERARCHY_INTER_LINE_SPACE*App::sc;


    if ( ( (it!=nullptr)&&((it->getLocalObjectProperty()&sim_objectproperty_collapsed)==0) )||(objectID<0) )
    {
        int xPosCopy=textPos[0];
        textPos[0]=textPos[0]+horizontalShift;
        int indentCopy=indentNb+1;

        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_AND_BROWSER_LINE_COLOR);

        std::vector<CHierarchyElement*> el;
        std::vector<int> elNb;

        for (int i=0;i<int(children.size());i++)
        { // This is needed to order the elements from least sub-elements to most sub-elements
            int els=children[i]->getNumberOfElements();
            if (App::userSettings->orderHierarchyAlphabetically)
                els=1;
            int j;
            for (j=0;j<int(elNb.size());j++)
            {
                if (els<elNb[j])
                    break;
            }
            elNb.insert(elNb.begin()+j,els);
            el.insert(el.begin()+j,children[i]);
        }

        for (int i=0;i<int(el.size());i++)
        {
            if (i!=int(el.size())-1)
            { // Vertical line going through (a T turned counter-clockwise)
                vertLines->push_back(indentNb);
                int txtYTmp=textPos[1];
                el[i]->renderElement_3DObject(hier,labelEditObjectID,bright,dontDisplay,
                                            renderingSize,textPos,indentCopy,
                                            vertLines,minRenderedPos,maxRenderedPos,forDragAndDrop,transparentForTreeObjects,dropID);
                vertLines->erase(vertLines->end()-1);
                if ((!dontDisplay)&&(!forDragAndDrop))
                {
                    C3DObject* childIt=App::ct->objCont->getObjectFromHandle(el[i]->getLinkedObjectID());
                    if (childIt->childList.size()==0)
                    {
                        ogl::drawSingle2dLine_i(xPosCopy+10*App::sc,txtYTmp+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,xPosCopy+10*App::sc,txtYTmp+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc);
                        ogl::drawSingle2dLine_i(xPosCopy+10*App::sc,txtYTmp+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,xPosCopy+17*App::sc,txtYTmp+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
                    }
                }
            }
            else
            { // Vertical line stopping in the middle (L)
                int txtYTmp=textPos[1];
                el[i]->renderElement_3DObject(hier,labelEditObjectID,bright,dontDisplay,
                                            renderingSize,textPos,indentCopy,
                                            vertLines,minRenderedPos,maxRenderedPos,forDragAndDrop,transparentForTreeObjects,dropID);
                if ((!dontDisplay)&&(!forDragAndDrop))
                {
                    C3DObject* childIt=App::ct->objCont->getObjectFromHandle(el[i]->getLinkedObjectID());
                    if (childIt->childList.size()==0)
                    {
                        ogl::drawSingle2dLine_i(xPosCopy+10*App::sc,txtYTmp+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,xPosCopy+10*App::sc,txtYTmp+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
                        ogl::drawSingle2dLine_i(xPosCopy+10*App::sc,txtYTmp+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,xPosCopy+17*App::sc,txtYTmp+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
                    }
                }
            }
        }
        textPos[0]=xPosCopy;
    }
}
#endif

#ifdef KEYWORD__NOT_DEFINED_FORMELY_BR
int CHierarchyElement::_drawIcon_brm(CHierarchy* hier,int tPosX,int tPosY,C3DObject* it,int pictureID,bool drawIt,float transparencyFactor,bool forDragAndDrop)
//*
{ // pictureID is -1 by default. It is then ignored. The size of the icon is 16x16
    int retVal=0;
    if (pictureID!=NO_TREE_PICTURE)
    {
        if (pictureID>=0)
        {
            if (drawIt)
            {
                App::ct->globalGuiTextureCont->startTextureDisplay(pictureID);
                if ((it!=nullptr)&&(!forDragAndDrop))
                {
                    if (((pictureID==PLUS_SIGN_TREE_PICTURE)||(pictureID==MINUS_SIGN_TREE_PICTURE)))
                    {
                        hier->inflateIconPosition.push_back(tPosX);
                        hier->inflateIconPosition.push_back(tPosY);
                        hier->inflateIconPosition.push_back(it->getID());
                    }
                    else
                    {
                        /*
                        if (it->getDynamicSimulationIconCode()!=sim_dynamicsimicon_none)
                        {
                            hier->simulationIconPosition.push_back(tPosX);
                            hier->simulationIconPosition.push_back(tPosY);
                            hier->simulationIconPosition.push_back(it->getID());
                        }
                        */
                    }
                }
                _drawTexturedIcon(tPosX,tPosY,HIERARCHY_ICON_WIDTH*App::sc,HIERARCHY_ICON_HEIGHT*App::sc,transparencyFactor);
            }
            retVal+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*App::sc;
        }
        else
        {
            int objectOrWorldIconID=-1;
            if (drawIt)
            {
                if (it==nullptr)
                {
                    objectOrWorldIconID=WORLD_TREE_PICTURE;
                    if (!isLocalWorld())
                        transparencyFactor=0.6f;
                }
            }
            if (drawIt)
            {
                if (it!=nullptr)
                { // We have to draw a model icon before the object icon:
                    App::ct->globalGuiTextureCont->startTextureDisplay(MODEL_TREE_PICTURE);
                    _drawTexturedIcon(tPosX+retVal,tPosY,HIERARCHY_ICON_WIDTH*App::sc,HIERARCHY_ICON_HEIGHT*App::sc,transparencyFactor);
                    if (!forDragAndDrop)
                    {
                        hier->modelIconPosition.push_back(tPosX+retVal);
                        hier->modelIconPosition.push_back(tPosY);
                        hier->modelIconPosition.push_back(it->getID());
                    }
                }
//                retVal+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*App::sc;
                retVal+=(HIERARCHY_ICON_WIDTH)*App::sc;

               if (objectOrWorldIconID==WORLD_TREE_PICTURE)
               {
                    App::ct->globalGuiTextureCont->startTextureDisplay(objectOrWorldIconID);
                    _drawTexturedIcon(tPosX+retVal,tPosY,HIERARCHY_ICON_WIDTH*App::sc,HIERARCHY_ICON_HEIGHT*App::sc,transparencyFactor);
               }
               if (!forDragAndDrop)
               {
                   if ( (it!=nullptr)||(pictureID<0) )
                   {
                       hier->objectIconPosition.push_back(tPosX+retVal);
                       hier->objectIconPosition.push_back(tPosY);
                       if (it==nullptr)
                           hier->objectIconPosition.push_back(objectID);
                       else
                           hier->objectIconPosition.push_back(it->getID());
                   }
               }
               if (objectOrWorldIconID==WORLD_TREE_PICTURE)
                   retVal+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*App::sc;
            }
            else
            {
                if (it!=nullptr)
                { // We have to consider a model icon before the object icon:
                    if (!forDragAndDrop)
                    {
                        hier->modelIconPosition.push_back(tPosX+retVal);
                        hier->modelIconPosition.push_back(tPosY);
                        hier->modelIconPosition.push_back(it->getID());
                    }
//                    retVal+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*App::sc;
                    retVal+=(HIERARCHY_ICON_WIDTH)*App::sc;
                }
                if (!forDragAndDrop)
                {
                    hier->objectIconPosition.push_back(tPosX+retVal);
                    hier->objectIconPosition.push_back(tPosY);
                    if (it==nullptr)
                        hier->objectIconPosition.push_back(-1); // World icon
                    else
                        hier->objectIconPosition.push_back(it->getID());
                }
                if (objectOrWorldIconID==WORLD_TREE_PICTURE)
                    retVal+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*App::sc;
            }

        }
    }
    else
        retVal+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*App::sc;
    return(retVal);
}
#else
int CHierarchyElement::_drawIcon_3DObject(CHierarchy* hier,int tPosX,int tPosY,C3DObject* it,int pictureID,bool drawIt,float transparencyFactor,bool forDragAndDrop)
{ // pictureID is -1 by default. It is then ignored. The size of the icon is 16x16
    int retVal=0;
    if (pictureID!=NO_TREE_PICTURE)
    {
        if (pictureID>=0)
        {
            if (drawIt)
            {
                App::ct->globalGuiTextureCont->startTextureDisplay(pictureID);
                if ((it!=nullptr)&&(!forDragAndDrop))
                {
                    if (((pictureID==PLUS_SIGN_TREE_PICTURE)||(pictureID==MINUS_SIGN_TREE_PICTURE)))
                    {
                        hier->inflateIconPosition.push_back(tPosX);
                        hier->inflateIconPosition.push_back(tPosY);
                        hier->inflateIconPosition.push_back(it->getObjectHandle());
                    }
                    else
                    {
                        if (it->getDynamicSimulationIconCode()!=sim_dynamicsimicon_none)
                        {
                            hier->simulationIconPosition.push_back(tPosX);
                            hier->simulationIconPosition.push_back(tPosY);
                            hier->simulationIconPosition.push_back(it->getObjectHandle());
                        }
                    }
                }
                _drawTexturedIcon(tPosX,tPosY,HIERARCHY_ICON_WIDTH*App::sc,HIERARCHY_ICON_HEIGHT*App::sc,transparencyFactor);
            }
            retVal+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*App::sc;
        }
        else
        {
            int type=-1; // World
            if (it!=nullptr)
                type=it->getObjectType();
            int objectOrWorldIconID=-1;
            if (drawIt)
            {
                if (type==-1)
                {
                    objectOrWorldIconID=WORLD_TREE_PICTURE;
                    if (!isLocalWorld())
                        transparencyFactor=0.6f;
                }
                if (type==sim_object_shape_type)
                {
                    if (((CShape*)it)->isCompound())
                    {
                        if (((CShape*)it)->geomData->geomInfo->isPure())
                            objectOrWorldIconID=PURE_MULTISHAPE_TREE_PICTURE;
                        else if (((CShape*)it)->geomData->geomInfo->isConvex())
                            objectOrWorldIconID=CONVEX_MULTISHAPE_TREE_PICTURE;
                        else
                            objectOrWorldIconID=MULTI_SHAPE_TREE_PICTURE;
                    }
                    else
                    {
                        if (((CShape*)it)->geomData->geomInfo->isPure())
                        {
                            if (((CGeometric*)((CShape*)it)->geomData->geomInfo)->getPurePrimitiveType()==sim_pure_primitive_heightfield)
                                objectOrWorldIconID=HEIGHTFIELD_SHAPE_TREE_PICTURE;
                            else
                                objectOrWorldIconID=PURE_SHAPE_TREE_PICTURE;
                        }
                        else if (((CShape*)it)->geomData->geomInfo->isConvex())
                            objectOrWorldIconID=CONVEX_SHAPE_TREE_PICTURE;
                        else
                            objectOrWorldIconID=SIMPLE_SHAPE_TREE_PICTURE;
                    }
                }
                if (type==sim_object_joint_type)
                {
                    if (((CJoint*)it)->getJointType()==sim_joint_revolute_subtype)
                        objectOrWorldIconID=REVOLUTE_JOINT_TREE_PICTURE;
                    if (((CJoint*)it)->getJointType()==sim_joint_prismatic_subtype)
                        objectOrWorldIconID=PRISMATIC_JOINT_TREE_PICTURE;
                    if (((CJoint*)it)->getJointType()==sim_joint_spherical_subtype)
                        objectOrWorldIconID=SPHERICAL_JOINT_TREE_PICTURE;
                }
                if (type==sim_object_camera_type)
                    objectOrWorldIconID=CAMERA_TREE_PICTURE;
                if (type==sim_object_mirror_type)
                {
                    if (((CMirror*)it)->getIsMirror())
                        objectOrWorldIconID=MIRROR_TREE_PICTURE;
                    else
                        objectOrWorldIconID=CLIPPLANE_TREE_PICTURE;
                }
                if (type==sim_object_light_type)
                {
                    int subType=((CLight*)it)->getLightType();
                    if (subType==sim_light_omnidirectional_subtype)
                    {
                        if (((CLight*)it)->getLightActive())
                            objectOrWorldIconID=OMNI_LIGHT_ON_TREE_PICTURE;
                        else
                            objectOrWorldIconID=OMNI_LIGHT_OFF_TREE_PICTURE;
                    }
                    if (subType==sim_light_spot_subtype)
                    {
                        if (((CLight*)it)->getLightActive())
                            objectOrWorldIconID=SPOT_LIGHT_ON_TREE_PICTURE;
                        else
                            objectOrWorldIconID=SPOT_LIGHT_OFF_TREE_PICTURE;
                    }
                    if (subType==sim_light_directional_subtype)
                    {
                        if (((CLight*)it)->getLightActive())
                            objectOrWorldIconID=DIR_LIGHT_ON_TREE_PICTURE;
                        else
                            objectOrWorldIconID=DIR_LIGHT_OFF_TREE_PICTURE;
                    }
                }
                if (type==sim_object_graph_type)
                    objectOrWorldIconID=GRAPH_TREE_PICTURE;
                if (type==sim_object_dummy_type)
                    objectOrWorldIconID=DUMMY_TREE_PICTURE;
                if (type==sim_object_pointcloud_type)
                    objectOrWorldIconID=POINTCLOUD_TREE_PICTURE;
                if (type==sim_object_octree_type)
                    objectOrWorldIconID=OCTREE_TREE_PICTURE;
                if (type==sim_object_proximitysensor_type)
                    objectOrWorldIconID=PROXIMITYSENSOR_TREE_PICTURE;
                if (type==sim_object_visionsensor_type)
                    objectOrWorldIconID=VISIONSENSOR_TREE_PICTURE;
                if (type==sim_object_path_type)
                    objectOrWorldIconID=PATH_TREE_PICTURE;
                if (type==sim_object_mill_type)
                    objectOrWorldIconID=MILL_TREE_PICTURE;
                if (type==sim_object_forcesensor_type)
                    objectOrWorldIconID=FORCE_SENSOR_TREE_PICTURE;
            }
            if (drawIt)
            {
                if ((it!=nullptr)&&it->getModelBase())
                { // We have to draw a model icon before the object icon:
                    App::ct->globalGuiTextureCont->startTextureDisplay(MODEL_TREE_PICTURE);
                    _drawTexturedIcon(tPosX+retVal,tPosY,HIERARCHY_ICON_WIDTH*App::sc,HIERARCHY_ICON_HEIGHT*App::sc,transparencyFactor);
                    if (!forDragAndDrop)
                    {
                        hier->modelIconPosition.push_back(tPosX+retVal);
                        hier->modelIconPosition.push_back(tPosY);
                        hier->modelIconPosition.push_back(it->getObjectHandle());
                    }
                }
                retVal+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*App::sc;

                App::ct->globalGuiTextureCont->startTextureDisplay(objectOrWorldIconID);
                _drawTexturedIcon(tPosX+retVal,tPosY,HIERARCHY_ICON_WIDTH*App::sc,HIERARCHY_ICON_HEIGHT*App::sc,transparencyFactor);
                if (!forDragAndDrop)
                {
                    if ( (it!=nullptr)||((pictureID<0) ))
                    {
                        hier->objectIconPosition.push_back(tPosX+retVal);
                        hier->objectIconPosition.push_back(tPosY);
                        if (it==nullptr)
                            hier->objectIconPosition.push_back(objectID);
                        else
                            hier->objectIconPosition.push_back(it->getObjectHandle());
                    }
                }
                retVal+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*App::sc;
            }
            else
            {
                if ((it!=nullptr)&&it->getModelBase())
                { // We have to consider a model icon before the object icon:
                    if (!forDragAndDrop)
                    {
                        hier->modelIconPosition.push_back(tPosX+retVal);
                        hier->modelIconPosition.push_back(tPosY);
                        hier->modelIconPosition.push_back(it->getObjectHandle());
                    }
                    retVal+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*App::sc;
                }
                if (!forDragAndDrop)
                {
                    hier->objectIconPosition.push_back(tPosX+retVal);
                    hier->objectIconPosition.push_back(tPosY);
                    if (it==nullptr)
                        hier->objectIconPosition.push_back(-1); // World icon
                    else
                        hier->objectIconPosition.push_back(it->getObjectHandle());
                }
                retVal+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*App::sc;
            }

        }
    }
    else
        retVal+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*App::sc;
    return(retVal);
}
#endif

void CHierarchyElement::renderElement_editModeList(CHierarchy* hier,int labelEditObjectID,bool& bright,bool dontDisplay,
                                int renderingSize[2],int textPos[2],int indentNb,int minRenderedPos[2],int maxRenderedPos[2],
                                int selectedState,int editModeType)
{
    bright=!bright;
    if (textPos[0]<minRenderedPos[0])
        minRenderedPos[0]=textPos[0];
    if (textPos[1]<minRenderedPos[1])
        minRenderedPos[1]=textPos[1];
    if (textPos[1]>maxRenderedPos[1])
        maxRenderedPos[1]=textPos[1];

    if (textPos[1]<-HIERARCHY_INTER_LINE_SPACE*App::sc)
        dontDisplay=true; // We don't display what is outside of the view!
    bool textInside=(textPos[1]<renderingSize[1]+HIERARCHY_INTER_LINE_SPACE*App::sc);
    std::string theText;
    if (editModeType==VERTEX_EDIT_MODE)
    {
        theText="Vertex ";
        theText+=boost::lexical_cast<std::string>(objectID+1);
    }
    if (editModeType==TRIANGLE_EDIT_MODE)
    {
        theText="Triangle ";
        theText+=boost::lexical_cast<std::string>(objectID+1);
    }
    if (editModeType==EDGE_EDIT_MODE)
    {
        theText="Edge ";
        theText+=boost::lexical_cast<std::string>(objectID+1);
    }
    if (editModeType==PATH_EDIT_MODE)
    {
        theText="Path point ";
        theText+=boost::lexical_cast<std::string>(objectID+1);
    }
    if (editModeType==MULTISHAPE_EDIT_MODE)
    {
        theText="Shape component ";
        theText+=boost::lexical_cast<std::string>(objectID+1);
        if (objectID==App::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex())
            selectedState=2;
    }
    if (editModeType==BUTTON_EDIT_MODE)
    {
        if (objectID==App::ct->buttonBlockContainer->getBlockInEdition())
            selectedState=2;
        CButtonBlock* blk=App::ct->buttonBlockContainer->getBlockWithID(objectID);
        if (blk!=nullptr)
            theText=blk->getBlockName();
        else
            theText="ERROR";
    }
    if ((selectedState!=0)&&textInside&&(!dontDisplay))
    {
        if (selectedState==2)
            ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_AND_BROWSER_LAST_SELECTION_COLOR);
        else
        {
            if (bright)
                ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_NOT_LAST_SELECTION_COLOR_BRIGHT);
            else
                ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_NOT_LAST_SELECTION_COLOR_DARK);
        }
        glBegin(GL_QUADS);
        glVertex3i(0,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
        glVertex3i(renderingSize[0],textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
        glVertex3i(renderingSize[0],textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
        glVertex3i(0,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
        glEnd();
    }
    else if (textInside&&(!dontDisplay))
    {
        if (!bright)
            ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_DARK);
        else
            ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT);
        glBegin(GL_QUADS);
        glVertex3i(0,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
        glVertex3i(renderingSize[0],textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
        glVertex3i(renderingSize[0],textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
        glVertex3i(0,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
        glEnd();
    }

    if (textInside&&(!dontDisplay))
    {
        hier->objectPosition.push_back(textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
        hier->objectPosition.push_back(objectID);
    }
    int off=2;

    if (editModeType==VERTEX_EDIT_MODE)
        off=off+_drawIcon_editModeList(hier,textPos[0]+off+8*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,VERTEX_PICTURE,!dontDisplay);
    if (editModeType==TRIANGLE_EDIT_MODE)
        off=off+_drawIcon_editModeList(hier,textPos[0]+off+8*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,TRIANGLE_PICTURE,!dontDisplay);
    if (editModeType==EDGE_EDIT_MODE)
        off=off+_drawIcon_editModeList(hier,textPos[0]+off+8*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,EDGE_PICTURE,!dontDisplay);
    if (editModeType==PATH_EDIT_MODE)
        off=off+_drawIcon_editModeList(hier,textPos[0]+off+8*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,PATH_POINT_PICTURE,!dontDisplay);
    if (editModeType==BUTTON_EDIT_MODE)
        off=off+_drawIcon_editModeList(hier,textPos[0]+off+8*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,BUTTON_BLOCK_PICTURE,!dontDisplay);
    if (editModeType==MULTISHAPE_EDIT_MODE)
    {
        if ( (objectID>=0)&&(objectID<App::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentsSize()) )
        {
            CGeometric* geom=App::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentAtIndex(objectID);
            int pic=0;
            if (geom->isPure())
                pic=PURE_SHAPE_TREE_PICTURE;
            else
            {
                if (geom->isConvex())
                    pic=CONVEX_SHAPE_TREE_PICTURE;
                else
                    pic=SIMPLE_SHAPE_TREE_PICTURE;
            }
            off=off+_drawIcon_editModeList(hier,textPos[0]+off+8*App::sc,textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc,pic,!dontDisplay);
        }
    }
        
    int textEndPosition=textPos[0]+off+ogl::getTextLengthInPixels(theText);
    hier->textPosition.push_back(textPos[0]);
    hier->textPosition.push_back(textPos[1]);
    hier->textPosition.push_back(textPos[0]+off);
    hier->textPosition.push_back(textEndPosition);
    hier->textPosition.push_back(textPos[1]+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
    hier->textPosition.push_back(objectID);

    if (textEndPosition>maxRenderedPos[0])
        maxRenderedPos[0]=textEndPosition;

    if (textInside&&(!dontDisplay))
    {
        ogl::setTextColor(ogl::colorBlack);
        if (labelEditObjectID!=objectID)
            ogl::drawText(textPos[0]+off,textPos[1],0,theText);
        else
            hier->drawEditionLabel(textPos[0]+off,textPos[1]);
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorBlack);
    }
    textPos[1]=textPos[1]-HIERARCHY_INTER_LINE_SPACE*App::sc;
}

int CHierarchyElement::_drawIcon_editModeList(CHierarchy* hier,int tPosX,int tPosY,int pictureID,bool drawIt)
{ // The size of the icon is 16x16
    const int sizeX=16*App::sc;
    const int sizeY=16*App::sc;
    const int interIconSpacing=4*App::sc;
    int retVal=0;
    if (pictureID!=NO_TREE_PICTURE)
    {
        if (pictureID!=-1)
        {
            if (drawIt)
            {
                App::ct->globalGuiTextureCont->startTextureDisplay(pictureID);
                _drawTexturedIcon(tPosX,tPosY,sizeX,sizeY,0.0f);
            }
            hier->objectIconPosition.push_back(tPosX);
            hier->objectIconPosition.push_back(tPosY);
            hier->objectIconPosition.push_back(objectID);
        }
    }
    retVal+=sizeX+interIconSpacing;
    return(retVal);
}

void CHierarchyElement::_drawTexturedIcon(int tPosX,int tPosY,int sizeX,int sizeY,float transparencyFactor)
{
    ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // We turn blending on!

    if (transparencyFactor>0.01f)
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f-transparencyFactor);
    else
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // opaque

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

    glTexEnvf (GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
    ogl::disableLighting_useWithCare(); // only temporarily
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,0.0f);
    glVertex3i(tPosX-sizeX/2,tPosY-sizeY/2,0);
    glTexCoord2f(0.625f,0.0f); // icons themselves are 20x16, but since some gfx cards don't support that non-power of 2 res, we save them as 32x16!
    glVertex3i(tPosX+sizeX/2,tPosY-sizeY/2,0);
    glTexCoord2f(0.625f,1.0f); // icons themselves are 20x16, but since some gfx cards don't support that non-power of 2 res, we save them as 32x16!
    glVertex3i(tPosX+sizeX/2,tPosY+sizeY/2,0);
    glTexCoord2f(0.0f,1.0f);
    glVertex3i(tPosX-sizeX/2,tPosY+sizeY/2,0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    ogl::enableLighting_useWithCare();
    ogl::setBlending(false);
    glDisable(GL_ALPHA_TEST);
    App::ct->globalGuiTextureCont->endTextureDisplay();
}

bool CHierarchyElement::renderDummyElement(bool& bright,int renderingSize[2],int textPos[2])
{ // Just renders the background and returns true when inside the visible zone
    bright=!bright;
    if (textPos[1]<-HIERARCHY_INTER_LINE_SPACE*App::sc)
        return(false);
    if (textPos[1]<renderingSize[1]+HIERARCHY_INTER_LINE_SPACE*App::sc)
    {
        if (!bright)
            ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_DARK);
        else
            ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT);
        glBegin(GL_QUADS);
        glVertex3i(0,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
        glVertex3i(renderingSize[0],textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET-HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
        glVertex3i(renderingSize[0],textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
        glVertex3i(0,textPos[1]+(HIERARCHY_TEXT_CENTER_OFFSET+HIERARCHY_HALF_INTER_LINE_SPACE)*App::sc,0);
        glEnd();
        textPos[1]=textPos[1]-HIERARCHY_INTER_LINE_SPACE*App::sc;
        return(true);
    }
    return(false);
}


int CHierarchyElement::getNumberOfElements()
{
    if (objectID<0)
    {
        if (!isLocalWorld())
            return(999999); // for the other instances!
    }
    return(numberOfElements);
}

bool CHierarchyElement::isLocalWorld()
{
    if (objectID<0)
        return(-App::ct->getCurrentInstanceIndex()-1==objectID);
    return(false);
}

int CHierarchyElement::computeNumberOfElements()
{
    numberOfElements=1;
    for (size_t i=0;i<children.size();i++)
    {
        children[i]->computeNumberOfElements();
        numberOfElements=numberOfElements+children[i]->getNumberOfElements();
    }
    return(numberOfElements);
}


