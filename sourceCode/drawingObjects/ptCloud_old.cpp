
#include "ptCloud_old.h"
#include "app.h"
#include "ptCloudRendering_old.h"

CPtCloud_old::CPtCloud_old(int pageMask,int layerMask,int objectHandle,int options,float pointSize,int ptCnt,const float* vertices,const unsigned char* colors,const float* normals,const unsigned char* defaultColors)
{
    _pageMask=pageMask;
    _layerMask=layerMask;
    _objectHandle=objectHandle;
    _options=options;
    _pointSize=pointSize;
    _vertices.assign(vertices,vertices+ptCnt*3);
    if (defaultColors!=nullptr)
    {
        for (int i=0;i<4;i++)
        {
            _defaultColors[4*i+0]=float(defaultColors[3*i+0])/255.0f;
            _defaultColors[4*i+1]=float(defaultColors[3*i+1])/255.0f;
            _defaultColors[4*i+2]=float(defaultColors[3*i+2])/255.0f;
            _defaultColors[4*i+3]=1.0f;
        }
    }
    else
    {
        _defaultColors[0]=0.5f;
        _defaultColors[1]=0.5f;
        _defaultColors[2]=0.5f;
        _defaultColors[3]=1.0f;
        _defaultColors[4]=0.25f;
        _defaultColors[5]=0.25f;
        _defaultColors[6]=0.25f;
        _defaultColors[7]=1.0f;
        _defaultColors[8]=0.25f;
        _defaultColors[9]=0.25f;
        _defaultColors[10]=0.25f;
        _defaultColors[11]=1.0f;
        _defaultColors[12]=0.0f;
        _defaultColors[13]=0.0f;
        _defaultColors[14]=0.0f;
        _defaultColors[15]=1.0f;
    }

    if (colors!=nullptr)
    {
        _colors.resize(ptCnt*4);
        for (int i=0;i<ptCnt;i++)
        {
            _colors[4*i+0]=float(colors[3*i+0])/255.0f;
            _colors[4*i+1]=float(colors[3*i+1])/255.0f;
            _colors[4*i+2]=float(colors[3*i+2])/255.0f;
            _colors[4*i+3]=1.0f;
        }
    }
    if (normals!=nullptr)
        _normals.assign(normals,normals+ptCnt*3);

    C3DObject* it=App::ct->objCont->getObjectFromHandle(_objectHandle);
    if (it!=nullptr)
    {
        C7Vector tr(it->getCumulativeTransformationPart1_forDisplay(true));
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

bool CPtCloud_old::announceObjectWillBeErased(int objectHandleAttachedTo)
{ // return value true means: destroy me!
    return(_objectHandle==objectHandleAttachedTo);
}

void CPtCloud_old::draw(int displayAttrib)
{
    if ( ((displayAttrib&sim_displayattribute_forvisionsensor)==0)||((_options&2)==0) )
    {
#ifdef SIM_WITH_GUI
        int currentPage=App::ct->pageContainer->getActivePageIndex();
        int p=1<<currentPage;
        if ( (_pageMask==0) || ((_pageMask&p)!=0) )
#endif
        {
            int currentLayers=App::ct->mainSettings->getActiveLayers();
            if ( ((currentLayers&_layerMask)!=0)&&(_vertices.size()!=0) )
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(_objectHandle);
                displayPtCloud_old(this,it);
            }
        }
    }
}
