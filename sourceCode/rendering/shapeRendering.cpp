/*
Source code based on the V-REP library source code from Coppelia
Robotics AG on September 2019

Copyright (C) 2006-2023 Coppelia Robotics AG
Copyright (C) 2019 Robot Nordic ApS

All rights reserved.

GNU GPL license:
================
The code in this file is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

See the GNU General Public License for more details.
<http://www.gnu.org/licenses/>
*/

#include <shapeRendering.h>

#ifdef SIM_WITH_GUI
#include <meshWrapper.h>
#include <mesh.h>
#include <guiApp.h>

void displayShape(CShape* shape,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every scene object display routine:
    _commonStart(shape,renderingObject);

    bool guiIsRendering=((displayAttrib&sim_displayattribute_forvisionsensor)==0);

    C3Vector normalVectorForLinesAndPoints(shape->getFullCumulativeTransformation().Q.getInverse()*C3Vector::unitZVector);

    bool editNormals=false;
    bool editVertices=false;
    bool editEdges=false;
    bool editMultishape=false;
#ifdef SIM_WITH_GUI
    if ( (GuiApp::mainWindow!=nullptr)&&(GuiApp::mainWindow->editModeContainer->getEditModeObject()==shape) )
    {
        editNormals=( (!shape->isCompound())&&(GuiApp::getEditModeType()&TRIANGLE_EDIT_MODE));
        editVertices=( (!shape->isCompound())&&(GuiApp::getEditModeType()&VERTEX_EDIT_MODE));
        editEdges=( (!shape->isCompound())&&(GuiApp::getEditModeType()&EDGE_EDIT_MODE));
        editMultishape=( shape->isCompound()&&(GuiApp::getEditModeType()==MULTISHAPE_EDIT_MODE));
    }
#endif
    bool editMode=editNormals||editVertices||editEdges||editMultishape;

    if (shape->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib)||editMode)
    {
        if ((GuiApp::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE_OLD)==0)
        {
            if (shape->getObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
                glLoadName(shape->getModelSelectionHandle());
            else
                glLoadName(shape->getObjectHandle());
        }
        else
            glLoadName(-1);

#ifdef SIM_WITH_GUI
        C3Vector hs(shape->getBBHSize());
        double refSize=(hs(0)+hs(1)+hs(2))/2.0;
        if (editNormals)
        {
            if ((displayAttrib&sim_displayattribute_renderpass)!=0)
                ogl::drawReference(refSize);
            GuiApp::mainWindow->editModeContainer->getShapeEditMode()->displayFaceOrientation(displayAttrib);
        }
        else if (editVertices)
        {
            if ((displayAttrib&sim_displayattribute_renderpass)!=0)
                ogl::drawReference(refSize);
            GuiApp::mainWindow->editModeContainer->getShapeEditMode()->displayVertices(displayAttrib);
        }
        else if (editEdges)
        {
            if ((displayAttrib&sim_displayattribute_renderpass)!=0)
                ogl::drawReference(refSize);
            GuiApp::mainWindow->editModeContainer->getShapeEditMode()->displayEdgeEditMode(displayAttrib);
        }
        else if (editMultishape)
        {
            if ((displayAttrib&sim_displayattribute_renderpass)!=0)
                ogl::drawReference(refSize);
            displayAttrib|=(sim_displayattribute_forbidedges|sim_displayattribute_forcewireframe|sim_displayattribute_trianglewireframe);
            displayAttrib-=(sim_displayattribute_forbidedges|sim_displayattribute_forcewireframe|sim_displayattribute_trianglewireframe);
            bool textEnabledSaved=App::currentWorld->environment->getShapeTexturesEnabled();
            App::currentWorld->environment->setShapeTexturesEnabled(true);
            if (shape->getContainsTransparentComponent())
            {
                GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->displayAllGeometricComponents(shape,displayAttrib,nullptr,0,2);
                GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->displayAllGeometricComponents(shape,displayAttrib,nullptr,0,1);
            }
            else
                GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->displayAllGeometricComponents(shape,displayAttrib,nullptr,0,0);
            App::currentWorld->environment->setShapeTexturesEnabled(textEnabledSaved);
        }
        else
#endif
        {
            _enableAuxClippingPlanes(shape->getObjectHandle());

            CColorObject otherColor;
            CColorObject* otherColorP=nullptr;
            if ((displayAttrib&sim_displayattribute_originalcolors)==0)
            {
                bool setOtherColor=(App::currentWorld->collisions->getCollisionColor(shape->getObjectHandle())!=0);
                for (size_t i=0;i<App::currentWorld->collections->getObjectCount();i++)
                {
                    if (App::currentWorld->collections->getObjectFromIndex(i)->isObjectInCollection(shape->getObjectHandle()))
                        setOtherColor|=(App::currentWorld->collisions->getCollisionColor(App::currentWorld->collections->getObjectFromIndex(i)->getCollectionHandle())!=0);
                }
                if (setOtherColor)
                {
                    App::currentWorld->mainSettings->collisionColor.copyYourselfInto(&otherColor);
                    otherColorP=&otherColor;
                }
            }

            if (renderingObject->isObjectInsideView(shape->getCumulativeTransformation()*shape->getBB(nullptr),shape->getBBHSize()))
            { // the bounding box is inside of the view (at least some part of it!)
                if ((displayAttrib&sim_displayattribute_colorcoded)==0)
                { // normal visualization
                    if (shape->getContainsTransparentComponent())
                    {
                        shape->getMesh()->display(C7Vector::identityTransformation,shape,displayAttrib,otherColorP,shape->getDynamicFlag(),2,false);
                        shape->getMesh()->display(C7Vector::identityTransformation,shape,displayAttrib,otherColorP,shape->getDynamicFlag(),1,false);
                    }
                    else
                        shape->getMesh()->display(C7Vector::identityTransformation,shape,displayAttrib,otherColorP,shape->getDynamicFlag(),0,false);
                }
                else
                    shape->getMesh()->display_colorCoded(C7Vector::identityTransformation,shape,shape->getObjectHandle(),displayAttrib); // color-coded visualization
            }
            _disableAuxClippingPlanes();
        }
    }

    // At the end of every scene object display routine:
    _commonFinish(shape,renderingObject);
}

void _displayInertia(const C7Vector& tr,const C3Vector& pmi,double comFrameSize)
{
    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    glTranslated(tr.X(0),tr.X(1),tr.X(2));
    C4Vector axis=tr.Q.getAngleAndAxis();
    glRotated(axis(0)*radToDeg,axis(1),axis(2),axis(3));
    ogl::setMaterialColor(ogl::colorRed,ogl::colorBlack,ogl::colorBlack);
    ogl::setAlpha(0.1);
    C3Vector p(pmi);
    for (size_t i=0;i<3;i++)
    {
        if (p(i)<0.0000001)
            p(i)=0.0000001;
    }
    C3Vector hs(sqrt(6*(-p(0)+p(1)+p(2))),sqrt(6*(p(0)-p(1)+p(2))),sqrt(6*(p(0)+p(1)-p(2))));
    hs*=1.01; // a tiny bit larger

    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    ogl::drawBox(hs(0),hs(1),hs(2),true,nullptr);
    ogl::setAlpha(1.0);
    ogl::drawBox(hs(0),hs(1),hs(2),false,nullptr);


    _displayFrame(C7Vector::identityTransformation,comFrameSize,2);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glPopAttrib();
    glPopMatrix();
}

void _displayTriangles(CMesh* geometric,int geomModifCounter,CTextureProperty* tp)
{
    if ((!App::currentWorld->environment->getShapeTexturesEnabled())||CEnvironment::getShapeTexturesTemporarilyDisabled())
        tp=nullptr;
    std::vector<float>* textureCoords=nullptr;
    int* texCoordBufferIdPtr=nullptr;
    const std::vector<float>& _vertices=geometric->getVerticesForDisplayAndDisk()[0];
    const std::vector<int>& _indices=geometric->getIndices()[0];
    const std::vector<float>& _normals=geometric->getNormalsForDisplayAndDisk()[0];
    if (tp!=nullptr)
    {
        textureCoords=tp->getTextureCoordinates(geomModifCounter,_vertices,_indices);
        if (textureCoords==nullptr)
            return; // Should normally never happen (should be caught before!)
        texCoordBufferIdPtr=tp->getTexCoordBufferIdPointer();

        _start3DTextureDisplay(tp);
        _drawTriangles(&_vertices[0],(int)_vertices.size()/3,&_indices[0],(int)_indices.size(),&_normals[0],&textureCoords->at(0),geometric->getVertexBufferIdPtr(),geometric->getNormalBufferIdPtr(),texCoordBufferIdPtr);
        _end3DTextureDisplay(tp);
    }
    else
        _drawTriangles(&_vertices[0],(int)_vertices.size()/3,&_indices[0],(int)_indices.size(),&_normals[0],nullptr,geometric->getVertexBufferIdPtr(),geometric->getNormalBufferIdPtr(),nullptr);
}


void displayGeometric(const C7Vector& cumulIFrameTr,CMesh* geometric,CShape* geomData,int displayAttrib,CColorObject* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected)
{
//    printf("A: %f, %f, %f\n",cumulIFrameTr.X(0),cumulIFrameTr.X(1),cumulIFrameTr.X(2));
//    printf("B: %f, %f, %f\n",geometric->getVFrame()(0),geometric->getVFrame()(1),geometric->getVFrame()(2));
//    printf("C: %f, %f, %f\n",geometric->getVertices()->at(0),geometric->getVertices()->at(1),geometric->getVertices()->at(2));
    if (transparencyHandling!=0)
    {// 0=display always, 1=display transparent only, 2=display non-transparent only
        if ((transparencyHandling==1)&&(!geometric->getContainsTransparentComponents()))
            return;
        if ((transparencyHandling==2)&&(geometric->getContainsTransparentComponents()))
            return;
    }
    C7Vector _verticeLocalFrame(cumulIFrameTr*geometric->getBB(nullptr));
    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    glTranslated(_verticeLocalFrame.X(0),_verticeLocalFrame.X(1),_verticeLocalFrame.X(2));
    C4Vector axis=_verticeLocalFrame.Q.getAngleAndAxis();
    glRotated(axis(0)*radToDeg,axis(1),axis(2),axis(3));
    if (geometric->getDisplayInverted_DEPRECATED())
        glFrontFace(GL_CW);

    bool wire=(geometric->getWireframe_OLD()||(displayAttrib&sim_displayattribute_forcewireframe))&&((displayAttrib&sim_displayattribute_depthpass)==0);
    bool meantAsWire=wire;
    if (displayAttrib&sim_displayattribute_forbidwireframe)
        wire=false;
    bool edges=geometric->getVisibleEdges();
    if (displayAttrib&sim_displayattribute_forbidedges)
        edges=false;

    if ((!meantAsWire)||((displayAttrib&sim_displayattribute_forvisionsensor)==0)) // added on 2010/08/11 because the wireframe part of the default floor would be rendered solid
    {
        // We now display this object itself:
        bool nonPureDynamicDisplay=false;
        if (displayAttrib&sim_displayattribute_dynamiccontentonly)
        {
            if (dynObjFlag_forVisualization==1)
                ogl::setMaterialColor(0.9f,0.9f,0.9f,0.5f,0.5f,0.5f,0.0f,0.0f,0.0f);
            if (dynObjFlag_forVisualization==3)
                ogl::setMaterialColor(0.9f,0.11f,0.11f,0.5f,0.5f,0.5f,0.0f,0.0f,0.0f);
            if (dynObjFlag_forVisualization==2)
                ogl::setMaterialColor(0.9f,0.11f,0.9f,0.5f,0.5f,0.5f,0.0f,0.0f,0.0f);
            nonPureDynamicDisplay=(geometric->getPurePrimitiveType()==sim_primitiveshape_none);
        }
        else
        {
            if (collisionColor==nullptr)
                geometric->color.makeCurrentColor2(false,(displayAttrib&sim_displayattribute_useauxcomponent)!=0);
            else
                collisionColor->makeCurrentColor2(false,(displayAttrib&sim_displayattribute_useauxcomponent)!=0);
        }

        if (geometric->getCulling()||(!geometric->getInsideAndOutsideFacesSameColor_DEPRECATED()))
            glEnable(GL_CULL_FACE);
        // REMOVED ON 2010/07/29 so that we see edges over non-edges shapes (e.g. wall elements)    if (edges||wire)
        {
            glPolygonOffset(0.5,0.0); // Second argument set to 0.0 on 2009.01.05 (otherwise strange effects on some graphic cards)
            glEnable(GL_POLYGON_OFFSET_FILL);
        }

        if (!nonPureDynamicDisplay)
        {
            if ((!wire)||(displayAttrib&sim_displayattribute_trianglewireframe))
            {
                for (int pss=0;pss<2;pss++)
                { // we have only a second pass if the inside color is different and no backface culling!
                    if (displayAttrib&sim_displayattribute_trianglewireframe)
                        glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);

                    if ( (displayAttrib&sim_displayattribute_trianglewireframe)||(collisionColor!=nullptr)||(displayAttrib&sim_displayattribute_dynamiccontentonly)||(displayAttrib&sim_displayattribute_useauxcomponent)||(displayAttrib&sim_displayattribute_depthpass) )
                        _displayTriangles(geometric,geomData->getMeshModificationCounter(),nullptr);
                    else
                        _displayTriangles(geometric,geomData->getMeshModificationCounter(),geometric->getTextureProperty());

                    if (displayAttrib&sim_displayattribute_trianglewireframe)
                        glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
                    if (geometric->getInsideAndOutsideFacesSameColor_DEPRECATED()||geometric->getCulling())
                        break; // we leave here.. inside and outside colors are same

                    if ((displayAttrib&sim_displayattribute_dynamiccontentonly)==0)
                    {
                        if (pss==0)
                        {
                            if (collisionColor==nullptr)
                                geometric->insideColor_DEPRECATED.makeCurrentColor2(false,(displayAttrib&sim_displayattribute_useauxcomponent)!=0);
                            glCullFace(GL_FRONT);
                        }
                        else
                        { // we reset to initial state
                            if (collisionColor==nullptr)
                                geometric->color.makeCurrentColor2(false,(displayAttrib&sim_displayattribute_useauxcomponent)!=0);
                            glCullFace(GL_BACK);
                        }
                    }
                }
            }
        }
        else
        { // we have to show that we have a non-pure shape dynamically simulated:
            // we show the triangles edges in green (random mesh):
            // we show the triangles edges in grey (convex mesh):
            if ((displayAttrib&sim_displayattribute_trianglewireframe)==0)
            {
                glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
                _displayTriangles(geometric,geomData->getMeshModificationCounter(),nullptr);
            }
            glDisable(GL_POLYGON_OFFSET_FILL);
            glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);

            if (geometric->isConvex())
                ogl::setMaterialColor(1.0,1.0,1.0,0.0,0.0,0.0,0.0,0.0,0.0);
            else
                ogl::setMaterialColor(0.0,1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0);
            _displayTriangles(geometric,geomData->getMeshModificationCounter(),nullptr);
        }

        glDisable(GL_POLYGON_OFFSET_FILL);

        if ( (edges||wire)&&((displayAttrib&sim_displayattribute_trianglewireframe)==0)&&(!nonPureDynamicDisplay) )
        {

            if (!wire)
            {
                if ((displayAttrib&sim_displayattribute_dynamiccontentonly)==0)
                    geometric->edgeColor_DEPRECATED.makeCurrentColor2(false,(displayAttrib&sim_displayattribute_useauxcomponent)!=0);
                else
                    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
            }
            else
            {
                if ((displayAttrib&sim_displayattribute_dynamiccontentonly)==0)
                    ogl::setMaterialColor(nullptr,ogl::colorBlack,nullptr);
                else
                    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
            }

            if (displayAttrib&sim_displayattribute_thickEdges)
                glLineWidth(float(4*geometric->getEdgeWidth_DEPRECATED()));
            else
                glLineWidth(float(geometric->getEdgeWidth_DEPRECATED()));

            if (App::userSettings->antiAliasing&&edges&&(!wire))
            {
                glEnable (GL_LINE_SMOOTH);
                glEnable (GL_BLEND);
                glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
            }
            std::vector<float>& _vertices=geometric->getVerticesForDisplayAndDisk()[0];
            std::vector<int>& _indices=geometric->getIndices()[0];
            std::vector<unsigned char>& _edges=geometric->getEdges()[0];
            bool nothingDisplayed=(!_drawEdges(&_vertices[0],(int)_vertices.size()/3,&_indices[0],(int)_indices.size(),&_edges[0],geometric->getEdgeBufferIdPtr()));

            // following 2 to reset antialiasing:
            glDisable (GL_LINE_SMOOTH);
            glDisable (GL_BLEND);

            glLineWidth(1.0);

            // Added following on 25/02/2011 (because above we disable the diffuse and specular components!)
            if (displayAttrib&sim_displayattribute_dynamiccontentonly)
            {
                if (dynObjFlag_forVisualization==1)
                    ogl::setMaterialColor(0.9f,0.9f,0.9f,0.5f,0.5f,0.5f,0.0f,0.0f,0.0f);
                if (dynObjFlag_forVisualization==3)
                    ogl::setMaterialColor(0.9f,0.11f,0.11f,0.5f,0.5f,0.5f,0.0f,0.0f,0.0f);
                if (dynObjFlag_forVisualization==2)
                    ogl::setMaterialColor(0.9f,0.11f,0.9f,0.5f,0.5f,0.5f,0.0f,0.0f,0.0f);
            }
            else
            {
                if (collisionColor==nullptr)
                    geometric->color.makeCurrentColor2(false,(displayAttrib&sim_displayattribute_useauxcomponent)!=0);
                else
                    collisionColor->makeCurrentColor2(false,(displayAttrib&sim_displayattribute_useauxcomponent)!=0);
            }

            if (nothingDisplayed&&wire)
            { // Special case, when no edge was displayed (e.g. a smooth sphere)
                for (int pss=0;pss<2;pss++)
                { // we have only a second pass if the inside color is different!
                    glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);
                    _displayTriangles(geometric,geomData->getMeshModificationCounter(),nullptr);
                    glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
                    if (geometric->getInsideAndOutsideFacesSameColor_DEPRECATED()||geometric->getCulling())
                        break; // we leave here.. inside and outside colors are same
                    if ((displayAttrib&sim_displayattribute_dynamiccontentonly)==0)
                    {
                        if (pss==0)
                        {
                            if (wire&&(collisionColor==nullptr))
                                geometric->insideColor_DEPRECATED.makeCurrentColor2(false,(displayAttrib&sim_displayattribute_useauxcomponent)!=0);
                            glCullFace(GL_FRONT);
                        }
                        else
                        { // we reset to initial state
                            if (wire&&(collisionColor==nullptr))
                                geometric->color.makeCurrentColor2(false,(displayAttrib&sim_displayattribute_useauxcomponent)!=0);
                            glCullFace(GL_BACK);
                        }
                    }
                }
            }
        }
        //if (displayAttrib&sim_displayattribute_selected)
        //  multishapeEditSelected=true;
        if (multishapeEditSelected)
        {
            glLineWidth(3.0);
            glLineStipple(1,15);
            glEnable(GL_LINE_STIPPLE);
            ogl::setMaterialColor(0.0,0.0,0.0,0.0,0.0,0.0,1.0,0.0,1.0);
            glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);
            _displayTriangles(geometric,geomData->getMeshModificationCounter(),nullptr);
            glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
            glDisable(GL_LINE_STIPPLE);
            glLineWidth(1.0);
        }
    }
    glFrontFace(GL_CCW);
    glDisable(GL_CULL_FACE);
    glPopAttrib();
    glPopMatrix();
}

void displayGeometric_colorCoded(const C7Vector& cumulIFrameTr,CMesh* geometric,CShape* geomData,int objectId,int displayAttrib)
{
    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    C7Vector _verticeLocalFrame(cumulIFrameTr*geometric->getBB(nullptr));
    glTranslated(_verticeLocalFrame.X(0),_verticeLocalFrame.X(1),_verticeLocalFrame.X(2));
    C4Vector axis=_verticeLocalFrame.Q.getAngleAndAxis();
    glRotated(axis(0)*radToDeg,axis(1),axis(2),axis(3));
    if (geometric->getDisplayInverted_DEPRECATED())
        glFrontFace(GL_CW);

    // set the color-coded object ID:
    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack); // probably not needed
    glColor3ub(objectId&255,(objectId>>8)&255,(objectId>>16)&255);

    if (geometric->getCulling())
        glEnable(GL_CULL_FACE);


    if ((displayAttrib&sim_displayattribute_colorcodedtriangles)!=0)
    {
        std::vector<float>& _vertices=geometric->getVerticesForDisplayAndDisk()[0];
        std::vector<int>& _indices=geometric->getIndices()[0];
        std::vector<float>& _normals=geometric->getNormalsForDisplayAndDisk()[0];
        _drawColorCodedTriangles(&_vertices[0],(int)_vertices.size()/3,&_indices[0],(int)_indices.size(),&_normals[0],geometric->getVertexBufferIdPtr(),geometric->getNormalBufferIdPtr());
    }
    else
        _displayTriangles(geometric,geomData->getMeshModificationCounter(),nullptr);
    glDisable(GL_CULL_FACE);

    glFrontFace(GL_CCW);
    glPopAttrib();
    glPopMatrix();
}

void displayGeometricGhost(const C7Vector& cumulIFrameTr,CMesh* geometric,CShape* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,double transparency,const float* newColors)
{
    if (originalColors)
    {
        displayGeometric(cumulIFrameTr,geometric,geomData,displayAttrib,nullptr,0,0,false);
        return;
    }

    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    C7Vector _verticeLocalFrame(cumulIFrameTr*geometric->getBB(nullptr));
    glTranslated(_verticeLocalFrame.X(0),_verticeLocalFrame.X(1),_verticeLocalFrame.X(2));
    C4Vector axis=_verticeLocalFrame.Q.getAngleAndAxis();
    glRotated(axis(0)*radToDeg,axis(1),axis(2),axis(3));
    if (geometric->getDisplayInverted_DEPRECATED())
        glFrontFace(GL_CW);

    bool wire=(geometric->getWireframe_OLD()||(displayAttrib&sim_displayattribute_forcewireframe))&&((displayAttrib&sim_displayattribute_depthpass)==0);
    bool meantAsWire=wire;
    if (displayAttrib&sim_displayattribute_forbidwireframe)
        wire=false;
    bool edges=geometric->getVisibleEdges();
    if (displayAttrib&sim_displayattribute_forbidedges)
        edges=false;
    if ((!meantAsWire)||((displayAttrib&sim_displayattribute_forvisionsensor)==0)) // added on 2010/08/11 because the wireframe part of the default floor would be rendered solid
    {
        // We now display this object itself:
        ogl::setMaterialColor(newColors,newColors+6,newColors+9);
        ogl::setShininess(geometric->color.getShininess());
        ogl::setAlpha(transparency);
        if (transparency>0.01)
            ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);


        if (geometric->getCulling()||(!geometric->getInsideAndOutsideFacesSameColor_DEPRECATED())||backfaceCulling)
            glEnable(GL_CULL_FACE);
        // REMOVED ON 2010/07/29 so that we see edges over non-edges shapes (e.g. wall elements)    if (edges||wire)
        {
            glPolygonOffset(0.5,0.0); // Second argument set to 0.0 on 2009.01.05 (otherwise strange effects on some graphic cards)
            glEnable(GL_POLYGON_OFFSET_FILL);
        }

        if ((!wire)||(displayAttrib&sim_displayattribute_trianglewireframe))
        {
            for (int pss=0;pss<2;pss++)
            { // we have only a second pass if the inside color is different and no backface culling!
                if (displayAttrib&sim_displayattribute_trianglewireframe)
                    glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);

                if ( (displayAttrib&sim_displayattribute_trianglewireframe)||(displayAttrib&sim_displayattribute_dynamiccontentonly)||(displayAttrib&sim_displayattribute_depthpass) )
                    _displayTriangles(geometric,geomData->getMeshModificationCounter(),nullptr);
                else
                    _displayTriangles(geometric,geomData->getMeshModificationCounter(),geometric->getTextureProperty());

                if (displayAttrib&sim_displayattribute_trianglewireframe)
                    glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
                if (geometric->getInsideAndOutsideFacesSameColor_DEPRECATED()||geometric->getCulling())
                    break; // we leave here.. inside and outside colors are same

                if ((displayAttrib&sim_displayattribute_dynamiccontentonly)==0)
                {
                    ogl::setMaterialColor(newColors,newColors+6,newColors+9);
                    ogl::setShininess(geometric->color.getShininess());
                    ogl::setAlpha(transparency);
                    if (transparency!=0.0)
                        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                    if (pss==0)
                        glCullFace(GL_FRONT);
                    else
                    { // we reset to initial state
                        glCullFace(GL_BACK);
                    }
                }
            }
        }
        glDisable(GL_POLYGON_OFFSET_FILL);

        if ( (edges||wire)&&((displayAttrib&sim_displayattribute_trianglewireframe)==0) )
        {

            if (!wire)
            {
                geometric->edgeColor_DEPRECATED.makeCurrentColor2(false,(displayAttrib&sim_displayattribute_useauxcomponent)!=0);
            }
            else
                ogl::setMaterialColor(nullptr,ogl::colorBlack,nullptr);

            if (displayAttrib&sim_displayattribute_thickEdges)
                glLineWidth(float(4*geometric->getEdgeWidth_DEPRECATED()));
            else
                glLineWidth(float(geometric->getEdgeWidth_DEPRECATED()));


            if (App::userSettings->antiAliasing&&edges&&(!wire))
            {
                glEnable (GL_LINE_SMOOTH);
                glEnable (GL_BLEND);
                glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
            }

            std::vector<float>& _vertices=geometric->getVerticesForDisplayAndDisk()[0];
            std::vector<int>& _indices=geometric->getIndices()[0];
            std::vector<unsigned char>& _edges=geometric->getEdges()[0];
            bool nothingDisplayed=(!_drawEdges(&_vertices[0],(int)_vertices.size()/3,&_indices[0],(int)_indices.size(),&_edges[0],geometric->getEdgeBufferIdPtr()));

            // following 2 to reset antialiasing:
            glDisable (GL_LINE_SMOOTH);
            glDisable (GL_BLEND);

            glLineWidth(1.0);

            ogl::setMaterialColor(newColors,newColors+6,newColors+9);
            ogl::setShininess(geometric->color.getShininess());
            ogl::setAlpha(transparency);
            if (transparency>0.01)
                ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

            if (nothingDisplayed&&wire)
            { // Special case, when no edge was displayed (e.g. a smooth sphere)
                for (int pss=0;pss<2;pss++)
                { // we have only a second pass if the inside color is different!
                    glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);
                    _displayTriangles(geometric,geomData->getMeshModificationCounter(),nullptr);
                    glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
                    if (geometric->getInsideAndOutsideFacesSameColor_DEPRECATED()||geometric->getCulling())
                        break; // we leave here.. inside and outside colors are same
                    if ((displayAttrib&sim_displayattribute_dynamiccontentonly)==0)
                    {
                        if (pss==0)
                            glCullFace(GL_FRONT);
                        else
                        { // we reset to initial state
                            glCullFace(GL_BACK);
                        }
                    }
                }
            }
        }
    }
    glFrontFace(GL_CCW);
    glDisable(GL_CULL_FACE);
    glPopAttrib();
    glPopMatrix();
}

#else

void displayShape(CShape* shape,CViewableBase* renderingObject,int displayAttrib)
{

}

void displayGeometric(const C7Vector& cumulIFrameTr,CMesh* geometric,CShape* geomData,int displayAttrib,CColorObject* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected)
{

}

void displayGeometric_colorCoded(const C7Vector& cumulIFrameTr,CMesh* geometric,CShape* geomData,int objectId,int displayAttrib)
{

}


void displayGeometricGhost(const C7Vector& cumulIFrameTr,CMesh* geometric,CShape* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,double transparency,const float* newColors)
{

}

#endif



