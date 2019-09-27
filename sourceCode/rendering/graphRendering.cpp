
#include "graphRendering.h"

#ifdef SIM_WITH_OPENGL
#include "graphingRoutines.h"

void displayGraph(CGraph* graph,CViewableBase* renderingObject,int displayAttrib)
{ // This is a quite ugly routine which requires refactoring!
    // At the beginning of every 3DObject display routine:
    _commonStart(graph,renderingObject,displayAttrib);

    // Bounding box display:
    float size=graph->getSize();
    if ( (displayAttrib&sim_displayattribute_renderpass)&&(!graph->getJustDrawCurves()) )
        _displayBoundingBox(graph,displayAttrib,true,size/2.0f);

    C3Vector normalVectorForLinesAndPoints(graph->getCumulativeTransformation().Q.getInverse()*C3Vector::unitZVector);

    // Object display:
    if (graph->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
        {
            if (graph->getLocalObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
                glLoadName(graph->getModelSelectionHandle());
            else
                glLoadName(graph->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ( (displayAttrib&sim_displayattribute_forcewireframe)&&(displayAttrib&sim_displayattribute_renderpass) )
            glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

        _enableAuxClippingPlanes(graph->getObjectHandle());
        graph->getColor()->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
        if (graph->xYZPlanesDisplay&&(!graph->getJustDrawCurves())&&((displayAttrib&sim_displayattribute_forvisionsensor)==0))
        {
            int subdiv=5;
            float step=size/(float)subdiv;
            ogl::buffer.clear();
            for (int i=0;i<subdiv+1;i++)
            {
                ogl::addBuffer3DPoints(-size/2.0f+i*step,-size/2.0f,0.0f);
                ogl::addBuffer3DPoints(-size/2.0f+i*step,+size/2.0f,0.0f);
                ogl::addBuffer3DPoints(-size/2.0f,-size/2.0f+i*step,0.0f);
                ogl::addBuffer3DPoints(+size/2.0f,-size/2.0f+i*step,0.0f);

                ogl::addBuffer3DPoints(0.0f,-size/2.0f+i*step,-size/2.0f);
                ogl::addBuffer3DPoints(0.0f,-size/2.0f+i*step,+size/2.0f);
                ogl::addBuffer3DPoints(0.0f,-size/2.0f,-size/2.0f+i*step);
                ogl::addBuffer3DPoints(0.0f,+size/2.0f,-size/2.0f+i*step);

                ogl::addBuffer3DPoints(-size/2.0f+i*step,0.0f,-size/2.0f);
                ogl::addBuffer3DPoints(-size/2.0f+i*step,0.0f,+size/2.0f);
                ogl::addBuffer3DPoints(-size/2.0f,0.0f,-size/2.0f+i*step);
                ogl::addBuffer3DPoints(+size/2.0f,0.0f,-size/2.0f+i*step);
            }
            ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,false,normalVectorForLinesAndPoints.data);
            ogl::buffer.clear();
        }
        if ( (!graph->getJustDrawCurves())&&((displayAttrib&sim_displayattribute_forvisionsensor)==0) )
            ogl::drawSphere(size/32.0f,10,5,true);

        if ((!graph->getJustDrawCurves())&&((displayAttrib&(sim_displayattribute_forvisionsensor|sim_displayattribute_selected))==0))
            ogl::drawReference(size/2.0f,true,true,false,normalVectorForLinesAndPoints.data);


        // Display the trajectories..
        ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
        C7Vector thisInv(graph->getCumulativeTransformation_forDisplay((displayAttrib&sim_displayattribute_forvisionsensor)==0).getInverse());

        for (int i=0;i<int(graph->threeDPartners.size());i++)
        {
            if (graph->threeDPartners[i]->getVisible())
            {
                if ( (graph->getJustDrawCurves()&&graph->threeDPartners[i]->getVisibleOnTopOfEverything()) ||
                    ((!graph->getJustDrawCurves())&&(!graph->threeDPartners[i]->getVisibleOnTopOfEverything())) )
                {
                    CGraphData* part0=graph->getGraphData(graph->threeDPartners[i]->data[0]);
                    CGraphData* part1=graph->getGraphData(graph->threeDPartners[i]->data[1]);
                    CGraphData* part2=graph->getGraphData(graph->threeDPartners[i]->data[2]);
                    int pos=0;
                    int absIndex;
                    float point[3];
                    glLineWidth(graph->threeDPartners[i]->get3DCurveWidth());
                    glPointSize(graph->threeDPartners[i]->get3DCurveWidth());
                    if (graph->threeDPartners[i]->getVisibleOnTopOfEverything())
                    {
                        glDepthMask(GL_FALSE);
                        glDisable(GL_DEPTH_TEST);
                    }
                    bool cyclic0,cyclic1,cyclic2;
                    float range0,range1,range2;
                    if (part0!=nullptr)
                        CGraphingRoutines::getCyclicAndRangeValues(part0,cyclic0,range0);
                    if (part1!=nullptr)
                        CGraphingRoutines::getCyclicAndRangeValues(part1,cyclic1,range1);
                    if (part2!=nullptr)
                        CGraphingRoutines::getCyclicAndRangeValues(part2,cyclic2,range2);

                    ogl::buffer.clear();
                    while (graph->getAbsIndexOfPosition(pos++,absIndex))
                    {
                        bool dataIsValid=true;
                        if (part0!=nullptr)
                        {
                            if(!graph->getData(part0,absIndex,point[0],cyclic0,range0,false))
                                dataIsValid=false;
                        }
                        else
                            dataIsValid=false;
                        if (part1!=nullptr)
                        {
                            if(!graph->getData(part1,absIndex,point[1],cyclic1,range1,false))
                                dataIsValid=false;
                        }
                        else
                            dataIsValid=false;
                        if (part2!=nullptr)
                        {
                            if(!graph->getData(part2,absIndex,point[2],cyclic2,range2,false))
                                dataIsValid=false;
                        }
                        else
                            dataIsValid=false;

                        if (dataIsValid)
                        {
                            C3Vector pp(point);
                            if (graph->threeDPartners[i]->getCurveRelativeToWorld())
                                pp=thisInv*pp;
                            ogl::addBuffer3DPoints(pp.data);
                        }
                    }
                    if (ogl::buffer.size()>0)
                    {
                        ogl::setMaterialColor(graph->threeDPartners[i]->curveColor.colors,ogl::colorBlack,graph->threeDPartners[i]->curveColor.colors+9);
                        if (graph->threeDPartners[i]->getLinkPoints())
                            ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,true,normalVectorForLinesAndPoints.data);
                        else
                            ogl::drawRandom3dPoints(&ogl::buffer[0],(int)ogl::buffer.size()/3,normalVectorForLinesAndPoints.data);
                        if (graph->threeDPartners[i]->getLabel()&&(displayAttrib&sim_displayattribute_renderpass)&&((displayAttrib&sim_displayattribute_forvisionsensor)==0))
                            ogl::drawBitmapTextTo3dPosition(&ogl::buffer[0],graph->threeDPartners[i]->getName(),normalVectorForLinesAndPoints.data);
                    }
                    ogl::buffer.clear();
                    glDepthMask(GL_TRUE);
                    glEnable(GL_DEPTH_TEST);
                    glLineWidth(1.0f);
                    glPointSize(1.0f);
                }
            }
        }

        // Static curves now:
        if ((displayAttrib&sim_displayattribute_forvisionsensor)==0)
        {
            for (int i=0;i<int(graph->_staticCurves.size());i++)
            {
                if (graph->_staticCurves[i]->getCurveType()==2)
                {
                    CStaticGraphCurve* it=graph->_staticCurves[i];
                    glLineWidth(it->getCurveWidth());
                    glPointSize(it->getCurveWidth());
                    glLineStipple(1,0xE187);
                    glEnable(GL_LINE_STIPPLE);
                    ogl::buffer.clear();

                    for (int j=0;j<int(it->values.size()/3);j++)
                    {
                        C3Vector pp(&it->values[3*j+0]);
                        if (it->getRelativeToWorld())
                            pp=thisInv*pp;
                        ogl::addBuffer3DPoints(pp.data);
                    }

                    if (ogl::buffer.size()>0)
                    {
                        ogl::setMaterialColor(it->ambientColor,ogl::colorBlack,it->emissiveColor);
                        if (it->getLinkPoints())
                            ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,true,normalVectorForLinesAndPoints.data);
                        else
                            ogl::drawRandom3dPoints(&ogl::buffer[0],(int)ogl::buffer.size()/3,normalVectorForLinesAndPoints.data);
                        if (it->getLabel()&&(displayAttrib&sim_displayattribute_renderpass)&&((displayAttrib&sim_displayattribute_forvisionsensor)==0))
                            ogl::drawBitmapTextTo3dPosition(&ogl::buffer[0],it->getName()+" [STATIC]",normalVectorForLinesAndPoints.data);
                    }
                    ogl::buffer.clear();
                    glLineWidth(1.0f);
                    glPointSize(1.0f);
                    glDisable(GL_LINE_STIPPLE);
                }
            }
        }

        _disableAuxClippingPlanes();
    }

    // At the end of every 3DObject display routine:
    _commonFinish(graph,renderingObject);
}

#else

void displayGraph(CGraph* graph,CViewableBase* renderingObject,int displayAttrib)
{

}

#endif
