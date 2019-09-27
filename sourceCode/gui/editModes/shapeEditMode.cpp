
#include "shapeEditMode.h"
#include "v_repConst.h"
#include "geometric.h"
#include "global.h"
#include "meshManip.h"
#include "oGL.h"
#include "v_repStringTable.h"
#include "app.h"
#include "rendering.h"
#include "tt.h"

CShapeEditMode::CShapeEditMode(CShape* shape,int editModeType,CObjCont* objCont,CTextureContainer* textureCont,CUiThread* uiThread,bool identicalVerticesCheck,bool identicalTrianglesCheck,float identicalVerticesTolerance)
{
    _shape=shape;
    _editModeType=editModeType;
    _objCont=objCont;
    _textureCont=textureCont;
    _uiThread=uiThread;
    _identicalVerticesCheck=identicalVerticesCheck;
    _identicalTrianglesCheck=identicalTrianglesCheck;
    _identicalVerticesTolerance=identicalVerticesTolerance;
    showHiddenVerticeAndEdges=false;
    automaticallyFollowEdges=true;
    edgeMaxAngle=135.0f*degToRad_f;
    edgeDirectionChangeMaxAngle=45.0f*degToRad_f;

    _shape->geomData->geomInfo->getCumulativeMeshes(_editionVertices,&_editionIndices,&_editionNormals);
    _editionTextureProperty=((CGeometric*)_shape->geomData->geomInfo)->getTextureProperty();
    if (_editionTextureProperty!=nullptr)
    {
        if (!((CGeometric*)_shape->geomData->geomInfo)->getNonCalculatedTextureCoordinates(_editionTextureCoords))
            _editionTextureProperty=nullptr; // texture coordinates are calculated, so we don't care
    }
    if (_editionTextureProperty!=nullptr)
    {
        CTextureObject* to=_textureCont->getObject(_editionTextureProperty->getTextureObjectID());
        if (to!=nullptr)
        {
            to->getTextureBuffer(_editionTexture);
            to->lightenUp();
        }
    }
    if (_editionTextureCoords.size()==0)
        _editionTextureCoords.resize(_editionIndices.size()*2,0.0f); // when we work with no textures, we just use dummy texture coordinates

    actualizeEditModeEditionEdges();
}

CShapeEditMode::~CShapeEditMode()
{
}

bool CShapeEditMode::endEditMode(bool cancelChanges)
{ // return true means: select the edition object, otherwise it was erased
    // delete this object right after calling this function!

    bool retVal=true;

    if (!cancelChanges)
    {
        C7Vector oldTr(_shape->getCumulativeTransformationPart1());
        CGeomProxy* g=_shape->geomData;
        CGeometric* gc=(CGeometric*)g->geomInfo;
        gc->setPurePrimitiveType(sim_pure_primitive_none,1.0f,1.0f,1.0f); // disable the pure characteristic
        CMeshManip::checkVerticesIndicesNormalsTexCoords(_editionVertices,_editionIndices,nullptr,&_editionTextureCoords,_identicalVerticesCheck,_identicalVerticesTolerance,_identicalTrianglesCheck);

        if (_editionVertices.size()!=0)
        { // The shape is not empty
            gc->setMesh(_editionVertices,_editionIndices,nullptr,C7Vector::identityTransformation); // will do the convectivity test
            gc->actualizeGouraudShadingAndVisibleEdges();
            g->removeCollisionInformation();
            // handle textures:
            CTextureProperty* tp=gc->getTextureProperty();
            if (tp!=nullptr)
            {
                if (tp->getFixedCoordinates())
                {
                    if (_editionTextureCoords.size()/2!=_editionIndices.size())
                    { // should normally never happen
                        _textureCont->announceGeneralObjectWillBeErased(_shape->getObjectHandle(),-1);
                        delete tp;
                        gc->setTextureProperty(nullptr);
                    }
                    else
                        tp->setFixedCoordinates(&_editionTextureCoords);
                }
            }

            _shape->alignBoundingBoxWithMainAxis();
            _shape->incrementMemorizedConfigurationValidCounter();
        }
        else
        { // The shape is empty!!! We have to remove it!
            _objCont->eraseObject(_shape,false);
            retVal=false;
        }
    }

    if (_editionTextureProperty!=nullptr)
    { // reset to original texture
        CTextureObject* to=_textureCont->getObject(_editionTextureProperty->getTextureObjectID());
        if (to!=nullptr)
            to->setTextureBuffer(_editionTexture);
    }
    return(retVal);
}

int CShapeEditMode::getEditModeType()
{
    return(_editModeType);
}

CShape* CShapeEditMode::getEditModeShape()
{
    return(_shape);
}

bool CShapeEditMode::getShowHiddenVerticeAndEdges()
{
    return(showHiddenVerticeAndEdges);
}

void CShapeEditMode::setShowHiddenVerticeAndEdges(bool show)
{
    showHiddenVerticeAndEdges=show;
}

bool CShapeEditMode::getAutomaticallyFollowEdges()
{
    return(automaticallyFollowEdges);
}

void CShapeEditMode::setAutomaticallyFollowEdges(bool follow)
{
    automaticallyFollowEdges=follow;
}

float CShapeEditMode::getEdgeMaxAngle()
{
    return(edgeMaxAngle);
}

void CShapeEditMode::setEdgeMaxAngle(float a)
{
    edgeMaxAngle=a;
}

float CShapeEditMode::getEdgeDirectionChangeMaxAngle()
{
    return(edgeDirectionChangeMaxAngle);
}

void CShapeEditMode::setEdgeDirectionChangeMaxAngle(float a)
{
    edgeDirectionChangeMaxAngle=a;
}

int CShapeEditMode::getEditionIndicesSize()
{
    return(int(_editionIndices.size()));
}

int CShapeEditMode::getEditionVerticesSize()
{
    return(int(_editionVertices.size()));
}

int CShapeEditMode::getEditionEdgesSize()
{
    return(int(_edgeCont.allEdges.size()));
}

void CShapeEditMode::swapShapeEditModeType(int theType)
{
    _editModeType=theType;
}

int CShapeEditMode::getEditModeBufferSize()
{
    return(int(editModeBuffer.size()));
}

int CShapeEditMode::getLastEditModeBufferValue()
{
    return(editModeBuffer[editModeBuffer.size()-1]);
}

C3Vector CShapeEditMode::getEditionVertex(int index)
{
    return(C3Vector(&_editionVertices[3*index]));
}

void CShapeEditMode::setEditionVertex(int index,const C3Vector& v)
{
    _editionVertices[3*index+0]=v(0);
    _editionVertices[3*index+1]=v(1);
    _editionVertices[3*index+2]=v(2);
}

void CShapeEditMode::getEditionTriangle(int index,int tri[3])
{
    tri[0]=_editionIndices[3*index+0];
    tri[1]=_editionIndices[3*index+1];
    tri[2]=_editionIndices[3*index+2];
}

void CShapeEditMode::getEditionEdge(int index,int edge[2])
{
    edge[0]=_edgeCont.allEdges[2*index+0];
    edge[1]=_edgeCont.allEdges[2*index+1];
}

int CShapeEditMode::getEditModeBufferValue(int index)
{
    return(editModeBuffer[index]);
}

std::vector<int>* CShapeEditMode::getEditModeBuffer()
{
    return(&editModeBuffer);
}

void CShapeEditMode::deselectEditModeBuffer()
{
    editModeBuffer.clear();
}

void CShapeEditMode::removeItemFromEditModeBuffer(int item)
{
    for (int i=0;i<int(editModeBuffer.size());i++)
    {
        if (editModeBuffer[i]==item)
        {
            editModeBuffer.erase(editModeBuffer.begin()+i);
            break;
        }
    }
}

bool CShapeEditMode::isEditModeItemAValidItem(int item)
{
    if (item<0)
        return(false);
    if (_editModeType&VERTEX_EDIT_MODE)
    {
        if (item>=int(_editionVertices.size())/3)
            return(false);
    }
    if (_editModeType&TRIANGLE_EDIT_MODE)
    {
        if (item>=int(_editionIndices.size())/3)
            return(false);
    }
    if (_editModeType&EDGE_EDIT_MODE)
    {
        if (item>=int(_edgeCont.allEdges.size())/2)
            return(false);
    }
    return(true);
}



void CShapeEditMode::displayVertices(int displayAttrib) // all edit mode routines should go somewhere else!!!
{ // Use only in edit mode!!! Solid is true by default
    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
    if ((displayAttrib&sim_displayattribute_colorcodedpickpass)==0)
    {
        ogl::setMaterialColor(0.5f,0.5f,0.8f,0.25f,0.25f,0.25f,0.0f,0.0f,0.0f);
        glPushAttrib(GL_POLYGON_BIT);


        glPolygonOffset(0.5f,0.0f);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
        glLoadName(-1);

        if (_editionTextureProperty!=nullptr)
            _start3DTextureDisplay(_editionTextureProperty);

        // Draw the filled triangles
        glBegin(GL_TRIANGLES);
        if (_editionTextureProperty!=nullptr)
        {
            for (int i=0;i<int(_editionIndices.size());i++)
            {
                glNormal3fv(&_editionNormals[3*i]);
                glTexCoord2fv(&_editionTextureCoords[2*i]);
                glVertex3fv(&_editionVertices[3*_editionIndices[i]]);
            }
        }
        else
        {
            for (int i=0;i<int(_editionIndices.size());i++)
            {
                glNormal3fv(&_editionNormals[3*i]);
                glVertex3fv(&_editionVertices[3*_editionIndices[i]]);
            }
        }
        glEnd();

        if (_editionTextureProperty!=nullptr)
            _end3DTextureDisplay(_editionTextureProperty);

        glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
        glDisable(GL_POLYGON_OFFSET_FILL);
        if (displayAttrib&sim_displayattribute_renderpass)
        {
            glLoadName(-1);
            glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
            glDisable(GL_CULL_FACE);
            ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
            glBegin(GL_TRIANGLES);
            for (int i=0;i<int(_editionIndices.size());i++)
                glVertex3f(_editionVertices[3*_editionIndices[i]+0],_editionVertices[3*_editionIndices[i]+1],_editionVertices[3*_editionIndices[i]+2]);
            glEnd();
        }


        // We prepare an array indicating which vertices were already drawn:
        std::vector<bool> drawn(_editionVertices.size()/3,false);

        // First we display the selected vertices:
        glPointSize(4.0f);
        if (showHiddenVerticeAndEdges)
        {
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
        }
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorYellow);
        for (int j=0;j<int(editModeBuffer.size());j++)
        {
            int i=editModeBuffer[j];
            glLoadName(i);
            drawn[i]=true;
            if (j==int(editModeBuffer.size())-1)
                ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorWhite); // First selection
            ogl::drawSingle3dPoint(&_editionVertices[3*i],nullptr); // needs to be called individually for each point because glLoadName doesn't work between glBegin and glEnd
        }
        // Now we draw all unselected vertices
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorRed);
        for (int i=0;i<int(_editionVertices.size())/3;i++)
        {
            if (!drawn[i])
            {
                glLoadName(i);
                ogl::drawSingle3dPoint(&_editionVertices[3*i],nullptr); // needs to be called individually for each point because glLoadName doesn't work between glBegin and glEnd
            }
        }
        glDisable(GL_POLYGON_OFFSET_POINT);
        glPointSize(1.0f); // Important to reset it
        glLoadName(-1);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glPopAttrib();
    }
    else
    {
        glPushAttrib(GL_POLYGON_BIT);
        glPolygonOffset(0.5f,0.0f);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
        glDisable(GL_DITHER);
        glColor3ub(0,0,0);

        // Draw the filled triangles
        glBegin(GL_TRIANGLES);
        for (int i=0;i<int(_editionIndices.size());i++)
            glVertex3fv(&_editionVertices[3*_editionIndices[i]]);
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);
        glPointSize(4.0f);
        if (showHiddenVerticeAndEdges)
        {
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
        }

        int j;
        for (int i=0;i<int(_editionVertices.size())/3;i++)
        {
            j=i+1;
            glColor3ub(j&255,(j>>8)&255,(j>>16)&255);
            ogl::drawSingle3dPoint(&_editionVertices[3*i],nullptr);
        }

        glDisable(GL_POLYGON_OFFSET_POINT);
        glPointSize(1.0f); // Important to reset it
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_DITHER);
        glPopAttrib();
    }
}

void CShapeEditMode::displayFaceOrientation(int displayAttrib) // all edit mode routines should go somewhere else!!!
{ // Use only in edit mode!!!
    if ((displayAttrib&sim_displayattribute_colorcodedpickpass)==0)
    {
        glPushAttrib(GL_POLYGON_BIT);
        glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
        glPolygonOffset(0.5f,0.0f); // Second argument set to 0.0 on 2009.01.05 (otherwise strange effects on some graphic cards)
        glEnable(GL_POLYGON_OFFSET_FILL);

        if (_editionTextureProperty!=nullptr)
            _start3DTextureDisplay(_editionTextureProperty);

        // Now we display the inside faces...
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        for (int i=0;i<int(_editionIndices.size())/3;i++)
        {
            glLoadName(i);
            if (alreadyInEditModeBuffer(i))
            {
                if (editModeBuffer[editModeBuffer.size()-1]==i)
                    ogl::setMaterialColor(1.0f,1.0f,1.0f,0.25f,0.25f,0.25f,0.0f,0.0f,0.0f);
                else
                    ogl::setMaterialColor(1.0f,1.0f,0.0f,0.25f,0.25f,0.25f,0.0f,0.0f,0.0f);
            }
            else
                ogl::setMaterialColor(0.8f,0.5f,0.5f,0.25f,0.25f,0.25f,0.0f,0.0f,0.0f);

            glBegin(GL_TRIANGLES);
            if (_editionTextureProperty!=nullptr)
            {
                glNormal3fv(&_editionNormals[9*i+0]);
                glTexCoord2fv(&_editionTextureCoords[6*i+0]);
                glVertex3fv(&_editionVertices[3*_editionIndices[3*i+0]]);

                glNormal3fv(&_editionNormals[9*i+3]);
                glTexCoord2fv(&_editionTextureCoords[6*i+2]);
                glVertex3fv(&_editionVertices[3*_editionIndices[3*i+1]]);

                glNormal3fv(&_editionNormals[9*i+6]);
                glTexCoord2fv(&_editionTextureCoords[6*i+4]);
                glVertex3fv(&_editionVertices[3*_editionIndices[3*i+2]]);
            }
            else
            {
                glNormal3fv(&_editionNormals[9*i+0]);
                glVertex3fv(&_editionVertices[3*_editionIndices[3*i+0]]);

                glNormal3fv(&_editionNormals[9*i+3]);
                glVertex3fv(&_editionVertices[3*_editionIndices[3*i+1]]);

                glNormal3fv(&_editionNormals[9*i+6]);
                glVertex3fv(&_editionVertices[3*_editionIndices[3*i+2]]);
            }
            glEnd();
        }
        // ...and now the outside faces:
        glCullFace(GL_BACK);
        for (int i=0;i<int(_editionIndices.size())/3;i++)
        {
            glLoadName(i);
            if (alreadyInEditModeBuffer(i))
            {
                if (editModeBuffer[editModeBuffer.size()-1]==i)
                    ogl::setMaterialColor(1.0f,1.0f,1.0f,0.25f,0.25f,0.25f,0.0f,0.0f,0.0f);
                else
                    ogl::setMaterialColor(1.0f,1.0f,0.0f,0.25f,0.25f,0.25f,0.0f,0.0f,0.0f);
            }
            else
                ogl::setMaterialColor(0.5f,0.5f,0.8f,0.25f,0.25f,0.25f,0.0f,0.0f,0.0f);

            glBegin(GL_TRIANGLES);
            if (_editionTextureProperty!=nullptr)
            {
                glNormal3fv(&_editionNormals[9*i+0]);
                glTexCoord2fv(&_editionTextureCoords[6*i+0]);
                glVertex3fv(&_editionVertices[3*_editionIndices[3*i+0]]);

                glNormal3fv(&_editionNormals[9*i+3]);
                glTexCoord2fv(&_editionTextureCoords[6*i+2]);
                glVertex3fv(&_editionVertices[3*_editionIndices[3*i+1]]);

                glNormal3fv(&_editionNormals[9*i+6]);
                glTexCoord2fv(&_editionTextureCoords[6*i+4]);
                glVertex3fv(&_editionVertices[3*_editionIndices[3*i+2]]);
            }
            else
            {
                glNormal3fv(&_editionNormals[9*i+0]);
                glVertex3fv(&_editionVertices[3*_editionIndices[3*i+0]]);

                glNormal3fv(&_editionNormals[9*i+3]);
                glVertex3fv(&_editionVertices[3*_editionIndices[3*i+1]]);

                glNormal3fv(&_editionNormals[9*i+6]);
                glVertex3fv(&_editionVertices[3*_editionIndices[3*i+2]]);
            }
            glEnd();
        }

        if (_editionTextureProperty!=nullptr)
            _end3DTextureDisplay(_editionTextureProperty);

        // We display all edges in black:
        glDisable(GL_POLYGON_OFFSET_FILL);
        if (displayAttrib&sim_displayattribute_renderpass)
        {
            glLoadName(-1);
            glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
            glDisable(GL_CULL_FACE);
            ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
            glBegin(GL_TRIANGLES);
            for (int i=0;i<int(_editionIndices.size());i++)
                glVertex3f(_editionVertices[3*_editionIndices[i]+0],_editionVertices[3*_editionIndices[i]+1],_editionVertices[3*_editionIndices[i]+2]);
            glEnd();
        }
        glPopAttrib();
    }
    else
    {
        ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
        glPushAttrib(GL_POLYGON_BIT);
        glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
        glDisable(GL_DITHER);
        unsigned int j;
        for (int i=0;i<int(_editionIndices.size())/3;i++)
        {
            j=i+1;
            glColor3ub(j&255,(j>>8)&255,(j>>16)&255);
            glBegin(GL_TRIANGLES);
            glVertex3fv(&_editionVertices[3*_editionIndices[3*i+0]]);
            glVertex3fv(&_editionVertices[3*_editionIndices[3*i+1]]);
            glVertex3fv(&_editionVertices[3*_editionIndices[3*i+2]]);
            glEnd();
        }
        glEnable(GL_DITHER);
        glPopAttrib();
    }
}

void CShapeEditMode::displayEdgeEditMode(int displayAttrib) // all edit mode routines should go somewhere else!!!
{ // Use only in edit mode!!! Solid is true by default
    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
    if ((displayAttrib&sim_displayattribute_colorcodedpickpass)==0)
    {
        float selColor[3]={1.0f,1.0f,0.0f};
        float nselColor[3]={1.0f,0.0f,0.0f};
        ogl::setMaterialColor(0.5f,0.5f,0.8f,0.25f,0.25f,0.25f,0.0f,0.0f,0.0f);
        glPushAttrib(GL_POLYGON_BIT);


        glPolygonOffset(0.5f,0.0f); // Second argument set to 0.0 on 2009.01.05 (otherwise strange effects on some graphic cards)
        glEnable(GL_POLYGON_OFFSET_FILL); //
        glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
        glLoadName(-1);

        if (_editionTextureProperty!=nullptr)
            _start3DTextureDisplay(_editionTextureProperty);

        // Draw the filled triangles
        glBegin(GL_TRIANGLES);
        if (_editionTextureProperty!=nullptr)
        {
            for (int i=0;i<int(_editionIndices.size());i++)
            {
                glNormal3fv(&_editionNormals[3*i]);
                glTexCoord2fv(&_editionTextureCoords[2*i]);
                glVertex3fv(&_editionVertices[3*_editionIndices[i]]);
            }
        }
        else
        {
            for (int i=0;i<int(_editionIndices.size());i++)
            {
                glNormal3fv(&_editionNormals[3*i]);
                glVertex3fv(&_editionVertices[3*_editionIndices[i]]);
            }
        }
        glEnd();

        if (_editionTextureProperty!=nullptr)
            _end3DTextureDisplay(_editionTextureProperty);

        glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);

        glDisable(GL_POLYGON_OFFSET_FILL);
        if (displayAttrib&sim_displayattribute_renderpass)
        {
            glLoadName(-1);
            glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
            glDisable(GL_CULL_FACE);
            ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
            glBegin(GL_TRIANGLES);
            for (int i=0;i<int(_editionIndices.size());i++)
                glVertex3f(_editionVertices[3*_editionIndices[i]+0],_editionVertices[3*_editionIndices[i]+1],_editionVertices[3*_editionIndices[i]+2]);
            glEnd();
        }
        std::vector<bool> usedEdges(_edgeCont.allEdges.size()/2,false);
        if (displayAttrib&sim_displayattribute_pickpass)
            glLineWidth(6.0f);
        else
            glLineWidth(2.0f);
        if (showHiddenVerticeAndEdges)
        {
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
        }
        // first selected edges:
        ogl::setMaterialColor(sim_colorcomponent_emission,selColor);
        for (int i=0;i<int(editModeBuffer.size());i++)
        {
            int ind[2]={_edgeCont.allEdges[2*editModeBuffer[i]+0],_edgeCont.allEdges[2*editModeBuffer[i]+1]};
            usedEdges[editModeBuffer[i]]=true;
            glLoadName(editModeBuffer[i]);
            ogl::drawSingle3dLine(&_editionVertices[3*ind[0]+0],&_editionVertices[3*ind[1]+0],nullptr);
        }

        if (displayAttrib&sim_displayattribute_pickpass)
            glLineWidth(6.0f);
        else
            glLineWidth(1.0f);

        // Now non-selected edges:
        ogl::setMaterialColor(sim_colorcomponent_emission,nselColor);
        for (int i=0;i<int(_edgeCont.allEdges.size()/2);i++)
        {
            if (!usedEdges[i])
            {
                int ind[2]={_edgeCont.allEdges[2*i+0],_edgeCont.allEdges[2*i+1]};
                glLoadName(i);
                ogl::drawSingle3dLine(&_editionVertices[3*ind[0]+0],&_editionVertices[3*ind[1]+0],nullptr);
            }
        }

        glLineWidth(1.0f);

        glLoadName(-1);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glPopAttrib();
    }
    else
    {
        glPushAttrib(GL_POLYGON_BIT);
        glPolygonOffset(0.5f,0.0f); // Second argument set to 0.0 on 2009.01.05 (otherwise strange effects on some graphic cards)
        glEnable(GL_POLYGON_OFFSET_FILL); //
        glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
        glDisable(GL_DITHER);
        glColor3ub(0,0,0);

        // Draw the filled triangles
        glBegin(GL_TRIANGLES);
        for (int i=0;i<int(_editionIndices.size());i++)
            glVertex3fv(&_editionVertices[3*_editionIndices[i]]);
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);
        glLineWidth(6.0f);
        if (showHiddenVerticeAndEdges)
        {
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
        }

        int j;
        for (int i=0;i<int(_edgeCont.allEdges.size()/2);i++)
        {
            int ind[2]={_edgeCont.allEdges[2*i+0],_edgeCont.allEdges[2*i+1]};
            j=i+1;
            glColor3ub(j&255,(j>>8)&255,(j>>16)&255);
            ogl::drawSingle3dLine(&_editionVertices[3*ind[0]+0],&_editionVertices[3*ind[1]+0],nullptr);
        }

        glLineWidth(1.0f);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_DITHER);
        glPopAttrib();
    }
}


void CShapeEditMode::flipTriangle(int index) // all edit mode routines should go somewhere else!!!
{ // Use only in edit mode!!
    int save;
    float normSave;
    save=_editionIndices[3*index+0];
    _editionIndices[3*index+0]=_editionIndices[3*index+1];
    _editionIndices[3*index+1]=save;
    // We have to report the changes to the _editionIndicesToEdgesIndex:
    save=_editionIndicesToEdgesIndex[3*index+0];
    _editionIndicesToEdgesIndex[3*index+0]=_editionIndicesToEdgesIndex[3*index+1];
    _editionIndicesToEdgesIndex[3*index+1]=save;
    // Now the normals:
    normSave=-_editionNormals[3*(3*index+0)+0];
    _editionNormals[3*(3*index+0)+0]=-_editionNormals[3*(3*index+1)+0];
    _editionNormals[3*(3*index+2)+0]=-_editionNormals[3*(3*index+2)+0];
    _editionNormals[3*(3*index+1)+0]=normSave;
    normSave=-_editionNormals[3*(3*index+0)+1];
    _editionNormals[3*(3*index+0)+1]=-_editionNormals[3*(3*index+1)+1];
    _editionNormals[3*(3*index+2)+1]=-_editionNormals[3*(3*index+2)+1];
    _editionNormals[3*(3*index+1)+1]=normSave;
    normSave=-_editionNormals[3*(3*index+0)+2];
    _editionNormals[3*(3*index+0)+2]=-_editionNormals[3*(3*index+1)+2];
    _editionNormals[3*(3*index+2)+2]=-_editionNormals[3*(3*index+2)+2];
    _editionNormals[3*(3*index+1)+2]=normSave;
}

void CShapeEditMode::addTriangle(int ind1,int ind2,int ind3) // all edit mode routines should go somewhere else!!!
{ // Use only in edit mode!!
    _editionIndices.push_back(ind1);
    _editionIndices.push_back(ind2);
    _editionIndices.push_back(ind3);
    int index=_edgeCont.getEdgeIndex(ind1,ind2);
    if (index==-1)
    { // edge doesn't not yet exist
        index=_edgeCont.addEdge(ind1,ind2);
    }
    _editionIndicesToEdgesIndex.push_back(index);
    index=_edgeCont.getEdgeIndex(ind1,ind3);
    if (index==-1)
    { // edge doesn't not yet exist
        index=_edgeCont.addEdge(ind1,ind3);
    }
    _editionIndicesToEdgesIndex.push_back(index);
    index=_edgeCont.getEdgeIndex(ind3,ind2);
    if (index==-1)
    { // edge doesn't not yet exist
        index=_edgeCont.addEdge(ind3,ind2);
    }
    _editionIndicesToEdgesIndex.push_back(index);
    float v[3][3];
    v[0][0]=_editionVertices[3*ind1+0];
    v[0][1]=_editionVertices[3*ind1+1];
    v[0][2]=_editionVertices[3*ind1+2];
    v[1][0]=_editionVertices[3*ind2+0];
    v[1][1]=_editionVertices[3*ind2+1];
    v[1][2]=_editionVertices[3*ind2+2];
    v[2][0]=_editionVertices[3*ind3+0];
    v[2][1]=_editionVertices[3*ind3+1];
    v[2][2]=_editionVertices[3*ind3+2];
    float n[3];
    CMeshManip::calcNormal(v,n);
    for (int i=0;i<3;i++)
    {
        _editionNormals.push_back(n[0]);
        _editionNormals.push_back(n[1]);
        _editionNormals.push_back(n[2]);
    }
    for (int i=0;i<6;i++)
        _editionTextureCoords.push_back(0.0f);
}

void CShapeEditMode::actualizeEditModeEditionEdges() // all edit mode routines should go somewhere else!!!
{
    _edgeCont.clearEdges();
    _editionIndicesToEdgesIndex.clear();
    for (int i=0;i<int(_editionIndices.size()/3);i++)
    {
        int ind[3]={_editionIndices[3*i+0],_editionIndices[3*i+1],_editionIndices[3*i+2]};
        int index=_edgeCont.getEdgeIndex(ind[0],ind[1]);
        if (index==-1)
        { // edge doesn't yet exist
            index=_edgeCont.addEdge(ind[0],ind[1]);
        }
        _editionIndicesToEdgesIndex.push_back(index);
        index=_edgeCont.getEdgeIndex(ind[0],ind[2]);
        if (index==-1)
        { // edge doesn't yet exist
            index=_edgeCont.addEdge(ind[0],ind[2]);
        }
        _editionIndicesToEdgesIndex.push_back(index);
        index=_edgeCont.getEdgeIndex(ind[2],ind[1]);
        if (index==-1)
        { // edge doesn't yet exist
            index=_edgeCont.addEdge(ind[2],ind[1]);
        }
        _editionIndicesToEdgesIndex.push_back(index);
    }
}

float CShapeEditMode::getEdgeAngle(int edgeID) // all edit mode routines should go somewhere else!!!
{
    int ev[2]={_edgeCont.allEdges[2*edgeID+0],_edgeCont.allEdges[2*edgeID+1]};
    std::vector<C3Vector> normalVectors;
    for (int i=0;i<int(_editionIndices.size()/3);i++)
    {
        int ind[3]={_editionIndices[3*i+0],_editionIndices[3*i+1],_editionIndices[3*i+2]};
        if (    ((ev[0]==ind[0])&&(ev[1]==ind[1]))||
                ((ev[1]==ind[0])&&(ev[0]==ind[1]))||
                ((ev[0]==ind[0])&&(ev[1]==ind[2]))||
                ((ev[1]==ind[0])&&(ev[0]==ind[2]))||
                ((ev[0]==ind[1])&&(ev[1]==ind[2]))||
                ((ev[1]==ind[1])&&(ev[0]==ind[2])) )
        { // that triangle shares the edge!
            // We search for the vertex that is not contained in the edge:
            int ne=-1;
            for (int j=0;j<3;j++)
            {
                if ( (ind[j]!=ev[0])&&(ind[j]!=ev[1]) )
                {
                    ne=j;
                    break;
                }
            }
            C3Vector v0(&_editionVertices[3*ev[0]+0]);
            C3Vector v1(&_editionVertices[3*ev[1]+0]);
            C3Vector v2(&_editionVertices[3*ind[ne]+0]);
            C3Vector dv0(v1-v0);
            C3Vector dv1(v2-v0);
            C3Vector n(dv1^dv0);
            n.normalize();
            normalVectors.push_back(n);
        }
    }
    if (normalVectors.size()<2)
        return(piValue_f);
    // now we search for the smallest scalar product (that gives the smallest edge value):
    float smallestScalarProduct=1.0f; // corresponds to 0 degree edge
    for (int i=0;i<int(normalVectors.size()-1);i++)
    {
        C3Vector v(normalVectors[i]);
        for (int j=i+1;j<int(normalVectors.size());j++)
        {
            C3Vector w(normalVectors[j]);
            float sp=fabs(v*w);
            if (sp<smallestScalarProduct)
                smallestScalarProduct=sp;
        }
    }
    return(CMath::robustAcos(smallestScalarProduct));
}

void CShapeEditMode::getNextEdges(int rearVertex,int frontVertex,std::vector<int>& edgeList) // all edit mode routines should go somewhere else!!!
{
    edgeList.clear();
    std::vector<bool> alreadyTaken(_edgeCont.allEdges.size()/2,false);
    for (int i=0;i<int(_editionIndices.size())/3;i++)
    {
        int ind[3]={_editionIndices[3*i+0],_editionIndices[3*i+1],_editionIndices[3*i+2]};
        if (ind[0]==frontVertex)
        {
            if ( (ind[1]!=rearVertex)&&(!alreadyTaken[_editionIndicesToEdgesIndex[3*i+0]]) )
            {
                edgeList.push_back(_editionIndicesToEdgesIndex[3*i+0]);
                alreadyTaken[_editionIndicesToEdgesIndex[3*i+0]]=true;
            }
            if ( (ind[2]!=rearVertex)&&(!alreadyTaken[_editionIndicesToEdgesIndex[3*i+1]]) )
            {
                edgeList.push_back(_editionIndicesToEdgesIndex[3*i+1]);
                alreadyTaken[_editionIndicesToEdgesIndex[3*i+1]]=true;
            }
        }
        else if (ind[1]==frontVertex)
        {
            if ( (ind[0]!=rearVertex)&&(!alreadyTaken[_editionIndicesToEdgesIndex[3*i+0]]) )
            {
                edgeList.push_back(_editionIndicesToEdgesIndex[3*i+0]);
                alreadyTaken[_editionIndicesToEdgesIndex[3*i+0]]=true;
            }
            if ( (ind[2]!=rearVertex)&&(!alreadyTaken[_editionIndicesToEdgesIndex[3*i+2]]) )
            {
                edgeList.push_back(_editionIndicesToEdgesIndex[3*i+2]);
                alreadyTaken[_editionIndicesToEdgesIndex[3*i+2]]=true;
            }
        }
        else if (ind[2]==frontVertex)
        {
            if ( (ind[0]!=rearVertex)&&(!alreadyTaken[_editionIndicesToEdgesIndex[3*i+1]]) )
            {
                edgeList.push_back(_editionIndicesToEdgesIndex[3*i+1]);
                alreadyTaken[_editionIndicesToEdgesIndex[3*i+1]]=true;
            }
            if ( (ind[1]!=rearVertex)&&(!alreadyTaken[_editionIndicesToEdgesIndex[3*i+2]]) )
            {
                edgeList.push_back(_editionIndicesToEdgesIndex[3*i+2]);
                alreadyTaken[_editionIndicesToEdgesIndex[3*i+2]]=true;
            }
        }
    }
}

void CShapeEditMode::selectionFromTriangleToVertexEditMode(std::vector<int>* newVertexSel)
{
    // newVertexSel is nullptr by default. In that case, the change is made in the editModeBuffer
    bool inPlace=false;
    if (newVertexSel==nullptr)
    {
        newVertexSel=new std::vector<int>;
        inPlace=true;
    }
    newVertexSel->reserve(editModeBuffer.size());
    newVertexSel->clear();
    std::vector<bool> used(_editionVertices.size(),false);
    for (int i=0;i<int(editModeBuffer.size());i++)
    {
        for (int j=0;j<3;j++)
        {
            int vertInd=_editionIndices[3*editModeBuffer[i]+j];
            if (!used[vertInd])
            {
                newVertexSel->push_back(vertInd);
                used[vertInd]=true;
            }
        }
    }

    if (inPlace)
    {
        editModeBuffer.reserve(newVertexSel->size());
        editModeBuffer.clear();
        editModeBuffer.insert(editModeBuffer.begin(),newVertexSel->begin(),newVertexSel->end());
        newVertexSel->clear();
        delete newVertexSel;
    }
}

void CShapeEditMode::selectionFromTriangleToEdgeEditMode(std::vector<int>* newEdgeSel)
{
    // newVertexSel is nullptr by default. In that case, the change is made in the editModeBuffer
    bool inPlace=false;
    if (newEdgeSel==nullptr)
    {
        newEdgeSel=new std::vector<int>;
        inPlace=true;
    }
    newEdgeSel->clear();
    std::vector<bool> used(_edgeCont.allEdges.size(),false);
    for (int i=0;i<int(editModeBuffer.size());i++)
    {
        for (int j=0;j<3;j++)
        {
            int edgeInd=_editionIndicesToEdgesIndex[3*editModeBuffer[i]+j];
            if (!used[edgeInd])
            {
                newEdgeSel->push_back(edgeInd);
                used[edgeInd]=true;
            }
        }
    }
    if (inPlace)
    {
        editModeBuffer.reserve(newEdgeSel->size());
        editModeBuffer.clear();
        editModeBuffer.insert(editModeBuffer.begin(),newEdgeSel->begin(),newEdgeSel->end());
        newEdgeSel->clear();
        delete newEdgeSel;
    }
}

void CShapeEditMode::selectionFromEdgeToVertexEditMode(std::vector<int>* newVertexSel)
{
    // newVertexSel is nullptr by default. In that case, the change is made in the editModeBuffer
    bool inPlace=false;
    if (newVertexSel==nullptr)
    {
        newVertexSel=new std::vector<int>;
        inPlace=true;
    }
    newVertexSel->clear();
    std::vector<bool> used(_editionVertices.size(),false);
    for (int i=0;i<int(editModeBuffer.size());i++)
    {
        for (int j=0;j<2;j++)
        {
            int vertInd=_edgeCont.allEdges[2*editModeBuffer[i]+j];
            if (!used[vertInd])
            {
                newVertexSel->push_back(vertInd);
                used[vertInd]=true;
            }
        }
    }

    if (inPlace)
    {
        editModeBuffer.reserve(newVertexSel->size());
        editModeBuffer.clear();
        editModeBuffer.insert(editModeBuffer.begin(),newVertexSel->begin(),newVertexSel->end());
        newVertexSel->clear();
        delete newVertexSel;
    }
}

void CShapeEditMode::selectionFromVertexToTriangleEditMode(std::vector<int>* newTriangleSel)
{
    // newTriangleSel is nullptr by default. In that case, the change is made in the editModeBuffer
    std::vector<bool> used(_editionVertices.size(),false);
    for (int i=0;i<int(editModeBuffer.size());i++)
        used[editModeBuffer[i]]=true;
    if (newTriangleSel==nullptr)
    {
        editModeBuffer.reserve(3*editModeBuffer.size()); // Max size
        editModeBuffer.clear();
    }
    else
    {
        newTriangleSel->reserve(3*editModeBuffer.size()); // Max size
        newTriangleSel->clear();
    }
    for (int i=0;i<int(_editionIndices.size())/3;i++)
    {
        if (used[_editionIndices[3*i+0]]&&
            used[_editionIndices[3*i+1]]&&
            used[_editionIndices[3*i+2]] )
        {
            if (newTriangleSel==nullptr)
                editModeBuffer.push_back(i);
            else
                newTriangleSel->push_back(i);
        }
    }
}

void CShapeEditMode::selectionFromVertexToEdgeEditMode(std::vector<int>* newEdgeSel)
{
    // newEdgeSel is nullptr by default. In that case, the change is made in the editModeBuffer
    std::vector<bool> used(_editionVertices.size(),false);
    for (int i=0;i<int(editModeBuffer.size());i++)
        used[editModeBuffer[i]]=true;
    if (newEdgeSel==nullptr)
    {
        editModeBuffer.reserve(3*editModeBuffer.size()); // Max size
        editModeBuffer.clear();
    }
    else
    {
        newEdgeSel->reserve(3*editModeBuffer.size()); // Max size
        newEdgeSel->clear();
    }
    for (int i=0;i<int(_edgeCont.allEdges.size())/2;i++)
    {
        if (used[_edgeCont.allEdges[2*i+0]]&&
            used[_edgeCont.allEdges[2*i+1]] )
        {
            if (newEdgeSel==nullptr)
                editModeBuffer.push_back(i);
            else
                newEdgeSel->push_back(i);
        }
    }
}

void CShapeEditMode::selectionFromEdgeToTriangleEditMode(std::vector<int>* newTriangleSel)
{
    // newTriangleSel is nullptr by default. In that case, the change is made in the editModeBuffer
    std::vector<bool> used(_edgeCont.allEdges.size(),false);
    for (int i=0;i<int(editModeBuffer.size());i++)
        used[editModeBuffer[i]]=true;
    if (newTriangleSel==nullptr)
        editModeBuffer.clear();
    else
        newTriangleSel->clear();
    for (int i=0;i<int(_editionIndicesToEdgesIndex.size())/3;i++)
    {
        if (used[_editionIndicesToEdgesIndex[3*i+0]]&&
            used[_editionIndicesToEdgesIndex[3*i+1]]&&
            used[_editionIndicesToEdgesIndex[3*i+2]] )
        {
            if (newTriangleSel==nullptr)
                editModeBuffer.push_back(i);
            else
                newTriangleSel->push_back(i);
        }
    }
}

void CShapeEditMode::xorAddItemToEditModeBuffer(int item,bool disableEdgeFollowing)
{
    if (isEditModeItemAValidItem(item))
    {
        if (!alreadyInEditModeBuffer(item))
            addItemToEditModeBuffer(item,disableEdgeFollowing);
        else
            removeItemFromEditModeBuffer(item);
    }
    else
        editModeBuffer.clear();
}

void CShapeEditMode::addItemToEditModeBuffer(int item,bool disableEdgeFollowing)
{
    if (isEditModeItemAValidItem(item))
    {
        if (!alreadyInEditModeBuffer(item))
        {
            editModeBuffer.push_back(item);
            while (automaticallyFollowEdges&&(_editModeType&EDGE_EDIT_MODE)&&(!disableEdgeFollowing))
            {
                int lowVertex;
                int highVertex;
                if (editModeBuffer.size()==1)
                {
                    lowVertex=_edgeCont.allEdges[2*item+0];
                    highVertex=_edgeCont.allEdges[2*item+1];
                }
                else
                {
                    int prevItem=editModeBuffer[editModeBuffer.size()-2];
                    if ( (_edgeCont.allEdges[2*item+0]==_edgeCont.allEdges[2*prevItem+0])||
                        (_edgeCont.allEdges[2*item+0]==_edgeCont.allEdges[2*prevItem+1]) )
                    {
                        lowVertex=_edgeCont.allEdges[2*item+0];
                        highVertex=_edgeCont.allEdges[2*item+1];
                    }
                    else
                    {
                        lowVertex=_edgeCont.allEdges[2*item+1];
                        highVertex=_edgeCont.allEdges[2*item+0];
                    }
                }
                std::vector<int> nextEdges;
                getNextEdges(lowVertex,highVertex,nextEdges);
                int highestAngleIndex=0;
                float highestAngleValue=0.0f;
                bool foundValid=false;
                for (int i=0;i<int(nextEdges.size());i++)
                {
                    // 1. We need the edge angle:
                    float a=getEdgeAngle(nextEdges[i]);
                    // 2. and we need the angle between previous and next edge: (added on 2009/03/14)
                    int ind[2]={_edgeCont.allEdges[2*nextEdges[i]+0],_edgeCont.allEdges[2*nextEdges[i]+1]};
                    if (ind[1]==highVertex)
                        ind[1]=ind[0];
                    C3Vector v0(&_editionVertices[3*lowVertex+0]);
                    C3Vector v1(&_editionVertices[3*highVertex+0]);
                    C3Vector v2(&_editionVertices[3*ind[1]+0]);
                    float a2=(v1-v0).getAngle(v2-v1);
                    if ( (a>highestAngleValue)&&(a2<edgeDirectionChangeMaxAngle)&&(a>(piValue_f-edgeMaxAngle)) ) // a>20.0... added on 2009/05/08
                    {
                        highestAngleValue=a;
                        highestAngleIndex=i;
                        foundValid=true;
                    }
                }
                if (!foundValid)
                    break; // we didn't find an appropriate edge
                item=nextEdges[highestAngleIndex];
                int ind[2]={_edgeCont.allEdges[2*item+0],_edgeCont.allEdges[2*item+1]};
                int newVertex=ind[0];
                if (newVertex==highVertex)
                    newVertex=ind[1];
                // We make sure that this edge is not yet present in the selection buffer:
                if (alreadyInEditModeBuffer(item))
                    break;
                // now we check if that vertex already exists in the selection:
                bool alreadyExists=false;
                for (int i=0;i<int(editModeBuffer.size());i++)
                {
                    ind[0]=_edgeCont.allEdges[2*editModeBuffer[i]+0];
                    ind[1]=_edgeCont.allEdges[2*editModeBuffer[i]+1];
                    if ( (ind[0]==newVertex)||(ind[1]==newVertex) )
                    {
                        alreadyExists=true;
                        break;
                    }
                }
                editModeBuffer.push_back(item);
                if (alreadyExists)
                    break;
            }
        }
    }
}

bool CShapeEditMode::alreadyInEditModeBuffer(int item)
{
    for (int i=0;i<int(editModeBuffer.size());i++)
        if (editModeBuffer[i]==item)
            return(true);
    return(false);
}

void CShapeEditMode::copySelection(std::vector<int>* selection)
{
    if (_editModeType&TRIANGLE_EDIT_MODE)
    { // We have to copy the selected triangles with their vertices
        copySelectedFaces(selection,&editBufferVerticesCopy,
                            &editBufferIndicesCopy,
                            &editBufferNormalsCopy,
                            &editBufferTextureCoordsCopy);
        deselectEditModeBuffer();
    }

    if (_editModeType&VERTEX_EDIT_MODE)
    { // We copy only the vertices
        editBufferVerticesCopy.reserve(selection->size()*3);
        editBufferVerticesCopy.clear();
        editBufferIndicesCopy.clear();
        editBufferNormalsCopy.clear();
        editBufferTextureCoordsCopy.clear();
        for (int i=0;i<int(selection->size());i++)
        {
            int index=selection->at(i);
            editBufferVerticesCopy.push_back(_editionVertices[3*index+0]);
            editBufferVerticesCopy.push_back(_editionVertices[3*index+1]);
            editBufferVerticesCopy.push_back(_editionVertices[3*index+2]);
        }
        deselectEditModeBuffer();
    }
}

void CShapeEditMode::paste()
{
    if (_editModeType&TRIANGLE_EDIT_MODE)
    {
        deselectEditModeBuffer();
        int startInd=(int)_editionVertices.size()/3;
        for (size_t i=0;i<editBufferVerticesCopy.size();i++)
            _editionVertices.push_back(editBufferVerticesCopy[i]);
        int orSize=(int)_editionIndices.size()/3;
        for (size_t i=0;i<editBufferIndicesCopy.size()/3;i++)
        {
            int ind[3];
            ind[0]=editBufferIndicesCopy[3*i+0]+startInd;
            ind[1]=editBufferIndicesCopy[3*i+1]+startInd;
            ind[2]=editBufferIndicesCopy[3*i+2]+startInd;
            _editionIndices.push_back(ind[0]);
            _editionIndices.push_back(ind[1]);
            _editionIndices.push_back(ind[2]);
            int index=_edgeCont.getEdgeIndex(ind[0],ind[1]);
            if (index==-1)
            { // edge doesn't yet exist
                index=_edgeCont.addEdge(ind[0],ind[1]);
            }
            _editionIndicesToEdgesIndex.push_back(index);
            index=_edgeCont.getEdgeIndex(ind[0],ind[2]);
            if (index==-1)
            { // edge doesn't yet exist
                index=_edgeCont.addEdge(ind[0],ind[2]);
            }
            _editionIndicesToEdgesIndex.push_back(index);
            index=_edgeCont.getEdgeIndex(ind[2],ind[1]);
            if (index==-1)
            { // edge doesn't yet exist
                index=_edgeCont.addEdge(ind[2],ind[1]);
            }
            _editionIndicesToEdgesIndex.push_back(index);
            addItemToEditModeBuffer(orSize+(int)i,true);
        }
        for (size_t i=0;i<editBufferNormalsCopy.size();i++)
            _editionNormals.push_back(editBufferNormalsCopy[i]);
        for (size_t i=0;i<editBufferTextureCoordsCopy.size();i++)
            _editionTextureCoords.push_back(editBufferTextureCoordsCopy[i]);
    }

    if (_editModeType&VERTEX_EDIT_MODE)
    {
        deselectEditModeBuffer();
        int startInd=(int)_editionVertices.size()/3;
        for (int i=0;i<int(editBufferVerticesCopy.size())/3;i++)
        {
            _editionVertices.push_back(editBufferVerticesCopy[3*i+0]);
            _editionVertices.push_back(editBufferVerticesCopy[3*i+1]);
            _editionVertices.push_back(editBufferVerticesCopy[3*i+2]);
            addItemToEditModeBuffer(startInd,true);
            startInd++;
        }
    }
}

void CShapeEditMode::cutSelection(std::vector<int>* selection)
{
    copySelection(selection);
    deleteSelection(selection);
}

void CShapeEditMode::deleteSelection(std::vector<int>* selection)
{
    if (_editModeType&TRIANGLE_EDIT_MODE)
    {
        while (selection->size()!=0)
        { // We have to remove the triangles from the highest index to the lowest:
            int max=0;
            int maxIndex=0;
            for (int i=0;i<int(selection->size());i++)
            {
                if (selection->at(i)>max)
                {
                    max=selection->at(i);
                    maxIndex=i;
                }
            }
            _editionIndices.erase(_editionIndices.begin()+3*max,_editionIndices.begin()+3*max+3);
            _editionNormals.erase(_editionNormals.begin()+9*max,_editionNormals.begin()+9*max+9);
            _editionTextureCoords.erase(_editionTextureCoords.begin()+6*max,_editionTextureCoords.begin()+6*max+6);
            selection->erase(selection->begin()+maxIndex);
        }
        actualizeEditModeEditionEdges();
        deselectEditModeBuffer();
    }

    if (_editModeType&VERTEX_EDIT_MODE)
    {
        while (selection->size()!=0)
        { // We have to remove the selected vertices. We don't remove them now,
            // we just set the x value to > 1'000'000. We then don't display vertices
            // > than that value.
            int max=0;
            int maxIndex=0;
            for (int i=0;i<int(selection->size());i++)
            {
                if (selection->at(i)>max)
                {
                    max=selection->at(i);
                    maxIndex=i;
                }
            }
            _editionVertices.erase(_editionVertices.begin()+3*max,_editionVertices.begin()+3*max+3);
            // We have to decrease all value higher than max in the indices.
            // Indices pointing to max are set to -1:
            for (int i=0;i<int(_editionIndices.size());i++)
            {
                if (_editionIndices[i]==max)
                    _editionIndices[i]=-1;
                if (_editionIndices[i]>max)
                    _editionIndices[i]--;
            }
            selection->erase(selection->begin()+maxIndex);
        }
        // Now we remove all indices/normals where one index is -1:
        int i=0;
        while (i<int(_editionIndices.size())/3)
        {
            if ( (_editionIndices[3*i+0]==-1)||
                (_editionIndices[3*i+1]==-1) ||
                (_editionIndices[3*i+2]==-1) )
            {
                _editionIndices.erase(_editionIndices.begin()+3*i,_editionIndices.begin()+3*i+3);
                _editionNormals.erase(_editionNormals.begin()+9*i,_editionNormals.begin()+9*i+9);
                _editionTextureCoords.erase(_editionTextureCoords.begin()+6*i,_editionTextureCoords.begin()+6*i+6);
            }
            else
                i++;
        }
        actualizeEditModeEditionEdges();
        deselectEditModeBuffer();
    }

    if (_editModeType&EDGE_EDIT_MODE)
    {
        // We prepare an array indicating which edge needs to be removed:
        std::vector<bool> edgesWhichNeedToBeRemoved(_edgeCont.allEdges.size(),false);
        for (int i=0;i<int(editModeBuffer.size());i++)
            edgesWhichNeedToBeRemoved[editModeBuffer[i]]=true;
        // We now go through all triangles (from highest to lowest) and remove the ones that have a listed edge:
        for (int i=int((_editionIndices.size())/3)-1;i>=0;i--)
        {
            int ind[3];
            ind[0]=_editionIndicesToEdgesIndex[3*i+0];
            ind[1]=_editionIndicesToEdgesIndex[3*i+1];
            ind[2]=_editionIndicesToEdgesIndex[3*i+2];
            if (edgesWhichNeedToBeRemoved[ind[0]] ||
                edgesWhichNeedToBeRemoved[ind[1]] ||
                edgesWhichNeedToBeRemoved[ind[2]] )
            { // needs to be removed
                _editionIndices.erase(_editionIndices.begin()+3*i,_editionIndices.begin()+3*i+3);
                _editionNormals.erase(_editionNormals.begin()+9*i,_editionNormals.begin()+9*i+9);
                _editionTextureCoords.erase(_editionTextureCoords.begin()+6*i,_editionTextureCoords.begin()+6*i+6);
            }
        }
        actualizeEditModeEditionEdges();
        deselectEditModeBuffer();
    }
}

void CShapeEditMode::copySelectedFaces(std::vector<int>* sel,std::vector<float>* vert,std::vector<int>* ind,std::vector<float>* norm,std::vector<float>* tex)
{  // norm or tex can be nullptr
    if (_editModeType&TRIANGLE_EDIT_MODE)
    {
        std::vector<int> vertInd;
        vertInd.reserve(sel->size()*3);// max value
        vertInd.clear();
        // We add only once every vertex:
        for (int i=0;i<int(sel->size());i++)
        {
            for (int j=0;j<3;j++)
            {
                int val=_editionIndices[3*sel->at(i)+j];
                // We check if already present:
                bool present=false;
                for (int k=0;k<int(vertInd.size());k++)
                {
                    if (vertInd[k]==val)
                    {
                        present=true;
                        break;
                    }
                }
                if (!present)
                    vertInd.push_back(val);
            }
        }
        vert->reserve(3*vertInd.size());
        vert->clear();
        ind->reserve(3*sel->size());
        ind->clear();
        if (norm!=nullptr)
        {
            norm->reserve(9*sel->size());
            norm->clear();
        }
        if (tex!=nullptr)
        {
            tex->reserve(6*sel->size());
            tex->clear();
        }
        // We copy the vertices:
        for (int i=0;i<int(vertInd.size());i++)
        {
            for (int j=0;j<3;j++)
                vert->push_back(_editionVertices[3*vertInd[i]+j]);
        }
        // We copy the indices and normals and tex coords:
        for (int i=0;i<int(sel->size());i++)
        {
            for (int j=0;j<3;j++)
            {
                int oldIndex=_editionIndices[3*sel->at(i)+j];
                // We have to find the new index:
                int newIndex=0;
                for (int k=0;k<int(vertInd.size());k++)
                {
                    if (vertInd[k]==oldIndex)
                    {
                        newIndex=k;
                        break;
                    }
                }
                ind->push_back(newIndex);
                if (norm!=nullptr)
                {
                    norm->push_back(_editionNormals[9*sel->at(i)+3*j+0]);
                    norm->push_back(_editionNormals[9*sel->at(i)+3*j+1]);
                    norm->push_back(_editionNormals[9*sel->at(i)+3*j+2]);
                }
                if (tex!=nullptr)
                {
                    tex->push_back(_editionTextureCoords[6*sel->at(i)+2*j+0]);
                    tex->push_back(_editionTextureCoords[6*sel->at(i)+2*j+1]);
                }
            }
        }
    }
}

void CShapeEditMode::addMenu(VMenu* menu)
{
    int selSize=getEditModeBufferSize();
    int buffSize=(int)editBufferVerticesCopy.size();
    bool triangleEditMode=((_editModeType&TRIANGLE_EDIT_MODE)!=0);
    bool vertexEditMode=((_editModeType&VERTEX_EDIT_MODE)!=0);
    bool edgeEditMode=((_editModeType&EDGE_EDIT_MODE)!=0);

    menu->appendMenuItem(true,triangleEditMode,SHAPE_EDIT_MODE_SWAP_TO_TRIANGLE_EDIT_MODE_EMCMD,IDS_TRIANGLE_EDIT_MODE_MENU_ITEM,true);
    menu->appendMenuItem(true,vertexEditMode,SHAPE_EDIT_MODE_SWAP_TO_VERTEX_EDIT_MODE_EMCMD,IDS_VERTEX_EDIT_MODE_MENU_ITEM,true);
    menu->appendMenuItem(true,edgeEditMode,SHAPE_EDIT_MODE_SWAP_TO_EDGE_EDIT_MODE_EMCMD,IDS_EDGE_EDIT_MODE_MENU_ITEM,true);

    menu->appendMenuSeparator();

    if (_editModeType&VERTEX_EDIT_MODE)
    {
        menu->appendMenuItem(true,showHiddenVerticeAndEdges,SHAPE_EDIT_MODE_TOGGLE_HIDDEN_VERTICE_EDGE_SHOW_EMCMD,IDS_SHOW_HIDDEN_VERTICES_MENU_ITEM,true);

        menu->appendMenuSeparator();

        menu->appendMenuItem(selSize>0,false,SHAPE_EDIT_MODE_TRIANGLE_VERTEX_COPY_EMCMD,IDS_COPY_SELECTED_VERTICES_MENU_ITEM);
        menu->appendMenuItem(buffSize>0,false,SHAPE_EDIT_MODE_PASTE_TRIANGLE_VERTEX_EMCMD,IDS_PASTE_VERTICES_MENU_ITEM);
        menu->appendMenuItem(selSize>0,false,SHAPE_EDIT_MODE_DELETE_TRIANGLE_VERTEX_EDGE_EMCMD,IDS_DELETE_SELECTED_VERTICES_MENU_ITEM);
        menu->appendMenuItem(selSize>0,false,SHAPE_EDIT_MODE_TRIANGLE_VERTEX_CUT_EMCMD,IDS_CUT_SELECTED_VERTICES_MENU_ITEM);

        menu->appendMenuSeparator();

        menu->appendMenuItem(selSize>=3,false,SHAPE_EDIT_MODE_INSERT_TRIANGLES_EMCMD,IDS_INSERT_TRIANGLES_MENU_ITEM);
        menu->appendMenuItem(selSize>=3,false,SHAPE_EDIT_MODE_INSERT_TRIANGLE_FAN_EMCMD,IDS_INSERT_TRIANGLE_FAN_MENU_ITEM);
        menu->appendMenuItem(selSize>0,false,SHAPE_EDIT_MODE_MAKE_DUMMIES_WITH_SELECTED_VERTICES_EMCMD,IDS_MAKE_DUMMIES_WITH_SELECTED_VERTICES_MENU_ITEM);

        menu->appendMenuSeparator();

        menu->appendMenuItem(true,false,SHAPE_EDIT_MODE_SELECT_ALL_ITEMS_EMCMD,IDSN_SELECT_ALL_MENU_ITEM);
    }

    if (_editModeType&TRIANGLE_EDIT_MODE)
    {
        menu->appendMenuItem(selSize>0,false,SHAPE_EDIT_MODE_TRIANGLE_VERTEX_COPY_EMCMD,IDS_COPY_SELECTED_TRIANGLES_MENU_ITEM);
        menu->appendMenuItem(buffSize>0,false,SHAPE_EDIT_MODE_PASTE_TRIANGLE_VERTEX_EMCMD,IDS_PASTE_TRIANGLES_MENU_ITEM);
        menu->appendMenuItem(selSize>0,false,SHAPE_EDIT_MODE_DELETE_TRIANGLE_VERTEX_EDGE_EMCMD,IDS_DELETE_SELECTED_TRIANGLES_MENU_ITEM);
        menu->appendMenuItem(selSize>0,false,SHAPE_EDIT_MODE_TRIANGLE_VERTEX_CUT_EMCMD,IDS_CUT_SELECTED_TRIANGLES_MENU_ITEM);

        menu->appendMenuSeparator();

        menu->appendMenuItem(selSize>0,false,SHAPE_EDIT_MODE_FLIP_TRIANGLES_SIDE_EMCMD,IDS_FLIP_SELECTED_TRIANGLES_MENU_ITEM);
        menu->appendMenuItem(selSize>0,false,SHAPE_EDIT_MODE_MAKE_SHAPE_WITH_SELECTED_TRIANGLES_EMCMD,IDS_MAKE_SHAPE_WITH_SELECTED_TRIANGLES_MENU_ITEM);
        menu->appendMenuItem(selSize>0,false,SHAPE_EDIT_MODE_MAKE_RECTANGLE_WITH_SELECTED_TRIANGLES_EMCMD,IDS_MAKE_RECTANGLE_WITH_SELECTED_TRIANGLES_MENU_ITEM);
        menu->appendMenuItem(selSize>0,false,SHAPE_EDIT_MODE_MAKE_SPHERE_WITH_SELECTED_TRIANGLES_EMCMD,IDS_MAKE_SPHERE_WITH_SELECTED_TRIANGLES_MENU_ITEM);
        menu->appendMenuItem(selSize>0,false,SHAPE_EDIT_MODE_MAKE_CYLINDER_WITH_SELECTED_TRIANGLES_EMCMD,IDS_MAKE_CYLINDER_WITH_SELECTED_TRIANGLES_MENU_ITEM);
        menu->appendMenuItem(true,false,SHAPE_EDIT_MODE_DIVIDE_LARGEST_TRIANGLES_EMCMD,IDS_REDUCE_TRIANGLE_SIZE_MENU_ITEM);

        menu->appendMenuSeparator();

        menu->appendMenuItem(true,false,SHAPE_EDIT_MODE_SELECT_ALL_ITEMS_EMCMD,IDSN_SELECT_ALL_MENU_ITEM);
    }

    if (_editModeType&EDGE_EDIT_MODE)
    {
        menu->appendMenuItem(true,showHiddenVerticeAndEdges,SHAPE_EDIT_MODE_TOGGLE_HIDDEN_VERTICE_EDGE_SHOW_EMCMD,IDS_SHOW_HIDDEN_EDGES_MENU_ITEM,true);
        menu->appendMenuItem(true,automaticallyFollowEdges,SHAPE_EDIT_MODE_TOGGLE_AUTOMATIC_EDGE_FOLLOWING_EMCMD,IDS_AUTOMATIC_EDGE_FOLLOWING_MENU_ITEM,true);

        menu->appendMenuSeparator();

        menu->appendMenuItem(selSize>0,false,SHAPE_EDIT_MODE_DELETE_TRIANGLE_VERTEX_EDGE_EMCMD,IDS_DELETE_SELECTED_EDGES_MENU_ITEM);

        menu->appendMenuSeparator();

        menu->appendMenuItem(selSize>0,false,SHAPE_EDIT_MODE_MAKE_PATH_WITH_SELECTED_EDGES_EMCMD,IDS_MAKE_PATH_WITH_SELECTED_EDGES_MENU_ITEM);

        menu->appendMenuSeparator();

        menu->appendMenuItem(true,false,SHAPE_EDIT_MODE_SELECT_ALL_ITEMS_EMCMD,IDSN_SELECT_ALL_MENU_ITEM);
    }
}

bool CShapeEditMode::processCommand(int commandID)
{ // Return value is true means success
    int retVal=true;
    if (commandID==SHAPE_EDIT_MODE_SWAP_TO_VERTEX_EDIT_MODE_EMCMD)
    {
        if (_editModeType&VERTEX_EDIT_MODE)
            retVal=false;

        if (_editModeType&TRIANGLE_EDIT_MODE)
        {
            selectionFromTriangleToVertexEditMode();
            swapShapeEditModeType(VERTEX_EDIT_MODE);
        }

        if (_editModeType&EDGE_EDIT_MODE)
        {
            selectionFromEdgeToVertexEditMode();
            swapShapeEditModeType(VERTEX_EDIT_MODE);
        }

        editBufferVerticesCopy.clear();
        editBufferIndicesCopy.clear();
        editBufferNormalsCopy.clear();
        editBufferTextureCoordsCopy.clear();
        return(retVal);
    }

    if (commandID==SHAPE_EDIT_MODE_SWAP_TO_TRIANGLE_EDIT_MODE_EMCMD)
    {
        if (_editModeType&TRIANGLE_EDIT_MODE)
            retVal=false;

        if (_editModeType&VERTEX_EDIT_MODE)
        {
            selectionFromVertexToTriangleEditMode();
            swapShapeEditModeType(TRIANGLE_EDIT_MODE);
        }

        if (_editModeType&EDGE_EDIT_MODE)
        {
            selectionFromEdgeToTriangleEditMode();
            swapShapeEditModeType(TRIANGLE_EDIT_MODE);
        }

        editBufferVerticesCopy.clear();
        editBufferIndicesCopy.clear();
        editBufferNormalsCopy.clear();
        editBufferTextureCoordsCopy.clear();
        return(retVal);
    }

    if (commandID==SHAPE_EDIT_MODE_SWAP_TO_EDGE_EDIT_MODE_EMCMD)
    {
        if (_editModeType&VERTEX_EDIT_MODE)
        {
            selectionFromVertexToEdgeEditMode();
            swapShapeEditModeType(EDGE_EDIT_MODE);
        }

        if (_editModeType&TRIANGLE_EDIT_MODE)
        {
            selectionFromTriangleToEdgeEditMode();
            swapShapeEditModeType(EDGE_EDIT_MODE);
        }

        if (_editModeType&EDGE_EDIT_MODE)
            retVal=false;

        editBufferVerticesCopy.clear();
        editBufferIndicesCopy.clear();
        editBufferNormalsCopy.clear();
        editBufferTextureCoordsCopy.clear();
        return(retVal);
    }

    if (commandID==SHAPE_EDIT_MODE_TRIANGLE_VERTEX_COPY_EMCMD)
    {
        if ((_editModeType&EDGE_EDIT_MODE)==0)
        {
            std::vector<int> sel;
            for (int i=0;i<getEditModeBufferSize();i++)
                sel.push_back(editModeBuffer[i]);
            copySelection(&sel);
        }
        else
            retVal=false;

        return(retVal);
    }

    if (commandID==SHAPE_EDIT_MODE_PASTE_TRIANGLE_VERTEX_EMCMD)
    {
        if ((_editModeType&EDGE_EDIT_MODE)==0)
            paste();
        else
            retVal=false;

        return(retVal);
    }

    if (commandID==SHAPE_EDIT_MODE_EMPTY_TRIANGLE_VERTEX_COPY_BUFFER_EMCMD)
    {
        editBufferVerticesCopy.clear();
        editBufferIndicesCopy.clear();
        editBufferNormalsCopy.clear();
        editBufferTextureCoordsCopy.clear();
        return(retVal);
    }

    if (commandID==SHAPE_EDIT_MODE_DELETE_TRIANGLE_VERTEX_EDGE_EMCMD)
    {
        std::vector<int> sel;
        for (int i=0;i<getEditModeBufferSize();i++)
            sel.push_back(editModeBuffer[i]);
        deleteSelection(&sel);
        return(retVal);
    }

    if (commandID==SHAPE_EDIT_MODE_SELECT_ALL_ITEMS_EMCMD)
    {
        if (_editModeType&TRIANGLE_EDIT_MODE)
        {
            for (int i=0;i<int(_editionIndices.size())/3;i++)
                addItemToEditModeBuffer(i,true);
        }

        if (_editModeType&VERTEX_EDIT_MODE)
        {
            for (int i=0;i<int(_editionVertices.size())/3;i++)
                addItemToEditModeBuffer(i,true);
        }

        if (_editModeType&EDGE_EDIT_MODE)
        {
            for (int i=0;i<int(_edgeCont.allEdges.size())/2;i++)
                addItemToEditModeBuffer(i,true);
        }

        return(retVal);
    }

    if (commandID==SHAPE_EDIT_MODE_TOGGLE_HIDDEN_VERTICE_EDGE_SHOW_EMCMD)
    {
        showHiddenVerticeAndEdges=!showHiddenVerticeAndEdges;
        return(retVal);
    }

    if (commandID==SHAPE_EDIT_MODE_TOGGLE_AUTOMATIC_EDGE_FOLLOWING_EMCMD)
    {
        automaticallyFollowEdges=!automaticallyFollowEdges;
        return(retVal);
    }

    if (commandID==SHAPE_EDIT_MODE_TRIANGLE_VERTEX_CUT_EMCMD)
    {
        if ((_editModeType&EDGE_EDIT_MODE)==0)
        {
            std::vector<int> sel;
            for (int i=0;i<getEditModeBufferSize();i++)
                sel.push_back(editModeBuffer[i]);
            cutSelection(&sel);
        }
        else
            retVal=false;

        return(retVal);
    }


    return(false);
}

void CShapeEditMode::makeShape()
{
    std::vector<int> sel;
    for (int i=0;i<getEditModeBufferSize();i++)
        sel.push_back(editModeBuffer[i]);
    std::vector<float> nVertices;
    std::vector<int> nIndices;
    std::vector<float> nNormals;
    std::vector<float> nTexCoords;
    copySelectedFaces(&sel,&nVertices,&nIndices,&nNormals,&nTexCoords);
    if (nVertices.size()!=0)
    {   // Now we have to transform all vertices with the cumulative transform
        // matrix of the shape beeing edited:
        CShape* it=_shape;
        C7Vector m(it->getCumulativeTransformation());
        C3Vector v;
        for (int i=0;i<int(nVertices.size())/3;i++)
        {
            v(0)=nVertices[3*i+0];
            v(1)=nVertices[3*i+1];
            v(2)=nVertices[3*i+2];
            v*=m;
            nVertices[3*i+0]=v(0);
            nVertices[3*i+1]=v(1);
            nVertices[3*i+2]=v(2);
        }

        SSimulationThreadCommand cmd;
        cmd.cmdId=SHAPEEDIT_MAKESHAPE_GUITRIGGEREDCMD;
        int toid=-1;
        if (_editionTextureProperty!=nullptr)
            toid=_editionTextureProperty->getTextureObjectID();
        cmd.intParams.push_back(toid);
        cmd.intVectorParams.push_back(nIndices);
        cmd.floatVectorParams.push_back(nVertices);
        cmd.floatVectorParams.push_back(nNormals);
        cmd.floatVectorParams.push_back(nTexCoords);
        App::appendSimulationThreadCommand(cmd);
    }
}

void CShapeEditMode::makePrimitive(int what)
{
    std::vector<int> sel;
    for (int i=0;i<getEditModeBufferSize();i++)
        sel.push_back(editModeBuffer[i]);
    std::vector<float> nVertices;
    std::vector<int> nIndices;
    std::vector<float> nNormals;
    copySelectedFaces(&sel,&nVertices,&nIndices,&nNormals,nullptr);
    if (nVertices.size()!=0)
    {   // Now we have to transform all vertices with the cumulative transform
        // matrix of the shape beeing edited:
        CShape* it=_shape;
        C7Vector m(it->getCumulativeTransformation());
        C3Vector v;
        for (int i=0;i<int(nVertices.size())/3;i++)
        {
            C3Vector v(&nVertices[3*i+0]);
            v*=m;
            nVertices[3*i+0]=v(0);
            nVertices[3*i+1]=v(1);
            nVertices[3*i+2]=v(2);
        }
        SSimulationThreadCommand cmd;
        cmd.cmdId=SHAPEEDIT_MAKEPRIMITIVE_GUITRIGGEREDCMD;
        cmd.intParams.push_back(what);
        cmd.intVectorParams.push_back(nIndices);
        cmd.floatVectorParams.push_back(nVertices);
        App::appendSimulationThreadCommand(cmd);
    }
}

void CShapeEditMode::flipTriangles()
{
    if (_editModeType&TRIANGLE_EDIT_MODE)
    {
        std::vector<int> sel;
        for (int i=0;i<getEditModeBufferSize();i++)
            sel.push_back(editModeBuffer[i]);
        for (size_t i=0;i<sel.size();i++)
            flipTriangle(sel[i]);
    }
}

void CShapeEditMode::subdivideTriangles()
{
    if (_editModeType&TRIANGLE_EDIT_MODE)
    {
        deselectEditModeBuffer();
        CMeshManip::reduceTriangleSize(_editionVertices,_editionIndices,&_editionNormals,&_editionTextureCoords,0.0f,0.00001f); // 0.0 simple uses the half of the max edge!
        actualizeEditModeEditionEdges();
    }
}

void CShapeEditMode::insertTriangles()
{
    if (_editModeType&VERTEX_EDIT_MODE)
    {
        std::vector<int> sel;
        for (int i=0;i<getEditModeBufferSize();i++)
            sel.push_back(editModeBuffer[i]);
        for (size_t i=0;i<sel.size()/3;i++)
            addTriangle(sel[3*i+0],sel[3*i+1],sel[3*i+2]);
    }
    deselectEditModeBuffer();
}

void CShapeEditMode::insertTriangleFan()
{
    if (_editModeType&VERTEX_EDIT_MODE)
    {
        std::vector<int> sel;
        for (int i=0;i<getEditModeBufferSize();i++)
            sel.push_back(editModeBuffer[i]);
        for (size_t i=0;i<(sel.size()-1)/2;i++)
        {
            addTriangle(sel[2*i+0],sel[2*i+1],sel[sel.size()-1]);
            if (2*i+2<(sel.size()-1))
                addTriangle(sel[2*i+1],sel[2*i+2],sel[sel.size()-1]);
        }
    }
    deselectEditModeBuffer();
}

void CShapeEditMode::makeDummies()
{
    bool proceed=true;
    if (getEditModeBufferSize()>50)
        proceed=(VMESSAGEBOX_REPLY_YES==App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_VERTICES),strTranslate(IDS_LARGE_QUANTITY_OF_OBJECT_WARNING),VMESSAGEBOX_YES_NO));
    if (proceed)
    {
        CShape* it=_shape;
        C7Vector tr(it->getCumulativeTransformation());
        for (int i=0;i<getEditModeBufferSize();i++)
        {
            int ind=editModeBuffer[i];
            C3Vector v(_editionVertices[3*ind+0],_editionVertices[3*ind+1],_editionVertices[3*ind+2]);
            v=tr*v;
            SSimulationThreadCommand cmd;
            cmd.cmdId=PATHEDIT_MAKEDUMMY_GUITRIGGEREDCMD;
            cmd.stringParams.push_back("ExtractedDummy");
            cmd.floatParams.push_back(0.01f);
            cmd.transfParams.push_back(v);
            App::appendSimulationThreadCommand(cmd);
        }
        editModeBuffer.clear();
    }
}


void CShapeEditMode::makePath()
{
    std::vector<int> sel;
    for (int i=0;i<getEditModeBufferSize();i++)
        sel.push_back(editModeBuffer[i]);
    CShape* shape=_shape;
    if (sel.size()>0)
    { // Only consecutive edges will be used! NOOOO! (modified on 2009/03/14)
        CPath* newObject=new CPath();
        newObject->pathContainer->enableActualization(false);
        int verticeInd[2];
        verticeInd[0]=_edgeCont.allEdges[2*sel[0]+0];
        verticeInd[1]=_edgeCont.allEdges[2*sel[0]+1];
        C3Vector v0(&_editionVertices[3*verticeInd[0]+0]);
        C3Vector v1(&_editionVertices[3*verticeInd[1]+0]);
        CSimplePathPoint* it=nullptr;
        C7Vector sctm(shape->getCumulativeTransformation());
        C3Vector lastAddedPoint;
        if (sel.size()==1)
        { // We simply add the two points:
            it=new CSimplePathPoint();
            it->setBezierPointCount(1);
            C7Vector trtmp(it->getTransformation());
            trtmp.X=sctm*v0;
            it->setTransformation(trtmp,newObject->pathContainer->getAttributes());
            newObject->pathContainer->addSimplePathPoint(it);
            it=new CSimplePathPoint();
            it->setBezierPointCount(1);
            trtmp=it->getTransformation();
            lastAddedPoint=v1;
            trtmp.X=sctm*v1;
            it->setTransformation(trtmp,newObject->pathContainer->getAttributes());
            newObject->pathContainer->addSimplePathPoint(it);
        }
        else
        {
            int firstPointIndex=-1;
            for (int i=1;i<int(sel.size());i++)
            {
                int in0=_edgeCont.allEdges[2*sel[i]+0];
                int in1=_edgeCont.allEdges[2*sel[i]+1];
                if (i==1)
                { // We have to add the two first points:
                    if ( (in0==verticeInd[0])||(in1==verticeInd[0]) )
                    {
                        it=new CSimplePathPoint();
                        it->setBezierPointCount(1);
                        C7Vector trtmp(it->getTransformation());
                        trtmp.X=sctm*v1;
                        it->setTransformation(trtmp,newObject->pathContainer->getAttributes());
                        newObject->pathContainer->addSimplePathPoint(it);
                        it=new CSimplePathPoint();
                        it->setBezierPointCount(1);
                        trtmp=it->getTransformation();
                        lastAddedPoint=v0;
                        trtmp.X=sctm*v0;
                        it->setTransformation(trtmp,newObject->pathContainer->getAttributes());
                        newObject->pathContainer->addSimplePathPoint(it);
                        firstPointIndex=verticeInd[1];
                    }
                    else
                    {
                        it=new CSimplePathPoint();
                        it->setBezierPointCount(1);
                        C7Vector trtmp(it->getTransformation());
                        trtmp.X=sctm*v0;
                        it->setTransformation(trtmp,newObject->pathContainer->getAttributes());
                        newObject->pathContainer->addSimplePathPoint(it);
                        it=new CSimplePathPoint();
                        it->setBezierPointCount(1);
                        trtmp=it->getTransformation();
                        lastAddedPoint=v1;
                        trtmp.X=sctm*v1;
                        it->setTransformation(trtmp,newObject->pathContainer->getAttributes());
                        newObject->pathContainer->addSimplePathPoint(it);
                        firstPointIndex=verticeInd[0];
                    }
                }
                // We have to add the vertex that wasn't added yet. If both weren't added yet, we add the closest one first, then the other one
                if ( (in0==verticeInd[0])||(in0==verticeInd[1]) )
                { // We have to add in1
                    if ( (in1==firstPointIndex)&&(i==int(sel.size())-1) )
                    { // we make a closed path only if the is the last edge!
                        newObject->pathContainer->setAttributes(newObject->pathContainer->getAttributes()|sim_pathproperty_closed_path);
                        break;
                    }
                    C3Vector v(&_editionVertices[3*in1+0]);
                    it=new CSimplePathPoint();
                    it->setBezierPointCount(1);
                    C7Vector trtmp(it->getTransformation());
                    lastAddedPoint=v;
                    trtmp.X=sctm*v;
                    it->setTransformation(trtmp,newObject->pathContainer->getAttributes());
                    newObject->pathContainer->addSimplePathPoint(it);
                }
                else if ( (in1==verticeInd[0])||(in1==verticeInd[1]) )
                { // We have to add in0
                    if ( (in0==firstPointIndex)&&(i==int(sel.size())-1) )
                    { // we make a closed path only if the is the last edge!
                        newObject->pathContainer->setAttributes(newObject->pathContainer->getAttributes()|sim_pathproperty_closed_path);
                        break;
                    }
                    C3Vector v(&_editionVertices[3*in0+0]);
                    it=new CSimplePathPoint();
                    it->setBezierPointCount(1);
                    C7Vector trtmp(it->getTransformation());
                    lastAddedPoint=v;
                    trtmp.X=sctm*v;
                    it->setTransformation(trtmp,newObject->pathContainer->getAttributes());
                    newObject->pathContainer->addSimplePathPoint(it);
                }
                else
                { // Following replaces the break command (since 2009/03/14)
                    // Here none of the two vertices were added. We first add the closest one, then the other one:
                    C3Vector w0(&_editionVertices[3*in0+0]);
                    C3Vector w1(&_editionVertices[3*in1+0]);
                    float l0=(w0-lastAddedPoint).getLength();
                    float l1=(w1-lastAddedPoint).getLength();
                    if (l0>l1)
                    { // We swap both
                        int in0Cop=in0;
                        in0=in1;
                        in1=in0Cop;
                        C3Vector v(w0);
                        w0=w1;
                        w1=v;
                    }
                    // the first one:
                    it=new CSimplePathPoint();
                    it->setBezierPointCount(1);
                    C7Vector trtmp(it->getTransformation());
                    lastAddedPoint=w0;
                    trtmp.X=sctm*w0;
                    it->setTransformation(trtmp,newObject->pathContainer->getAttributes());
                    newObject->pathContainer->addSimplePathPoint(it);
                    // Now the second one:
                    if ( (in1==firstPointIndex)&&(i==int(sel.size())-1) )
                    { // we make a closed path only if the is the last edge!
                        newObject->pathContainer->setAttributes(newObject->pathContainer->getAttributes()|sim_pathproperty_closed_path);
                        break;
                    }
                    it=new CSimplePathPoint();
                    it->setBezierPointCount(1);
                    trtmp=it->getTransformation();
                    lastAddedPoint=w1;
                    trtmp.X=sctm*w1;
                    it->setTransformation(trtmp,newObject->pathContainer->getAttributes());
                    newObject->pathContainer->addSimplePathPoint(it);
                }
                verticeInd[0]=in0;
                verticeInd[1]=in1;
            }
        }
        newObject->pathContainer->enableActualization(true);
        newObject->pathContainer->actualizePath();
        newObject->setObjectName_objectNotYetInScene("ExtractedPath");
        newObject->setObjectAltName_objectNotYetInScene(tt::getObjectAltNameFromObjectName(newObject->getObjectName()));
        SSimulationThreadCommand cmd;
        cmd.cmdId=ADD_OBJECTTOSCENE_GUITRIGGEREDCMD;
        cmd.intParams.push_back(sim_object_path_type);
        cmd.objectParams.push_back(newObject);
        App::appendSimulationThreadCommand(cmd);
        editModeBuffer.clear();
    }
}
