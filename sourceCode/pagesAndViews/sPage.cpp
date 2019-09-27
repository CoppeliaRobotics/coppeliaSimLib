
// This file requires some serious refactoring!

#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "sPage.h"
#include "global.h"
#include "tt.h"
#include "app.h"
#include "pageRendering.h"

#define FLOATING_VIEW_MIN_SIZE 80

enum {AUX_VIEW_SHIFTING=0,AUX_VIEW_TOP_BORDER,AUX_VIEW_BOTTOM_BORDER,AUX_VIEW_RIGHT_BORDER,
    AUX_VIEW_LEFT_BORDER,AUX_VIEW_TOP_RIGHT_BORDER,AUX_VIEW_TOP_LEFT_BORDER,
    AUX_VIEW_BOTTOM_RIGHT_BORDER,AUX_VIEW_BOTTOM_LEFT_BORDER,AUX_VIEW_CLOSING_BUTTON};

const int auxViewsBtSize=12;


CSPage::CSPage(int thePageType)
{
    if (!setPageType(thePageType))
        setPageType(SINGLE_VIEW);
    auxViewResizingAction=-1;
    floatingViewAddOffset=0.0f;
    _caughtElements=0;
    _lastMouseDownViewIndex=-1;
    _initialValuesInitialized=false;
}

CSPage::~CSPage()
{
    for (size_t i=0;i<_allViews.size();i++)
        delete _allViews[i];
    _allViews.clear();
    _allViewAuxSizesAndPos.clear();
}

bool CSPage::isViewValid(CSView* v) const
{
    for (size_t i=0;i<_allViews.size();i++)
    {
        if (_allViews[i]==v)
            return(true);
    }
    return(false);
}

void CSPage::initializeInitialValues(bool simulationIsRunning,int initializeOnlyForThisNewObject)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    for (size_t i=0;i<_allViews.size();i++)
    {
        int loID=_allViews[i]->getLinkedObjectID();
        if ((initializeOnlyForThisNewObject==-1)||((loID!=-1)&&(loID==initializeOnlyForThisNewObject)))
            _allViews[i]->initializeInitialValues(simulationIsRunning);
    }
    if (initializeOnlyForThisNewObject==-1)
    {
        _initialValuesInitialized=simulationIsRunning;
        if (simulationIsRunning)
        {
            // make sure we memorize the floating view sizes and positions:
            _initialAuxViewSizesAndPos.clear();
            _initialAuxViewUniqueIDs.clear();
            for (int i=getRegularViewCount();i<int(_allViews.size());i++)
            {
                _initialAuxViewSizesAndPos.push_back(_allViewAuxSizesAndPos[4*i+0]);
                _initialAuxViewSizesAndPos.push_back(_allViewAuxSizesAndPos[4*i+1]);
                _initialAuxViewSizesAndPos.push_back(_allViewAuxSizesAndPos[4*i+2]);
                _initialAuxViewSizesAndPos.push_back(_allViewAuxSizesAndPos[4*i+3]);
                _initialAuxViewUniqueIDs.push_back(_allViews[i]->getUniqueID());
            }
        }
    }
    else
    { // this was called for a specific object!
        if (_initialValuesInitialized&&simulationIsRunning)
        {
            for (int i=getRegularViewCount();i<int(_allViews.size());i++)
            {
                if (_allViews[i]->getLinkedObjectID()==initializeOnlyForThisNewObject)
                {
                    _allViews[i]->initializeInitialValues(simulationIsRunning);
                    _initialAuxViewSizesAndPos.push_back(_allViewAuxSizesAndPos[4*i+0]);
                    _initialAuxViewSizesAndPos.push_back(_allViewAuxSizesAndPos[4*i+1]);
                    _initialAuxViewSizesAndPos.push_back(_allViewAuxSizesAndPos[4*i+2]);
                    _initialAuxViewSizesAndPos.push_back(_allViewAuxSizesAndPos[4*i+3]);
                    _initialAuxViewUniqueIDs.push_back(_allViews[i]->getUniqueID());
                }
            }
        }
    }
}

void CSPage::simulationEnded()
{
    for (int i=0;i<int(_allViews.size());i++)
    {
        if (_allViews[i]->simulationEnded())
        { //  Following part is from 26/6/2011: we have to remove a floating view that requested destruction at simulation end
            // Make sure it is a floating view:
            if (removeFloatingView(i))
                i--; // We have to reprocess this position
        }
    }

    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
    {
        // Make sure we restore floating view's initial positions and sizes:
        for (int i=getRegularViewCount();i<int(_allViews.size());i++)
        {
            int uniqueID=_allViews[i]->getUniqueID();
            for (int j=0;j<int(_initialAuxViewUniqueIDs.size());j++)
            {
                if (_initialAuxViewUniqueIDs[j]==uniqueID)
                {
                    _allViewAuxSizesAndPos[4*i+0]=_initialAuxViewSizesAndPos[4*j+0];
                    _allViewAuxSizesAndPos[4*i+1]=_initialAuxViewSizesAndPos[4*j+1];
                    _allViewAuxSizesAndPos[4*i+2]=_initialAuxViewSizesAndPos[4*j+2];
                    _allViewAuxSizesAndPos[4*i+3]=_initialAuxViewSizesAndPos[4*j+3];
                    break;
                }
            }
        }
        setViewSizesAndPositions(); // needed to reflect the change
    }
    _initialValuesInitialized=false;
}

int CSPage::getPageType() const
{
    return(_pageType);
}

bool CSPage::setPageType(int newType)
{
    if ( (newType<0)||(newType>=CLOSING_VIEW_ITEM) )
        return(false);
    _pageType=newType;
    for (int i=0;i<int(_allViews.size());i++)
        delete _allViews[i];
    _allViews.clear();
    _allViewAuxSizesAndPos.clear();
    int newSize=getRegularViewCount();
    for (int i=0;i<newSize;i++)
    {
        CSView* it=new CSView(-1);
        _allViews.push_back(it);
        // Positions and sizes are relative now (2009/05/22)
        _allViewAuxSizesAndPos.push_back(0.25f);
        _allViewAuxSizesAndPos.push_back(0.25f);
        _allViewAuxSizesAndPos.push_back(0.05f);
        _allViewAuxSizesAndPos.push_back(0.05f);
    }
    setViewSizesAndPositions();
    return(true);
}

void CSPage::getBorderCorrectedFloatingViewPosition(int posX,int posY,int sizeX,int sizeY,int& newPosX,int& newPosY) const
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    newPosX=posX;
    newPosY=posY;
    if (newPosX+sizeX>_pageSize[0])
        newPosX=_pageSize[0]-sizeX;
    if (newPosY+sizeY>_pageSize[1])
        newPosY=_pageSize[1]-sizeY;
    if (newPosX<0)
        newPosX=0;
    if (newPosY<0)
        newPosY=0;
}

void CSPage::announceObjectWillBeErased(int objectID)
{ // Never called from copy buffer!
    int i=0;
    while (i<int(_allViews.size()))
    {
        if (_allViews[i]->announceObjectWillBeErased(objectID)&&(i>=getRegularViewCount()))
        { // we have to destroy this view!
            removeFloatingView(i);
            i=0; // ordering might have changed!
        }
        else
            i++;
    }
}
void CSPage::performObjectLoadingMapping(std::vector<int>* map)
{
    for (int i=0;i<int(_allViews.size());i++)
        _allViews[i]->performObjectLoadingMapping(map);
}

void CSPage::getPageSizeAndPosition(int& sizeX,int& sizeY,int& posX,int& posY) const
{
    sizeX=_pageSize[0];
    sizeY=_pageSize[1];
    posX=_pagePosition[0];
    posY=_pagePosition[1];
}

void CSPage::setPageSizeAndPosition(int sizeX,int sizeY,int posX,int posY)
{
    // The size and position of a view group is same as for all views:
    _pageSize[0]=sizeX;
    _pageSize[1]=sizeY;
    _pagePosition[0]=posX;
    _pagePosition[1]=posY;
    // We set the view position and size for all subViews:
    setViewSizesAndPositions();
}

void CSPage::getViewSizeAndPosition(int sViewSize[2],int sViewPos[2],int subViewIndex) const
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if (_pageType==SINGLE_VIEW)
    {
        sViewSize[0]=_pageSize[0];
        sViewSize[1]=_pageSize[1];
        sViewPos[0]=_pagePosition[0];
        sViewPos[1]=_pagePosition[1];
    }
    if (_pageType==FOUR_VIEWS)
    {
        sViewSize[0]=(_pageSize[0]+1)/2; // +1 because of some flickering: page size 15 would be 7+7
        sViewSize[1]=(_pageSize[1]+1)/2;
        if ( (subViewIndex==0)||(subViewIndex==2) )
            sViewPos[0]=_pagePosition[0];
        else
            sViewPos[0]=_pagePosition[0]+_pageSize[0]/2;
        if ( (subViewIndex==2)||(subViewIndex==3) )
            sViewPos[1]=_pagePosition[1];
        else
            sViewPos[1]=_pagePosition[1]+_pageSize[1]/2;
    }
    if (_pageType==SIX_VIEWS_OLD)
    {
        if (subViewIndex==0)
        {
            sViewSize[0]=2*(_pageSize[0]+2)/3;
            sViewSize[1]=2*(_pageSize[1]+2)/3;
            sViewPos[0]=_pagePosition[0]+_pageSize[0]/3;
            sViewPos[1]=_pagePosition[1];
        }
        else
        {
            sViewSize[0]=(_pageSize[0]+2)/3;
            sViewSize[1]=(_pageSize[1]+2)/3;
            if ( (subViewIndex==1)||(subViewIndex==4)||(subViewIndex==5) )
                sViewPos[0]=_pagePosition[0];
            else if (subViewIndex==2)
                sViewPos[0]=_pagePosition[0]+_pageSize[0]/3;
            else
                sViewPos[0]=_pagePosition[0]+2*_pageSize[0]/3;
            if ( (subViewIndex==1)||(subViewIndex==2)||(subViewIndex==3) )
                sViewPos[1]=_pagePosition[1]+2*_pageSize[1]/3;
            else if (subViewIndex==4)
                sViewPos[1]=_pagePosition[1]+_pageSize[1]/3;
            else
                sViewPos[1]=_pagePosition[1];
        }
    }
    if (_pageType==HORIZONTALLY_DIVIDED)
    {
        sViewSize[0]=_pageSize[0];
        sViewSize[1]=(_pageSize[1]+1)/2;
        sViewPos[0]=_pagePosition[0];
        if (subViewIndex==0)
            sViewPos[1]=_pagePosition[1]+_pageSize[1]/2;
        else
            sViewPos[1]=_pagePosition[1];
    }
    if (_pageType==VERTICALLY_DIVIDED)
    {
        sViewSize[0]=(_pageSize[0]+1)/2;
        sViewSize[1]=_pageSize[1];
        sViewPos[1]=_pagePosition[1];
        if (subViewIndex==0)
            sViewPos[0]=_pagePosition[0];
        else
            sViewPos[0]=_pagePosition[0]+_pageSize[0]/2;
    }
    if (_pageType==HORIZONTALLY_DIVIDED_3)
    {
        if (subViewIndex==0)
        {
            sViewSize[0]=_pageSize[0];
            sViewSize[1]=(_pageSize[1]+1)/2;
            sViewPos[0]=_pagePosition[0];
            sViewPos[1]=_pagePosition[1]+_pageSize[1]/2;
        }
        if (subViewIndex==1)
        {
            sViewSize[0]=(_pageSize[0]+1)/2;
            sViewSize[1]=(_pageSize[1]+1)/2;
            sViewPos[0]=_pagePosition[0];
            sViewPos[1]=_pagePosition[1];
        }
        if (subViewIndex==2)
        {
            sViewSize[0]=(_pageSize[0]+1)/2;
            sViewSize[1]=(_pageSize[1]+1)/2;
            sViewPos[0]=_pagePosition[0]+_pageSize[0]/2;
            sViewPos[1]=_pagePosition[1];
        }
    }
    if (_pageType==VERTICALLY_DIVIDED_3)
    {
        if (subViewIndex==0)
        {
            sViewSize[0]=(_pageSize[0]+1)/2;
            sViewSize[1]=_pageSize[1];
            sViewPos[0]=_pagePosition[0];
            sViewPos[1]=_pagePosition[1];
        }
        if (subViewIndex==1)
        {
            sViewSize[0]=(_pageSize[0]+1)/2;
            sViewSize[1]=(_pageSize[1]+1)/2;
            sViewPos[0]=_pagePosition[0]+_pageSize[0]/2;
            sViewPos[1]=_pagePosition[1]+_pageSize[1]/2;
        }
        if (subViewIndex==2)
        {
            sViewSize[0]=(_pageSize[0]+1)/2;
            sViewSize[1]=(_pageSize[1]+1)/2;
            sViewPos[0]=_pagePosition[0]+_pageSize[0]/2;
            sViewPos[1]=_pagePosition[1];
        }
    }
    if (_pageType==HORIZONTAL_1_PLUS_3_VIEWS)
    {
        if (subViewIndex==0)
        {
            sViewSize[0]=_pageSize[0];
            sViewSize[1]=2*(_pageSize[1]+2)/3;
            sViewPos[0]=_pagePosition[0];
            sViewPos[1]=_pagePosition[1]+_pageSize[1]/3;
        }
        if (subViewIndex==1)
        {
            sViewSize[0]=(_pageSize[0]+2)/3;
            sViewSize[1]=(_pageSize[1]+2)/3;
            sViewPos[0]=_pagePosition[0];
            sViewPos[1]=_pagePosition[1];
        }
        if (subViewIndex==2)
        {
            sViewSize[0]=(_pageSize[0]+2)/3;
            sViewSize[1]=(_pageSize[1]+2)/3;
            sViewPos[0]=_pagePosition[0]+_pageSize[0]/3;
            sViewPos[1]=_pagePosition[1];
        }
        if (subViewIndex==3)
        {
            sViewSize[0]=(_pageSize[0]+2)/3;
            sViewSize[1]=(_pageSize[1]+2)/3;
            sViewPos[0]=_pagePosition[0]+2*_pageSize[0]/3;
            sViewPos[1]=_pagePosition[1];
        }
    }
    if (_pageType==VERTICAL_1_PLUS_3_VIEWS)
    {
        if (subViewIndex==0)
        {
            sViewSize[0]=2*(_pageSize[0]+2)/3;
            sViewSize[1]=_pageSize[1];
            sViewPos[0]=_pagePosition[0];
            sViewPos[1]=_pagePosition[1];
        }
        if (subViewIndex==1)
        {
            sViewSize[0]=(_pageSize[0]+2)/3;
            sViewSize[1]=(_pageSize[1]+2)/3;
            sViewPos[0]=_pagePosition[0]+2*_pageSize[0]/3;
            sViewPos[1]=_pagePosition[1]+2*_pageSize[1]/3;
        }
        if (subViewIndex==2)
        {
            sViewSize[0]=(_pageSize[0]+2)/3;
            sViewSize[1]=(_pageSize[1]+2)/3;
            sViewPos[0]=_pagePosition[0]+2*_pageSize[0]/3;
            sViewPos[1]=_pagePosition[1]+_pageSize[1]/3;
        }
        if (subViewIndex==3)
        {
            sViewSize[0]=(_pageSize[0]+2)/3;
            sViewSize[1]=(_pageSize[1]+2)/3;
            sViewPos[0]=_pagePosition[0]+2*_pageSize[0]/3;
            sViewPos[1]=_pagePosition[1];
        }
    }
    if (_pageType==HORIZONTAL_1_PLUS_4_VIEWS)
    {
        if (subViewIndex==0)
        {
            sViewSize[0]=_pageSize[0];
            sViewSize[1]=3*(_pageSize[1]+3)/4;
            sViewPos[0]=_pagePosition[0];
            sViewPos[1]=_pagePosition[1]+_pageSize[1]/4;
        }
        if (subViewIndex==1)
        {
            sViewSize[0]=(_pageSize[0]+3)/4;
            sViewSize[1]=(_pageSize[1]+3)/4;
            sViewPos[0]=_pagePosition[0];
            sViewPos[1]=_pagePosition[1];
        }
        if (subViewIndex==2)
        {
            sViewSize[0]=(_pageSize[0]+3)/4;
            sViewSize[1]=(_pageSize[1]+3)/4;
            sViewPos[0]=_pagePosition[0]+_pageSize[0]/4;
            sViewPos[1]=_pagePosition[1];
        }
        if (subViewIndex==3)
        {
            sViewSize[0]=(_pageSize[0]+3)/4;
            sViewSize[1]=(_pageSize[1]+3)/4;
            sViewPos[0]=_pagePosition[0]+2*_pageSize[0]/4;
            sViewPos[1]=_pagePosition[1];
        }
        if (subViewIndex==4)
        {
            sViewSize[0]=(_pageSize[0]+3)/4;
            sViewSize[1]=(_pageSize[1]+3)/4;
            sViewPos[0]=_pagePosition[0]+3*_pageSize[0]/4;
            sViewPos[1]=_pagePosition[1];
        }
    }
    if (_pageType==VERTICAL_1_PLUS_4_VIEWS)
    {
        if (subViewIndex==0)
        {
            sViewSize[0]=3*(_pageSize[0]+3)/4;
            sViewSize[1]=_pageSize[1];
            sViewPos[0]=_pagePosition[0];
            sViewPos[1]=_pagePosition[1];
        }
        if (subViewIndex==1)
        {
            sViewSize[0]=(_pageSize[0]+3)/4;
            sViewSize[1]=(_pageSize[1]+3)/4;
            sViewPos[0]=_pagePosition[0]+3*_pageSize[0]/4;
            sViewPos[1]=_pagePosition[1]+3*_pageSize[1]/4;
        }
        if (subViewIndex==2)
        {
            sViewSize[0]=(_pageSize[0]+3)/4;
            sViewSize[1]=(_pageSize[1]+3)/4;
            sViewPos[0]=_pagePosition[0]+3*_pageSize[0]/4;
            sViewPos[1]=_pagePosition[1]+2*_pageSize[1]/4;
        }
        if (subViewIndex==3)
        {
            sViewSize[0]=(_pageSize[0]+3)/4;
            sViewSize[1]=(_pageSize[1]+3)/4;
            sViewPos[0]=_pagePosition[0]+3*_pageSize[0]/4;
            sViewPos[1]=_pagePosition[1]+_pageSize[1]/4;
        }
        if (subViewIndex==4)
        {
            sViewSize[0]=(_pageSize[0]+3)/4;
            sViewSize[1]=(_pageSize[1]+3)/4;
            sViewPos[0]=_pagePosition[0]+3*_pageSize[0]/4;
            sViewPos[1]=_pagePosition[1];
        }
    }
    if (_pageType==SIX_VIEWS)
    {
        if (subViewIndex==0)
        {
            sViewSize[0]=2*(_pageSize[0]+2)/3;
            sViewSize[1]=2*(_pageSize[1]+2)/3;
            sViewPos[0]=_pagePosition[0];
            sViewPos[1]=_pagePosition[1]+_pageSize[1]/3;
        }
        else
        {
            sViewSize[0]=(_pageSize[0]+2)/3;
            sViewSize[1]=(_pageSize[1]+2)/3;
            if ( (subViewIndex==1)||(subViewIndex==2)||(subViewIndex==5) )
                sViewPos[0]=_pagePosition[0]+2*_pageSize[0]/3;
            else if (subViewIndex==3)
                sViewPos[0]=_pagePosition[0];
            else
                sViewPos[0]=_pagePosition[0]+_pageSize[0]/3;
            if ( (subViewIndex==3)||(subViewIndex==4)||(subViewIndex==5) )
                sViewPos[1]=_pagePosition[1];
            else if (subViewIndex==2)
                sViewPos[1]=_pagePosition[1]+_pageSize[1]/3;
            else
                sViewPos[1]=_pagePosition[1]+2*_pageSize[1]/3;
        }
    }
    if (_pageType==EIGHT_VIEWS)
    {
        if (subViewIndex==0)
        {
            sViewSize[0]=3*(_pageSize[0]+3)/4;
            sViewSize[1]=3*(_pageSize[1]+3)/4;
            sViewPos[0]=_pagePosition[0];
            sViewPos[1]=_pagePosition[1]+_pageSize[1]/4;
        }
        else
        {
            sViewSize[0]=(_pageSize[0]+3)/4;
            sViewSize[1]=(_pageSize[1]+3)/4;
            if ( (subViewIndex==1)||(subViewIndex==2)||(subViewIndex==3)||(subViewIndex==7) )
                sViewPos[0]=_pagePosition[0]+3*_pageSize[0]/4;
            else if (subViewIndex==4)
                sViewPos[0]=_pagePosition[0];
            else if (subViewIndex==5)
                sViewPos[0]=_pagePosition[0]+_pageSize[0]/4;
            else
                sViewPos[0]=_pagePosition[0]+2*_pageSize[0]/4;
            if ( (subViewIndex==4)||(subViewIndex==5)||(subViewIndex==6)||(subViewIndex==7) )
                sViewPos[1]=_pagePosition[1];
            else if (subViewIndex==3)
                sViewPos[1]=_pagePosition[1]+_pageSize[1]/4;
            else if (subViewIndex==2)
                sViewPos[1]=_pagePosition[1]+2*_pageSize[1]/4;
            else
                sViewPos[1]=_pagePosition[1]+3*_pageSize[1]/4;
        }
    }
    if (subViewIndex>=getRegularViewCount())
    { // We have a floating window here:
        // Positions and sizes are relative now (2009/05/22)
        int fvs[2]={int(_allViewAuxSizesAndPos[4*subViewIndex+0]*float(_pageSize[0])),int(_allViewAuxSizesAndPos[4*subViewIndex+1]*float(_pageSize[1]))};
        int fvp[2]={int(_allViewAuxSizesAndPos[4*subViewIndex+2]*float(_pageSize[0])),int(_allViewAuxSizesAndPos[4*subViewIndex+3]*float(_pageSize[1]))};
        // Make sure we are not smaller than the minimum size (2009/05/22):
        if (fvs[0]<FLOATING_VIEW_MIN_SIZE)
            fvs[0]=FLOATING_VIEW_MIN_SIZE;
        if (fvs[1]<FLOATING_VIEW_MIN_SIZE)
            fvs[1]=FLOATING_VIEW_MIN_SIZE;
        getBorderCorrectedFloatingViewPosition(fvp[0],fvp[1],fvs[0],fvs[1],fvp[0],fvp[1]);

        sViewSize[0]=fvs[0];
        sViewSize[1]=fvs[1];
        sViewPos[0]=_pagePosition[0]+_pageSize[0]-fvp[0]-fvs[0];
        sViewPos[1]=_pagePosition[1]+_pageSize[1]-fvp[1]-fvs[1];
    }
}

void CSPage::getFloatingViewRelativeSizeAndPosition(float sViewSize[2],float sViewPos[2],int subViewIndex) const
{
    if (subViewIndex>=getRegularViewCount())
    { // We have a floating window here:
        sViewSize[0]=_allViewAuxSizesAndPos[4*subViewIndex+0];
        sViewSize[1]=_allViewAuxSizesAndPos[4*subViewIndex+1];
        sViewPos[0]=_allViewAuxSizesAndPos[4*subViewIndex+2];
        sViewPos[1]=_allViewAuxSizesAndPos[4*subViewIndex+3];
    }
}

void CSPage::setViewSizesAndPositions()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    for (int i=0;i<int(_allViews.size());i++)
    {
        int s[2];
        int p[2];
        getViewSizeAndPosition(s,p,i);
        int aa=0;
        if (i>=getRegularViewCount())
            aa=auxViewsBtSize;
        _allViews[i]->setViewSizeAndPosition(s[0],s[1]-aa,p[0],p[1]);
    }
}

CSView* CSPage::getView(int viewIndex)
{
    if ( (viewIndex<0)||(viewIndex>=int(_allViews.size())) )
        return(nullptr);
    return(_allViews[viewIndex]);
}

bool CSPage::removeFloatingView(int viewIndex)
{
    if (viewIndex>=getRegularViewCount())
    { // Only floating views can be removed!
        delete _allViews[viewIndex];
        _allViews.erase(_allViews.begin()+viewIndex);
        _allViewAuxSizesAndPos.erase(_allViewAuxSizesAndPos.begin()+4*viewIndex,_allViewAuxSizesAndPos.begin()+4*viewIndex+4);
        return(true);
    }
    return(false);
}

int CSPage::getViewIndexFromViewUniqueID(int uniqueID) const
{
    for (int i=0;i<int(_allViews.size());i++)
    {
        if (_allViews[i]->getUniqueID()==uniqueID)
            return(i);
    }
    return(-1);
}

int CSPage::getViewCount() const
{
    return(int(_allViews.size()));
}

int CSPage::getRegularViewCount() const
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if (_pageType==SINGLE_VIEW)
        return(1);
    if (_pageType==FOUR_VIEWS)
        return(4);
    if ((_pageType==SIX_VIEWS_OLD)||(_pageType==SIX_VIEWS))
        return(6);
    if (_pageType==EIGHT_VIEWS)
        return(8);
    if ( (_pageType==HORIZONTALLY_DIVIDED)||(_pageType==VERTICALLY_DIVIDED) )
        return(2);
    if ( (_pageType==HORIZONTALLY_DIVIDED_3)||(_pageType==VERTICALLY_DIVIDED_3) )
        return(3);
    if ( (_pageType==HORIZONTAL_1_PLUS_3_VIEWS)||(_pageType==VERTICAL_1_PLUS_3_VIEWS) )
        return(4);
    if ( (_pageType==HORIZONTAL_1_PLUS_4_VIEWS)||(_pageType==VERTICAL_1_PLUS_4_VIEWS) )
        return(5);
    return(0);
}

void CSPage::addFloatingView(CSView* theFloatingView,float relSize[2],float relPos[2])
{
    _allViews.push_back(theFloatingView);
    _allViewAuxSizesAndPos.push_back(relSize[0]);
    _allViewAuxSizesAndPos.push_back(relSize[1]);
    _allViewAuxSizesAndPos.push_back(relPos[0]);
    _allViewAuxSizesAndPos.push_back(relPos[1]);
    setViewSizesAndPositions();
}

void CSPage::addFloatingView()
{
    CSView* theSubView=new CSView(-1);
    _allViews.push_back(theSubView);
    _allViewAuxSizesAndPos.push_back(0.25f);
    _allViewAuxSizesAndPos.push_back(0.25f);
    _allViewAuxSizesAndPos.push_back(0.05f+floatingViewAddOffset);
    _allViewAuxSizesAndPos.push_back(0.05f+floatingViewAddOffset);
    setViewSizesAndPositions();
    floatingViewAddOffset+=0.04f;
}

void CSPage::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {
            ar.storeDataName("Vw2"); // has to be the first element to be stored
            ar << _pageType;
            ar.flush();

            int floatViewsNotToSaveCnt=0;
            for (int i=getRegularViewCount();i<int(_allViews.size());i++)
            {
                if (_allViews[i]->getDoNotSaveFloatingView())
                    floatViewsNotToSaveCnt++;
            }
            int totViewsToSaveCnt=int(_allViews.size())-floatViewsNotToSaveCnt;

            for (int i=0;i<int(_allViews.size());i++)
            {
                if ( (i<getRegularViewCount())||(!_allViews[i]->getDoNotSaveFloatingView()) )
                {
                    ar.storeDataName("Sv2");
                    ar.setCountingMode();
                    _allViews[i]->serialize(ar);
                    if (ar.setWritingMode())
                        _allViews[i]->serialize(ar);
                }
            }
            // Positions and sizes are relative now (2009/05/22)
            ar.storeDataName("Fvr");
            ar << totViewsToSaveCnt*4;
            for (int i=0;i<int(_allViews.size());i++)
            {
                if ( (i<getRegularViewCount())||(!_allViews[i]->getDoNotSaveFloatingView()) )
                {
                    ar << _allViewAuxSizesAndPos[4*i+0];
                    ar << _allViewAuxSizesAndPos[4*i+1];
                    ar << _allViewAuxSizesAndPos[4*i+2];
                    ar << _allViewAuxSizesAndPos[4*i+3];
                }
            }
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {
            for (int i=0;i<int(_allViews.size());i++)
                delete _allViews[i];
            _allViews.clear();
            _allViewAuxSizesAndPos.clear();
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Vwt")==0)
                    { // keep for backward compatibility (25/09/2013)
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _pageType;
                    }
                    if (theName.compare("Vw2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _pageType;
                    }
                    if (theName.compare("Sv2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        CSView* theSubView=new CSView(-1);
                        theSubView->serialize(ar);
                        _allViews.push_back(theSubView);
                    }
                    if (theName.compare("Fvr")==0)
                    { // Positions and sizes are relative now (2009/05/22)
                        noHit=false;
                        ar >> byteQuantity;
                        int n;
                        ar >> n;
                        for (int i=0;i<n;i++)
                        {
                            float dummy;
                            ar >> dummy;
                            _allViewAuxSizesAndPos.push_back(dummy);
                        }
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            // Following is to correct for a bug where deleted views would still have their sizes and pos stored (19/7/2011):
            std::vector<float> copy(_allViewAuxSizesAndPos);
            _allViewAuxSizesAndPos.assign(copy.begin(),copy.begin()+4*_allViews.size());
        }
    }
}

void CSPage::render()
{
    FUNCTION_DEBUG;
    displayPage(this,auxViewResizingAction,viewIndexOfResizingAction);
}

#ifdef SIM_WITH_GUI
bool CSPage::viewIsPassive(int viewIndex) const
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if (_pageType==SINGLE_VIEW)
        return(viewIndex>0);
    if (_pageType==FOUR_VIEWS)
        return(viewIndex>3);
    if ((_pageType==SIX_VIEWS_OLD)||(_pageType==SIX_VIEWS))
        return(viewIndex>5);
    if (_pageType==EIGHT_VIEWS)
        return(viewIndex>7);
    if (_pageType==HORIZONTALLY_DIVIDED)
        return(viewIndex>1);
    if (_pageType==VERTICALLY_DIVIDED)
        return(viewIndex>1);
    if (_pageType==HORIZONTALLY_DIVIDED_3)
        return(viewIndex>2);
    if (_pageType==VERTICALLY_DIVIDED_3)
        return(viewIndex>2);
    if (_pageType==HORIZONTAL_1_PLUS_3_VIEWS)
        return(viewIndex>3);
    if (_pageType==VERTICAL_1_PLUS_3_VIEWS)
        return(viewIndex>3);
    if (_pageType==HORIZONTAL_1_PLUS_4_VIEWS)
        return(viewIndex>4);
    if (_pageType==VERTICAL_1_PLUS_4_VIEWS)
        return(viewIndex>4);
    return(true);
}

void CSPage::clearAllMouseJustWentDownAndUpFlags()
{
    for (int i=0;i<int(_allViews.size());i++)
        _allViews[i]->clearMouseJustWentDownAndUpFlag();
}

int CSPage::getViewIndexOfMousePosition(int mouseX,int mouseY) const
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    for (int i=0;i<int(_allViews.size());i++)
    { // We have to go in reverse order because of the auxiliary window case!
        int inv=(int)_allViews.size()-i-1;
        int subViewSize[2];
        _allViews[inv]->getViewSize(subViewSize);
        int relX;
        int relY;
        getViewRelativeMousePosition(mouseX,mouseY,relX,relY,inv);
        if ( (relX>=0)&&(relX<=subViewSize[0])&&(relY>=0)&&(relY<=subViewSize[1]) )
            return(inv);
    }
    return(-1);
}

bool CSPage::doubleClickActionForView(int viewIndex)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    // return value false means no action was performed
    if ((_pageType>=SINGLE_VIEW)&&(_pageType<CLOSING_VIEW_ITEM))
    {
        // We can swap subviews with the main view:
        if ( (viewIndex>=getRegularViewCount())&&(viewIndex<int(_allViews.size())) )
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SWAP_VIEWS_CMD;
            cmd.intParams.push_back(App::ct->pageContainer->getActivePageIndex());
            cmd.intParams.push_back(viewIndex);
            cmd.intParams.push_back(0);
            cmd.boolParams.push_back(false);
            App::appendSimulationThreadCommand(cmd);
            return(true);
        }
        // We can now also swap some of the regular views with the main view (since 1/11/2014):
        if ( (viewIndex<getRegularViewCount())&&(viewIndex>0) )
        {
            if ( (_pageType==SIX_VIEWS_OLD)||(_pageType==SIX_VIEWS)||(_pageType==EIGHT_VIEWS)||(_pageType==HORIZONTALLY_DIVIDED_3)||(_pageType==VERTICALLY_DIVIDED_3)||(_pageType==HORIZONTAL_1_PLUS_3_VIEWS)||(_pageType==VERTICAL_1_PLUS_3_VIEWS)||(_pageType==HORIZONTAL_1_PLUS_4_VIEWS)||(_pageType==VERTICAL_1_PLUS_4_VIEWS) )
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=SWAP_VIEWS_CMD;
                cmd.intParams.push_back(App::ct->pageContainer->getActivePageIndex());
                cmd.intParams.push_back(viewIndex);
                cmd.intParams.push_back(0);
                cmd.boolParams.push_back(false);
                App::appendSimulationThreadCommand(cmd);
                return(true);
            }
        }
    }
    return(false);
}

void CSPage::swapViews(int index1,int index2,bool alsoSizeAndPosInfo)
{
    CSView* it=_allViews[index1];
    if (it->getCanSwapViewWithMainView()||((index1!=0)&&(index2!=0)) )
    {
        _allViews[index1]=_allViews[index2];
        _allViews[index2]=it;
        if (alsoSizeAndPosInfo)
        {
            for (int i=0;i<4;i++)
            {
                // Positions and sizes are relative now (2009/05/22)
                float v=_allViewAuxSizesAndPos[4*index1+i];
                _allViewAuxSizesAndPos[4*index1+i]=_allViewAuxSizesAndPos[4*index2+i];
                _allViewAuxSizesAndPos[4*index2+i]=v;
            }
        }
        setViewSizesAndPositions();
    }
}

int CSPage::bringViewToFrontIfPossible(int index)
{
    if (index<getRegularViewCount())
        return(index);
    // We do a kind of stupid step-by-step shifting:
    while (index<int(_allViews.size()-1))
    {
        swapViews(index,index+1,true);
        index++;
    }
    return(index);
}

int CSPage::getCaughtElements() const
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    int retVal=0;
    for (int i=0;i<int(_allViews.size());i++)
        retVal|=_allViews[i]->getCaughtElements();
    retVal|=_caughtElements;
    return(retVal);
}

void CSPage::clearCaughtElements(int keepMask)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    for (int i=0;i<int(_allViews.size());i++)
        _allViews[i]->clearCaughtElements(keepMask);
    _caughtElements&=keepMask;
}


void CSPage::clearLastMouseDownViewIndex()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    _lastMouseDownViewIndex=-1;
}

int CSPage::getLastMouseDownViewIndex() const
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    return(_lastMouseDownViewIndex);
}

int CSPage::getMousePosRelativeToFloatingViewBorders(int mouseX,int mouseY,int index) const
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    int tol=10;
    int relX;
    int relY;
    getViewRelativeMousePosition(mouseX,mouseY,relX,relY,index);
    int ss[2];
    int sp[2];
    getViewSizeAndPosition(ss,sp,index);
    int verticalBorder=-1;// -1 = outside, 0= left/bottom border, 1= middle, 2=right/top border
    int horizontalBorder=-1;
    if ( (relX>=ss[0]-auxViewsBtSize*App::sc)&&(relX<=ss[0]) )
    {
        if ( (relY<=ss[1])&&(relY>=ss[1]-auxViewsBtSize*App::sc) )
        {
            if (_allViews[index]->getCanBeClosed())
                return(AUX_VIEW_CLOSING_BUTTON);
            return(AUX_VIEW_SHIFTING);
        }
    }
    if ( (relX>=tol)&&(relX<=ss[0]-tol) )
        verticalBorder=1;
    if ( (relX>-tol)&&(relX<tol) )
        verticalBorder=0;
    if ( (relX>ss[0]-tol)&&(relX<ss[0]+tol) )
        verticalBorder=2;
    if ( (relY>=tol)&&(relY<=ss[1]-tol) )
        horizontalBorder=1;
    if ( (relY>-tol)&&(relY<tol) )
        horizontalBorder=0;
    if ( (relY>ss[1]-tol)&&(relY<ss[1]))
        horizontalBorder=2;
    if ( (verticalBorder==-1)||(horizontalBorder==-1) )
        return(-1);
    if ( (verticalBorder==1)&&(horizontalBorder==1) )
        return(AUX_VIEW_SHIFTING);
    if (verticalBorder==0)
    {
        if (horizontalBorder==0)
            return(AUX_VIEW_BOTTOM_LEFT_BORDER);
        if (horizontalBorder==1)
            return(AUX_VIEW_LEFT_BORDER);
        return(AUX_VIEW_SHIFTING);
    }
    if (verticalBorder==2)
    {
        if (horizontalBorder==0)
            return(AUX_VIEW_BOTTOM_RIGHT_BORDER);
        if (horizontalBorder==1)
            return(AUX_VIEW_RIGHT_BORDER);
        return(AUX_VIEW_SHIFTING);
    }
    if (horizontalBorder==0)
        return(AUX_VIEW_BOTTOM_BORDER);
    return(AUX_VIEW_SHIFTING);
}

void CSPage::getViewRelativeMousePosition(int mouseX,int mouseY,int& relMouseX,int& relMouseY,int index) const
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    int subViewSize[2];
    int subViewPos[2];
    getViewSizeAndPosition(subViewSize,subViewPos,index);
    relMouseX=_pagePosition[0]+mouseX-subViewPos[0];
    relMouseY=_pagePosition[1]+mouseY-subViewPos[1];
}

bool CSPage::getMouseRelPosObjectAndViewSize(int x,int y,int relPos[2],int& objType,int& objID,int vSize[2],bool& viewIsPerspective) const
{ // NOT FULLY IMPLEMENTED! objType=-1 --> not supported, 0 --> hierarchy, 1 --> 3DViewable
    if ( (x<0)||(x>_pageSize[0])||(y<0)||(y>_pageSize[1]) )
        return(false);
    // The position is in this view
    int relX,relY;
    for (int i=int(_allViews.size())-1;i>=0;i--)
    {
        getViewRelativeMousePosition(x,y,relX,relY,i);
        if (_allViews[i]->getMouseRelPosObjectAndViewSize(relX,relY,relPos,objType,objID,vSize,viewIsPerspective))
            return(true); // the position is in that view
    }
    return(false);
}

bool CSPage::leftMouseButtonDown(int x,int y,int selectionStatus)
{
    if ( (x<0)||(x>_pageSize[0])||(y<0)||(y>_pageSize[1]) )
        return(false);
    // The mouse went down in this view
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    previousMouseRelativePosition[0]=x;
    previousMouseRelativePosition[1]=y;
    for (int i=0;i<int(_allViews.size());i++)
        _allViews[i]->clearCaughtElements(0xffff-sim_left_button);
    _caughtElements&=0xffff-sim_left_button;

    // We first check for a floating view resizing action:
    for (int i=int(_allViews.size())-1;i>=getRegularViewCount();i--)
    {
        auxViewResizingAction=getMousePosRelativeToFloatingViewBorders(mouseRelativePosition[0],mouseRelativePosition[1],i);
        if (auxViewResizingAction!=-1)
        {
            viewIndexOfResizingAction=bringViewToFrontIfPossible(i);
            _lastMouseDownViewIndex=viewIndexOfResizingAction; // foc
            _caughtElements|=sim_left_button;
            return(true);
        }
    }

    int relX,relY;
    for (int i=0;i<int(_allViews.size());i++)
    {
        if (!viewIsPassive(i))
        { // We send that event only to active windows!
            getViewRelativeMousePosition(mouseRelativePosition[0],mouseRelativePosition[1],relX,relY,i);
            if (_allViews[i]->leftMouseButtonDown(relX,relY,selectionStatus))
            {
                _lastMouseDownViewIndex=i; // foc
                return(true);
            }
        }
    }
    return(false); // Not yet processed
}

void CSPage::leftMouseButtonUp(int x,int y)
{
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    if (_caughtElements&sim_left_button)
    {
        if (auxViewResizingAction==AUX_VIEW_CLOSING_BUTTON)
        { // We have to destroy this floating view!
            SSimulationThreadCommand cmd;
            cmd.cmdId=CLOSE_FLOATING_VIEW_CMD;
            cmd.intParams.push_back(viewIndexOfResizingAction);
            App::appendSimulationThreadCommand(cmd);
        }
        auxViewResizingAction=-1;
    }
    for (size_t i=0;i<_allViews.size();i++)
    {
        if (_allViews[i]->getCaughtElements()&sim_left_button)
        { // We only process it for views that caught the down action
            int relX;
            int relY;
            getViewRelativeMousePosition(mouseRelativePosition[0],mouseRelativePosition[1],relX,relY,(int)i);
            _allViews[i]->leftMouseButtonUp(relX,relY);
        }
    }
}

int CSPage::getCursor(int x,int y) const
{
    if ( (x<0)||(x>_pageSize[0])||(y<0)||(y>_pageSize[1]) )
        return(-1);
    // We first check for a floating view resizing action:
    for (int i=int(_allViews.size())-1;i>=getRegularViewCount();i--)
    {
        int act=getMousePosRelativeToFloatingViewBorders(mouseRelativePosition[0],mouseRelativePosition[1],i);
        if (act!=-1)
        {
            int retVal=-1;
            if (_allViews[i]->getCanBeResized())
            {
                if ( (act==AUX_VIEW_TOP_BORDER)||(act==AUX_VIEW_BOTTOM_BORDER) )
                    retVal=sim_cursor_vertical_directions;
                if ( (act==AUX_VIEW_RIGHT_BORDER)||(act==AUX_VIEW_LEFT_BORDER) )
                    retVal=sim_cursor_horizontal_directions;
                if ( (act==AUX_VIEW_TOP_RIGHT_BORDER)||(act==AUX_VIEW_BOTTOM_LEFT_BORDER) )
                    retVal=sim_cursor_slash_directions;
                if ( (act==AUX_VIEW_BOTTOM_RIGHT_BORDER)||(act==AUX_VIEW_TOP_LEFT_BORDER) )
                    retVal=sim_cursor_backslash_directions;
            }
            if (_allViews[i]->getCanBeShifted())
            {
                if (act==AUX_VIEW_SHIFTING)
                    retVal=sim_cursor_all_directions;
            }
            return(retVal);
        }
    }
    // Now check regular views:
    for (int i=0;i<getRegularViewCount();i++)
    {
        int relX;
        int relY;
        getViewRelativeMousePosition(x,y,relX,relY,i);
        int retVal=_allViews[i]->getCursor(relX,relY);
        if (retVal!=-1)
            return(retVal);
    }
    return(-1);
}

void CSPage::mouseWheel(int deltaZ,int x,int y)
{
    // We send this event to all subviews
    for (int i=int(_allViews.size())-1;i>=0;i--)
    {
        int relX;
        int relY;
        getViewRelativeMousePosition(x,y,relX,relY,i);
        if (_allViews[i]->mouseWheel(deltaZ,relX,relY))
        {
            _lastMouseDownViewIndex=i; // foc
            return;
        }
    }
}

void CSPage::mouseMove(int x,int y,bool passiveAndFocused)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;

    // We check if there is a resizing action:
    if (_caughtElements&sim_left_button)
    {
        if ((auxViewResizingAction!=-1)&&(viewIndexOfResizingAction<int(_allViewAuxSizesAndPos.size())/4))
        {

            int dx=mouseRelativePosition[0]-previousMouseRelativePosition[0];
            int dy=mouseRelativePosition[1]-previousMouseRelativePosition[1];

            int vs[2];
            int vp[2];
            // Positions and sizes are relative now (2009/05/22)
            vs[0]=int(_allViewAuxSizesAndPos[4*viewIndexOfResizingAction+0]*float(_pageSize[0])+0.5f);
            vs[1]=int(_allViewAuxSizesAndPos[4*viewIndexOfResizingAction+1]*float(_pageSize[1])+0.5f);
            vp[0]=int(_allViewAuxSizesAndPos[4*viewIndexOfResizingAction+2]*float(_pageSize[0])+0.5f);
            vp[1]=int(_allViewAuxSizesAndPos[4*viewIndexOfResizingAction+3]*float(_pageSize[1])+0.5f);
            // Make sure we are not smaller than the minimum size (2009/05/22):
            if (vs[0]<FLOATING_VIEW_MIN_SIZE)
                vs[0]=FLOATING_VIEW_MIN_SIZE;
            if (vs[1]<FLOATING_VIEW_MIN_SIZE)
                vs[1]=FLOATING_VIEW_MIN_SIZE;

            getBorderCorrectedFloatingViewPosition(vp[0],vp[1],vs[0],vs[1],vp[0],vp[1]);

            int px=_pageSize[0]-vp[0]-vs[0];
            int py=_pageSize[1]-vp[1]-vs[1];

            if (auxViewResizingAction==AUX_VIEW_SHIFTING)
            {
                vp[0]+=-dx;
                vp[1]+=-dy;
            }
            if ( (auxViewResizingAction==AUX_VIEW_LEFT_BORDER)||
                    (auxViewResizingAction==AUX_VIEW_BOTTOM_LEFT_BORDER)||
                    (auxViewResizingAction==AUX_VIEW_TOP_LEFT_BORDER) )
            {
                int val=_pageSize[0]-vp[0]-x;
                if (val<FLOATING_VIEW_MIN_SIZE)
                    val=FLOATING_VIEW_MIN_SIZE;
                vs[0]=val;
            }
            if ( (auxViewResizingAction==AUX_VIEW_BOTTOM_BORDER)||
                    (auxViewResizingAction==AUX_VIEW_BOTTOM_LEFT_BORDER)||
                    (auxViewResizingAction==AUX_VIEW_BOTTOM_RIGHT_BORDER) )
            {
                int val=_pageSize[1]-vp[1]-y;
                if (val<FLOATING_VIEW_MIN_SIZE)
                    val=FLOATING_VIEW_MIN_SIZE;
                vs[1]=val;
            }
            if ( (auxViewResizingAction==AUX_VIEW_RIGHT_BORDER)||
                    (auxViewResizingAction==AUX_VIEW_BOTTOM_RIGHT_BORDER)||
                    (auxViewResizingAction==AUX_VIEW_TOP_RIGHT_BORDER) )
            {
                int val=x-px;
                if (val<FLOATING_VIEW_MIN_SIZE)
                    val=FLOATING_VIEW_MIN_SIZE;
                vp[0]+=vs[0]-val;
                vs[0]=val;
            }
            if ( (auxViewResizingAction==AUX_VIEW_TOP_BORDER)||
                    (auxViewResizingAction==AUX_VIEW_TOP_LEFT_BORDER)||
                    (auxViewResizingAction==AUX_VIEW_TOP_RIGHT_BORDER) )
            {
                int val=y-py;
                if (val<FLOATING_VIEW_MIN_SIZE)
                    val=FLOATING_VIEW_MIN_SIZE;
                vp[1]+=vs[1]-val;
                vs[1]=val;
            }

            if (auxViewResizingAction!=AUX_VIEW_SHIFTING)
            { // Avoid resizing over the borders:
                if (vp[0]<0)
                {
                    vs[0]+=vp[0];
                    vp[0]=0;
                }
                if (vp[1]<0)
                {
                    vs[1]+=vp[1];
                    vp[1]=0;
                }
                if (vp[0]+vs[0]>_pageSize[0])
                    vs[0]-=vp[0]+vs[0]-_pageSize[0];
                if (vp[1]+vs[1]>_pageSize[1])
                    vs[1]-=vp[1]+vs[1]-_pageSize[1];
            }


            // Make sure we are not smaller than the minimum size (2009/05/22):
            if (vs[0]<FLOATING_VIEW_MIN_SIZE)
                vs[0]=FLOATING_VIEW_MIN_SIZE;
            if (vs[1]<FLOATING_VIEW_MIN_SIZE)
                vs[1]=FLOATING_VIEW_MIN_SIZE;

            getBorderCorrectedFloatingViewPosition(vp[0],vp[1],vs[0],vs[1],vp[0],vp[1]);

            if ( ( (auxViewResizingAction==AUX_VIEW_SHIFTING)&&(_allViews[viewIndexOfResizingAction]->getCanBeShifted()) ) ||
                ( (auxViewResizingAction!=AUX_VIEW_SHIFTING)&&(auxViewResizingAction!=AUX_VIEW_CLOSING_BUTTON)&&(_allViews[viewIndexOfResizingAction]->getCanBeResized()) ) )
            {
                if ((auxViewResizingAction!=AUX_VIEW_SHIFTING)&&(auxViewResizingAction!=AUX_VIEW_CLOSING_BUTTON))
                {
                    _allViewAuxSizesAndPos[4*viewIndexOfResizingAction+0]=float(vs[0])/float(_pageSize[0]);
                    _allViewAuxSizesAndPos[4*viewIndexOfResizingAction+1]=float(vs[1])/float(_pageSize[1]);
                }
                _allViewAuxSizesAndPos[4*viewIndexOfResizingAction+2]=float(vp[0])/float(_pageSize[0]);
                _allViewAuxSizesAndPos[4*viewIndexOfResizingAction+3]=float(vp[1])/float(_pageSize[1]);
            }

            previousMouseRelativePosition[0]=x;
            previousMouseRelativePosition[1]=y;
            setViewSizesAndPositions(); // needed to reflect the change

            if (auxViewResizingAction==AUX_VIEW_CLOSING_BUTTON)
            { // Check if we are still on the button:
                auxViewResizingAction=getMousePosRelativeToFloatingViewBorders(mouseRelativePosition[0],mouseRelativePosition[1],viewIndexOfResizingAction);
                if (auxViewResizingAction!=AUX_VIEW_CLOSING_BUTTON)
                    auxViewResizingAction=-1; // We are outside of the button now
            }
        }
    }

    int bts=sim_right_button|sim_middle_button|sim_left_button;
    if (App::userSettings->navigationBackwardCompatibility)
        bts=sim_right_button|sim_left_button;


    for (int i=0;i<int(_allViews.size());i++)
    {
        int relX,relY;
        if (!passiveAndFocused)
        {
            if (_allViews[i]->getCaughtElements()&bts)
            {
                getViewRelativeMousePosition(mouseRelativePosition[0],mouseRelativePosition[1],relX,relY,i);
                _allViews[i]->mouseMove(relX,relY,passiveAndFocused);
            }
        }
        else
        {
            getViewRelativeMousePosition(mouseRelativePosition[0],mouseRelativePosition[1],relX,relY,i);
            _allViews[i]->mouseMove(relX,relY,passiveAndFocused);
        }
    }
}

int CSPage::modelDragMoveEvent(int x,int y,C3Vector* desiredModelPosition)
{
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;

    for (int i=int(_allViews.size())-1;i>=0;i--)
    {
        int relX,relY;
        getViewRelativeMousePosition(mouseRelativePosition[0],mouseRelativePosition[1],relX,relY,i);
        int v=_allViews[i]->modelDragMoveEvent(relX,relY,desiredModelPosition);
        if (v>=0)
            return(v);
    }
    return(0);
}

bool CSPage::rightMouseButtonDown(int x,int y)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if ( (x<0)||(y<0)||(x>_pageSize[0])||(y>_pageSize[1]) )
        return(false);
    previousMouseRelativePosition[0]=x;
    previousMouseRelativePosition[1]=y;
    // The mouse went down in this window zone
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    for (int i=0;i<int(_allViews.size());i++)
        _allViews[i]->clearCaughtElements(0xffff-sim_right_button);
    _caughtElements&=0xffff-sim_right_button;

    // Which sub-view is concerned?
    for (int i=0;i<int(_allViews.size());i++)
    { // We have to go through it in reverse order because of the auxiliary view!
        int inv=(int)_allViews.size()-i-1;
        int relX,relY;
        getViewRelativeMousePosition(mouseRelativePosition[0],mouseRelativePosition[1],relX,relY,inv);
        if (_allViews[inv]->rightMouseButtonDown(relX,relY,viewIsPassive(inv)))
        {
            _lastMouseDownViewIndex=inv; // foc
            return(true);
        }
    }
    return(false); // Nothing caught that action
}
bool CSPage::rightMouseButtonUp(int x,int y,int absX,int absY,QWidget* mainWindow)
{ // return value true means we have to remove current view (command "Remove views")
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;

    for (int i=0;i<int(_allViews.size());i++)
    {
        if (_allViews[i]->getCaughtElements()&sim_right_button)
        { // We only process it for views that caught the down action
            int relX,relY;
            getViewRelativeMousePosition(mouseRelativePosition[0],mouseRelativePosition[1],relX,relY,i);
            if (_allViews[i]->rightMouseButtonUp(relX,relY,absX,absY,mainWindow,i))
                return(true);
        }
    }
    return(false);
}

bool CSPage::middleMouseButtonDown(int x,int y)
{
    if ( (x<0)||(y<0)||(x>_pageSize[0])||(y>_pageSize[1]) )
        return(false);
    previousMouseRelativePosition[0]=x;
    previousMouseRelativePosition[1]=y;
    // The mouse went down in this window zone
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    for (int i=0;i<int(_allViews.size());i++)
        _allViews[i]->clearCaughtElements(0xffff-sim_middle_button);
    _caughtElements&=0xffff-sim_middle_button;

    // Which sub-view is concerned?
    for (int i=0;i<int(_allViews.size());i++)
    { // We have to go through it in reverse order because of the auxiliary view!
        int inv=(int)_allViews.size()-i-1;
        int relX,relY;
        getViewRelativeMousePosition(mouseRelativePosition[0],mouseRelativePosition[1],relX,relY,inv);
        if (_allViews[inv]->middleMouseButtonDown(relX,relY,viewIsPassive(inv)))
        {
            _lastMouseDownViewIndex=inv; // foc
            return(true);
        }
    }
    return(false); // Nothing caught that action
}

void CSPage::middleMouseButtonUp(int x,int y)
{
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;

    for (int i=0;i<int(_allViews.size());i++)
    {
        if (_allViews[i]->getCaughtElements()&sim_middle_button)
        { // We only process it for views that caught the down action
            int relX,relY;
            getViewRelativeMousePosition(mouseRelativePosition[0],mouseRelativePosition[1],relX,relY,i);
            _allViews[i]->middleMouseButtonUp(relX,relY);
        }
    }
}


bool CSPage::leftMouseButtonDoubleClick(int x,int y,int selectionStatus)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    int index=getViewIndexOfMousePosition(mouseRelativePosition[0],mouseRelativePosition[1]);
    if (index!=-1)
    {
        for (int i=0;i<int(_allViews.size());i++)
        { // The mouse is actually down during a double-click.. that causes problems
            int relX;
            int relY;
            getViewRelativeMousePosition(mouseRelativePosition[0],mouseRelativePosition[1],relX,relY,i);
            _allViews[i]->leftMouseButtonUp(relX,relY);
        }
        return(doubleClickActionForView(index));
    }
    return(false); // Not processed
}
#endif
