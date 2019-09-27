
#include "vrepMainHeader.h"
#include "drawingObject2D.h"
#include "tt.h"
#include "v_repConst.h"
#include "drawingObject2DRendering.h"

CDrawingObject2D::CDrawingObject2D(int theObjectType,float* vertices,float col[3],float size,float dir,char* txt)
{
    size=tt::getLimitedFloat(0.001f,10000.0f,size);
    _size=size;
    _dir=dir;
    _objectType=theObjectType;
    _col[0]=col[0];
    _col[1]=col[1];
    _col[2]=col[2];
    int t=_objectType&255;
    if ( (t==sim_drawing2d_square)||(t==sim_drawing2d_text)||(t==sim_drawing2d_circle) )
    {
        for (int i=0;i<2;i++)
            _vertices[i]=vertices[i];
    }
    if (t==sim_drawing2d_line)
    {
        for (int i=0;i<4;i++)
            _vertices[i]=vertices[i];
    }
    if (t==sim_drawing2d_text)
    {
        if (txt!=nullptr)
            _txt=txt;
    }
}

CDrawingObject2D::~CDrawingObject2D()
{
}

void CDrawingObject2D::draw(int minViewPos[2],int maxViewPos[2])
{
    displayDrawingObject2D(this,minViewPos,maxViewPos);
}
