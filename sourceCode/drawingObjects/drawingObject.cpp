#include "simInternal.h"
#include "drawingObject.h"
#include "app.h"
#include "tt.h"
#include "easyLock.h"
#include "drawingObjectRendering.h"

float CDrawingObject::getSize() const
{
    return(_size);
}

int CDrawingObject::getMaxItemCount() const
{
    return(_maxItemCount);
}

int CDrawingObject::getStartItem() const
{
    return(_startItem);
}



std::vector<float>* CDrawingObject::getDataPtr()
{
    return(&_data);
}

CDrawingObject::CDrawingObject(int theObjectType,float size,float duplicateTolerance,int sceneObjId,int maxItemCount,int creatorHandle)
{
    _creatorHandle=creatorHandle;
    float tr=0.0f;
    if (theObjectType&sim_drawing_50percenttransparency)
        tr+=0.5f;
    if (theObjectType&sim_drawing_25percenttransparency)
        tr+=0.25f;
    if (theObjectType&sim_drawing_12percenttransparency)
        tr+=0.125f;
    color.setDefaultValues();

    if (tr!=0.0f)
    {
        color.setTranslucid(true);
        color.setOpacity(1.0f-tr);
    }

    _objectId=-1;
    _sceneObjectId=-1;
    _objectUid=-1;
    _sceneObjectUid=-1;

    size=tt::getLimitedFloat(0.0001f,100.0f,size);
    _size=size;
    if (maxItemCount==0)
        maxItemCount=100000;
    maxItemCount=tt::getLimitedInt(1,10000000,maxItemCount);
    _maxItemCount=maxItemCount;
    _startItem=0;
    int tmp=theObjectType&0x001f;
    if (theObjectType&sim_drawing_vertexcolors)
    {
        if ((tmp!=sim_drawing_lines)&&(tmp!=sim_drawing_triangles)&&(tmp!=sim_drawing_linestrip))
            theObjectType-=sim_drawing_vertexcolors;
        if ((theObjectType&sim_drawing_itemcolors)&&(theObjectType&sim_drawing_vertexcolors))
            theObjectType-=sim_drawing_vertexcolors;
        if (theObjectType&sim_drawing_itemtransparency)
            theObjectType-=sim_drawing_itemtransparency;

    }
    if (theObjectType&sim_drawing_itemsizes)
    {
        if (tmp==sim_drawing_triangles)
            theObjectType-=sim_drawing_itemsizes;
    }
    if (duplicateTolerance<0.0f)
        duplicateTolerance=0.0f;
    _duplicateTolerance=duplicateTolerance;
    _objectType=theObjectType;
    _setItemSizes();

    if (sceneObjId!=-1)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(sceneObjId);
        if (it!=nullptr)
        {
            _sceneObjectId=sceneObjId;
            _sceneObjectUid=it->getObjectUid();
        }
    }
}

CDrawingObject::~CDrawingObject()
{
}

int CDrawingObject::getObjectType() const
{
    return(_objectType);
}

int CDrawingObject::getSceneObjectId() const
{
    return(_sceneObjectId);
}

void CDrawingObject::setObjectId(int newId)
{
    _objectId=newId;
}

int CDrawingObject::getObjectId() const
{
    return(_objectId);
}

int CDrawingObject::getObjectUid() const
{
    return(_objectUid);
}

void CDrawingObject::setObjectUniqueId()
{
    _objectUid=App::getFreshUniqueId();
}

void CDrawingObject::adjustForFrameChange(const C7Vector& preCorrection)
{
    for (int i=0;i<int(_data.size())/floatsPerItem;i++)
    {
        for (int j=0;j<verticesPerItem;j++)
        {
            C3Vector v(&_data[floatsPerItem*i+j*3+0]);
            v*=preCorrection;
            v.copyTo(&_data[floatsPerItem*i+j*3+0]);
        }
        int off=verticesPerItem*3;
        for (int j=0;j<normalsPerItem;j++)
        {
            C3Vector n(&_data[floatsPerItem*i+off+j*3+0]);
            n=preCorrection.Q*n;
            n.copyTo(&_data[floatsPerItem*i+off+j*3+0]);
        }
    }
    _initBufferedEventData();
    pushAppendNewPointEvent(true);
}

void CDrawingObject::adjustForScaling(float xScale,float yScale,float zScale)
{
    float avgScaling=(xScale+yScale+zScale)/3.0f;
    int tmp=_objectType&0x001f;
    if ((tmp!=sim_drawing_points)&&(tmp!=sim_drawing_lines)&&(tmp!=sim_drawing_linestrip))
        _size*=avgScaling;

    for (int i=0;i<int(_data.size())/floatsPerItem;i++)
    {
        for (int j=0;j<verticesPerItem;j++)
        {
            C3Vector v(&_data[floatsPerItem*i+j*3+0]);
            v(0)*=xScale;
            v(1)*=yScale;
            v(2)*=zScale;
            v.copyTo(&_data[floatsPerItem*i+j*3+0]);
        }
        int off=verticesPerItem*3;
        if (_objectType&sim_drawing_itemcolors)
            off+=3;
        if (_objectType&sim_drawing_vertexcolors)
            off+=3*verticesPerItem;
        if (_objectType&sim_drawing_itemsizes)
        {
            _data[floatsPerItem*i+off+0]*=avgScaling;
            off+=1;
        }
        if (_objectType&sim_drawing_itemtransparency)
            off+=1;
    }
    _initBufferedEventData();
    pushAppendNewPointEvent(true);
}

void CDrawingObject::setItems(const float* itemData,size_t itemCnt)
{
    addItem(nullptr);
    size_t off=size_t(verticesPerItem*3+normalsPerItem*3+colorsPerItem*3+otherFloatsPerItem);
    for (size_t i=0;i<itemCnt;i++)
        addItem(itemData+off*i);
}

bool CDrawingObject::addItem(const float* itemData)
{
    EASYLOCK(_objectMutex);
    if (itemData==nullptr)
    {
        _data.clear();
        _bufferedEventData.clear();
        _startItem=0;

        if ( (otherFloatsPerItem==0)&&App::worldContainer->getEnableEvents() )
        {
            auto [event,data]=App::worldContainer->prepareEvent(EVENTTYPE_DRAWINGOBJECTCHANGED,_objectUid,nullptr,false);
            data->appendMapObject_stringBool("clearPoints",true);
            App::worldContainer->pushEvent(event);
        }

        return(false);
    }

    int newPos=_startItem;
    if (int(_data.size())/floatsPerItem>=_maxItemCount)
    { // the buffer is full
        if (_objectType&sim_drawing_cyclic)
        {
            _startItem++;
            if (_startItem>=_maxItemCount)
                _startItem=0;
        }
        else
            return(false); // saturated
    }

    C7Vector trInv;
    trInv.setIdentity();
    if (_sceneObjectId>=0)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectId);
        if (it==nullptr)
            _sceneObjectId=-2; // should normally never happen!
        else
            trInv=it->getCumulativeTransformation().getInverse();
    }

    if ( (_duplicateTolerance>0.0f)&&(verticesPerItem==1) )
    { // Check for duplicates
        C3Vector v(itemData);
        v*=trInv;
        for (int i=0;i<int(_data.size())/floatsPerItem;i++)
        {
            C3Vector w(&_data[floatsPerItem*i+0]);
            if ((w-v).getLength()<=_duplicateTolerance)
                return(false); // point already there!
        }
    }

    if (int(_data.size())/floatsPerItem<_maxItemCount)
    { // The buffer is not yet full!
        newPos=int(_data.size())/floatsPerItem;
        for (int i=0;i<floatsPerItem;i++)
            _data.push_back(0.0f);
    }

    if (_sceneObjectId!=-2)
    {
        int off=0;
        for (int i=0;i<verticesPerItem;i++)
        {
            C3Vector v(itemData+off);
            v*=trInv;
            _data[newPos*floatsPerItem+off+0]=v(0);
            _data[newPos*floatsPerItem+off+1]=v(1);
            _data[newPos*floatsPerItem+off+2]=v(2);
            if ( (otherFloatsPerItem==0)&&App::worldContainer->getEnableEvents() )
            {
                _bufferedEventData.push_back(v(0));
                _bufferedEventData.push_back(v(1));
                _bufferedEventData.push_back(v(2));
            }
            off+=3;
        }
        for (int i=0;i<normalsPerItem;i++)
        {
            C3Vector v(itemData+off);
            v=trInv.Q*v; // no translational part!
            _data[newPos*floatsPerItem+off+0]=v(0);
            _data[newPos*floatsPerItem+off+1]=v(1);
            _data[newPos*floatsPerItem+off+2]=v(2);
            if ( (otherFloatsPerItem==0)&&App::worldContainer->getEnableEvents() )
            {
                _bufferedEventData.push_back(v(0));
                _bufferedEventData.push_back(v(1));
                _bufferedEventData.push_back(v(2));
            }
            off+=3;
        }
        for (int i=0;i<colorsPerItem*3+otherFloatsPerItem;i++)
        {
            _data[newPos*floatsPerItem+off+i]=itemData[off+i];
            if ( (otherFloatsPerItem==0)&&App::worldContainer->getEnableEvents() )
                _bufferedEventData.push_back(itemData[off+i]);
        }
    }
    return(true);
}

void CDrawingObject::_setItemSizes()
{
    verticesPerItem=0;
    normalsPerItem=0;
    colorsPerItem=0;
    otherFloatsPerItem=0;
    int tmp=_objectType&0x001f;
    if ( (tmp==sim_drawing_points)||(tmp==sim_drawing_trianglepoints)||(tmp==sim_drawing_quadpoints)||(tmp==sim_drawing_discpoints)||(tmp==sim_drawing_cubepoints)||(tmp==sim_drawing_spherepoints) )
        verticesPerItem=1;
    if (tmp==sim_drawing_lines)
        verticesPerItem=2;
    if (tmp==sim_drawing_linestrip)
        verticesPerItem=1;
    if (tmp==sim_drawing_triangles)
        verticesPerItem=3;

    if ( (tmp==sim_drawing_trianglepoints)||(tmp==sim_drawing_quadpoints)||(tmp==sim_drawing_discpoints)||(tmp==sim_drawing_cubepoints) )
    {
        if ((_objectType&sim_drawing_facingcamera)==0)
            normalsPerItem=1;
    }

    if (_objectType&sim_drawing_itemcolors)
        colorsPerItem+=1;
    if (_objectType&sim_drawing_vertexcolors)
    { 
        if (tmp==sim_drawing_linestrip)
            colorsPerItem+=1;
        if (tmp==sim_drawing_lines)
            colorsPerItem+=2;
        if (tmp==sim_drawing_triangles)
            colorsPerItem+=3;
    }
    if (_objectType&sim_drawing_itemsizes)
        otherFloatsPerItem+=1;
    if (_objectType&sim_drawing_itemtransparency)
        otherFloatsPerItem+=1;

    floatsPerItem=3*verticesPerItem+3*normalsPerItem+3*colorsPerItem+otherFloatsPerItem;
}

bool CDrawingObject::announceObjectWillBeErased(int objId)
{
    return(_sceneObjectId==objId);
}

bool CDrawingObject::announceScriptStateWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript)
{
    return( (!sceneSwitchPersistentScript)&&(_creatorHandle==scriptHandle) );
}

bool CDrawingObject::canMeshBeExported() const
{
    int tmp=_objectType&0x001f;
    return((tmp==sim_drawing_triangles)||(tmp==sim_drawing_trianglepoints)||(tmp==sim_drawing_quadpoints)||
        (tmp==sim_drawing_discpoints)||(tmp==sim_drawing_cubepoints)||(tmp==sim_drawing_spherepoints));
}

void CDrawingObject::getExportableMesh(std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    C7Vector tr;
    tr.setIdentity();
    if (_sceneObjectId>=0)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectId);
        if (it!=nullptr)
            tr=it->getCumulativeTransformation();
    }
    int tmp=_objectType&0x001f;
    if (tmp==sim_drawing_triangles)
        _exportTriangles(tr,vertices,indices);
    if (tmp==sim_drawing_trianglepoints)
        _exportTrianglePoints(tr,vertices,indices);
    if (tmp==sim_drawing_quadpoints)
        _exportQuadPoints(tr,vertices,indices);
    if (tmp==sim_drawing_discpoints)
        _exportDiscPoints(tr,vertices,indices);
    if (tmp==sim_drawing_cubepoints)
        _exportCubePoints(tr,vertices,indices);
    if (tmp==sim_drawing_spherepoints)
        _exportSpherePoints(tr,vertices,indices);
}

void CDrawingObject::_exportTrianglePoints(C7Vector& tr,std::vector<float>& vertices,std::vector<int>& indices) const
{
    // finish this routine (and others linked to export functionality)

}

void CDrawingObject::_exportQuadPoints(C7Vector& tr,std::vector<float>& vertices,std::vector<int>& indices) const
{
    // finish this routine (and others linked to export functionality)

}

void CDrawingObject::_exportDiscPoints(C7Vector& tr,std::vector<float>& vertices,std::vector<int>& indices) const
{
    // finish this routine (and others linked to export functionality)

}

void CDrawingObject::_exportCubePoints(C7Vector& tr,std::vector<float>& vertices,std::vector<int>& indices) const
{
    // finish this routine (and others linked to export functionality)

}

void CDrawingObject::_exportSpherePoints(C7Vector& tr,std::vector<float>& vertices,std::vector<int>& indices) const
{
    // finish this routine (and others linked to export functionality)

}

void CDrawingObject::_exportTriangles(C7Vector& tr,std::vector<float>& vertices,std::vector<int>& indices) const
{
    // finish this routine (and others linked to export functionality)
    C3Vector v;
    for (int i=0;i<int(_data.size())/floatsPerItem;i++)
    {
        int p=_startItem+i;
        if (p>=_maxItemCount)
            p-=_maxItemCount;
        v.set(&_data[floatsPerItem*p+0]);
        v*=tr;
        v.set(&_data[floatsPerItem*p+3]);
        v*=tr;
        v.set(&_data[floatsPerItem*p+6]);
        v*=tr;
    }
}

void CDrawingObject::_exportTriOrQuad(C7Vector& tr,C3Vector* v0,C3Vector* v1,C3Vector* v2,C3Vector* v3,std::vector<float>& vertices,std::vector<int>& indices,int& nextIndex) const
{
    // finish this routine (and others linked to export functionality)
    v0[0]*=tr;
    v1[0]*=tr;
    v2[0]*=tr;
    if (v3!=nullptr)
        v3[0]*=tr;
}

void CDrawingObject::draw(bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM)
{
    if (displayAttrib&sim_displayattribute_colorcoded)
        return;
    EASYLOCK(_objectMutex);

    if (_objectType&sim_drawing_overlay)
    {
        if (!overlay)
            return;
    }
    else
    {
        if (overlay)
            return;
    }

    if (!overlay)
    {
        if (_objectType&(sim_drawing_50percenttransparency+sim_drawing_25percenttransparency+sim_drawing_12percenttransparency+sim_drawing_itemtransparency))
        {
            if (!transparentObject)
                return;
        }
        else
        {
            if (transparentObject)
                return;
        }
    }

    C7Vector tr;
    tr.setIdentity();

    if (_sceneObjectId>=0)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectId);
        if (it==nullptr)
            _sceneObjectId=-2; // should normally never happen
        else
        {
            tr=it->getCumulativeTransformation();
            if (_objectType&sim_drawing_followparentvisibility)
            {
                if ( ((App::currentWorld->environment->getActiveLayers()&it->getVisibilityLayer())==0)&&((displayAttrib&sim_displayattribute_ignorelayer)==0) )
                    return; // not visible
                if (it->isObjectPartOfInvisibleModel())
                    return; // not visible
                if ( ((_objectType&sim_drawing_painttag)==0)&&(displayAttrib&sim_displayattribute_forvisionsensor) )
                    return; // not visible
            }
            else
            {
                if ( ((_objectType&sim_drawing_painttag)==0)&&(displayAttrib&sim_displayattribute_forvisionsensor) )
                    return; // not visible
            }
        }
    }
    if (_sceneObjectId==-2)
        return;

    displayDrawingObject(this,tr,overlay,transparentObject,displayAttrib,cameraCTM);

}

void CDrawingObject::_initBufferedEventData()
{
    _bufferedEventData.resize(_data.size());
    size_t index=_startItem;
    for (size_t itemCnt=0;itemCnt<_data.size()/size_t(floatsPerItem);itemCnt++)
    {
        for (size_t i=0;i<size_t(floatsPerItem);i++)
            _bufferedEventData[itemCnt*floatsPerItem+i]=_data[index*floatsPerItem+i];

        index++;
        if (index>=_maxItemCount)
            index-=_maxItemCount;
    }

}

void CDrawingObject::_getEventData(std::vector<float>& vertices,std::vector<float>& normals,std::vector<float>& colors) const
{
    size_t w=0;
    if (_objectType&sim_drawing_itemcolors)
    {
        int tmp=_objectType&0x001f;
        if (tmp==sim_drawing_lines)
            w=2;
        if (tmp==sim_drawing_triangles)
            w=3;
    }

    for (size_t itemCnt=0;itemCnt<_bufferedEventData.size()/size_t(floatsPerItem);itemCnt++)
    {
        size_t t=itemCnt*floatsPerItem;

        for (size_t i=0;i<verticesPerItem;i++)
        {
            vertices.push_back(_bufferedEventData[t+0]);
            vertices.push_back(_bufferedEventData[t+1]);
            vertices.push_back(_bufferedEventData[t+2]);
            t+=3;
        }
        for (size_t i=0;i<normalsPerItem;i++)
        {
            normals.push_back(_bufferedEventData[t+0]);
            normals.push_back(_bufferedEventData[t+1]);
            normals.push_back(_bufferedEventData[t+2]);
            t+=3;
        }
        if (w==0)
        {
            for (size_t i=0;i<colorsPerItem;i++)
            {
                colors.push_back(_bufferedEventData[t+0]);
                colors.push_back(_bufferedEventData[t+1]);
                colors.push_back(_bufferedEventData[t+2]);
                t+=3;
            }
        }
        else
        {
            for (size_t i=0;i<w;i++)
            {
                colors.push_back(_bufferedEventData[t+0]);
                colors.push_back(_bufferedEventData[t+1]);
                colors.push_back(_bufferedEventData[t+2]);
            }
        }
    }
}

void CDrawingObject::pushAddEvent()
{
    if ( (otherFloatsPerItem==0)&&App::worldContainer->getEnableEvents() )
    {
        auto [event,data]=App::worldContainer->prepareEvent(EVENTTYPE_DRAWINGOBJECTADDED,_objectUid,nullptr,false);
        std::string tp;
        switch(_objectType&0x001f)
        {
            case sim_drawing_points : tp="point";
                break;
            case sim_drawing_lines : tp="line";
                break;
            case sim_drawing_linestrip : tp="lineStrip";
                break;
            case sim_drawing_triangles : tp="triangle";
                break;
            case sim_drawing_trianglepoints : tp="trianglePoint";
                break;
            case sim_drawing_quadpoints : tp="quadPoint";
                break;
            case sim_drawing_discpoints : tp="discPoint";
                break;
            case sim_drawing_cubepoints : tp="cubePoint";
                break;
            case sim_drawing_spherepoints : tp="spherePoint";
                break;
        }
        data->appendMapObject_stringString("type",tp.c_str(),0);

        float c[9];
        color.getColor(c+0,sim_colorcomponent_ambient_diffuse);
        color.getColor(c+3,sim_colorcomponent_specular);
        color.getColor(c+6,sim_colorcomponent_emission);
        data->appendMapObject_stringFloatArray("color",c,9);

        data->appendMapObject_stringInt32("maxCnt",_maxItemCount);

        data->appendMapObject_stringFloat("size",_size);

        data->appendMapObject_stringInt32("parent",_sceneObjectUid);

        data->appendMapObject_stringBool("cyclic",(_objectType&sim_drawing_cyclic)!=0);

        data->appendMapObject_stringBool("clearPoints",true);

        App::worldContainer->pushEvent(event);

        _initBufferedEventData();
    }
}

void CDrawingObject::pushAppendNewPointEvent(bool clearAllFirst)
{
    if ( (_bufferedEventData.size()>0)&&App::worldContainer->getEnableEvents() )
    {
        auto [event,data]=App::worldContainer->prepareEvent(EVENTTYPE_DRAWINGOBJECTCHANGED,_objectUid,nullptr,false);

        std::vector<float> points;
        std::vector<float> normals;
        std::vector<float> colors;
        _getEventData(points,normals,colors);

        CCbor obj(nullptr,0);
        size_t l;
        obj.appendFloatArray(points.data(),points.size());
        const char* buff=(const char*)obj.getBuff(l);
        data->appendMapObject_stringString("points",buff,l,true);

        obj.clear();
        obj.appendFloatArray(normals.data(),normals.size());
        buff=(const char*)obj.getBuff(l);
        data->appendMapObject_stringString("normals",buff,l,true);

        obj.clear();
        obj.appendFloatArray(colors.data(),colors.size());
        buff=(const char*)obj.getBuff(l);
        data->appendMapObject_stringString("colors",buff,l,true);

        data->appendMapObject_stringBool("clearPoints",clearAllFirst);

        _bufferedEventData.clear();

        App::worldContainer->pushEvent(event);
    }
}

