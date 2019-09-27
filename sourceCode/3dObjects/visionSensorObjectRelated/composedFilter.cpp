#include "vrepMainHeader.h"
#include "composedFilter.h"
#include "visionSensor.h"
#include "v_repConst.h"

CComposedFilter::CComposedFilter()
{
    buffer1=nullptr;
    buffer2=nullptr;
    workImage=nullptr;
}

CComposedFilter::~CComposedFilter()
{
    delete[] buffer1;
    delete[] buffer2;
    delete[] workImage;
    removeAllSimpleFilters();
}

int CComposedFilter::getSimpleFilterCount()
{
    return((int)_allSimpleFilters.size());
}
void CComposedFilter::insertSimpleFilter(CSimpleFilter* it)
{
    _allSimpleFilters.push_back(it);
}

CSimpleFilter* CComposedFilter::getSimpleFilter(int index)
{
    if ((index<0)||(index>=int(_allSimpleFilters.size())))
        return(nullptr);
    return(_allSimpleFilters[index]);
}

CSimpleFilter* CComposedFilter::getSimpleFilterFromUniqueID(int uniqueID)
{
    for (int i=0;i<int(_allSimpleFilters.size());i++)
    {
        if (_allSimpleFilters[i]->getUniqueID()==uniqueID)
            return(_allSimpleFilters[i]);
    }
    return(nullptr);
}

void CComposedFilter::removeSimpleFilter(int index)
{
    if ((index<0)||(index>=int(_allSimpleFilters.size())))
        return;
    delete _allSimpleFilters[index];
    _allSimpleFilters.erase(_allSimpleFilters.begin()+index);
}

void CComposedFilter::removeAllSimpleFilters()
{
    for (int i=0;i<int(_allSimpleFilters.size());i++)
        delete _allSimpleFilters[i];
    _allSimpleFilters.clear();
}

bool CComposedFilter::moveSimpleFilter(int index,bool up)
{
    if ((index<0)||(index>=int(_allSimpleFilters.size())))
        return(false);
    if ((index==0)&&up)
        return(false);
    if ((index==int(_allSimpleFilters.size())-1)&&(!up))
        return(false);
    if (up)
    {
        _allSimpleFilters.insert(_allSimpleFilters.begin()+index-1,_allSimpleFilters[index]);
        _allSimpleFilters.erase(_allSimpleFilters.begin()+index+1);
    }
    else
    {
        _allSimpleFilters.insert(_allSimpleFilters.begin()+index+2,_allSimpleFilters[index]);
        _allSimpleFilters.erase(_allSimpleFilters.begin()+index);
    }
    return(true);
}

CComposedFilter* CComposedFilter::copyYourself()
{
    CComposedFilter* newFilter=new CComposedFilter();
    for (int i=0;i<int(_allSimpleFilters.size());i++)
    {
        CSimpleFilter* it=_allSimpleFilters[i]->copyYourself();
        newFilter->_allSimpleFilters.push_back(it);
    }
    return(newFilter);
}

void CComposedFilter::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {
            for (int i=0;i<int(_allSimpleFilters.size());i++)
            {
                ar.storeDataName("Sfr");
                ar.setCountingMode();
                _allSimpleFilters[i]->serialize(ar);
                if (ar.setWritingMode())
                    _allSimpleFilters[i]->serialize(ar);
            }

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Sfr")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        CSimpleFilter* it=new CSimpleFilter();
                        it->serialize(ar);
                        _allSimpleFilters.push_back(it);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

void CComposedFilter::initializeInitialValues(bool simulationIsRunning)
{ // following comment was still from the 'simulationAboutToStart' routine. Maybe now we could solve the problem in another way!
    // nothing for now (the buffers cannot be reserved here... if a sensor is copied during the simulation--> problem)
    if (simulationIsRunning)
    {
    }
    else
    { // remember, simulationEnded is not guaranteed to be run (the object can be copied during a simulation and pasted after it ended!)
        removeBuffers();
    }
}

void CComposedFilter::simulationEnded()
{ // delete the buffers! Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    removeBuffers();
//  if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
//  {
//  }
}

void CComposedFilter::removeBuffers()
{
    delete[] buffer1;
    buffer1=nullptr;
    delete[] buffer2;
    buffer2=nullptr;
    delete[] workImage;
    workImage=nullptr;
}

bool CComposedFilter::includesDepthBufferModification()
{
    for (size_t i=0;i<_allSimpleFilters.size();i++)
    {
        CSimpleFilter* it=_allSimpleFilters[i];
        if ( (it->getFilterType()==sim_filtercomponent_todepthoutput)&&it->getEnabled() )
            return(true);
    }
    return(false);
}

bool CComposedFilter::processAndTrigger(CVisionSensor* sensor,int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* outputDepthBuffer,std::vector<std::vector<float> >& returnData)
{
    bool retVal=false;
    if (workImage==nullptr)
    {
        int s=sizeX*sizeY*3;
        buffer1=new float[s];
        buffer2=new float[s];
        workImage=new float[s];
        for (int i=0;i<s;i++)
        {
            outputImage[i]=0.0f;
            workImage[i]=0.0f;
            buffer1[i]=0.0f;
            buffer2[i]=0.0f;
        }
    }
    drawingContainer.removeAllObjects();
    for (size_t i=0;i<_allSimpleFilters.size();i++)
    {
        CSimpleFilter* it=_allSimpleFilters[i];
        std::vector<float> retDat;
        bool trigger=it->processAndTrigger(sensor,sizeX,sizeY,inputImage,inputDepth,outputImage,outputDepthBuffer,workImage,retDat,buffer1,buffer2,drawingContainer);
        if (retDat.size()!=0)
            returnData.push_back(retDat);
        if (trigger)
            retVal=true;
    }
    return(retVal);
}

void CComposedFilter::displayOverlay(int c0[2],int c1[2])
{
    drawingContainer.render(c0,c1);
}
