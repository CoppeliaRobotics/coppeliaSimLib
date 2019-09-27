
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "graphData.h"
#include "global.h"
#include "graph.h"
#include "tt.h"
#include "proximitySensor.h"
#include "jointObject.h"
#include "app.h"
#include "graphingRoutines.h"
#ifdef SIM_WITH_OPENGL
#include "oGL.h"
#endif

CGraphData::CGraphData()
{
    addCoeff=0.0f;
    _lifeID=-1;
    _movingAverageCount=1; // no moving average!
    _derivativeIntegralAndCumulative=DATA_STREAM_ORIGINAL;
}

CGraphData::CGraphData(int theDataType,int theDataObjectID,int theDataObjectAuxID)
{   
    dataType=theDataType;
    dataObjectID=theDataObjectID;
    dataObjectAuxID=theDataObjectAuxID;
    linkPoints=true;
    label=true;
    visible=true;
    identifier=-1;
    _floatData.clear();
    _transformedFloatData.clear();
    _floatDataValidFlags.clear();
    _transformedFloatDataValidFlags.clear();
    ambientColor[0]=1.0f;
    ambientColor[1]=ambientColor[2]=0.0f;
    zoomFactor=1.0f;
    addCoeff=0.0f;
    _derivativeIntegralAndCumulative=DATA_STREAM_ORIGINAL;
    _movingAverageCount=1; // no moving average!
    name="Data";
}

CGraphData::~CGraphData()
{
    _floatData.clear();
    _transformedFloatData.clear();
    _floatDataValidFlags.clear();
    _transformedFloatDataValidFlags.clear();
}

void CGraphData::setLabel(bool l)
{
    label=l;
}

void CGraphData::setMovingAverageCount(int c)
{
    c=tt::getLimitedInt(1,200,c);
    _movingAverageCount=c;
}

int CGraphData::getMovingAverageCount() const
{
    return(_movingAverageCount);    
}

void CGraphData::setDerivativeIntegralAndCumulative(int val)
{
    _derivativeIntegralAndCumulative=val;
    resetData((int)_floatData.size());
}

int CGraphData::getDerivativeIntegralAndCumulative() const
{
    return(_derivativeIntegralAndCumulative);
}

bool CGraphData::getLabel() const
{ 
    return(label);
}

bool CGraphData::getLinkPoints() const
{
    return(linkPoints);
}

int CGraphData::getIdentifier() const
{
    return(identifier);
}

int CGraphData::getDataType() const
{
    return(dataType);
}
int CGraphData::getDataObjectID() const
{
    return(dataObjectID);
}
int CGraphData::getDataObjectAuxID() const
{
    return(dataObjectAuxID);
}

float CGraphData::getZoomFactor() const
{
    return(zoomFactor);
}

float CGraphData::getAddCoeff() const
{
    return(addCoeff);
}

bool CGraphData::getVisible() const
{
    return(visible);
}

void CGraphData::setLinkPoints(bool l)
{
    linkPoints=l;
}
void CGraphData::setVisible(bool v)
{
    visible=v;
}

void CGraphData::setIdentifier(int newIdentifier)
{
    identifier=newIdentifier;
}
void CGraphData::setZoomFactor(float newZoomFactor)
{
    tt::limitValue(-100000000000.0f,100000000000.0f,newZoomFactor);
    zoomFactor=newZoomFactor;
}
void CGraphData::setAddCoeff(float newCoeff)
{
    tt::limitValue(-100000000000.0f,100000000000.0f,newCoeff);
    addCoeff=newCoeff;
}
void CGraphData::setDataObjectID(int newID)
{
    dataObjectID=newID;
}
void CGraphData::setDataObjectAuxID(int newID)
{
    dataObjectAuxID=newID;
}
void CGraphData::setName(std::string theName)
{
    name=theName;
}
std::string CGraphData::getName() const
{ 
    return(name); 
}

void CGraphData::performObjectLoadingMapping(std::vector<int>* map)
{
    if ( (dataType>GRAPH_3DOBJECT_START)&&(dataType<GRAPH_3DOBJECT_END) )
        dataObjectID=App::ct->objCont->getLoadingMapping(map,dataObjectID);
}
void CGraphData::performCollisionLoadingMapping(std::vector<int>* map)
{
    if ( (dataType>GRAPH_COLLISION_START)&&(dataType<GRAPH_COLLISION_END) )
        dataObjectID=App::ct->objCont->getLoadingMapping(map,dataObjectID);
}
void CGraphData::performDistanceLoadingMapping(std::vector<int>* map)
{
    if ( (dataType>GRAPH_DISTANCE_START)&&(dataType<GRAPH_DISTANCE_END) )
        dataObjectID=App::ct->objCont->getLoadingMapping(map,dataObjectID);
}
void CGraphData::performIkLoadingMapping(std::vector<int>* map)
{
    if ( (dataType>GRAPH_IK_START)&&(dataType<GRAPH_IK_END) )
        dataObjectID=App::ct->objCont->getLoadingMapping(map,dataObjectID);
}

void CGraphData::performGcsLoadingMapping(std::vector<int>* map)
{
    if ( (dataType>GRAPH_GCS_START)&&(dataType<GRAPH_GCS_END) )
        dataObjectID=App::ct->objCont->getLoadingMapping(map,dataObjectID);
}

bool CGraphData::announceObjectWillBeErased(int objID,bool copyBuffer)
{   // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(id)'-call or similar
    // Return value true means this CGraphData object needs to be destroyed!
    if ( (dataType>GRAPH_3DOBJECT_START)&&(dataType<GRAPH_3DOBJECT_END) )
    {
        if (dataObjectID==objID)
            return(true);
    }
    return(false);
}
bool CGraphData::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(id)'-call or similar
    // Return value true means this CGraphData object needs to be destroyed!
    if ( (dataType>GRAPH_COLLISION_START)&&(dataType<GRAPH_COLLISION_END) )
    {
        if (dataObjectID==collisionID)
            return(true);
    }
    return(false);
}
bool CGraphData::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(id)'-call or similar
    // Return value true means this CGraphData object needs to be destroyed!
    if ( (dataType>GRAPH_DISTANCE_START)&&(dataType<GRAPH_DISTANCE_END) )
    {
        if (dataObjectID==distanceID)
            return(true);
    }
    return(false);
}

bool CGraphData::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{
    if ( (dataType>GRAPH_IK_START)&&(dataType<GRAPH_IK_END) )
    {
        if (dataObjectID==ikGroupID)
            return(true);
    }
    return(false);
}

bool CGraphData::announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer)
{
    if ( (dataType>GRAPH_GCS_START)&&(dataType<GRAPH_GCS_END) )
    {
        if (dataObjectID==gcsObjectID)
            return(true);
    }
    return(false);
}

void CGraphData::setValueDirect(int absIndex,float theValue,bool firstValue,bool cyclic,float range,const std::vector<float>& times)
{
    _floatData[absIndex]=theValue;
    _floatDataValidFlags[absIndex/8]|=(1<<(absIndex&7)); // valid data
    if (firstValue)
    { // this is the very first point
        if (_derivativeIntegralAndCumulative==DATA_STREAM_ORIGINAL)
        {
            _transformedFloatData[absIndex]=_floatData[absIndex];
            _transformedFloatDataValidFlags[absIndex/8]|=(1<<(absIndex&7)); // valid data
        }
        if (_derivativeIntegralAndCumulative==DATA_STREAM_DERIVATIVE)
        { // invalid data
            _transformedFloatData[absIndex]=0.0f;
            _transformedFloatDataValidFlags[absIndex/8]&=255-(1<<(absIndex&7)); // invalid data
        }
        if (_derivativeIntegralAndCumulative==DATA_STREAM_INTEGRAL)
        {
            _transformedFloatData[absIndex]=0.0f;
            _transformedFloatDataValidFlags[absIndex/8]|=(1<<(absIndex&7)); // valid data
        }
        if (_derivativeIntegralAndCumulative==DATA_STREAM_CUMULATIVE)
        {
            _transformedFloatData[absIndex]=_floatData[absIndex];
            _transformedFloatDataValidFlags[absIndex/8]|=(1<<(absIndex&7)); // valid data
        }
    }
    else
    { // this is not the first point
        // We get the index of previous data:
        int prevIndex=absIndex-1;
        if (prevIndex<0)
            prevIndex+=(int)_floatData.size();
        float dt=(times[absIndex]-times[prevIndex]);
        if (_derivativeIntegralAndCumulative==DATA_STREAM_ORIGINAL)
        {
            _transformedFloatData[absIndex]=_floatData[absIndex];
            _transformedFloatDataValidFlags[absIndex/8]|=(1<<(absIndex&7)); // valid data
        }
        if (_derivativeIntegralAndCumulative==DATA_STREAM_DERIVATIVE)
        {
            if (dt==0.0f)
            { // invalid data
                _transformedFloatData[absIndex]=0.0f;
                _transformedFloatDataValidFlags[absIndex/8]&=255-(1<<(absIndex&7)); // invalid data
            }
            else
            {
                if ((_floatDataValidFlags[prevIndex/8]&(1<<(prevIndex&7)))!=0)
                { // previous data was valid
                    if (!cyclic)
                        _transformedFloatData[absIndex]=(_floatData[absIndex]-_floatData[prevIndex])/dt;
                    else
                        _transformedFloatData[absIndex]=(tt::getAngleMinusAlpha_range(_floatData[absIndex],_floatData[prevIndex],range))/dt;
                    _transformedFloatDataValidFlags[absIndex/8]|=(1<<(absIndex&7)); // valid data
                }
                else
                { // previous data was invalid
                    _transformedFloatData[absIndex]=0.0f;
                    _transformedFloatDataValidFlags[absIndex/8]&=255-(1<<(absIndex&7)); // invalid data
                }
            }
        }
        if (_derivativeIntegralAndCumulative==DATA_STREAM_INTEGRAL)
        {
            if ((_floatDataValidFlags[prevIndex/8]&(1<<(prevIndex&7)))!=0)
            { // previous data was valid
                if ((_transformedFloatDataValidFlags[prevIndex/8]&(1<<(prevIndex&7)))!=0)
                { // previous transformed data was valid
                    if (!cyclic)
                        _transformedFloatData[absIndex]=_transformedFloatData[prevIndex]+(_floatData[prevIndex]+_floatData[absIndex])*0.5f*dt;
                    else
                        _transformedFloatData[absIndex]=_transformedFloatData[prevIndex]+(_floatData[prevIndex]+tt::getAngleMinusAlpha_range(_floatData[absIndex],_floatData[prevIndex],range)*0.5f)*dt;
                }
                else
                    _transformedFloatData[absIndex]=(_floatData[prevIndex]+_floatData[absIndex])*0.5f*dt; // previous transformed data was invalid
            }
            else
                _transformedFloatData[absIndex]=0.0f; // previous data was invalid
            _transformedFloatDataValidFlags[absIndex/8]|=(1<<(absIndex&7)); // valid transformed data
        }
        if (_derivativeIntegralAndCumulative==DATA_STREAM_CUMULATIVE)
        {
            if ((_floatDataValidFlags[prevIndex/8]&(1<<(prevIndex&7)))!=0)
            { // previous data was valid
                if ((_transformedFloatDataValidFlags[prevIndex/8]&(1<<(prevIndex&7)))!=0)
                { // previous transformed data was valid
                    if (!cyclic)
                        _transformedFloatData[absIndex]=_transformedFloatData[prevIndex]+_floatData[absIndex];
                    else
                        _transformedFloatData[absIndex]=_transformedFloatData[prevIndex]+(_floatData[prevIndex]+tt::getAngleMinusAlpha_range(_floatData[absIndex],_floatData[prevIndex],range));
                }
                else
                    _transformedFloatData[absIndex]=_floatData[prevIndex]+_floatData[absIndex]; // previous transformed data was invalid
            }
            else
                _transformedFloatData[absIndex]=_floatData[absIndex]; // previous data was invalid
            _transformedFloatDataValidFlags[absIndex/8]|=(1<<(absIndex&7)); // valid transformed data
        }
    }
}

void CGraphData::setValue(const C7Vector* graphCTM,int absIndex,bool firstValue,bool cyclic,float range,const std::vector<float>& times)
{
    bool dataIsOkay=false;
    float theValue;
    if (dataType==GRAPH_NOOBJECT_USER_DEFINED)
    {
        if (_userDataValid)
        {
            theValue=_userData;
            dataIsOkay=true;
        }
    }
    else
    {
        if (CGraphingRoutines::loopThroughAllAndGetDataValue(dataType,dataObjectID,theValue,graphCTM))
            dataIsOkay=true;
    }
    if (dataIsOkay)
        setValueDirect(absIndex,theValue,firstValue,cyclic,range,times);
    else
    {
        _floatData[absIndex]=0.0f;
        _transformedFloatData[absIndex]=0.0f;
        _floatDataValidFlags[absIndex/8]&=255-(1<<(absIndex&7)); // invalid data
        _transformedFloatDataValidFlags[absIndex/8]&=255-(1<<(absIndex&7)); // invalid data
    }
}

bool CGraphData::getValue(int absIndex,float& v) const
{
    if (getValueRaw(absIndex,v))
    {
        v=v*zoomFactor+addCoeff;
        return(true);
    }
    return(false); // invalid data
}


bool CGraphData::getValueRaw(int absIndex,float& v) const
{ // isValid can be null;
    v=_transformedFloatData[absIndex];
    return ((_transformedFloatDataValidFlags[absIndex/8]&(1<<(absIndex&7)))!=0);
}

void CGraphData::setUserData(float data)
{
    _userData=data;
    _userDataValid=true;        
}

void CGraphData::clearUserData()
{
    _userDataValid=false;       
}

int CGraphData::getDataLength()
{
    return(int(_floatData.size()));
}

void CGraphData::resetData(int bufferSize)
{
    _floatData.reserve(bufferSize);
    _floatData.clear();
    _transformedFloatData.reserve(bufferSize);
    _transformedFloatData.clear();
    _floatDataValidFlags.clear();
    _transformedFloatDataValidFlags.clear();
    // We fill it with the default value:
    for (int i=0;i<bufferSize;i++)
    {
        _floatData.push_back(0.0f);
        _transformedFloatData.push_back(0.0f);
    }
    for (int i=0;i<(bufferSize/8)+1;i++)
    {
        _floatDataValidFlags.push_back(0); // all data is invalid!
        _transformedFloatDataValidFlags.push_back(0);
    }
    clearUserData();
}

CGraphData* CGraphData::copyYourself()
{   // We copy everything, even the name and the identifier
    CGraphData* newObj=new CGraphData(dataType,dataObjectID,dataObjectAuxID);
    newObj->zoomFactor=zoomFactor;
    newObj->addCoeff=addCoeff;
    newObj->_movingAverageCount=_movingAverageCount;
    newObj->identifier=identifier;
    newObj->visible=visible;
    newObj->linkPoints=linkPoints;
    newObj->label=label;
    newObj->name=name;
    newObj->_derivativeIntegralAndCumulative=_derivativeIntegralAndCumulative;
    for (int i=0;i<3;i++)
        newObj->ambientColor[i]=ambientColor[i];
    newObj->_floatData.assign(_floatData.begin(),_floatData.end());
    newObj->_transformedFloatData.assign(_transformedFloatData.begin(),_transformedFloatData.end());
    newObj->_floatDataValidFlags.assign(_floatDataValidFlags.begin(),_floatDataValidFlags.end());
    newObj->_transformedFloatDataValidFlags.assign(_transformedFloatDataValidFlags.begin(),_transformedFloatDataValidFlags.end());
    return(newObj);
}

void CGraphData::serialize(CSer& ar,void* it)
{
    CGraph* cg=(CGraph*)it;
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Nme");
            ar << name;
            ar.flush();

            ar.storeDataName("Oid");
            ar << identifier;
            ar.flush();

            ar.storeDataName("Dt2");
            ar << dataType << dataObjectID << dataObjectAuxID;
            ar.flush();

            ar.storeDataName("Col");
            ar << ambientColor[0] << ambientColor[1] << ambientColor[2];
            ar.flush();

            ar.storeDataName("Var");
            ar << zoomFactor << 0.0f << addCoeff;
            ar.flush();

            ar.storeDataName("Mac");
            ar << _movingAverageCount;
            ar.flush();

            ar.storeDataName("Dci");
            ar << _derivativeIntegralAndCumulative;
            ar.flush();

            ar.storeDataName("Pa2");
            unsigned char dummy=0;
            dummy=dummy+1*visible;
            dummy=dummy+2*linkPoints;
            dummy=dummy+4*label;
            ar << dummy;
            ar.flush();

            ar.storeDataName("Dt9"); // Should always come after nullValue
            for (int i=0;i<cg->getNumberOfPoints();i++)
            {
                int absIndex;
                cg->getAbsIndexOfPosition(i,absIndex);
                ar << _floatData[absIndex];
            }
            ar.flush();

            ar.storeDataName("Ifd");
            for (int i=0;i<cg->getNumberOfPoints();i++)
            {
                int absIndex;
                cg->getAbsIndexOfPosition(i,absIndex);
                ar << _transformedFloatData[absIndex];
            }
            ar.flush();

            ar.storeDataName("Bla");
            for (int i=0;i<cg->getNumberOfPoints();i++)
            {
                int absIndex;
                cg->getAbsIndexOfPosition(i,absIndex);
                if ((_floatDataValidFlags[absIndex/8]&(1<<(absIndex&7)))!=0)
                    ar << (unsigned char)1;
                else
                    ar << (unsigned char)0;
            }
            ar.flush();

            ar.storeDataName("Bli");
            for (int i=0;i<cg->getNumberOfPoints();i++)
            {
                int absIndex;
                cg->getAbsIndexOfPosition(i,absIndex);
                if ((_transformedFloatDataValidFlags[absIndex/8]&(1<<(absIndex&7)))!=0)
                    ar << (unsigned char)1;
                else
                    ar << (unsigned char)0;
            }
            ar.flush();

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
                    if (theName.compare("Nme")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> name;
                    }
                    if (theName.compare("Oid")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> identifier;
                    }
                    if (theName.compare("Dt2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> dataType >> dataObjectID >> dataObjectAuxID;
                    }
                    if (theName.compare("Col")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> ambientColor[0] >> ambientColor[1] >> ambientColor[2];
                    }
                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        float dummy;
                        ar >> byteQuantity;
                        ar >> zoomFactor >> dummy >> addCoeff;
                    }
                    if (theName.compare("Mac")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _movingAverageCount;
                    }

                    if (theName.compare("Dci")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _derivativeIntegralAndCumulative;
                    }
                    if (theName.compare("Pa2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        visible=(dummy&1)==1;
                        linkPoints=(dummy&2)==2;
                        label=(dummy&4)==4;
                    }
                    if (theName.compare("Dt9")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        _floatData.reserve(cg->getBufferSize());
                        _floatData.clear();
                        for (int i=0;i<cg->getBufferSize();i++)
                            _floatData.push_back(0.0f);
                        for (int i=0;i<byteQuantity/int(sizeof(float));i++)
                        {
                            float dummy;
                            ar >> dummy;
                            _floatData[i]=dummy;
                        }
                    }

                    if (theName.compare("Ifd")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        _transformedFloatData.reserve(cg->getBufferSize());
                        _transformedFloatData.clear();
                        for (int i=0;i<cg->getBufferSize();i++)
                            _transformedFloatData.push_back(0.0f);
                        for (int i=0;i<byteQuantity/int(sizeof(float));i++)
                        {
                            float dummy;
                            ar >> dummy;
                            _transformedFloatData[i]=dummy;
                        }
                    }
                    if (theName.compare("Bla")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        _floatDataValidFlags.clear();
                        for (int i=0;i<((cg->getBufferSize()/8)+1);i++)
                            _floatDataValidFlags.push_back(0); // None valid!
                        for (int i=0;i<byteQuantity;i++)
                        {
                            unsigned char dummy;
                            ar >> dummy;
                            if (dummy!=0)
                                _floatDataValidFlags[i/8]|=(1<<(i&7));
                        }
                    }
                    if (theName.compare("Bli")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        _transformedFloatDataValidFlags.clear();
                        for (int i=0;i<((cg->getBufferSize()/8)+1);i++)
                            _transformedFloatDataValidFlags.push_back(0);
                        for (int i=0;i<byteQuantity;i++)
                        {
                            unsigned char dummy;
                            ar >> dummy;
                            if (dummy!=0)
                                _transformedFloatDataValidFlags[i/8]|=(1<<(i&7));
                        }
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}
