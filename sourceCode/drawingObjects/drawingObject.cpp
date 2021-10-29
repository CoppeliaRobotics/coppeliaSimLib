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

CDrawingObject::CDrawingObject(int theObjectType,float size,float duplicateTolerance,int sceneObjID,int maxItemCount,int creatorHandle)
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
        color.setTransparencyFactor(1.0f-tr);
    }

    _objectID=0;
    _sceneObjectID=-1;
    size=tt::getLimitedFloat(0.0001f,100.0f,size);
    _size=size;
    if (maxItemCount==0)
        maxItemCount=10000000;
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

    if (sceneObjID==-1)
        _sceneObjectID=-1;
    else
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(sceneObjID);
        if (it!=nullptr)
            _sceneObjectID=sceneObjID;
    }
}

CDrawingObject::~CDrawingObject()
{
}

int CDrawingObject::getObjectType() const
{
    return(_objectType);
}

int CDrawingObject::getSceneObjectID() const
{
    return(_sceneObjectID);
}

void CDrawingObject::setObjectID(int newID)
{
    _objectID=newID;
}

int CDrawingObject::getObjectID() const
{
    return(_objectID);
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
}

void CDrawingObject::setItems(const float* itemData,size_t itemCnt)
{
    addItem(nullptr);
    size_t off=size_t(verticesPerItem*3+normalsPerItem*3+otherFloatsPerItem);
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
            auto [event,data]=App::worldContainer->createEvent(EVENTTYPE_DRAWINGOBJECTCHANGED,nullptr,_objectID);
            data->appendMapObject_stringFloatArray("points",nullptr,0);
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
    if (_sceneObjectID>=0)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectID);
        if (it==nullptr)
            _sceneObjectID=-2; // should normally never happen!
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

    if ( (otherFloatsPerItem==0)&&App::worldContainer->getEnableEvents() )
        _bufferedEventData.insert(_bufferedEventData.begin(),itemData,itemData+floatsPerItem);

    if (int(_data.size())/floatsPerItem<_maxItemCount)
    { // The buffer is not yet full!
        newPos=int(_data.size())/floatsPerItem;
        for (int i=0;i<floatsPerItem;i++)
            _data.push_back(0.0f);
    }


    if (_sceneObjectID!=-2)
    {
        int off=0;
        for (int i=0;i<verticesPerItem;i++)
        {
            C3Vector v(itemData+off);
            v*=trInv;
            _data[newPos*floatsPerItem+off+0]=v(0);
            _data[newPos*floatsPerItem+off+1]=v(1);
            _data[newPos*floatsPerItem+off+2]=v(2);
            off+=3;
        }
        for (int i=0;i<normalsPerItem;i++)
        {
            C3Vector v(itemData+off);
            v=trInv.Q*v; // no translational part!
            _data[newPos*floatsPerItem+off+0]=v(0);
            _data[newPos*floatsPerItem+off+1]=v(1);
            _data[newPos*floatsPerItem+off+2]=v(2);
            off+=3;
        }
        for (int i=0;i<otherFloatsPerItem;i++)
            _data[newPos*floatsPerItem+off+i]=itemData[off+i];
    }
    return(true);
}

void CDrawingObject::_setItemSizes()
{
    verticesPerItem=0;
    normalsPerItem=0;
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
        otherFloatsPerItem+=3;
    if (_objectType&sim_drawing_vertexcolors)
    { 
        if (tmp==sim_drawing_linestrip)
            otherFloatsPerItem+=3;
        if (tmp==sim_drawing_lines)
            otherFloatsPerItem+=6;
        if (tmp==sim_drawing_triangles)
            otherFloatsPerItem+=9;
    }
    if (_objectType&sim_drawing_itemsizes)
        otherFloatsPerItem+=1;
    if (_objectType&sim_drawing_itemtransparency)
        otherFloatsPerItem+=1;

    floatsPerItem=3*verticesPerItem+3*normalsPerItem+otherFloatsPerItem;
}

bool CDrawingObject::announceObjectWillBeErased(int objID)
{
    return(_sceneObjectID==objID);
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
    if (_sceneObjectID>=0)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectID);
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

    if (_sceneObjectID>=0)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectID);
        if (it==nullptr)
            _sceneObjectID=-2; // should normally never happen
        else
        {
            tr=it->getCumulativeTransformation();
            if (_objectType&sim_drawing_followparentvisibility)
            {
                if ( ((App::currentWorld->mainSettings->getActiveLayers()&it->getVisibilityLayer())==0)&&((displayAttrib&sim_displayattribute_ignorelayer)==0) )
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
    if (_sceneObjectID==-2)
        return;

    displayDrawingObject(this,tr,overlay,transparentObject,displayAttrib,cameraCTM);

}

void CDrawingObject::pushCreateContainerEvent()
{
    if ( (otherFloatsPerItem==0)&&App::worldContainer->getEnableEvents() )
    {
        auto [event,data]=App::worldContainer->createEvent(EVENTTYPE_DRAWINGOBJECTADDED,nullptr,_objectID);
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

        data->appendMapObject_stringInt32("parent",_sceneObjectID);

        data->appendMapObject_stringBool("cyclic",(_objectType&sim_drawing_cyclic)!=0);

        App::worldContainer->pushEvent(event);

        _bufferedEventData.assign(_data.begin(),_data.end());
    }
}

void CDrawingObject::pushAppendNewPointEvent()
{
    if ( _bufferedEventData.size()>0 )
    {
        auto [event,data]=App::worldContainer->createEvent(EVENTTYPE_DRAWINGOBJECTCHANGED,nullptr,_objectID);

        CCbor obj(nullptr,0);
        size_t l;
        obj.appendFloatArray(_bufferedEventData.data(),_bufferedEventData.size());
        const char* buff=(const char*)obj.getBuff(l);
        data->appendMapObject_stringString("points",buff,l,true);
        _bufferedEventData.clear();

        App::worldContainer->pushEvent(event);
    }
}

