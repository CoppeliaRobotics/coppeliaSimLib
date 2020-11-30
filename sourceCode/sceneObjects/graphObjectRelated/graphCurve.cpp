#include "graphCurve.h"

CGraphCurve::CGraphCurve()
{
    _scriptHandle=-1;
}

CGraphCurve::CGraphCurve(int dim,const int streamIds[3],const float defaultVals[3],const char* curveName,const char* unitStr,int options,const float* color,int curveWidth,int scriptHandle)
{
    _curveName=curveName;
    _isStatic=false;
    setBasics(dim,streamIds,defaultVals,unitStr,options,color,curveWidth,scriptHandle);
}

CGraphCurve::~CGraphCurve()
{
}

void CGraphCurve::setBasics(int dim,const int streamIds[3],const float defaultVals[3],const char* unitStr,int options,const float* color,int curveWidth,int scriptHandle)
{
    _dim=dim;
    _scriptHandle=scriptHandle;
    _streamIds[0]=streamIds[0];
    _streamIds[1]=-1;
    _streamIds[2]=-1;
    if (streamIds[1]!=-1)
        _streamIds[1]=streamIds[1];
    if (streamIds[2]!=-1)
        _streamIds[2]=streamIds[2];
    _defaultVals[0]=defaultVals[0];
    _defaultVals[1]=defaultVals[1];
    _defaultVals[2]=defaultVals[2];
    _unitStr.clear();
    if (unitStr!=nullptr)
        _unitStr=unitStr;
    _relativeToWorld=(options&1)!=0; // default is rel. to graph
    _showLabel=(options&2)==0;
    _linkPoints=(options&4)==0;
    _curveWidth=curveWidth;
    if (color!=nullptr)
    {
        _color[0]=color[0];
        _color[1]=color[1];
        _color[2]=color[2];
    }
    else
    {
        _color[0]=1.0f;
        _color[1]=1.0f;
        _color[2]=0.0f;
    }
}

void CGraphCurve::updateStreamIds(const std::vector<int>& allStreamIds)
{
    for (size_t i=0;i<3;i++)
    {
        int id=_streamIds[i];
        if (id!=-1)
        {
            bool f=false;
            for (size_t j=0;j<allStreamIds.size();j++)
            {
                if (allStreamIds[j]==id)
                {
                    f=true;
                    break;
                }
            }
            if (!f)
                _streamIds[i]=-1; // that stream was erased
        }
    }
}

void CGraphCurve::makeStatic(CGraphDataStream* streams[3],int bufferSize,int startPt,int ptCnt)
{
    if (!_isStatic)
    {
        std::vector<float> xVals;
        std::vector<float> yVals;
        std::vector<float> zVals;
        _staticCurveValues.clear();
        if (_dim==2)
        {
            getCurveData_xy(streams,nullptr,bufferSize,startPt,ptCnt,nullptr,xVals,yVals,zVals,nullptr,nullptr,nullptr);
            for (size_t i=0;i<xVals.size();i++)
            {
                _staticCurveValues.push_back(xVals[i]);
                _staticCurveValues.push_back(yVals[i]);
            }
        }
        if (_dim==3)
        {
            getCurveData_xyz(streams,nullptr,bufferSize,startPt,ptCnt,nullptr,xVals,yVals,zVals,nullptr,nullptr,nullptr);
            for (size_t i=0;i<xVals.size();i++)
            {
                _staticCurveValues.push_back(xVals[i]);
                _staticCurveValues.push_back(yVals[i]);
                _staticCurveValues.push_back(zVals[i]);
            }
        }
        _streamIds[0]=-1;
        _streamIds[1]=-1;
        _streamIds[2]=-1;
        _isStatic=true;
    }
}

void CGraphCurve::setId(int id)
{
    _id=id;
}

int CGraphCurve::getId() const
{
    return(_id);
}

void CGraphCurve::setCurveName(const char* nm)
{
    _curveName=nm;
}

std::string CGraphCurve::getCurveName() const
{
    return(_curveName);
}

std::string CGraphCurve::getUnitStr() const
{
    return(_unitStr);
}

int CGraphCurve::getOptions() const
{
    int retVal=0;
    if (_relativeToWorld)
        retVal|=1;
    if (!_showLabel)
        retVal|=2;
    if (!_linkPoints)
        retVal|=4;
    return(retVal);
}

bool CGraphCurve::getIsStatic() const
{
    return(_isStatic);
}

const int* CGraphCurve::getStreamIdsPtr() const
{
    return(_streamIds);
}

int CGraphCurve::getCurveWidth() const
{
    return(_curveWidth);
}

const float* CGraphCurve::getColorPtr() const
{
    return(_color);
}

int CGraphCurve::getDim() const
{
    return(_dim);
}

const float* CGraphCurve::getDefaultValsPtr() const
{
    return(_defaultVals);
}

int CGraphCurve::getScriptHandle() const
{
    return(_scriptHandle);
}

bool CGraphCurve::getCurveData_xy(CGraphDataStream* streams[3],int* index,int bufferSize,int startPt,int ptCnt,std::string* label,std::vector<float>& xVals,std::vector<float>& yVals,std::vector<float>& zVals,int* curveType,float col[3],float minMax[6]) const
{
    if ( (((streams==nullptr)&&(_isStatic))||((streams!=nullptr)&&(!_isStatic)))&&(_dim==2) )
    {
        if ( (index==nullptr)||(index[0]==0) )
        {
            if (label!=nullptr)
            {
                label[0]=_curveName;
                if (_unitStr.size()>0)
                    label[0]+=" ("+_unitStr+")";
            }
            if (curveType!=nullptr)
            {
                if (_linkPoints)
                    curveType[0]=0;
                else
                    curveType[0]=1;
                if (!_showLabel)
                    curveType[0]+=4;
            }
            if (col!=nullptr)
            {
                col[0]=_color[0];
                col[1]=_color[1];
                col[2]=_color[2];
            }
            if (streams!=nullptr)
            { // not static
                int cnt=0;
                for (int cnt=0;cnt<ptCnt;cnt++)
                {
                    int absIndex=startPt+cnt;
                    if (absIndex>=bufferSize) // i.e. bufferSize
                        absIndex-=bufferSize;
                    bool validPt=true;
                    float xVal=_defaultVals[0];
                    if (streams[0]!=nullptr)
                    {
                        if (!streams[0]->getTransformedValue(startPt,absIndex,xVal))
                            validPt=false;
                    }
                    float yVal=_defaultVals[1];
                    if (streams[1]!=nullptr)
                    {
                        if (!streams[1]->getTransformedValue(startPt,absIndex,yVal))
                            validPt=false;
                    }
                    if (validPt)
                    {
                        xVals.push_back(xVal);
                        yVals.push_back(yVal);
                        if (minMax!=nullptr)
                        {
                            if (xVals.size()==1)
                            {
                                minMax[0]=xVal;
                                minMax[1]=xVal;
                                minMax[2]=yVal;
                                minMax[3]=yVal;
                            }
                            else
                            {
                                if (xVal<minMax[0])
                                    minMax[0]=xVal;
                                if (xVal>minMax[1])
                                    minMax[1]=xVal;
                                if (yVal<minMax[2])
                                    minMax[2]=yVal;
                                if (yVal>minMax[3])
                                    minMax[3]=yVal;
                            }
                        }
                    }
                }
            }
            else
            { // static
                if (curveType!=nullptr)
                    curveType[0]+=2; // static
                for (size_t i=0;i<_staticCurveValues.size()/2;i++)
                {
                    float xVal=_staticCurveValues[2*i+0];
                    float yVal=_staticCurveValues[2*i+1];
                    xVals.push_back(xVal);
                    yVals.push_back(yVal);
                    if (minMax!=nullptr)
                    {
                        if (xVals.size()==1)
                        {
                            minMax[0]=xVal;
                            minMax[1]=xVal;
                            minMax[2]=yVal;
                            minMax[3]=yVal;
                        }
                        else
                        {
                            if (xVal<minMax[0])
                                minMax[0]=xVal;
                            if (xVal>minMax[1])
                                minMax[1]=xVal;
                            if (yVal<minMax[2])
                                minMax[2]=yVal;
                            if (yVal>minMax[3])
                                minMax[3]=yVal;
                        }
                    }
                }
            }
            return(true);
        }
        if (index!=nullptr)
            index[0]--;
    }
    return(false);
}

bool CGraphCurve::getCurveData_xyz(CGraphDataStream* streams[3],int* index,int bufferSize,int startPt,int ptCnt,std::string* label,std::vector<float>& xVals,std::vector<float>& yVals,std::vector<float>& zVals,int* curveType,float col[3],float minMax[6]) const
{
    if ( (((streams==nullptr)&&(_isStatic))||((streams!=nullptr)&&(!_isStatic)))&&(_dim==3) )
    {
        if ( (index==nullptr)||(index[0]==0) )
        {
            if (label!=nullptr)
            {
                label[0]=_curveName;
                if (_unitStr.size()>0)
                    label[0]+=" ("+_unitStr+")";
            }
            if (curveType!=nullptr)
            {
                if (_linkPoints)
                    curveType[0]=0;
                else
                    curveType[0]=1;
                if (!_showLabel)
                    curveType[0]+=4;
            }
            if (col!=nullptr)
            {
                col[0]=_color[0];
                col[1]=_color[1];
                col[2]=_color[2];
            }
            if (streams!=nullptr)
            { // not static
                int cnt=0;
                for (int cnt=0;cnt<ptCnt;cnt++)
                {
                    int absIndex=startPt+cnt;
                    if (absIndex>=bufferSize) // i.e. bufferSize
                        absIndex-=bufferSize;
                    bool validPt=true;
                    float xVal=_defaultVals[0];
                    if (streams[0]!=nullptr)
                    {
                        if (!streams[0]->getTransformedValue(startPt,absIndex,xVal))
                            validPt=false;
                    }
                    float yVal=_defaultVals[1];
                    if (streams[1]!=nullptr)
                    {
                        if (!streams[1]->getTransformedValue(startPt,absIndex,yVal))
                            validPt=false;
                    }
                    float zVal=_defaultVals[2];
                    if (streams[2]!=nullptr)
                    {
                        if (!streams[2]->getTransformedValue(startPt,absIndex,zVal))
                            validPt=false;
                    }
                    if (validPt)
                    {
                        xVals.push_back(xVal);
                        yVals.push_back(yVal);
                        zVals.push_back(zVal);
                        if (xVals.size()==1)
                        {
                            minMax[0]=xVal;
                            minMax[1]=xVal;
                            minMax[2]=yVal;
                            minMax[3]=yVal;
                            minMax[4]=zVal;
                            minMax[5]=zVal;
                        }
                        else
                        {
                            if (xVal<minMax[0])
                                minMax[0]=xVal;
                            if (xVal>minMax[1])
                                minMax[1]=xVal;
                            if (yVal<minMax[2])
                                minMax[2]=yVal;
                            if (yVal>minMax[3])
                                minMax[3]=yVal;
                            if (zVal<minMax[4])
                                minMax[4]=zVal;
                            if (zVal>minMax[5])
                                minMax[5]=zVal;
                        }
                    }
                }
            }
            else
            { // static
                if (curveType!=nullptr)
                    curveType[0]+=2; // static
                for (size_t i=0;i<_staticCurveValues.size()/3;i++)
                {
                    float xVal=_staticCurveValues[3*i+0];
                    float yVal=_staticCurveValues[3*i+1];
                    float zVal=_staticCurveValues[3*i+2];
                    xVals.push_back(xVal);
                    yVals.push_back(yVal);
                    zVals.push_back(zVal);
                    if (xVals.size()==1)
                    {
                        minMax[0]=xVal;
                        minMax[1]=xVal;
                        minMax[2]=yVal;
                        minMax[3]=yVal;
                        minMax[4]=zVal;
                        minMax[5]=zVal;
                    }
                    else
                    {
                        if (xVal<minMax[0])
                            minMax[0]=xVal;
                        if (xVal>minMax[1])
                            minMax[1]=xVal;
                        if (yVal<minMax[2])
                            minMax[2]=yVal;
                        if (yVal>minMax[3])
                            minMax[3]=yVal;
                        if (zVal<minMax[4])
                            minMax[4]=zVal;
                        if (zVal>minMax[5])
                            minMax[5]=zVal;
                    }
                }
            }
            return(true);
        }
        if (index!=nullptr)
            index[0]--;
    }
    return(false);
}

void CGraphCurve::serialize(CSer& ar,int startPt,int ptCnt,int bufferSize)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Nme");
            ar << _curveName << _unitStr;
            ar.flush();

            ar.storeDataName("Oid");
            ar << _id << _dim << _curveWidth;
            ar.flush();

            ar.storeDataName("Sch");
            ar << _scriptHandle;
            ar.flush();

            ar.storeDataName("Col");
            ar << _color[0] << _color[1] << _color[2];
            ar.flush();

            ar.storeDataName("Dev");
            ar << _defaultVals[0] << _defaultVals[1] << _defaultVals[2];
            ar.flush();

            ar.storeDataName("Str");
            ar << _streamIds[0] << _streamIds[1] << _streamIds[2];
            ar.flush();

            ar.storeDataName("Pa0");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_relativeToWorld);
            SIM_SET_CLEAR_BIT(nothing,1,_showLabel);
            SIM_SET_CLEAR_BIT(nothing,2,_linkPoints);
            SIM_SET_CLEAR_BIT(nothing,3,_isStatic);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Pts");
            ar << int(_staticCurveValues.size());
            for (int i=0;i<_staticCurveValues.size();i++)
                ar << _staticCurveValues[i];
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
                        ar >> _curveName >> _unitStr;
                    }
                    if (theName.compare("Oid")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _id >> _dim >> _curveWidth;
                    }
                    if (theName.compare("Sch")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _scriptHandle;
                    }
                    if (theName.compare("Col")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _color[0] >> _color[1] >> _color[2];
                    }
                    if (theName.compare("Dev")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _defaultVals[0] >> _defaultVals[1] >> _defaultVals[2];
                    }
                    if (theName.compare("Str")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _streamIds[0] >> _streamIds[1] >> _streamIds[2];
                    }
                    if (theName=="Pa0")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _relativeToWorld=SIM_IS_BIT_SET(nothing,0);
                        _showLabel=SIM_IS_BIT_SET(nothing,1);
                        _linkPoints=SIM_IS_BIT_SET(nothing,2);
                        _isStatic=SIM_IS_BIT_SET(nothing,3);
                    }
                    if (theName.compare("Pts")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        _staticCurveValues.resize(cnt);
                        for (int i=0;i<cnt;i++)
                            ar >> _staticCurveValues[i];
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        if (ar.isStoring())
        {
            ar.xmlAddNode_string("name",_curveName.c_str());
            ar.xmlAddNode_string("unitStr",_unitStr.c_str());
            ar.xmlAddNode_int("id",_id);
            ar.xmlAddNode_int("scriptHandle",_scriptHandle);
            ar.xmlAddNode_int("dim",_dim);
            ar.xmlAddNode_floats("color",_color,3);
            ar.xmlAddNode_floats("defaultValues",_defaultVals,3);
            ar.xmlAddNode_ints("streamIds",_streamIds,3);
            ar.xmlAddNode_int("curveWidth",_curveWidth);
            ar.xmlAddNode_floats("staticData",_staticCurveValues);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("relativeToWorld",_relativeToWorld);
            ar.xmlAddNode_bool("showLabel",_showLabel);
            ar.xmlAddNode_bool("linkPoints",_linkPoints);
            ar.xmlAddNode_bool("static",_isStatic);
            ar.xmlPopNode();
        }
        else
        {
            ar.xmlGetNode_string("name",_curveName);
            ar.xmlGetNode_string("unitStr",_unitStr);
            ar.xmlGetNode_int("id",_id);
            ar.xmlGetNode_int("scriptHandle",_scriptHandle);
            ar.xmlGetNode_int("dim",_dim);
            ar.xmlGetNode_floats("color",_color,3);
            ar.xmlGetNode_floats("defaultValues",_defaultVals,3);
            ar.xmlGetNode_int("curveWidth",_curveWidth);
            ar.xmlGetNode_floats("staticData",_staticCurveValues);

            if (ar.xmlPushChildNode("switches"))
            {
                ar.xmlGetNode_bool("relativeToWorld",_relativeToWorld);
                ar.xmlGetNode_bool("showLabel",_showLabel);
                ar.xmlGetNode_bool("linkPoints",_linkPoints);
                ar.xmlGetNode_bool("static",_isStatic);
                ar.xmlPopNode();
            }
        }
    }
}

CGraphCurve* CGraphCurve::copyYourself() const
{
    CGraphCurve* newObj=new CGraphCurve();
    newObj->_curveName=_curveName;
    newObj->_unitStr=_unitStr;
    newObj->_relativeToWorld=_relativeToWorld;
    newObj->_showLabel=_showLabel;
    newObj->_linkPoints=_linkPoints;
    newObj->_isStatic=_isStatic;
    newObj->_curveWidth=_curveWidth;
    newObj->_dim=_dim;
    newObj->_id=_id;
    for (int i=0;i<3;i++)
    {
        newObj->_color[i]=_color[i];
        newObj->_defaultVals[i]=_defaultVals[i];
        newObj->_streamIds[i]=_streamIds[i];
    }
    newObj->_staticCurveValues.assign(_staticCurveValues.begin(),_staticCurveValues.end());
    return(newObj);
}

bool CGraphCurve::announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript,bool copyBuffer)
{
    return( (scriptHandle==_scriptHandle)&&(!sceneSwitchPersistentScript) );
}

void CGraphCurve::performScriptLoadingMapping(const std::vector<int>* map)
{ // If (map[2*i+0]==old_script_handle) then new_script_handle=map[2*i+1]
    for (size_t i=0;i<map->size()/2;i++)
    {
        if (_scriptHandle==map->at(2*i+0))
        {
            _scriptHandle=map->at(2*i+1);
            break;
        }
    }
}
