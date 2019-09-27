
#include "shapeRendering.h"

#ifdef SIM_WITH_OPENGL
#include "geomWrap.h"
#include "geometric.h"
#include "pluginContainer.h"

void displayShape(CShape* shape,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(shape,renderingObject,displayAttrib);

    bool guiIsRendering=((displayAttrib&sim_displayattribute_forvisionsensor)==0);

    // Display the bounding box:
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(shape,displayAttrib,true,0.0);

    C3Vector normalVectorForLinesAndPoints(shape->getCumulativeTransformation().Q.getInverse()*C3Vector::unitZVector);

    bool editNormals=false;
    bool editVertices=false;
    bool editEdges=false;
    bool editMultishape=false;
#ifdef SIM_WITH_GUI
    if ( (App::mainWindow!=nullptr)&&(App::mainWindow->editModeContainer->getEditModeObject()==shape) )
    {
        editNormals=( (!shape->isCompound())&&(App::getEditModeType()&TRIANGLE_EDIT_MODE));
        editVertices=( (!shape->isCompound())&&(App::getEditModeType()&VERTEX_EDIT_MODE));
        editEdges=( (!shape->isCompound())&&(App::getEditModeType()&EDGE_EDIT_MODE));
        editMultishape=( shape->isCompound()&&(App::getEditModeType()==MULTISHAPE_EDIT_MODE));
    }
#endif
    bool editMode=editNormals||editVertices||editEdges||editMultishape;

    bool obbVisualizationMode=(shape->getDebugObbStructures()&&((displayAttrib&sim_displayattribute_forvisionsensor)==0));

    if (shape->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib)||editMode||obbVisualizationMode)
    {
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
        {
            if (shape->getLocalObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
                glLoadName(shape->getModelSelectionHandle());
            else
                glLoadName(shape->getObjectHandle());
        }
        else
            glLoadName(-1);

#ifdef SIM_WITH_GUI
        if (editNormals)
        {
            if ((displayAttrib&sim_displayattribute_renderpass)!=0)
                _drawReference(shape,0.0);
            App::mainWindow->editModeContainer->getShapeEditMode()->displayFaceOrientation(displayAttrib);
        }
        else if (editVertices)
        {
            if ((displayAttrib&sim_displayattribute_renderpass)!=0)
                _drawReference(shape,0.0);
            App::mainWindow->editModeContainer->getShapeEditMode()->displayVertices(displayAttrib);
        }
        else if (editEdges)
        {
            if ((displayAttrib&sim_displayattribute_renderpass)!=0)
                _drawReference(shape,0.0);
            App::mainWindow->editModeContainer->getShapeEditMode()->displayEdgeEditMode(displayAttrib);
        }
        else if (editMultishape)
        {
            if ((displayAttrib&sim_displayattribute_renderpass)!=0)
                _drawReference(shape,0.0);
            displayAttrib|=(sim_displayattribute_forbidedges|sim_displayattribute_forcewireframe|sim_displayattribute_trianglewireframe);
            displayAttrib-=(sim_displayattribute_forbidedges|sim_displayattribute_forcewireframe|sim_displayattribute_trianglewireframe);
            bool textEnabledSaved=App::ct->environment->getShapeTexturesEnabled();
            App::ct->environment->setShapeTexturesEnabled(true);
            if (shape->getContainsTransparentComponent())
            {
                App::mainWindow->editModeContainer->getMultishapeEditMode()->displayAllGeometricComponents(shape->geomData,displayAttrib,nullptr,0,2);
                App::mainWindow->editModeContainer->getMultishapeEditMode()->displayAllGeometricComponents(shape->geomData,displayAttrib,nullptr,0,1);
            }
            else
                App::mainWindow->editModeContainer->getMultishapeEditMode()->displayAllGeometricComponents(shape->geomData,displayAttrib,nullptr,0,0);
            App::ct->environment->setShapeTexturesEnabled(textEnabledSaved);
        }
        else
#endif
        {
            _enableAuxClippingPlanes(shape->getObjectHandle());

            CVisualParam otherColor;
            CVisualParam* otherColorP=nullptr;
            if ((displayAttrib&sim_displayattribute_originalcolors)==0)
            {
                bool setOtherColor=(App::ct->collisions->getCollisionColor(shape->getObjectHandle())!=0);
                for (size_t i=0;i<App::ct->collections->allCollections.size();i++)
                {
                    if (App::ct->collections->allCollections[i]->isObjectInCollection(shape->getObjectHandle()))
                        setOtherColor|=(App::ct->collisions->getCollisionColor(App::ct->collections->allCollections[i]->getCollectionID())!=0);
                }
                if (setOtherColor)
                {
                    App::ct->mainSettings->collisionColor.copyYourselfInto(&otherColor);
                    otherColorP=&otherColor;
                }
            }
/*
            if (displayAttrib&sim_displayattribute_selected)
            {
                if (displayAttrib&sim_displayattribute_mode1)
                {
                    otherColor.setDefaultValues();
                    otherColor.setColor(0.8f,0.8f,0.0,sim_colorcomponent_ambient_diffuse);
                    otherColorP=&otherColor;
                }
                if (displayAttrib&sim_displayattribute_mode2)
                {
                    otherColor.setDefaultValues();
                    otherColor.setColor(0.0f,0.8f,0.0,sim_colorcomponent_ambient_diffuse);
                    otherColorP=&otherColor;
                }
            }
*/
            // Display the mass and inertia:
            if ((displayAttrib&sim_displayattribute_inertiaonly)!=0)
            {
                C3Vector v(shape->geomData->getBoundingBoxHalfSizes()*2.0f);
                _displayInertia(shape->geomData->geomInfo,sqrt(v*v),normalVectorForLinesAndPoints.data);
                otherColor.setDefaultValues();
                otherColor.colors[0]=0.8f;
                otherColor.colors[1]=0.8f;
                otherColor.colors[2]=0.65f;
                otherColorP=&otherColor;
            }

            if (shape->geomData->isCollisionInformationInitialized()&&(CPluginContainer::mesh_getCalculatedPolygonCount(shape->geomData->collInfo)!=0))
                ((CGeometric*)shape->geomData->geomInfo)->displayForCutting(shape->geomData,displayAttrib,otherColorP,normalVectorForLinesAndPoints.data);
            else
            {
                if (renderingObject->isObjectInsideView(shape->getCumulativeTransformation(),shape->geomData->getBoundingBoxHalfSizes()))
                { // the bounding box is inside of the view (at least some part of it!)
                    if ((displayAttrib&sim_displayattribute_colorcoded)==0)
                    { // normal visualization
                        if (obbVisualizationMode)
                        { // visualize OBB calculation structures
                            CVisualParam fakeCol;
                            fakeCol.setDefaultValues();
                            if (shape->geomData->isCollisionInformationInitialized())
                            {
                                fakeCol.setColor(0.5f,0.1f,0.1f,0);
                                shape->geomData->geomInfo->display(shape->geomData,(displayAttrib|sim_displayattribute_trianglewireframe)-sim_displayattribute_trianglewireframe,&fakeCol,shape->getDynamicObjectFlag_forVisualization_forDisplay(guiIsRendering),0,false);
                            }
                            else
                            {
                                fakeCol.setColor(0.5f,0.5f,0.5f,0);
                                shape->geomData->geomInfo->display(shape->geomData,displayAttrib|sim_displayattribute_trianglewireframe,&fakeCol,shape->getDynamicObjectFlag_forVisualization_forDisplay(guiIsRendering),0,false);
                            }
                        }
                        else
                        { // normal visualization
                            if (shape->getContainsTransparentComponent())
                            {
                                shape->geomData->geomInfo->display(shape->geomData,displayAttrib,otherColorP,shape->getDynamicObjectFlag_forVisualization_forDisplay(guiIsRendering),2,false);
                                shape->geomData->geomInfo->display(shape->geomData,displayAttrib,otherColorP,shape->getDynamicObjectFlag_forVisualization_forDisplay(guiIsRendering),1,false);
                            }
                            else
                                shape->geomData->geomInfo->display(shape->geomData,displayAttrib,otherColorP,shape->getDynamicObjectFlag_forVisualization_forDisplay(guiIsRendering),0,false);
                        }
                    }
                    else
                        shape->geomData->geomInfo->display_colorCoded(shape->geomData,shape->getObjectHandle(),displayAttrib); // color-coded visualization
                }
            }
            _disableAuxClippingPlanes();
        }
    }

    // At the end of every 3DObject display routine:
    _commonFinish(shape,renderingObject);
}

void _displayInertia(CGeomWrap* geomWrap,float bboxDiagonal,const float normalVectorForPointsAndLines[3])
{
    C7Vector tr(geomWrap->getLocalInertiaFrame());
    glPushMatrix();
    glTranslatef(tr.X(0),tr.X(1),tr.X(2));
    C4Vector axis=tr.Q.getAngleAndAxisNoChecking();
    glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));

    ogl::setMaterialColor(ogl::colorRed,ogl::colorBlack,ogl::colorBlack);

    C3Vector me(geomWrap->getPrincipalMomentsOfInertia());
    C3Vector ma;
    ma(0)=(me(1)+me(2))*(me(1)+me(2));
    ma(1)=(me(0)+me(2))*(me(0)+me(2));
    ma(2)=(me(1)+me(0))*(me(1)+me(0));
    ma.normalize();
    float mf=bboxDiagonal*0.6f;
    ogl::buffer.clear();
    ogl::addBuffer3DPoints(-ma(0)*mf,-ma(1)*mf,+ma(2)*mf);
    ogl::addBuffer3DPoints(-ma(0)*mf,-ma(1)*mf,-ma(2)*mf);
    ogl::addBuffer3DPoints(+ma(0)*mf,-ma(1)*mf,-ma(2)*mf);
    ogl::addBuffer3DPoints(+ma(0)*mf,-ma(1)*mf,+ma(2)*mf);
    ogl::addBuffer3DPoints(-ma(0)*mf,-ma(1)*mf,+ma(2)*mf);
    ogl::addBuffer3DPoints(-ma(0)*mf,+ma(1)*mf,+ma(2)*mf);
    ogl::addBuffer3DPoints(+ma(0)*mf,+ma(1)*mf,+ma(2)*mf);
    ogl::addBuffer3DPoints(+ma(0)*mf,+ma(1)*mf,-ma(2)*mf);
    ogl::addBuffer3DPoints(-ma(0)*mf,+ma(1)*mf,-ma(2)*mf);
    ogl::addBuffer3DPoints(-ma(0)*mf,+ma(1)*mf,+ma(2)*mf);
    ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,true,normalVectorForPointsAndLines);
    ogl::buffer.clear();
    ogl::addBuffer3DPoints(-ma(0)*mf,-ma(1)*mf,-ma(2)*mf);
    ogl::addBuffer3DPoints(-ma(0)*mf,+ma(1)*mf,-ma(2)*mf);
    ogl::addBuffer3DPoints(+ma(0)*mf,-ma(1)*mf,-ma(2)*mf);
    ogl::addBuffer3DPoints(+ma(0)*mf,+ma(1)*mf,-ma(2)*mf);
    ogl::addBuffer3DPoints(+ma(0)*mf,-ma(1)*mf,+ma(2)*mf);
    ogl::addBuffer3DPoints(+ma(0)*mf,+ma(1)*mf,+ma(2)*mf);
    ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,false,normalVectorForPointsAndLines);
    ogl::buffer.clear();

    glLineWidth(3.0f);
    ogl::drawReference(bboxDiagonal*0.5f,true,false,false,normalVectorForPointsAndLines);

    ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,0.4f,0.0f,1.0f);
    float l=pow(geomWrap->getMass()/1000.0f,0.3333f); // Cubic root and mass density of 1000
    glLineWidth(6.0f);
    ogl::drawBox(l,l,l,false,normalVectorForPointsAndLines);
    glLineWidth(1.0f);

    glPopMatrix();
}

void _displayTriangles(CGeometric* geometric,int geomModifCounter,CTextureProperty* tp)
{
    if ((!App::ct->environment->getShapeTexturesEnabled())||CEnvironment::getShapeTexturesTemporarilyDisabled())
        tp=nullptr;
    std::vector<float>* textureCoords=nullptr;
    int* texCoordBufferIdPtr=nullptr;
    const std::vector<float>& _vertices=geometric->getVertices()[0];
    const std::vector<int>& _indices=geometric->getIndices()[0];
    const std::vector<float>& _normals=geometric->getNormals()[0];
    if (tp!=nullptr)
    {
        textureCoords=tp->getTextureCoordinates(geomModifCounter,geometric->getVerticeLocalFrame(),_vertices,_indices);
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


void displayGeometric(CGeometric* geometric,CGeomProxy* geomData,int displayAttrib,CVisualParam* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected)
{

    if (transparencyHandling!=0)
    {// 0=display always, 1=display transparent only, 2=display non-transparent only
        if ((transparencyHandling==1)&&(!geometric->getContainsTransparentComponents()))
            return;
        if ((transparencyHandling==2)&&(geometric->getContainsTransparentComponents()))
            return;
    }
    C7Vector _verticeLocalFrame(geometric->getVerticeLocalFrame());
    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    glTranslatef(_verticeLocalFrame.X(0),_verticeLocalFrame.X(1),_verticeLocalFrame.X(2));
    C4Vector axis=_verticeLocalFrame.Q.getAngleAndAxisNoChecking();
    glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
    if (geometric->getDisplayInverted_DEPRECATED())
        glFrontFace(GL_CW);

    bool wire=(geometric->getWireframe()||(displayAttrib&sim_displayattribute_forcewireframe))&&((displayAttrib&sim_displayattribute_depthpass)==0);
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
            nonPureDynamicDisplay=(geometric->getPurePrimitiveType()==sim_pure_primitive_none);
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
            glPolygonOffset(0.5f,0.0f); // Second argument set to 0.0 on 2009.01.05 (otherwise strange effects on some graphic cards)
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
                        _displayTriangles(geometric,geomData->getGeomDataModificationCounter(),nullptr);
                    else
                        _displayTriangles(geometric,geomData->getGeomDataModificationCounter(),geometric->getTextureProperty());

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
                _displayTriangles(geometric,geomData->getGeomDataModificationCounter(),nullptr);
            }
            glDisable(GL_POLYGON_OFFSET_FILL);
            glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);

            if (geometric->isConvex())
                ogl::setMaterialColor(1.0f,1.0f,1.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
            else
                ogl::setMaterialColor(0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
            _displayTriangles(geometric,geomData->getGeomDataModificationCounter(),nullptr);
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
            std::vector<float>& _vertices=geometric->getVertices()[0];
            std::vector<int>& _indices=geometric->getIndices()[0];
            std::vector<unsigned char>& _edges=geometric->getEdges()[0];
            bool nothingDisplayed=(!_drawEdges(&_vertices[0],(int)_vertices.size()/3,&_indices[0],(int)_indices.size(),&_edges[0],geometric->getEdgeBufferIdPtr()));

            // following 2 to reset antialiasing:
            glDisable (GL_LINE_SMOOTH);
            glDisable (GL_BLEND);

            glLineWidth(1.0f);

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
                    _displayTriangles(geometric,geomData->getGeomDataModificationCounter(),nullptr);
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
            glLineWidth(3.0f);
            glLineStipple(1,15);
            glEnable(GL_LINE_STIPPLE);
            ogl::setMaterialColor(0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,1.0f);
            glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);
            _displayTriangles(geometric,geomData->getGeomDataModificationCounter(),nullptr);
            glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
            glDisable(GL_LINE_STIPPLE);
            glLineWidth(1.0f);
        }
    }
    glFrontFace(GL_CCW);
    glDisable(GL_CULL_FACE);
    glPopAttrib();
    glPopMatrix();
}

void displayGeometric_colorCoded(CGeometric* geometric,CGeomProxy* geomData,int objectId,int displayAttrib)
{
    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    C7Vector _verticeLocalFrame(geometric->getVerticeLocalFrame());
    glTranslatef(_verticeLocalFrame.X(0),_verticeLocalFrame.X(1),_verticeLocalFrame.X(2));
    C4Vector axis=_verticeLocalFrame.Q.getAngleAndAxisNoChecking();
    glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
    if (geometric->getDisplayInverted_DEPRECATED())
        glFrontFace(GL_CW);

    // set the color-coded object ID:
    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack); // probably not needed
    glColor3ub(objectId&255,(objectId>>8)&255,(objectId>>16)&255);

    if (geometric->getCulling())
        glEnable(GL_CULL_FACE);


    if ((displayAttrib&sim_displayattribute_colorcodedtriangles)!=0)
    {
        std::vector<float>& _vertices=geometric->getVertices()[0];
        std::vector<int>& _indices=geometric->getIndices()[0];
        std::vector<float>& _normals=geometric->getNormals()[0];
        _drawColorCodedTriangles(&_vertices[0],(int)_vertices.size()/3,&_indices[0],(int)_indices.size(),&_normals[0],geometric->getVertexBufferIdPtr(),geometric->getNormalBufferIdPtr());
    }
    else
        _displayTriangles(geometric,geomData->getGeomDataModificationCounter(),nullptr);
    glDisable(GL_CULL_FACE);

    glFrontFace(GL_CCW);
    glPopAttrib();
    glPopMatrix();
}

void displayGeometricForCutting(CGeometric* geometric,CGeomProxy* geomData,int displayAttrib,CVisualParam* collisionColor,const float normalVectorForPointsAndLines[3])
{
    glPushAttrib(GL_POLYGON_BIT);

    bool wire=(geometric->getWireframe()||(displayAttrib&sim_displayattribute_forcewireframe))&&((displayAttrib&sim_displayattribute_depthpass)==0);
    if (displayAttrib&sim_displayattribute_forbidwireframe)
        wire=false;
    bool edges=geometric->getVisibleEdges();
    if (displayAttrib&sim_displayattribute_forbidedges)
        edges=false;
    if (collisionColor==nullptr)
        geometric->color.makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
    else
        collisionColor->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
    if (geometric->getCulling()||(!geometric->getInsideAndOutsideFacesSameColor_DEPRECATED()))
        glEnable(GL_CULL_FACE);
// REMOVED ON 2010/07/29 so that we see edges over non-edges shapes (e.g. wall elements)    if (edges||wire)
    {
        glPolygonOffset(0.5f,0.0f); // Second argument set to 0.0 on 2009.01.05 (otherwise strange effects on some graphic cards)
        glEnable(GL_POLYGON_OFFSET_FILL);
    }
    C3Vector v0,v1,v2;
    int ind[3];
    CVisualParam col;
    if ( (!wire)||(displayAttrib&sim_displayattribute_trianglewireframe) )
    {
        if (displayAttrib&sim_displayattribute_trianglewireframe)
            glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

        for (int pss=0;pss<2;pss++)
        { // we have only a second pass if the inside color is different!
            // FIRST TRIANGLES:
            glBegin(GL_TRIANGLES);
            int calcTriangleCount=CPluginContainer::mesh_getCalculatedTriangleCount(geomData->collInfo);
            int* calcIndices=CPluginContainer::mesh_getCalculatedTrianglesPointer(geomData->collInfo);
            float* calcVertices=CPluginContainer::mesh_getCalculatedVerticesPointer(geomData->collInfo);
            for (int i=0;i<calcTriangleCount;i++)
            {
                ind[0]=calcIndices[3*i+0];
                if (ind[0]!=-1)
                {
                    ind[1]=calcIndices[3*i+1];
                    ind[2]=calcIndices[3*i+2];
                    v0.set(calcVertices+3*ind[0]);
                    v1.set(calcVertices+3*ind[1]);
                    v2.set(calcVertices+3*ind[2]);
                    v1-=v0;
                    v2-=v0;
                    v0=(v1^v2).getNormalized();
                    glNormal3f(v0(0),v0(1),v0(2));
                    glVertex3fv(calcVertices+3*ind[0]);
                    glVertex3fv(calcVertices+3*ind[1]);
                    glVertex3fv(calcVertices+3*ind[2]);
                }
            }
            glEnd();

            // THEN POLYGONS:
            int calcPolygonCount=CPluginContainer::mesh_getCalculatedPolygonCount(geomData->collInfo);
            for (int i=0;i<calcPolygonCount;i++)
            {
                int polSize=CPluginContainer::mesh_getCalculatedPolygonSize(geomData->collInfo,i);
                int* pol=CPluginContainer::mesh_getCalculatedPolygonArrayPointer(geomData->collInfo,i);
                if (polSize!=0)
                {
                    glBegin(GL_TRIANGLE_FAN); // GL_POLYGON is problematic on certain graphic cards!
                    v0.set(calcVertices+3*pol[0]);
                    v1.set(calcVertices+3*pol[1]);
                    v2.set(calcVertices+3*pol[2]);
                    v1-=v0;
                    v2-=v0;
                    v0=(v1^v2).getNormalized();
                    glNormal3f(v0(0),v0(1),v0(2));
                    for (int j=0;j<polSize-2;j++)
                    {
                        glVertex3fv(calcVertices+3*pol[0]);
                        glVertex3fv(calcVertices+3*pol[j+1]);
                        glVertex3fv(calcVertices+3*pol[j+2]);
                    }
                    glEnd();
                }
            }
            if (geometric->getInsideAndOutsideFacesSameColor_DEPRECATED()||geometric->getCulling())
                break; // we leave here.. inside and outside colors are same
            if (pss==0)
            {
                if (collisionColor==nullptr)
                    geometric->insideColor_DEPRECATED.makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
                glCullFace(GL_FRONT);
            }
            else
            { // we reset to initial state
                if (collisionColor==nullptr)
                    geometric->color.makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
                glCullFace(GL_BACK);
            }
        }
        if (displayAttrib&sim_displayattribute_trianglewireframe)
            glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
    }

    glDisable(GL_POLYGON_OFFSET_FILL);

    if ( edges||(wire&&((displayAttrib&sim_displayattribute_trianglewireframe)==0)) )
    {
        if (!wire)
            geometric->edgeColor_DEPRECATED.makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);

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

        ogl::buffer.clear();
        int calcSegmentsCount=CPluginContainer::mesh_getCalculatedSegmentCount(geomData->collInfo);
        int* calcSegments=CPluginContainer::mesh_getCalculatedSegmentsPointer(geomData->collInfo);
        float* calcVertices=CPluginContainer::mesh_getCalculatedVerticesPointer(geomData->collInfo);
        for (int i=0;i<calcSegmentsCount;i++)
        {
            ind[0]=calcSegments[2*i+0];
            if (ind[0]!=-1)
            {
                ind[1]=calcSegments[2*i+1];
                ogl::addBuffer3DPoints(calcVertices+3*ind[0]);
                ogl::addBuffer3DPoints(calcVertices+3*ind[1]);
            }

        }
        if (ogl::buffer.size()!=0)
            ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,false,normalVectorForPointsAndLines);
        ogl::buffer.clear();

        // following 2 to reset antialiasing:
        glDisable (GL_LINE_SMOOTH);
        glDisable (GL_BLEND);

        glLineWidth(1.0f);
    }
    glDisable(GL_CULL_FACE);
    glPopAttrib();
}

void displayGeometricGhost(CGeometric* geometric,CGeomProxy* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,float transparency,const float* newColors)
{
    if (originalColors)
    {
        displayGeometric(geometric,geomData,displayAttrib,nullptr,0,0,false);
        return;
    }

    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    C7Vector _verticeLocalFrame(geometric->getVerticeLocalFrame());
    glTranslatef(_verticeLocalFrame.X(0),_verticeLocalFrame.X(1),_verticeLocalFrame.X(2));
    C4Vector axis=_verticeLocalFrame.Q.getAngleAndAxisNoChecking();
    glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
    if (geometric->getDisplayInverted_DEPRECATED())
        glFrontFace(GL_CW);

    bool wire=(geometric->getWireframe()||(displayAttrib&sim_displayattribute_forcewireframe))&&((displayAttrib&sim_displayattribute_depthpass)==0);
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
        ogl::setShininess(geometric->color.shininess);
        ogl::setAlpha(transparency);
        if (transparency>0.01f)
            ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);


        if (geometric->getCulling()||(!geometric->getInsideAndOutsideFacesSameColor_DEPRECATED())||backfaceCulling)
            glEnable(GL_CULL_FACE);
        // REMOVED ON 2010/07/29 so that we see edges over non-edges shapes (e.g. wall elements)    if (edges||wire)
        {
            glPolygonOffset(0.5f,0.0f); // Second argument set to 0.0 on 2009.01.05 (otherwise strange effects on some graphic cards)
            glEnable(GL_POLYGON_OFFSET_FILL);
        }

        if ((!wire)||(displayAttrib&sim_displayattribute_trianglewireframe))
        {
            for (int pss=0;pss<2;pss++)
            { // we have only a second pass if the inside color is different and no backface culling!
                if (displayAttrib&sim_displayattribute_trianglewireframe)
                    glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);

                if ( (displayAttrib&sim_displayattribute_trianglewireframe)||(displayAttrib&sim_displayattribute_dynamiccontentonly)||(displayAttrib&sim_displayattribute_depthpass) )
                    _displayTriangles(geometric,geomData->getGeomDataModificationCounter(),nullptr);
                else
                    _displayTriangles(geometric,geomData->getGeomDataModificationCounter(),geometric->getTextureProperty());

                if (displayAttrib&sim_displayattribute_trianglewireframe)
                    glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
                if (geometric->getInsideAndOutsideFacesSameColor_DEPRECATED()||geometric->getCulling())
                    break; // we leave here.. inside and outside colors are same

                if ((displayAttrib&sim_displayattribute_dynamiccontentonly)==0)
                {
                    ogl::setMaterialColor(newColors,newColors+6,newColors+9);
                    ogl::setShininess(geometric->color.shininess);
                    ogl::setAlpha(transparency);
                    if (transparency!=0.0f)
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

            std::vector<float>& _vertices=geometric->getVertices()[0];
            std::vector<int>& _indices=geometric->getIndices()[0];
            std::vector<unsigned char>& _edges=geometric->getEdges()[0];
            bool nothingDisplayed=(!_drawEdges(&_vertices[0],(int)_vertices.size()/3,&_indices[0],(int)_indices.size(),&_edges[0],geometric->getEdgeBufferIdPtr()));

            // following 2 to reset antialiasing:
            glDisable (GL_LINE_SMOOTH);
            glDisable (GL_BLEND);

            glLineWidth(1.0f);

            ogl::setMaterialColor(newColors,newColors+6,newColors+9);
            ogl::setShininess(geometric->color.shininess);
            ogl::setAlpha(transparency);
            if (transparency>0.01f)
                ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

            if (nothingDisplayed&&wire)
            { // Special case, when no edge was displayed (e.g. a smooth sphere)
                for (int pss=0;pss<2;pss++)
                { // we have only a second pass if the inside color is different!
                    glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);
                    _displayTriangles(geometric,geomData->getGeomDataModificationCounter(),nullptr);
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

void displayGeometric(CGeometric* geometric,CGeomProxy* geomData,int displayAttrib,CVisualParam* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected)
{

}

void displayGeometric_colorCoded(CGeometric* geometric,CGeomProxy* geomData,int objectId,int displayAttrib)
{

}

void displayGeometricForCutting(CGeometric* geometric,CGeomProxy* geomData,int displayAttrib,CVisualParam* collisionColor,const float normalVectorForPointsAndLines[3])
{

}

void displayGeometricGhost(CGeometric* geometric,CGeomProxy* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,float transparency,const float* newColors)
{

}

#endif



