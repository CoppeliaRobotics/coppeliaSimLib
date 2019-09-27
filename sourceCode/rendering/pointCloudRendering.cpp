
#include "pointCloudRendering.h"

#ifdef SIM_WITH_OPENGL
#include "pluginContainer.h"

void displayPointCloud(CPointCloud* pointCloud,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(pointCloud,renderingObject,displayAttrib);

    C3Vector mmaDim,mmiDim;
    pointCloud->getMaxMinDims(mmaDim,mmiDim);
    C3Vector d(mmaDim-mmiDim);
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(pointCloud,displayAttrib,true,cbrt(d(0)*d(1)*d(2))*0.6f);

    C3Vector normalVectorForLinesAndPoints(pointCloud->getCumulativeTransformation().Q.getInverse()*C3Vector::unitZVector);

    // Object display:
    if (pointCloud->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
        {
            if (pointCloud->getLocalObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
                glLoadName(pointCloud->getModelSelectionHandle());
            else
                glLoadName(pointCloud->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ( (displayAttrib&sim_displayattribute_forcewireframe)&&(displayAttrib&sim_displayattribute_renderpass) )
            glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);
        if ( (displayAttrib&sim_displayattribute_forcewireframe)==0)
            glEnable(GL_CULL_FACE);

        _enableAuxClippingPlanes(pointCloud->getObjectHandle());
//      if ((displayAttrib&sim_displayattribute_selected)!=0)
//          ogl::drawReference(size*1.2f,true,true,false,normalVectorForLinesAndPoints.data);
//      ogl::setMaterialColor(0.0f,0.0f,0.0f,0.5f,0.5f,0.5f,0.0f,0.0f,0.0f);
//      color.makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
//      ogl::drawBox(size,size,size,false,normalVectorForLinesAndPoints.data);
//      ogl::drawSphere(size/2.0f,12,6,false);

        std::vector<float>& _points=pointCloud->getPoints()[0];
        if (_points.size()>0)
        {
            bool setOtherColor=(App::ct->collisions->getCollisionColor(pointCloud->getObjectHandle())!=0);
            for (size_t i=0;i<App::ct->collections->allCollections.size();i++)
            {
                if (App::ct->collections->allCollections[i]->isObjectInCollection(pointCloud->getObjectHandle()))
                    setOtherColor|=(App::ct->collisions->getCollisionColor(App::ct->collections->allCollections[i]->getCollectionID())!=0);
            }

            if (displayAttrib&sim_displayattribute_forvisionsensor)
                setOtherColor=false;

            if (!setOtherColor)
                pointCloud->getColor()->makeCurrentColor(false);
            else
                App::ct->mainSettings->collisionColor.makeCurrentColor(false);

            if (pointCloud->getShowOctree()&&(pointCloud->getPointCloudInfo()!=nullptr)&&((displayAttrib&sim_displayattribute_forvisionsensor)==0))
            {
                std::vector<float> corners;
                CPluginContainer::mesh_getPointCloudDebugCorners(pointCloud->getPointCloudInfo(),corners);

                glBegin(GL_LINES);
                glNormal3fv(normalVectorForLinesAndPoints.data);

                for (size_t i=0;i<corners.size()/24;i++)
                {
                    glVertex3fv(&corners[0]+i*8*3+0);
                    glVertex3fv(&corners[0]+i*8*3+3);
                    glVertex3fv(&corners[0]+i*8*3+3);
                    glVertex3fv(&corners[0]+i*8*3+9);
                    glVertex3fv(&corners[0]+i*8*3+0);
                    glVertex3fv(&corners[0]+i*8*3+6);
                    glVertex3fv(&corners[0]+i*8*3+6);
                    glVertex3fv(&corners[0]+i*8*3+9);

                    glVertex3fv(&corners[0]+i*8*3+12);
                    glVertex3fv(&corners[0]+i*8*3+15);
                    glVertex3fv(&corners[0]+i*8*3+15);
                    glVertex3fv(&corners[0]+i*8*3+21);
                    glVertex3fv(&corners[0]+i*8*3+12);
                    glVertex3fv(&corners[0]+i*8*3+18);
                    glVertex3fv(&corners[0]+i*8*3+18);
                    glVertex3fv(&corners[0]+i*8*3+21);

                    glVertex3fv(&corners[0]+i*8*3+0);
                    glVertex3fv(&corners[0]+i*8*3+12);

                    glVertex3fv(&corners[0]+i*8*3+3);
                    glVertex3fv(&corners[0]+i*8*3+15);

                    glVertex3fv(&corners[0]+i*8*3+6);
                    glVertex3fv(&corners[0]+i*8*3+18);

                    glVertex3fv(&corners[0]+i*8*3+9);
                    glVertex3fv(&corners[0]+i*8*3+21);
                }
                glEnd();
            }


            glPointSize(float(pointCloud->getPointSize()));
            std::vector<float>* pts=&_points;
            std::vector<float>* cols=pointCloud->getColors();
            if (pointCloud->getDisplayPoints()->size()>0)
            {
                pts=pointCloud->getDisplayPoints();
                cols=pointCloud->getDisplayColors();
            }

            if ((cols->size()==0)||setOtherColor)
            {
                glBegin(GL_POINTS);
                glNormal3fv(normalVectorForLinesAndPoints.data);
                for (size_t i=0;i<pts->size()/3;i++)
                    glVertex3fv(&(pts[0])[3*i]);
                glEnd();
            }
            else
            {
                glBegin(GL_POINTS);
                glNormal3fv(normalVectorForLinesAndPoints.data);
                if (pointCloud->getColorIsEmissive())
                {
                    const float blk[4]={0.0,0.0,0.0,0.0};
                    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,blk);
                    for (size_t i=0;i<pts->size()/3;i++)
                    {
                        glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,&(cols[0])[4*i]);
                        glVertex3fv(&(pts[0])[3*i]);
                    }
                }
                else
                {
                    for (size_t i=0;i<pts->size()/3;i++)
                    {
                        glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,&(cols[0])[4*i]);
                        glVertex3fv(&(pts[0])[3*i]);
                    }
                }
                glEnd();
            }
            glPointSize(1.0);
        }


        glDisable(GL_CULL_FACE);
        _disableAuxClippingPlanes();
    }

    // At the end of every 3DObject display routine:
    _commonFinish(pointCloud,renderingObject);
}

#else

void displayPointCloud(CPointCloud* pointCloud,CViewableBase* renderingObject,int displayAttrib)
{

}

#endif



