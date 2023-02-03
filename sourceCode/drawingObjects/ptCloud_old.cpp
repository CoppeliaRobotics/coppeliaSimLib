#include <ptCloud_old.h>
#include <app.h>
#include <ptCloudRendering_old.h>

CPtCloud_old::CPtCloud_old(int pageMask,int layerMask,int parentHandle,int options,double pointSize,int ptCnt,const double* vertices,const unsigned char* colors,const double* normals,const unsigned char* defaultColors)
{
    _pageMask=pageMask;
    _layerMask=layerMask;
    _parentHandle=parentHandle;
    _options=options;
    _pointSize=pointSize;
    _vertices.assign(vertices,vertices+ptCnt*3);
    if (defaultColors!=nullptr)
    {
        for (int i=0;i<4;i++)
        {
            _defaultColors[4*i+0]=defaultColors[3*i+0]/255.0;
            _defaultColors[4*i+1]=defaultColors[3*i+1]/255.0;
            _defaultColors[4*i+2]=defaultColors[3*i+2]/255.0;
            _defaultColors[4*i+3]=1.0;
        }
    }
    else
    {
        _defaultColors[0]=0.5;
        _defaultColors[1]=0.5;
        _defaultColors[2]=0.5;
        _defaultColors[3]=1.0;
        _defaultColors[4]=0.25;
        _defaultColors[5]=0.25;
        _defaultColors[6]=0.25;
        _defaultColors[7]=1.0;
        _defaultColors[8]=0.25;
        _defaultColors[9]=0.25;
        _defaultColors[10]=0.25;
        _defaultColors[11]=1.0;
        _defaultColors[12]=0.0;
        _defaultColors[13]=0.0;
        _defaultColors[14]=0.0;
        _defaultColors[15]=1.0;
    }

    if (colors!=nullptr)
    {
        _colors.resize(ptCnt*3);
        for (int i=0;i<ptCnt;i++)
        {
            _colors[3*i+0]=float(colors[3*i+0])/255.0;
            _colors[3*i+1]=float(colors[3*i+1])/255.0;
            _colors[3*i+2]=float(colors[3*i+2])/255.0;
        }
    }
    if (normals!=nullptr)
        _normals.assign(normals,normals+ptCnt*3);

    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_parentHandle);
    _parentUniqueId=-1;
    if (it!=nullptr)
    {
        _parentUniqueId=it->getObjectUid();
        C7Vector tr(it->getCumulativeTransformation());
        C7Vector trInv(tr.getInverse());
        for (int i=0;i<ptCnt;i++)
        {
            C3Vector v(&_vertices[3*i]);
            v=trInv*v;
            _vertices[3*i+0]=v(0);
            _vertices[3*i+1]=v(1);
            _vertices[3*i+2]=v(2);
            if (_normals.size()!=0)
            {
                C3Vector n(&_normals[3*i]);
                v=trInv.Q*v;
                _normals[3*i+0]=n(0);
                _normals[3*i+1]=n(1);
                _normals[3*i+2]=n(2);
            }
        }
    }
}

CPtCloud_old::~CPtCloud_old()
{
}

bool CPtCloud_old::isPersistent() const
{
    return((_options&1)!=0);
}

void CPtCloud_old::setObjectID(int id)
{
    _id=id;
}

int CPtCloud_old::getObjectID() const
{
    return(_id);
}

void CPtCloud_old::setObjectUniqueId()
{
    _uniqueId=App::getFreshUniqueId();
}

long long int CPtCloud_old::getObjectUniqueId() const
{
    return(_uniqueId);
}

bool CPtCloud_old::announceObjectWillBeErased(int objectHandleAttachedTo)
{ // return value true means: destroy me!
    return(_parentHandle==objectHandleAttachedTo);
}

void CPtCloud_old::draw(int displayAttrib)
{
    if ( ((displayAttrib&sim_displayattribute_forvisionsensor)==0)||((_options&2)==0) )
    {
#ifdef SIM_WITH_GUI
        int currentPage=App::currentWorld->pageContainer->getActivePageIndex();
        int p=1<<currentPage;
        if ( (_pageMask==0) || ((_pageMask&p)!=0) )
#endif
        {
            int currentLayers=App::currentWorld->environment->getActiveLayers();
            if ( ((currentLayers&_layerMask)!=0)&&(_vertices.size()!=0) )
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_parentHandle);
                displayPtCloud_old(this,it);
            }
        }
    }
}

void CPtCloud_old::pushAddEvent()
{
    if (App::worldContainer->getEventsEnabled())
    {
        auto [event,data]=App::worldContainer->prepareEvent(EVENTTYPE_DRAWINGOBJECTADDED,_uniqueId,nullptr,false);

        data->appendMapObject_stringString("type","point",0);

        float c[9];
        c[0]=_defaultColors[0];
        c[1]=_defaultColors[1];
        c[2]=_defaultColors[2];
        c[3]=_defaultColors[8];
        c[4]=_defaultColors[9];
        c[5]=_defaultColors[10];
        c[6]=_defaultColors[12];
        c[7]=_defaultColors[13];
        c[8]=_defaultColors[14];
        data->appendMapObject_stringFloatArray("color",c,9);

        data->appendMapObject_stringInt32("maxCnt",int(_vertices.size()/3));

        data->appendMapObject_stringFloat("size",_pointSize);

        data->appendMapObject_stringInt64("parentUid",_parentUniqueId);

        data->appendMapObject_stringBool("cyclic",false);

        data->appendMapObject_stringBool("clearPoints",true);

        App::worldContainer->pushEvent(event);

        std::tie(event,data)=App::worldContainer->prepareEvent(EVENTTYPE_DRAWINGOBJECTCHANGED,_uniqueId,nullptr,false);

        CCbor obj(nullptr,0);
        size_t l;
        std::vector<float> bla;
        bla.resize(_vertices.size());
        for (size_t i=0;i<_vertices.size();i++)
            bla[i]=(float)_vertices[i];
        obj.appendFloatArray(bla.data(),bla.size());
        const char* buff=(const char*)obj.getBuff(l);
        data->appendMapObject_stringString("points",buff,l,true);

        obj.clear();
        obj.appendFloatArray(_colors.data(),_colors.size());
        buff=(const char*)obj.getBuff(l);
        data->appendMapObject_stringString("colors",buff,l,true);


        data->appendMapObject_stringBool("clearPoints",true);

        App::worldContainer->pushEvent(event);
    }
}
