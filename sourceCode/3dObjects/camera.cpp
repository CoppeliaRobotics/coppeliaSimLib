
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "camera.h"
#include "v_rep_internal.h"
#include "tt.h"
#include "meshManip.h"
#include "global.h"
#include "sceneObjectOperations.h"
#include "graphingRoutines.h"
#include "pluginContainer.h"
#include "v_repStrings.h"
#include "vDateTime.h"
#include "ttUtil.h"
#include "easyLock.h"
#include "app.h"
#include "cameraRendering.h"
#ifdef SIM_WITH_OPENGL
#include "rendering.h"
#include "oGL.h"
#include "glShader.h"
#endif

unsigned int CCamera::selectBuff[SELECTION_BUFFER_SIZE*4];

CCamera::CCamera()
{
    commonInit();
}

std::string CCamera::getObjectTypeInfo() const
{
    return(IDSOGL_CAMERA);
}
std::string CCamera::getObjectTypeInfoExtended() const
{
    return(IDSOGL_CAMERA);
}
bool CCamera::isPotentiallyCollidable() const
{
    return(false);
}
bool CCamera::isPotentiallyMeasurable() const
{
    return(false);
}
bool CCamera::isPotentiallyDetectable() const
{
    return(false);
}
bool CCamera::isPotentiallyRenderable() const
{
    return(false);
}
bool CCamera::isPotentiallyCuttable() const
{
    return(false);
}

CVisualParam* CCamera::getColor(bool secondPart)
{
    if (secondPart)
        return(&colorPart2);
    return(&colorPart1);
}

void CCamera::frameSceneOrSelectedObjects(float windowWidthByHeight,bool forPerspectiveProjection,std::vector<int>* selectedObjects,bool useSystemSelection,bool includeModelObjects,float scalingFactor,CSView* optionalView)
{
    std::vector<float> pts;
    C7Vector camTr(getCumulativeTransformation());
    C7Vector camTrInv(camTr.getInverse());
    int editMode=NO_EDIT_MODE;
    int displAttributes=0;
    if (optionalView!=nullptr)
    {
        if (optionalView->getVisualizeOnlyInertias())
            displAttributes=sim_displayattribute_inertiaonly;
    }
    if (App::ct->simulation->getDynamicContentVisualizationOnly())
        displAttributes=sim_displayattribute_dynamiccontentonly;

#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
    {
        editMode=App::getEditModeType();
        if ((editMode&SHAPE_EDIT_MODE)!=0)
        { // just take the vertices
            if (App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVerticesSize()==0)
                editMode=NO_EDIT_MODE;
            else
            {
                C3DObject* parentObj=App::mainWindow->editModeContainer->getEditModeShape();
                if (parentObj!=nullptr)
                {
                    C7Vector parentTr(parentObj->getCumulativeTransformation());
                    if (editMode==VERTEX_EDIT_MODE)
                    {
                        for (int i=0;i<App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVerticesSize()/3;i++)
                        {
                            C3Vector v(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(i));
                            v=camTrInv*parentTr*v;
                            pts.push_back(v(0));
                            pts.push_back(v(1));
                            pts.push_back(v(2));
                        }
                        scalingFactor*=0.98f;
                    }
                    else
                    { // Triangle or edge edit mode:
                        if (App::mainWindow->editModeContainer->getShapeEditMode()->getEditionIndicesSize()!=0)
                        {
                            for (int i=0;i<App::mainWindow->editModeContainer->getShapeEditMode()->getEditionIndicesSize()/3;i++)
                            { // here we will get every vertex many times, but it doesn't really matter
                                int ind[3];
                                App::mainWindow->editModeContainer->getShapeEditMode()->getEditionTriangle(i,ind);
                                for (int j=0;j<3;j++)
                                {
                                    C3Vector v(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind[j]));
                                    v=camTrInv*parentTr*v;
                                    pts.push_back(v(0));
                                    pts.push_back(v(1));
                                    pts.push_back(v(2));
                                }
                            }
                            scalingFactor*=0.98f;
                        }
                        else
                            editMode=NO_EDIT_MODE;
                    }
                }
                else
                    editMode=NO_EDIT_MODE;
            }
        }
        if ((editMode&PATH_EDIT_MODE)!=0)
        { // just take the path points
            if (App::mainWindow->editModeContainer->getEditModePathContainer()==nullptr)
                editMode=NO_EDIT_MODE;
            else
            {
                int cnt=App::mainWindow->editModeContainer->getEditModePathContainer()->getSimplePathPointCount();
                CPath* path=App::mainWindow->editModeContainer->getEditModePath();
                if ((cnt!=0)&&(path!=nullptr))
                {
                    C7Vector parentTr(path->getCumulativeTransformation());
                    for (int i=0;i<cnt;i++)
                    {
                        CSimplePathPoint* pp=App::mainWindow->editModeContainer->getEditModePathContainer()->getSimplePathPoint(i);
                        C3Vector v(camTrInv*parentTr*pp->getTransformation().X);
                        pts.push_back(v(0));
                        pts.push_back(v(1));
                        pts.push_back(v(2));
                    }
                    scalingFactor*=0.98f;
                }
                else
                    editMode=NO_EDIT_MODE;
            }
        }
    }
#endif

    if ( (editMode==NO_EDIT_MODE)||((editMode&MULTISHAPE_EDIT_MODE)!=0) )
    {
        std::vector<C3DObject*> sel;
        if (editMode==NO_EDIT_MODE)
        {
            C3DObject* skybox=App::ct->objCont->getObjectFromName(IDSOGL_SKYBOX_DO_NOT_RENAME);
            // 1. List of all visible objects, excluding this camera, the skybox and objects flaged as "ignoreViewFitting":
            std::vector<C3DObject*> visibleObjs;
            for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
                if (it!=this)
                {
                    bool displayMaybe=it->getShouldObjectBeDisplayed(_objectHandle,displAttributes);
                    if (displayMaybe)
                    {
                        if ( ((skybox==nullptr)||(!it->isObjectParentedWith(skybox)))&&(!it->getIgnoredByViewFitting()) )
                            visibleObjs.push_back(it);
                    }
                }
            }
            // 1.b if the list of visible objects is empty, include the objects normally ignored for view fitting!
            if (visibleObjs.size()==0)
            {
                for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
                {
                    C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
                    if (it!=this)
                    {
                        bool displayMaybe=it->getShouldObjectBeDisplayed(_objectHandle,displAttributes);
                        if (displayMaybe)
                        {
                            if ((skybox==nullptr)||(!it->isObjectParentedWith(skybox)))
                                visibleObjs.push_back(it);
                        }
                    }
                }
            }
            // 2. List of current selection, excluding this camera and the skybox objects:
            std::vector<int> tmp;
            if (useSystemSelection)
            {
                for (int i=0;i<App::ct->objCont->getSelSize();i++)
                    tmp.push_back(App::ct->objCont->getSelID(i));
            }
            else
            {
                if (selectedObjects!=nullptr)
                    tmp.assign(selectedObjects->begin(),selectedObjects->end());
            }
            if (includeModelObjects)
                CSceneObjectOperations::addRootObjectChildrenToSelection(tmp);
            std::vector<C3DObject*> selectionVisibleObjs;
            for (int i=0;i<int(tmp.size());i++)
            { // We only wanna have visible objects, otherwise we get strange behaviour with some models!! And only objects that are not ignored by the view-fitting:
                C3DObject* it=App::ct->objCont->getObjectFromHandle(tmp[i]);
                if ((it!=nullptr)&&(it!=this))
                {
                    bool displayMaybe=it->getShouldObjectBeDisplayed(_objectHandle,displAttributes);
                    if (displayMaybe)
                    {
                        if ( ((skybox==nullptr)||(!it->isObjectParentedWith(skybox)))&&(!it->getIgnoredByViewFitting()) )
                            selectionVisibleObjs.push_back(it);
                    }
                }
            }
            // 3. set-up a list of all objects that should be framed:
            if (selectionVisibleObjs.size()!=0)
                sel.assign(selectionVisibleObjs.begin(),selectionVisibleObjs.end());
            else
                sel.assign(visibleObjs.begin(),visibleObjs.end());
        }

#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
        {
            if ((editMode&MULTISHAPE_EDIT_MODE)!=0)
            {
                CShape* sh=App::mainWindow->editModeContainer->getEditModeShape();
                if (sh!=nullptr)
                    sel.push_back(sh);
            }
        }
#endif

        // 4. Now do the calculations:
        // 4.a Get all relative coordinates of all oriented bounding box corners, or all relative vertices:
        for (int i=0;i<int(sel.size());i++)
        {
            C3DObject* it=sel[i];
            bool done=false;
            if (it->getObjectType()==sim_object_path_type)
            {
                done=true;
                CPath* path=(CPath*)it;
                int cnt=path->pathContainer->getSimplePathPointCount();
                if ((cnt!=0)&&(path!=nullptr))
                {
                    C7Vector parentTr(path->getCumulativeTransformation());
                    for (int i=0;i<cnt;i++)
                    {
                        CSimplePathPoint* pp=path->pathContainer->getSimplePathPoint(i);
                        C3Vector v(camTrInv*parentTr*pp->getTransformation().X);
                        pts.push_back(v(0));
                        pts.push_back(v(1));
                        pts.push_back(v(2));
                    }
                }
            }
            if (it->getObjectType()==sim_object_shape_type)
            {
                done=true;
                CShape* shape=(CShape*)it;
                C7Vector trr(camTrInv*shape->getCumulativeTransformation());
                std::vector<float> wvert;
                shape->geomData->geomInfo->getCumulativeMeshes(wvert,nullptr,nullptr);
                for (int j=0;j<int(wvert.size())/3;j++)
                {
                    C3Vector vq(&wvert[3*j+0]);
                    vq*=trr;
                    pts.push_back(vq(0));
                    pts.push_back(vq(1));
                    pts.push_back(vq(2));
                }
            }
            if (it->getObjectType()==sim_object_pointcloud_type)
            {
                done=true;
                CPointCloud* ptCloud=(CPointCloud*)it;
                C7Vector trr(camTrInv*ptCloud->getCumulativeTransformation());
                std::vector<float>* wvert=ptCloud->getPoints();
                for (int j=0;j<int(wvert->size())/3;j++)
                {
                    C3Vector vq(&(wvert[0])[3*j+0]);
                    vq*=trr;
                    pts.push_back(vq(0));
                    pts.push_back(vq(1));
                    pts.push_back(vq(2));
                }
            }
            if (it->getObjectType()==sim_object_octree_type)
            {
                done=true;
                COctree* octree=(COctree*)it;
                C7Vector trr(camTrInv*octree->getCumulativeTransformation());
                std::vector<float>* wvert=octree->getCubePositions();
                for (int j=0;j<int(wvert->size())/3;j++)
                {
                    C3Vector vq(&(wvert[0])[3*j+0]);
                    vq*=trr;
                    pts.push_back(vq(0));
                    pts.push_back(vq(1));
                    pts.push_back(vq(2));
                }
            }
            if (it->getObjectType()==sim_object_graph_type)
            { // here we add the 3D curves
                done=true;
                CGraph* gr=(CGraph*)it;

                C7Vector trr(camTrInv*gr->getCumulativeTransformation());

                for (int k=0;k<int(gr->threeDPartners.size());k++)
                {
                    CGraphData* part0=gr->getGraphData(gr->threeDPartners[k]->data[0]);
                    CGraphData* part1=gr->getGraphData(gr->threeDPartners[k]->data[1]);
                    CGraphData* part2=gr->getGraphData(gr->threeDPartners[k]->data[2]);
                    int pos=0;
                    int absIndex;
                    float point[3];
                    bool cyclic0,cyclic1,cyclic2;
                    float range0,range1,range2;
                    if (part0!=nullptr)    
                        CGraphingRoutines::getCyclicAndRangeValues(part0,cyclic0,range0);
                    if (part1!=nullptr)    
                        CGraphingRoutines::getCyclicAndRangeValues(part1,cyclic1,range1);
                    if (part2!=nullptr)    
                        CGraphingRoutines::getCyclicAndRangeValues(part2,cyclic2,range2);
                    while (gr->getAbsIndexOfPosition(pos++,absIndex))
                    {
                        bool dataIsValid=true;
                        if (part0!=nullptr)
                        {
                            if(!gr->getData(part0,absIndex,point[0],cyclic0,range0,true))
                                dataIsValid=false;
                        }
                        else
                            dataIsValid=false;
                        if (part1!=nullptr)
                        {
                            if(!gr->getData(part1,absIndex,point[1],cyclic1,range1,true))
                                dataIsValid=false;
                        }
                        else
                            dataIsValid=false;
                        if (part2!=nullptr)
                        {
                            if(!gr->getData(part2,absIndex,point[2],cyclic2,range2,true))
                                dataIsValid=false;
                        }
                        else
                            dataIsValid=false;
                        if (dataIsValid)
                        {
                            C3Vector pp(point);
                            if (gr->threeDPartners[k]->getCurveRelativeToWorld())
                                pp=camTrInv*pp;
                            else
                                pp=trr*pp;
                            pts.push_back(pp(0));
                            pts.push_back(pp(1));
                            pts.push_back(pp(2));
                        }
                    }
                }

                // Static 3D curves now:
                for (int k=0;k<int(gr->_staticCurves.size());k++)
                {
                    CStaticGraphCurve* itg=gr->_staticCurves[k];
                    if (itg->getCurveType()==2)
                    {
                        for (int j=0;j<int(itg->values.size()/3);j++)
                        {
                            C3Vector pp(itg->values[3*j+0],itg->values[3*j+1],itg->values[3*j+2]);
                            if (itg->getRelativeToWorld())
                                pp=camTrInv*pp;
                            else
                                pp=trr*pp;
                            pts.push_back(pp(0));
                            pts.push_back(pp(1));
                            pts.push_back(pp(2));
                        }
                    }
                }

                // Now the graph object itself:
                if (gr->xYZPlanesDisplay)
                {
                    C3Vector minV,maxV;
                    it->getMarkingBoundingBox(minV,maxV);
                    minV*=trr;
                    maxV*=trr;
                    for (int k=0;k<2;k++)
                    {
                        for (int l=0;l<2;l++)
                        {
                            for (int m=0;m<2;m++)
                            {
                                pts.push_back(minV(0)+(maxV(0)-minV(0))*k);
                                pts.push_back(minV(1)+(maxV(1)-minV(1))*l);
                                pts.push_back(minV(2)+(maxV(2)-minV(2))*m);
                            }
                        }
                    }
                }


            }
            if (!done)
            {
                C3Vector minV,maxV;
                it->getBoundingBoxEncompassingBoundingBox(camTrInv,minV,maxV,false);
                for (int k=0;k<2;k++)
                {
                    for (int l=0;l<2;l++)
                    {
                        for (int m=0;m<2;m++)
                        {
                            pts.push_back(minV(0)+(maxV(0)-minV(0))*k);
                            pts.push_back(minV(1)+(maxV(1)-minV(1))*l);
                            pts.push_back(minV(2)+(maxV(2)-minV(2))*m);
                        }
                    }
                }
            }
        }
    }

    if (pts.size()==0)
        return;

    if (getTrackedObjectID()!=-1)
    { // When tracking an object, we should stay on the current view axis. To do this, we simply reflect all points left/right/top/bottom relative to the camera!
        std::vector<float> ptsC(pts);
        pts.clear();
        for (int i=0;i<int(ptsC.size())/3;i++)
        {
            C3Vector wv(&ptsC[3*i+0]);
            pts.push_back(wv(0));
            pts.push_back(wv(1));
            pts.push_back(wv(2));
            pts.push_back(-wv(0));
            pts.push_back(wv(1));
            pts.push_back(wv(2));
            pts.push_back(wv(0));
            pts.push_back(-wv(1));
            pts.push_back(wv(2));
            pts.push_back(-wv(0));
            pts.push_back(-wv(1));
            pts.push_back(wv(2));
        }
    }
/* // Here trying to scale the points, instead of camera angle/camera ortho view size
   //
    float qwmin,qwmax;
    for (int i=0;i<int(pts.size())/3;i++)
    {
        if (i==0)
        {
            qwmin=pts[3*i+2];
            qwmax=pts[3*i+2];
        }
        else
        {
            if (qwmin>pts[3*i+2])
                qwmin=pts[3*i+2];
            if (qwmax<pts[3*i+2])
                qwmax=pts[3*i+2];
        }
    }

    float qwavg=(qwmax+qwmin)*0.5f;
    for (int i=0;i<int(pts.size())/3;i++)
    {
        pts[3*i+0]*=1.0f/scalingFactor;
        pts[3*i+1]*=1.0f/scalingFactor;
        pts[3*i+2]=(pts[3*i+2]-qwavg)*(1.0f/scalingFactor)+qwavg;
    }
*/
    C3Vector relativeCameraTranslation;
    relativeCameraTranslation.clear();
    float nearClippingPlaneCorrection=0.0f;
    float farClippingPlaneCorrection=0.0f;
    float viewSizeCorrection=0.0f;
    if (forPerspectiveProjection)
    {
        C3Vector relativeCameraTranslation_verticalAdjustment;
        C3Vector relativeCameraTranslation_horizontalAdjustment;
        relativeCameraTranslation_verticalAdjustment.clear();
        relativeCameraTranslation_horizontalAdjustment.clear();
        std::vector<float> cop(pts);
        //*****************
        float effectiveAngle=getViewAngle()*scalingFactor;
        if (effectiveAngle>piValue_f)
            effectiveAngle=piValue_f;

        if (windowWidthByHeight>1.0f)
            effectiveAngle/=windowWidthByHeight;

        C4X4Matrix lowerLimit;
        lowerLimit.buildXRotation(effectiveAngle*0.5f);
        C4X4Matrix lowerLimitInv(lowerLimit.getInverse());
        C3Vector minw;
        C3Vector maxw;
        for (int i=0;i<int(pts.size())/3;i++)
        {
            C3Vector w(pts[3*i+0],pts[3*i+1],pts[3*i+2]);
            w*=lowerLimitInv;
            if (i==0)
            {
                minw=w;
                maxw=w;
            }
            else
            {
                minw.keepMin(w);
                maxw.keepMax(w);
            }
        }
        C4X4Matrix cm(getLocalTransformation().getMatrix()*lowerLimit);
        relativeCameraTranslation_verticalAdjustment=cm.M.axis[1]*minw(1);

        C3Vector camShift(lowerLimit.M.axis[1]*minw(1));
        for (int i=0;i<int(pts.size())/3;i++)
        {
            pts[3*i+0]-=camShift(0);
            pts[3*i+1]-=camShift(1);
            pts[3*i+2]-=camShift(2);
        }

        C4X4Matrix upperLimit;
        upperLimit.buildXRotation(-effectiveAngle*0.5f);
        C4X4Matrix upperLimitInv(upperLimit.getInverse());
        for (int i=0;i<int(pts.size())/3;i++)
        {
            C3Vector w(pts[3*i+0],pts[3*i+1],pts[3*i+2]);
            w*=upperLimitInv;
            if (i==0)
            {
                minw=w;
                maxw=w;
            }
            else
            {
                minw.keepMin(w);
                maxw.keepMax(w);
            }
        }

        cm=getLocalTransformation().getMatrix()*lowerLimit;
        float dist=-maxw(1)/sin(effectiveAngle);
        relativeCameraTranslation_verticalAdjustment+=cm.M.axis[2]*dist;

    // ***************** left-right

        pts.assign(cop.begin(),cop.end());

        effectiveAngle=getViewAngle()*scalingFactor;
        if (windowWidthByHeight<1.0f)
            effectiveAngle*=windowWidthByHeight;

        C4X4Matrix leftLimit;
        leftLimit.buildYRotation(effectiveAngle*0.5f);
        C4X4Matrix leftLimitInv(leftLimit.getInverse());
        for (int i=0;i<int(pts.size())/3;i++)
        {
            C3Vector w(pts[3*i+0],pts[3*i+1],pts[3*i+2]);
            w*=leftLimitInv;
            if (i==0)
            {
                minw=w;
                maxw=w;
            }
            else
            {
                minw.keepMin(w);
                maxw.keepMax(w);
            }
        }
        cm=getLocalTransformation().getMatrix()*leftLimit;
        relativeCameraTranslation_horizontalAdjustment=cm.M.axis[0]*maxw(0);

        camShift=leftLimit.M.axis[0]*maxw(0);
        for (int i=0;i<int(pts.size())/3;i++)
        {
            pts[3*i+0]-=camShift(0);
            pts[3*i+1]-=camShift(1);
            pts[3*i+2]-=camShift(2);
        }

        C4X4Matrix rightLimit;
        rightLimit.buildYRotation(-effectiveAngle*0.5f);
        C4X4Matrix rightLimitInv(rightLimit.getInverse());
        for (int i=0;i<int(pts.size())/3;i++)
        {
            C3Vector w(pts[3*i+0],pts[3*i+1],pts[3*i+2]);
            w*=rightLimitInv;
            if (i==0)
            {
                minw=w;
                maxw=w;
            }
            else
            {
                minw.keepMin(w);
                maxw.keepMax(w);
            }
        }

        cm=getLocalTransformation().getMatrix()*leftLimit;
        dist=minw(0)/sin(effectiveAngle);
        relativeCameraTranslation_horizontalAdjustment+=cm.M.axis[2]*dist;

        // Decide which coords to select
        cm=getLocalTransformation().getMatrix();
        C3Vector vert(cm.getInverse()*relativeCameraTranslation_verticalAdjustment);
        C3Vector horiz(cm.getInverse()*relativeCameraTranslation_horizontalAdjustment);
        C3Vector f;
        f(0)=horiz(0);
        f(1)=vert(1);
        f(2)=vert(2);
        if (horiz(2)<vert(2))
            f(2)=horiz(2);
        relativeCameraTranslation=cm*f;

        // Adjust the near and far clipping planes if needed:
        pts.assign(cop.begin(),cop.end());
        C4X4Matrix ncm(cm);
        ncm.X+=relativeCameraTranslation;
        C4X4Matrix tr(ncm.getInverse()*cm);
        for (int i=0;i<int(pts.size())/3;i++)
        {
            C3Vector w(pts[3*i+0],pts[3*i+1],pts[3*i+2]);
            w*=tr;
            if (i==0)
            {
                minw=w;
                maxw=w;
            }
            else
            {
                minw.keepMin(w);
                maxw.keepMax(w);
            }
        }
        float zCorr=0.0f;
        if (minw(2)<getNearClippingPlane())
            zCorr=minw(2)-getNearClippingPlane();
        relativeCameraTranslation+=cm.M.axis[2]*zCorr; // zCorr is negative or 0
        if (maxw(2)>getFarClippingPlane()+zCorr)
            farClippingPlaneCorrection=maxw(2)-zCorr-getFarClippingPlane();

    }
    else
    { // orthographic projection:
        C3Vector minw;
        C3Vector maxw;
        for (int i=0;i<int(pts.size())/3;i++)
        {
            C3Vector w(pts[3*i+0],pts[3*i+1],pts[3*i+2]);
            if (i==0)
            {
                minw=w;
                maxw=w;
            }
            else
            {
                minw.keepMin(w);
                maxw.keepMax(w);
            }
        }
        C3Vector center((minw+maxw)/2.0f);
        // Move the camera to look at the center of those points (just 2 translations) + backshift if needed:
        float backShift=0.0f;
        if (minw(2)<getNearClippingPlane())
            backShift=minw(2)-getNearClippingPlane();
        C4X4Matrix cm(getLocalTransformation().getMatrix());
        relativeCameraTranslation=cm.M.axis[0]*center(0)+cm.M.axis[1]*center(1)+cm.M.axis[2]*backShift;
        if (maxw(2)>((getFarClippingPlane()+backShift)/1.2f))
            farClippingPlaneCorrection=(maxw(2)-(getFarClippingPlane()+backShift))*1.2f;
        float vs=getOrthoViewSize();
        float dx=(maxw(0)-minw(0))/scalingFactor;
        float dy=(maxw(1)-minw(1))/scalingFactor;
        float rap=9999999.0f;
        if (dy!=0.0f)
            rap=dx/dy;
        if (windowWidthByHeight>1.0f)
        {
            if (rap>=windowWidthByHeight)
                viewSizeCorrection=dx-vs;
            else
                viewSizeCorrection=(dy-vs/windowWidthByHeight)*windowWidthByHeight;
        }
        else
        {
            if (rap<=windowWidthByHeight)
                viewSizeCorrection=dy-vs;
            else
                viewSizeCorrection=(dx-vs*windowWidthByHeight)/windowWidthByHeight;
        }
    }
    C4X4Matrix cm(getLocalTransformation().getMatrix());
    cm.X+=relativeCameraTranslation;

    setNearClippingPlane(getNearClippingPlane()+nearClippingPlaneCorrection);
    setFarClippingPlane(getFarClippingPlane()+farClippingPlaneCorrection);
    setOrthoViewSize(getOrthoViewSize()+viewSizeCorrection);
    C3DObject* cameraParentProxy=nullptr;
    if (getUseParentObjectAsManipulationProxy())
        cameraParentProxy=getParentObject();

    if (cameraParentProxy!=nullptr)
    { // We report the same camera opening to all cameras attached to cameraParentProxy
        if (!forPerspectiveProjection)
        {
            for (int i=0;i<int(cameraParentProxy->childList.size());i++)
            {
                if (cameraParentProxy->childList[i]->getObjectType()==sim_object_camera_type)
                    ((CCamera*)cameraParentProxy->childList[i])->setOrthoViewSize(getOrthoViewSize());
            }
        }
        cameraParentProxy->setLocalTransformation(cameraParentProxy->getLocalTransformation()*cm.getTransformation()*getLocalTransformation().getInverse());
    }
    else
        setLocalTransformation(cm.getTransformation());
}


void CCamera::commonInit()
{
    _initialValuesInitialized=false;
    setObjectType(sim_object_camera_type);
    _nearClippingPlane=0.05f;
    _farClippingPlane=30.0f;
    cameraSize=0.05f;
    _renderMode=sim_rendermode_opengl;
    _renderModeDuringSimulation=false;
    _renderModeDuringRecording=false;

    _viewAngle=60.0f*degToRad_f;
    _orthoViewSize=2.0f;
    _showFogIfAvailable=true;
    _useLocalLights=false;
    _allowPicking=true;
    if (_extensionString.size()!=0)
        _extensionString+=" ";
    _extensionString+="povray {focalBlur {false} focalDist {2.00} aperture{0.05} blurSamples{10}}";
    hitForMouseUpProcessing_minus2MeansIgnore=-2;
    _attributesForRendering=sim_displayattribute_renderpass;
    _textureNameForExtGeneratedView=(unsigned int)-1;

    _cameraManipulationModePermissions=0x1f;

    _objectManipulationModePermissions=0x013;

    trackedObjectIdentifier_NeverDirectlyTouch=-1;
    layer=CAMERA_LIGHT_LAYER;
    _localObjectSpecialProperty=0;
    _useParentObjectAsManipulationProxy=false;

    colorPart1.setDefaultValues();
    colorPart1.setColor(0.9f,0.2f,0.2f,sim_colorcomponent_ambient_diffuse);
    colorPart2.setDefaultValues();
    colorPart2.setColor(0.45f,0.45f,0.45f,sim_colorcomponent_ambient_diffuse);

    _objectName=IDSOGL_CAMERA;
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName);
}

void CCamera::setCameraManipulationModePermissions(int p)
{ // bit coded: own x, own y, own z, full rotation, tilting, never tilting
    // full rotation requires free x, y and z movement
    // never tilting and tilting cannot be activated together
    p&=0x03f;
    if (((_cameraManipulationModePermissions&0x008)==0)&&(p&0x008))
        p|=0x007; // we want full rotation! x,y and z have to be enabled
    if (((_cameraManipulationModePermissions&0x020)==0)&&(p&0x020))
        p&=0x02f; // we want to never tilt! disable manipulation tilting
    if (((_cameraManipulationModePermissions&0x010)==0)&&(p&0x010))
        p&=0x01f; // we want to tilt during manip! disable "never tilting"
    if ((p&0x007)!=0x007)
        p&=0x037; // We restrain x,y or z translation. disable full rotation
    _cameraManipulationModePermissions=p;
}

int CCamera::getCameraManipulationModePermissions() const
{ // bit coded: own x, own y, own z, full rotation, tilting, never tilting
    return(_cameraManipulationModePermissions);
}

void CCamera::shiftCameraInCameraManipulationMode(const C3Vector& newLocalPos)
{
    C4X4Matrix oldLocal(_transformation.getMatrix());
    C4X4Matrix newLocal(oldLocal);
    newLocal.X=newLocalPos;
    C4X4Matrix tr(oldLocal.getInverse()*newLocal);
    if ((_cameraManipulationModePermissions&0x001)==0)
        tr.X(0)=0.0f;
    if ((_cameraManipulationModePermissions&0x002)==0)
        tr.X(1)=0.0f;
    if ((_cameraManipulationModePermissions&0x004)==0)
        tr.X(2)=0.0f;
    _transformation=oldLocal*tr;
}
void CCamera::rotateCameraInCameraManipulationMode(const C7Vector& newLocalConf)
{
    if (_cameraManipulationModePermissions&0x008)
        _transformation=newLocalConf;
}
void CCamera::tiltCameraInCameraManipulationMode(float tiltAmount)
{
    if (_cameraManipulationModePermissions&0x010)
    {
        C4X4Matrix oldLocal(_transformation.getMatrix());
        C3X3Matrix rot;
        rot.buildZRotation(tiltAmount);
        oldLocal.M=oldLocal.M*rot;
        _transformation=oldLocal;
    }
}

bool CCamera::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    minV(0)=-0.5f*cameraSize;
    maxV(0)=0.5f*cameraSize;
    minV(1)=-0.5f*cameraSize;
    maxV(1)=2.2f*cameraSize;
    minV(2)=-2.6f*cameraSize;
    maxV(2)=cameraSize;
    return(true);
}

bool CCamera::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(getFullBoundingBox(minV,maxV));
}

CCamera::~CCamera()
{
    if (_textureNameForExtGeneratedView!=(unsigned int)-1)
    {
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=DESTROY_GL_TEXTURE_UITHREADCMD;
        cmdIn.uintParams.push_back(_textureNameForExtGeneratedView);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
        _textureNameForExtGeneratedView=(unsigned int)-1;
    }
}

bool CCamera::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    return(false); // for now
}

void CCamera::handleTrackingAndHeadAlwaysUp()
{
    FUNCTION_DEBUG;
    // 1. First tracking:
    // Check if the tracked object is not parented with that camera
    // (camera would follow the object which would follow the camera which...)
    C3DObject* tr=App::ct->objCont->getObjectFromHandle(trackedObjectIdentifier_NeverDirectlyTouch);
    if ((tr==nullptr)||(tr==this)||tr->isObjectParentedWith(this))
    {
        trackedObjectIdentifier_NeverDirectlyTouch=-1;
        tr=nullptr;
    }
    if (tr!=nullptr)
    {
        C7Vector tracked(tr->getCumulativeTransformation());
        C7Vector self(getCumulativeTransformation());
        C4Vector rot1(self.Q.getAxis(2),tracked.X-self.X);
        self.Q=rot1*self.Q;
        // We check if the camera looks to +Z or -Z:
        C3Vector zAxis(self.Q.getAxis(2));
        if ( (fabs(zAxis(0))>0.00001f)||(fabs(zAxis(1))>0.00001f) )
        { // Camera does not look to +Z or -Z:
            C3Vector rotAxis(zAxis^C3Vector(0.0f,0.0f,1.0f));
            C4Vector rot(piValue_f/2.0f,rotAxis);
            zAxis=rot*zAxis;
            C4Vector rot2(self.Q.getAxis(1),zAxis);
            self.Q=rot2*self.Q;
            C7Vector parentInv(getParentCumulativeTransformation().getInverse());
            setLocalTransformation(parentInv*self);
        }
    }

    // 2. Now permanent head up:
    if (_cameraManipulationModePermissions&0x020)
    { // We have to keep head up
        C7Vector cameraCTM(getCumulativeTransformationPart1());
        C3X3Matrix trM2(cameraCTM.Q);
        if ( (fabs(trM2.axis[2](0))>0.00001f)||(fabs(trM2.axis[2](1))>0.00001f) )
        { // We have to do it:
            float val=1.0f;
            if (trM2.axis[1](2)<0.0f)
                val=-1.0f;
            C3Vector rotAx(trM2.axis[2]^C3Vector(0.0f,0.0f,val));
            C3Vector target(C4Vector(piValue_f/2.0f,rotAx)*trM2.axis[2]);
            C4Vector rot(trM2.axis[1],target);
            cameraCTM.Q=rot*cameraCTM.Q;
            setLocalTransformation(getParentCumulativeTransformation().getInverse()*cameraCTM);
        }
    }
}


void CCamera::scaleObject(float scalingFactor)
{
    cameraSize*=scalingFactor;
    _nearClippingPlane*=scalingFactor;
    _farClippingPlane*=scalingFactor;
    _orthoViewSize*=scalingFactor;

    scaleObjectMain(scalingFactor);
}

void CCamera::scaleObjectNonIsometrically(float x,float y,float z)
{
    scaleObject(cbrt(x*y*z));
}

void CCamera::setCameraSize(float size)
{
    tt::limitValue(0.001f,100.0f,size);
    cameraSize=size;
}

float CCamera::getCameraSize() const
{
    return(cameraSize);
}

int CCamera::getTrackedObjectID() const
{
    return(trackedObjectIdentifier_NeverDirectlyTouch);
}

void CCamera::setUseParentObjectAsManipulationProxy(bool useParent)
{
    _useParentObjectAsManipulationProxy=useParent;
}

bool CCamera::getUseParentObjectAsManipulationProxy() const
{
    return(_useParentObjectAsManipulationProxy);
}

void CCamera::setTrackedObjectID(int trackedObjID)
{
    if (trackedObjID==_objectHandle)
        return;
    if (trackedObjID==-1)
    {
        trackedObjectIdentifier_NeverDirectlyTouch=-1;
        return;
    }
    if (App::ct->objCont->getObjectFromHandle(trackedObjID)==nullptr)
        return;
    trackedObjectIdentifier_NeverDirectlyTouch=trackedObjID;
    App::setLightDialogRefreshFlag();
}

void CCamera::removeSceneDependencies()
{
    removeSceneDependenciesMain();
    trackedObjectIdentifier_NeverDirectlyTouch=-1;
}

C3DObject* CCamera::copyYourself()
{   
    CCamera* newCamera=(CCamera*)copyYourselfMain();

    // Various
    newCamera->cameraSize=cameraSize;
    newCamera->_renderMode=_renderMode;
    newCamera->_renderModeDuringSimulation=_renderModeDuringSimulation;
    newCamera->_renderModeDuringRecording=_renderModeDuringRecording;
    newCamera->_viewAngle=_viewAngle;
    newCamera->_orthoViewSize=_orthoViewSize;
    newCamera->_nearClippingPlane=_nearClippingPlane;
    newCamera->_farClippingPlane=_farClippingPlane;
    newCamera->_showFogIfAvailable=_showFogIfAvailable;
    newCamera->trackedObjectIdentifier_NeverDirectlyTouch=trackedObjectIdentifier_NeverDirectlyTouch;
    newCamera->_useParentObjectAsManipulationProxy=_useParentObjectAsManipulationProxy;
    newCamera->_cameraManipulationModePermissions=_cameraManipulationModePermissions;
    newCamera->_useLocalLights=_useLocalLights;
    newCamera->_allowPicking=_allowPicking;

    // Colors:
    colorPart1.copyYourselfInto(&newCamera->colorPart1);
    colorPart2.copyYourselfInto(&newCamera->colorPart2);

    return(newCamera);
}

bool CCamera::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(id)'-call or similar
    // Return value true means 'this' has to be erased too!
    bool retVal=announceObjectWillBeErasedMain(objectHandle,copyBuffer);
    if (trackedObjectIdentifier_NeverDirectlyTouch==objectHandle)
        trackedObjectIdentifier_NeverDirectlyTouch=-1;
    return(retVal);
}
void CCamera::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollectionWillBeErasedMain(groupID,copyBuffer);
}
void CCamera::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollisionWillBeErasedMain(collisionID,copyBuffer);
}
void CCamera::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceDistanceWillBeErasedMain(distanceID,copyBuffer);
}
void CCamera::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceIkObjectWillBeErasedMain(ikGroupID,copyBuffer);
}
void CCamera::announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceGcsObjectWillBeErasedMain(gcsObjectID,copyBuffer);
}
void CCamera::performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    performObjectLoadingMappingMain(map,loadingAmodel);
    trackedObjectIdentifier_NeverDirectlyTouch=App::ct->objCont->getLoadingMapping(map,trackedObjectIdentifier_NeverDirectlyTouch);
}
void CCamera::performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollectionLoadingMappingMain(map,loadingAmodel);
}
void CCamera::performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollisionLoadingMappingMain(map,loadingAmodel);
}
void CCamera::performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performDistanceLoadingMappingMain(map,loadingAmodel);
}
void CCamera::performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
    performIkLoadingMappingMain(map,loadingAmodel);
}
void CCamera::performGcsLoadingMapping(std::vector<int>* map)
{
    performGcsLoadingMappingMain(map);
}

void CCamera::performTextureObjectLoadingMapping(std::vector<int>* map)
{
    performTextureObjectLoadingMappingMain(map);
}

void CCamera::performDynMaterialObjectLoadingMapping(std::vector<int>* map)
{
    performDynMaterialObjectLoadingMappingMain(map);
}

void CCamera::bufferMainDisplayStateVariables()
{
    bufferMainDisplayStateVariablesMain();
}

void CCamera::bufferedMainDisplayStateVariablesToDisplay()
{
    bufferedMainDisplayStateVariablesToDisplayMain();
}

int CCamera::getViewOrientation() const
{
    C3X3Matrix m(getCumulativeTransformation().Q);
    if (fabs(m.axis[2].getAngle(C3Vector(-1.0f,0.0f,0.0f))*radToDeg_f)<0.1f)
        return(POSITIVE_X_VIEW);
    if (fabs(m.axis[2].getAngle(C3Vector(+1.0f,0.0f,0.0f))*radToDeg_f)<0.1f)
        return(NEGATIVE_X_VIEW);
    if (fabs(m.axis[2].getAngle(C3Vector(0.0f,-1.0f,0.0f))*radToDeg_f)<0.1f)
        return(POSITIVE_Y_VIEW);
    if (fabs(m.axis[2].getAngle(C3Vector(0.0f,+1.0f,0.0f))*radToDeg_f)<0.1f)
        return(NEGATIVE_Y_VIEW);
    if (fabs(m.axis[2].getAngle(C3Vector(0.0f,0.0f,-1.0f))*radToDeg_f)<0.1f)
        return(POSITIVE_Z_VIEW);
    if (fabs(m.axis[2].getAngle(C3Vector(0.0f,0.0f,+1.0f))*radToDeg_f)<0.1f)
        return(NEGATIVE_Z_VIEW);
    return(NO_SPECIFIC_VIEW);
}
void CCamera::setViewOrientation(int ori,bool setPositionAlso)
{ // setPositionAlso is false by default
    float alpha,beta,gamma,x,y,z;
    bool done=false;
    if (ori==POSITIVE_X_VIEW)
    {
        alpha=0.0f;
        beta=-90.0f;
        gamma=-90.0f;
        x=+3.0f;
        y=0.0f;
        z=0.0f;
        done=true;
    }
    if (ori==NEGATIVE_X_VIEW)
    {
        alpha=0.0f;
        beta=+90.0f;
        gamma=+90.0f;
        x=-3.0f;
        y=0.0f;
        z=0.0f;
        done=true;
    }
    if (ori==POSITIVE_Y_VIEW)
    {
        alpha=+90.0f;
        beta=0.0f;
        gamma=0.0f;
        x=0.0f;
        y=+3.0f;
        z=0.0f;
        done=true;
    }
    if (ori==NEGATIVE_Y_VIEW)
    {
        alpha=-90.0f;
        beta=0.0f;
        gamma=+180.0f;
        x=0.0f;
        y=-3.0f;
        z=0.0f;
        done=true;
    }
    if (ori==POSITIVE_Z_VIEW)
    {
        alpha=+180.0f;
        beta=0.0f;
        gamma=0.0f;
        x=0.0f;
        y=0.0f;
        z=+3.0f;
        done=true;
    }
    if (ori==NEGATIVE_Z_VIEW)
    {
        alpha=0.0f;
        beta=0.0f;
        gamma=+180.0f;
        x=0.0f;
        y=0.0f;
        z=-3.0f;
        done=true;
    }
    if (done)
    {
        C7Vector tot(getCumulativeTransformation());
        tot.Q.setEulerAngles(C3Vector(alpha*degToRad_f,beta*degToRad_f,gamma*degToRad_f));
        if (setPositionAlso)
            tot.X.set(x,y,z);
        C7Vector parentInv(getParentCumulativeTransformation().getInverse());
        setLocalTransformation(parentInv*tot);
    }
}

void CCamera::setAllowPicking(bool a)
{
    _allowPicking=a;
}

bool CCamera::getAllowPicking() const
{
    return(_allowPicking);
}

void CCamera::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    initializeInitialValuesMain(simulationIsRunning);
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        _initialViewAngle=_viewAngle;
        _initialOrthoViewSize=_orthoViewSize;
    }
}

void CCamera::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CCamera::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
    {
        _viewAngle=_initialViewAngle;
        _orthoViewSize=_initialOrthoViewSize;
    }
    _initialValuesInitialized=false;
    simulationEndedMain();
}

void CCamera::serialize(CSer& ar)
{
    serializeMain(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            int trck=-1;
            if (trackedObjectIdentifier_NeverDirectlyTouch!=-1)
                trck=trackedObjectIdentifier_NeverDirectlyTouch;
            ar.storeDataName("Cp4");
            ar << trck << cameraSize;
            ar.flush();

            ar.storeDataName("Cp3");
            ar << _orthoViewSize << _viewAngle;
            ar.flush();

            ar.storeDataName("Ccp");
            ar << _nearClippingPlane << _farClippingPlane;
            ar.flush();

            ar.storeDataName("Cmp");
            ar << _cameraManipulationModePermissions;
            ar.flush();

            ar.storeDataName("Rmd");
            ar << int(sim_rendermode_opengl); // for backward compatibility 28/06/2019
            ar.flush();

            ar.storeDataName("Rm2");
            ar << _renderMode;
            ar.flush();

            ar.storeDataName("Ca2");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_useParentObjectAsManipulationProxy);
            SIM_SET_CLEAR_BIT(nothing,1,!_showFogIfAvailable);
            SIM_SET_CLEAR_BIT(nothing,2,_useLocalLights);
            SIM_SET_CLEAR_BIT(nothing,3,!_allowPicking);
            // RESERVED SIM_SET_CLEAR_BIT(nothing,4,_povFocalBlurEnabled);
            SIM_SET_CLEAR_BIT(nothing,5,_renderModeDuringSimulation);
            SIM_SET_CLEAR_BIT(nothing,6,_renderModeDuringRecording);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Cl1");
            ar.setCountingMode();
            colorPart1.serialize(ar,0);
            if (ar.setWritingMode())
                colorPart1.serialize(ar,0);

            ar.storeDataName("Cl2");
            ar.setCountingMode();
            colorPart2.serialize(ar,0);
            if (ar.setWritingMode())
                colorPart2.serialize(ar,0);

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            bool povFocalBlurEnabled_backwardCompatibility_3_2_2016=false;
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Cp4")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> trackedObjectIdentifier_NeverDirectlyTouch >> cameraSize;
                    }
                    if (theName.compare("Cp3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _orthoViewSize >> _viewAngle;
                    }
                    if (theName.compare("Ccp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _nearClippingPlane >> _farClippingPlane;
                    }
                    if (theName=="Caz")
                    { // keep for backward compatibility (2010/07/13)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _useParentObjectAsManipulationProxy=SIM_IS_BIT_SET(nothing,0);
                        bool headUp=SIM_IS_BIT_SET(nothing,1);
                        bool keepHeadAlwaysUp=SIM_IS_BIT_SET(nothing,2);
                        if (headUp)
                            _cameraManipulationModePermissions&=0x02f;
                        else
                            _cameraManipulationModePermissions|=0x010;
                        if (keepHeadAlwaysUp)
                            _cameraManipulationModePermissions|=0x020;
                        else
                            _cameraManipulationModePermissions&=0x01f;
                    }
                    if (theName=="Ca2")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _useParentObjectAsManipulationProxy=SIM_IS_BIT_SET(nothing,0);
                        _showFogIfAvailable=!SIM_IS_BIT_SET(nothing,1);
                        _useLocalLights=SIM_IS_BIT_SET(nothing,2);
                        _allowPicking=!SIM_IS_BIT_SET(nothing,3);
                        povFocalBlurEnabled_backwardCompatibility_3_2_2016=SIM_IS_BIT_SET(nothing,4);
                        _renderModeDuringSimulation=SIM_IS_BIT_SET(nothing,5);
                        _renderModeDuringRecording=SIM_IS_BIT_SET(nothing,6);
                    }
                    if (theName.compare("Rm2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _renderMode;
                    }
                    if (theName.compare("Rmd")==0)
                    { // keep for backward compatibility 28/06/2019
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _renderMode;
                        if (_renderMode!=0)
                        {
                            if (_renderMode==1)
                            {
                                _renderMode=sim_rendermode_povray;
                                _renderModeDuringSimulation=true;
                                _renderModeDuringRecording=true;
                            }
                            else if (_renderMode==2)
                            {
                                _renderMode=sim_rendermode_povray;
                                _renderModeDuringSimulation=true;
                                _renderModeDuringRecording=false;
                            }
                            else if (_renderMode==5)
                                _renderMode=sim_rendermode_extrenderer;
                            else
                                _renderMode=sim_rendermode_opengl;
                        }
                    }
                    if (theName.compare("Pv1")==0)
                    { // Keep for backward compatibility (3/2/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        float povFocalDistance, povAperture;
                        int povBlurSamples;
                        ar >> povFocalDistance >> povAperture >> povBlurSamples;
                        _extensionString="povray {focalBlur {";
                        if (povFocalBlurEnabled_backwardCompatibility_3_2_2016)
                            _extensionString+="true} focalDist {";
                        else
                            _extensionString+="false} focalDist {";
                        _extensionString+=tt::FNb(0,povFocalDistance,3,false);
                        _extensionString+="} aperture {";
                        _extensionString+=tt::FNb(0,povAperture,3,false);
                        _extensionString+="} blurSamples {";
                        _extensionString+=tt::FNb(0,povBlurSamples,false);
                        _extensionString+="}}";
                    }
                    if (theName.compare("Cl1")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        colorPart1.serialize(ar,0);
                    }
                    if (theName.compare("Cl2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        colorPart2.serialize(ar,0);
                    }
                    if (theName.compare("Cmp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _cameraManipulationModePermissions;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }

            if (ar.getVrepVersionThatWroteThisFile()<20509)
            { // For backward compatibility (27/7/2011)
                if ( (_objectName.compare("DefaultXViewCamera")==0)||(_objectName.compare("DefaultYViewCamera")==0)||(_objectName.compare("DefaultZViewCamera")==0) )
                    _showFogIfAvailable=false;
            }

            if (ar.getSerializationVersionThatWroteThisFile()<17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                CTTUtil::scaleColorUp_(colorPart1.colors);
                CTTUtil::scaleColorUp_(colorPart2.colors);
            }
        }
    }
}

void CCamera::serializeWExtIk(CExtIkSer& ar)
{
    serializeWExtIkMain(ar);
    CDummy::serializeWExtIkStatic(ar);
}

void CCamera::display(CViewableBase* renderingObject,int displayAttrib)
{
    FUNCTION_INSIDE_DEBUG("CCamera::display");
    EASYLOCK(_objectMutex);
    displayCamera(this,renderingObject,displayAttrib);
}

#ifdef SIM_WITH_GUI
void CCamera::lookIn(int windowSize[2],CSView* subView,bool drawText,bool passiveSubView)
{ // drawText is false and passiveSubView is true by default
    FUNCTION_DEBUG;
    // Default values (used for instance in view selection mode)

    // This routine is quite messy and needs refactoring!!!
    bool isPerspective=true;
    int renderingMode=RENDERING_MODE_SOLID;
    bool displ_ref=false;
    int currentWinSize[2];
    int mouseRelativePosition[2];
    int mouseDownRelativePosition[2];
    int selectionStatus=NOSELECTION;
    bool mouseIsDown=false;
    bool mouseJustWentDown=false;
    bool mouseJustWentUp=false;
    bool mouseMovedWhileDown=false;
    int navigationMode=sim_navigation_passive;
    if (windowSize!=nullptr)
    {
        currentWinSize[0]=windowSize[0];
        currentWinSize[1]=windowSize[1];
    }
    if (subView!=nullptr)
    {
        isPerspective=subView->getPerspectiveDisplay();
        renderingMode=subView->getRenderingMode();
        displ_ref=true;
        subView->getViewSize(currentWinSize);
        subView->getMouseRelativePosition(mouseRelativePosition);
        subView->getMouseDownRelativePosition(mouseDownRelativePosition);
        if (!passiveSubView)
        {
            selectionStatus=subView->getSelectionStatus();
            mouseIsDown=subView->isMouseDown();
            mouseJustWentDown=subView->didMouseJustGoDown();
            mouseJustWentUp=subView->didMouseJustGoUp();
            mouseMovedWhileDown=subView->didMouseMoveWhileDown();
            navigationMode=App::getMouseMode()&0x00ff;
        }
    }

    // Set data for view frustum culling
    _planesCalculated=false;
    _currentViewSize[0]=currentWinSize[0];
    _currentViewSize[1]=currentWinSize[1];
    _currentPerspective=isPerspective;

    // There is a maximum of 3 passes:
    // Pass 0 : DEPTHPASS : Pass used when doing depth perception
    //          Everything should be drawn as
    //          glPolygonMode (GL_FRONT_AND_BACK,GL_FILL)
    //          This pass is never displayed and can be skipped
    // Pass 1 : PICKPASS : Pass used for picking
    //          This pass might be displayed and can be skipped
    // Pass 2 : RENDERPASS : Pass which is always displayed and cannot be skipped

    int passes[3]={RENDERPASS,-1,-1}; // last should always be rendering pass!

    bool specialSelectionAndNavigationPass=false;
    bool regularObjectsCannotBeSelected=false;
    bool processHitForMouseUpProcessing=false;
    if (subView!=nullptr)
    {
        if (hitForMouseUpProcessing_minus2MeansIgnore!=-2)
        {
            int mousePos[2];
            int mouseDownPos[2];
            subView->getMouseRelativePosition(mousePos);
            subView->getMouseDownRelativePosition(mouseDownPos);
            int dx[2]={mouseDownPos[0]-mousePos[0],mouseDownPos[1]-mousePos[1]};
            if (dx[0]*dx[0]+dx[1]*dx[1]>1)
                hitForMouseUpProcessing_minus2MeansIgnore=-2;
        }
        if (mouseJustWentDown)
        {
            hitForMouseUpProcessing_minus2MeansIgnore=-2;
            if (mouseJustWentUp)
            {
                processHitForMouseUpProcessing=true;
                if (selectionStatus==NOSELECTION)
                {
                    passes[0]=PICKPASS;
                    passes[1]=RENDERPASS;
                    mouseIsDown=false;
                }
                else
                {
                    passes[0]=PICKPASS;
                    passes[1]=RENDERPASS;
                }
            }
            else
            {
                if (selectionStatus==NOSELECTION)
                { // no ctrl or shift key
                    passes[0]=DEPTHPASS;
                    passes[1]=RENDERPASS;
// Until 3.2.1:
//                  passes[0]=DEPTHPASS;
//                  passes[1]=PICKPASS;
//                  passes[2]=RENDERPASS;
                    if ((App::getMouseMode()&sim_navigation_clickselection)==0)
                        regularObjectsCannotBeSelected=true;
                    else
                        specialSelectionAndNavigationPass=true;
                }
                else if (selectionStatus==SHIFTSELECTION)
                { // shift key
//                  pass=RENDERPASS;
//                  numberOfPasses=1;
                }
                else
                { // ctrl key
                    passes[0]=PICKPASS;
                    passes[1]=RENDERPASS;
                }
            }
        }
        else if (mouseJustWentUp)
        {
            processHitForMouseUpProcessing=true;
            if (selectionStatus==SHIFTSELECTION)
            {
                if (((App::getEditModeType()&SHAPE_EDIT_MODE)!=0)&&(App::mainWindow->getKeyDownState()&1))
                    passes[0]=COLORCODEDPICKPASS;
                else
                    passes[0]=PICKPASS;
                passes[1]=RENDERPASS;
            }
            else
            {
                // New since 3.2.2:
                if (App::mainWindow->getMouseButtonState()&16)
                { // last mouse down was left and no ctrl key was pressed
                    int mousePos[2];
                    int mouseDownPos[2];
                    subView->getMouseRelativePosition(mousePos);
                    subView->getMouseDownRelativePosition(mouseDownPos);
                    if ( (abs(mousePos[0]-mouseDownPos[0])<2)&&(abs(mousePos[1]-mouseDownPos[1])<2) )
                    {
                        passes[0]=PICKPASS;
                        passes[1]=RENDERPASS;
                    }
                }
            }
        }
    }

//  App::ct->objCont->setUniqueSelectedPathID(-1);


    if (getInternalRendering())
    { // regular rendering
        for (int passIndex=0;passIndex<3;passIndex++)
        {
            // Keep following 2, it seems on Mac we otherwise always have a problem. Even if it seems that the depth test is always restored after disabling it...
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);

            int pass=passes[passIndex];
            if (pass==-1)
                break;
            if (pass==RENDERPASS)
                App::ct->environment->setBackgroundColor(currentWinSize);
            else
            {
                glClearColor(0.0,0.0,0.0,0.0);
                glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
            }

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            if (pass==PICKPASS)
            {
                if (!_allowPicking)
                    break;
                GLint viewport[4];
                glSelectBuffer(SELECTION_BUFFER_SIZE*4-4,selectBuff); //+4 is for...? Sometime
                // the application badly crashes during picking, and I guess it might be because
                // openGL is accessing forbidden memory... But I don't know!!

                glGetIntegerv(GL_VIEWPORT,viewport);
                glRenderMode(GL_SELECT);
                glInitNames();
                glPushName(0);
                int mouseX=mouseRelativePosition[0];
                int mouseY=mouseRelativePosition[1];
                tt::limitValue(1,currentWinSize[0],mouseX);
                tt::limitValue(1,currentWinSize[1],mouseY);
                int pickSizeX=abs(mouseX-mouseDownRelativePosition[0]);
                int pickSizeY=abs(mouseY-mouseDownRelativePosition[1]);
                tt::limitValue(1,10000,pickSizeX);
                tt::limitValue(1,10000,pickSizeY);
                int centerX=(mouseX+mouseDownRelativePosition[0])/2;
                int centerY=(mouseY+mouseDownRelativePosition[1])/2;
                if (selectionStatus==SHIFTSELECTION)
                    ogl::pickMatrixSpecial(centerX+viewport[0],centerY+viewport[1],pickSizeX,pickSizeY,viewport);
                else
                    ogl::pickMatrixSpecial(mouseX+viewport[0],mouseY+viewport[1],3,3,viewport);
            }

            if (pass!=PICKPASS)
                glRenderMode(GL_RENDER);

            float ratio=(float)(currentWinSize[0]/(float)currentWinSize[1]);
            if (isPerspective)
            {
                if (ratio>1.0f)
                {
                    float a=2.0f*(float)atan(tan(_viewAngle/2.0f)/ratio)*radToDeg_f;
                    ogl::perspectiveSpecial(a,ratio,_nearClippingPlane,_farClippingPlane);
                }
                else
                    ogl::perspectiveSpecial(_viewAngle*radToDeg_f,ratio,_nearClippingPlane,_farClippingPlane);
            }
            else
            {
                if (ratio>1.0f)
                    glOrtho(-_orthoViewSize*0.5f,_orthoViewSize*0.5f,-_orthoViewSize*0.5f/ratio,_orthoViewSize*0.5f/ratio,ORTHO_CAMERA_NEAR_CLIPPING_PLANE,ORTHO_CAMERA_FAR_CLIPPING_PLANE);
                else
                    glOrtho(-_orthoViewSize*0.5f*ratio,_orthoViewSize*0.5f*ratio,-_orthoViewSize*0.5f,_orthoViewSize*0.5f,ORTHO_CAMERA_NEAR_CLIPPING_PLANE,ORTHO_CAMERA_FAR_CLIPPING_PLANE);
            }
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            C4X4Matrix m4(getCumulativeTransformation_forDisplay(true).getMatrix());

            if (App::mainWindow->getHasStereo())
            { // handle stereo cameras correctly
                C4X4Matrix displ;
                displ.setIdentity();
                if (App::mainWindow->getLeftEye())
                    displ.X(0)=App::mainWindow->getStereoDistance()/2;
                else
                    displ.X(0)=-App::mainWindow->getStereoDistance()/2;
                m4=m4*displ;
            }

            // The following 6 instructions have the same effect as gluLookAt()
            m4.inverse();
            m4.rotateAroundY(piValue_f);
            float m4_[4][4];
            m4.copyTo(m4_);
            CMeshManip::transposeMatrix_4x4Array(m4_);
            glLoadMatrixf((float*)m4_);


            if (pass==RENDERPASS)
            {
                App::ct->environment->activateAmbientLight(true);
                App::ct->environment->activateFogIfEnabled(this,App::ct->simulation->getDynamicContentVisualizationOnly());
                _activateNonAmbientLights(-1,this);
            }
            else
            { // Without lights is faster
                App::ct->environment->activateAmbientLight(false);
                App::ct->environment->deactivateFog();
                _activateNonAmbientLights(-2,nullptr);
            }

            if ((pass==DEPTHPASS)||(pass==RENDERPASS))
            {
                _handleMirrors(renderingMode,selectionStatus==NOSELECTION,pass,navigationMode,currentWinSize,subView);
            }



            bool secondPartOfManipOverlayNeeded=false;

            // Display the sphere for rotation and shift-operations:
            //-----------------------------------------------------------------------------------------------------
            if (mouseIsDown&&(!passiveSubView)&&(pass==RENDERPASS)&&(selectionStatus==NOSELECTION)&&( (navigationMode==sim_navigation_camerarotate)||
                (navigationMode==sim_navigation_camerashift)||(navigationMode==sim_navigation_cameratilt)||
                (navigationMode==sim_navigation_objectshift)||
                (navigationMode==sim_navigation_objectrotate) ) )
            {
                float centerPos[3];
                subView->getCenterPosition(centerPos);
                float mousePosDepth=subView->getMousePositionDepth();
                glLoadName(-1);
                ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
                if ((navigationMode==sim_navigation_camerarotate)||
                    (navigationMode==sim_navigation_camerashift)||
                    (navigationMode==sim_navigation_cameratilt))
                {
                    ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorRed);
                    glPushMatrix();
                    glTranslatef(centerPos[0],centerPos[1],centerPos[2]);
                    float sphereRadius=10.0f;
                    if (isPerspective)
                    {
                        if (ratio>1.0f)
                            sphereRadius=mousePosDepth*tan(_viewAngle*0.5f)*sphereRadius/currentWinSize[0];
                        else
                            sphereRadius=mousePosDepth*tan(_viewAngle*0.5f)*sphereRadius/currentWinSize[1];
                    }
                    else
                    {
                        if (ratio>1.0f)
                            sphereRadius=_orthoViewSize*0.5f*sphereRadius/currentWinSize[0];
                        else
                            sphereRadius=_orthoViewSize*0.5f*sphereRadius/currentWinSize[1];
                    }
                    App::ct->environment->temporarilyDeactivateFog();

                    ogl::drawSphere(sphereRadius,10,5,true);
                    glPopMatrix();

                    // Following is a bit dirty, but quick ;)
                    float clippNear=_nearClippingPlane;
                    if (!isPerspective)
                        clippNear=ORTHO_CAMERA_NEAR_CLIPPING_PLANE;
                    if ( (mousePosDepth==clippNear)&&(navigationMode!=sim_navigation_cameratilt) )
                    { // We should display a differentiated thing here (kind of half-error!)
                        C7Vector cct(getCumulativeTransformationPart1());
                        C7Vector icct(cct.getInverse());
                        C3Vector c(centerPos);
                        C3Vector rc(icct*c);
                        float di=2.0f;
                        for (int jk=0;jk<2;jk++)
                        {
                            for (float i0=-di;i0<2.2f*di;i0+=2.0f*di)
                            {
                                for (float j0=-di;j0<2.2f*di;j0+=2.0f*di)
                                {
                                    c=rc+C3Vector(i0*sphereRadius,j0*sphereRadius,0.0f);
                                    c=cct*c;
                                    glPushMatrix();
                                    glTranslatef(c(0),c(1),c(2));
                                    ogl::drawSphere(sphereRadius,10,5,true);
                                    glPopMatrix();
                                }
                            }
                            di*=2.0f;
                        }
                    }
                    else
                    { // we display some neat wireframe!
                    }
                    App::ct->environment->reactivateFogThatWasTemporarilyDisabled();
                }
                else
                { // We only display the manip mode overlay grids (no more green ball):
                    secondPartOfManipOverlayNeeded=true;
                    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
                    for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
                    {
                        C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
                        it->displayManipulationModeOverlayGrid(false);
                    }
                }
            }
            //-----------------------------------------------------------------------------------------------------

            // very normal rendering:
            _drawObjects(renderingMode,pass,currentWinSize,subView,false);

            if (secondPartOfManipOverlayNeeded)
            {
                ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
                ogl::setAlpha(0.25f);

                for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
                {
                    C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
                    it->displayManipulationModeOverlayGrid(true);
                }
            }

            if (pass==DEPTHPASS)
            {
                performDepthPerception(subView,isPerspective);
                // The following instruct. is important depending on the depth-testing type
                // we perform. It can be omitted with glDepthFunc(GL_LEQUAL)
    //          App::ct->environment->setBackgroundColor(currentWinSize);
            }
            if (pass==PICKPASS)
                hitForMouseUpProcessing_minus2MeansIgnore=handleHits(glRenderMode(GL_RENDER),selectBuff);

            if (pass==COLORCODEDPICKPASS)
            {
                GLint viewport[4];
                glGetIntegerv(GL_VIEWPORT,viewport);

                int mouseX=mouseRelativePosition[0];
                int mouseY=mouseRelativePosition[1];
                tt::limitValue(1,currentWinSize[0],mouseX);
                tt::limitValue(1,currentWinSize[1],mouseY);
                int pickSizeX=abs(mouseX-mouseDownRelativePosition[0]);
                int pickSizeY=abs(mouseY-mouseDownRelativePosition[1]);
                tt::limitValue(1,10000,pickSizeX);
                tt::limitValue(1,10000,pickSizeY);
                unsigned char* bf=new unsigned char[pickSizeX*pickSizeY*3];
                glPixelStorei(GL_PACK_ALIGNMENT,1);
                glReadPixels(SIM_MIN(mouseX,mouseDownRelativePosition[0])+viewport[0],SIM_MIN(mouseY,mouseDownRelativePosition[1])+viewport[1],pickSizeX,pickSizeY,GL_RGB,GL_UNSIGNED_BYTE,bf);
                glPixelStorei(GL_PACK_ALIGNMENT,4); // important to restore! Really?
                std::vector<bool> sel;
                sel.resize(1000000,false);
                for (int i=0;i<pickSizeX*pickSizeY;i++)
                {
                    unsigned int v=bf[3*i+0]+(bf[3*i+1]<<8)+(bf[3*i+2]<<16);
                    if (v<1000000)
                        sel[v]=true;
                }

                for (int i=1;i<1000000;i++)
                {
                    if (sel[i])
                        App::mainWindow->editModeContainer->addItemToEditModeBuffer(i-1,true);
                }

                delete[] bf;
            }

            if (processHitForMouseUpProcessing)
            {
                if ( ((App::getMouseMode()&0x0300)&sim_navigation_clickselection) && ((App::mainWindow->getKeyDownState()&3)==0) && (hitForMouseUpProcessing_minus2MeansIgnore!=-2) )
                    handleMouseUpHit(hitForMouseUpProcessing_minus2MeansIgnore);
                hitForMouseUpProcessing_minus2MeansIgnore=-2;
                processHitForMouseUpProcessing=false;
            }

            glRenderMode(GL_RENDER);
            pass++;
        }
    }


    App::ct->environment->deactivateFog();

    // Following allows to hand the rendered camera image to a plugin, that might modify it and return it!
    // Or - following is also used to generate an externally rendered view:
    bool executeNextRoutine=CPluginContainer::shouldSend_openglcameraview_msg();
    executeNextRoutine|=!getInternalRendering();
    if (executeNextRoutine)
    {
        char* buff=new char[_currentViewSize[0]*_currentViewSize[1]*3];
        bool applyNewImage=false;
        if (!getInternalRendering())
        {
            int rendererIndex=_renderMode-sim_rendermode_povray;

            bool renderView=true;
            if (App::mainWindow->simulationRecorder->getIsRecording()&&(rendererIndex<2))
                renderView=App::mainWindow->simulationRecorder->willNextFrameBeRecorded();

            if (renderView)
            { // When using a ray-tracer during video recording, and we want to record not every frame, don't render those frames!!
                App::mainWindow->openglWidget->doneCurrent();

#ifdef SIM_WITH_GUI
                if (!_extRenderer_prepareView(rendererIndex,_currentViewSize,isPerspective))
                {
                    if (rendererIndex==0)
                    {
                        static bool alreadyShown=false;
                        if (!alreadyShown)
                            App::uiThread->messageBox_information(App::mainWindow,"POV-Ray plugin",strTranslate("The POV-Ray plugin was not found, or could not be loaded. You can find the required binary and source code at https://github.com/CoppeliaRobotics/v_repExtPovRay"),VMESSAGEBOX_OKELI);
                        alreadyShown=true;
                    }
                }
#endif
                _extRenderer_prepareLights();
                _extRenderer_prepareMirrors();

                if ((_renderMode==sim_rendermode_povray)||(_renderMode==sim_rendermode_opengl3))
                    setFrustumCullingTemporarilyDisabled(true); // important with ray-tracers and similar

                // Draw objects:
                _drawObjects(RENDERING_MODE_SOLID,RENDERPASS,currentWinSize,subView,false);

                if ((_renderMode==sim_rendermode_povray)||(_renderMode==sim_rendermode_opengl3))
                    setFrustumCullingTemporarilyDisabled(false); // important with ray-tracers and similar

                _extRenderer_retrieveImage(buff);

                App::mainWindow->openglWidget->makeCurrent();

                applyNewImage=true;
            }
        }
        else
        { // regular openGl
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT,viewport);

            glPixelStorei(GL_PACK_ALIGNMENT,1);
            glReadPixels(viewport[0],viewport[1], _currentViewSize[0], _currentViewSize[1], GL_RGB, GL_UNSIGNED_BYTE, buff);
            glPixelStorei(GL_PACK_ALIGNMENT,4); // important to restore! Really?

            int auxVals[4];
            int retVals[4];
            auxVals[0]=_currentViewSize[0];
            auxVals[1]=_currentViewSize[1];
            auxVals[2]=-1;
            if (subView!=nullptr)
                auxVals[2]=subView->getViewIndex();
            auxVals[3]=0;
            CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_openglcameraview,auxVals,buff,retVals);
            applyNewImage=(auxVals[3]!=0);
        }

        if (applyNewImage)
        { // we want to apply a new image!
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0,_currentViewSize[0],0,_currentViewSize[1],-100,100);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity ();
            glDisable(GL_DEPTH_TEST);
            ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorWhite);
            if (_textureNameForExtGeneratedView==(unsigned int)-1)
                glGenTextures(1,&_textureNameForExtGeneratedView);
            glBindTexture(GL_TEXTURE_2D,_textureNameForExtGeneratedView);
            glPixelStorei(GL_UNPACK_ALIGNMENT,1);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,_currentViewSize[0],_currentViewSize[1],0,GL_RGB,GL_UNSIGNED_BYTE,buff);
            glPixelStorei(GL_UNPACK_ALIGNMENT,4); // important to restore! Really?

            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // keep to GL_LINEAR here!!
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
            glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
            glTexEnvi (GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,_textureNameForExtGeneratedView);

            glColor3f(1.0f,1.0f,1.0f);

            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f);
            glVertex3i(0,0,0);
            glTexCoord2f(0.0f,1.0f);
            glVertex3i(0,_currentViewSize[1],0);
            glTexCoord2f(1.0f,1.0f);
            glVertex3i(_currentViewSize[0],_currentViewSize[1],0);
            glTexCoord2f(1.0f,0.0f);
            glVertex3i(_currentViewSize[0],0,0);
            glEnd();

            glDisable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);
        }
        delete[] buff;
    }

    _drawOverlay(passiveSubView,drawText,displ_ref,currentWinSize,subView);
}

void CCamera::_handleMirrors(int renderingMode,bool noSelection,int pass,int navigationMode,int currentWinSize[2],CSView* subView)
{
    if (App::ct->objCont->mirrorList.size()==0)
        return;

    C7Vector camTr(getCumulativeTransformation());
    C7Vector camTri(camTr.getInverse());
    setFrustumCullingTemporarilyDisabled(true);
    // Prep stencil buffer:
    glEnable(GL_STENCIL_TEST);
    glClearStencil(0);
    glClear (GL_STENCIL_BUFFER_BIT);
    int drawOk=1;

    std::vector<int> allMirrors;
    std::vector<float> allMirrorDist;
    for (int mir=0;mir<int(App::ct->objCont->mirrorList.size());mir++)
    {
        CMirror* myMirror=App::ct->objCont->getMirror(App::ct->objCont->mirrorList[mir]);
        C7Vector mmtr(myMirror->getCumulativeTransformation());
        mmtr=camTri*mmtr;

        if ( (!myMirror->isObjectPartOfInvisibleModel())&&(App::ct->mainSettings->getActiveLayers()&myMirror->layer)&&myMirror->getIsMirror() )
        {
            allMirrors.push_back(App::ct->objCont->mirrorList[mir]);
            allMirrorDist.push_back(mmtr.X(2));
        }
    }
    tt::orderAscending(allMirrorDist,allMirrors);

    for (int mir=0;mir<int(allMirrors.size());mir++)
    {
        CMirror* myMirror=App::ct->objCont->getMirror(allMirrors[mir]);

        C7Vector mtr(myMirror->getCumulativeTransformation());
        C7Vector mtri(mtr.getInverse());
        C3Vector mtrN(mtr.Q.getMatrix().axis[2]);
        C4Vector mtrAxis=mtr.Q.getAngleAndAxisNoChecking();
        C4Vector mtriAxis=mtri.Q.getAngleAndAxisNoChecking();
        float d=(mtrN*mtr.X);
        C3Vector v0(+myMirror->getMirrorWidth()*0.5f,-myMirror->getMirrorHeight()*0.5f,0.0f);
        C3Vector v1(+myMirror->getMirrorWidth()*0.5f,+myMirror->getMirrorHeight()*0.5f,0.0f);
        C3Vector v2(-myMirror->getMirrorWidth()*0.5f,+myMirror->getMirrorHeight()*0.5f,0.0f);
        C3Vector v3(-myMirror->getMirrorWidth()*0.5f,-myMirror->getMirrorHeight()*0.5f,0.0f);
        v0*=mtr;
        v1*=mtr;
        v2*=mtr;
        v3*=mtr;

        C3Vector MirrCam(camTr.X-mtr.X);
        bool inFrontOfMirror=(((MirrCam*mtrN)>0.0f)&&myMirror->getActive()&&(!App::ct->mainSettings->mirrorsDisabled) );

        glStencilFunc(GL_ALWAYS, drawOk, drawOk); // we can draw everywhere
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // we draw drawOk where depth test passes
        glDepthMask(GL_FALSE);
        glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
        glBegin (GL_QUADS);
        glVertex3fv(v0.data);
        glVertex3fv(v1.data);
        glVertex3fv(v2.data);
        glVertex3fv(v3.data);
        glEnd ();
        glDepthMask(GL_TRUE);
        glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

        // Enable stencil masking:
        glStencilFunc(GL_EQUAL, drawOk, drawOk); // we draw only where stencil is drawOk
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        // Draw the mirror view:
        if (inFrontOfMirror)
        {
            glEnable(GL_CLIP_PLANE0);
            double cpv[4]={-mtrN(0),-mtrN(1),-mtrN(2),d};
            glClipPlane(GL_CLIP_PLANE0,cpv);
            glPushMatrix();
            glTranslatef(mtr.X(0),mtr.X(1),mtr.X(2));
            glRotatef(mtrAxis(0)*radToDeg_f,mtrAxis(1),mtrAxis(2),mtrAxis(3));
            glScalef (1., 1., -1.);
            glTranslatef(mtri.X(0),mtri.X(1),mtri.X(2));
            glRotatef(mtriAxis(0)*radToDeg_f,mtriAxis(1),mtriAxis(2),mtriAxis(3));
            glFrontFace (GL_CW);
            CMirror::currentMirrorContentBeingRendered=myMirror->getObjectHandle();
            _drawObjects(renderingMode,pass,currentWinSize,subView,true);
            CMirror::currentMirrorContentBeingRendered=-1;
            glFrontFace (GL_CCW);
            glPopMatrix();
            glDisable(GL_CLIP_PLANE0);
        }

        // Now draw the mirror overlay:
        glPushAttrib (0xffffffff);
        glDepthFunc(GL_ALWAYS);
        ogl::disableLighting_useWithCare(); // only temporarily
        if (inFrontOfMirror)
        {
            glEnable (GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f (myMirror->mirrorColor[0],myMirror->mirrorColor[1],myMirror->mirrorColor[2],1.0f-myMirror->getReflectance());
        }
        else
        {
            glColor4f (myMirror->mirrorColor[0],myMirror->mirrorColor[1],myMirror->mirrorColor[2],1.0f);
        }
        glBegin (GL_QUADS);
        glVertex3fv(v0.data);
        glVertex3fv(v1.data);
        glVertex3fv(v2.data);
        glVertex3fv(v3.data);
        glEnd ();
        ogl::enableLighting_useWithCare();
        glPopAttrib();
        glDepthFunc(GL_LEQUAL);
        drawOk++;
    }
    glDisable(GL_STENCIL_TEST);

    setFrustumCullingTemporarilyDisabled(false);
}

bool CCamera::_extRenderer_prepareView(int extRendererIndex,int resolution[2],bool perspective)
{   // Set-up the resolution, clear color, camera properties and camera pose:

    bool retVal=CPluginContainer::selectExtRenderer(extRendererIndex);

    void* data[30];
    data[0]=resolution+0;
    data[1]=resolution+1;
    data[2]=App::ct->environment->fogBackgroundColor;
    C7Vector tr(getCumulativeTransformation_forDisplay(true));
    data[3]=tr.X.data;
    data[4]=tr.Q.data;
    int options=0;
    float xAngle_size;
    float yAngle_size;
    float ratio=(float)(resolution[0]/(float)resolution[1]);
    float nearClip,farClip;
    if (perspective)
    {
        if (ratio>1.0f)
        {
            xAngle_size=_viewAngle;
            yAngle_size=2.0f*(float)atan(tan(_viewAngle/2.0f)/ratio);
        }
        else
        {
            xAngle_size=2.0f*(float)atan(tan(_viewAngle/2.0f)*ratio);
            yAngle_size=_viewAngle;
        }
        nearClip=_nearClippingPlane;
        farClip=_farClippingPlane;
    }
    else
    {
        options|=1;
        if (ratio>1.0f)
        {
            xAngle_size=_orthoViewSize;
            yAngle_size=_orthoViewSize/ratio;
        }
        else
        {
            xAngle_size=_orthoViewSize*ratio;
            yAngle_size=_orthoViewSize;
        }
        nearClip=ORTHO_CAMERA_NEAR_CLIPPING_PLANE;
        farClip=ORTHO_CAMERA_FAR_CLIPPING_PLANE;
    }
    data[5]=&options;
    data[6]=&xAngle_size;
    data[7]=&yAngle_size;
    data[8]=&_viewAngle;
    data[9]=&nearClip;
    data[10]=&farClip;
    data[11]=App::ct->environment->ambientLightColor;
    data[12]=App::ct->environment->fogBackgroundColor;
    int fogType=App::ct->environment->getFogType();
    float fogStart=App::ct->environment->getFogStart();
    float fogEnd=App::ct->environment->getFogEnd();
    float fogDensity=App::ct->environment->getFogDensity();
    bool fogEnabled=App::ct->environment->getFogEnabled();
    data[13]=&fogType;
    data[14]=&fogStart;
    data[15]=&fogEnd;
    data[16]=&fogDensity;
    data[17]=&fogEnabled;
    data[18]=&_orthoViewSize;
    data[19]=&_objectHandle;
    data[20]=nullptr;
    data[21]=nullptr;

    // Following actually free since V-REP 3.3.0
    // But the older PovRay plugin version crash without this:
    float povFogDist=4.0f;
    float povFogTransp=0.5f;
    bool povFocalBlurEnabled=false;
    float povFocalLength,povAperture;
    int povBlurSamples;
    data[22]=&povFogDist;
    data[23]=&povFogTransp;
    data[24]=&povFocalBlurEnabled;
    data[25]=&povFocalLength;
    data[26]=&povAperture;
    data[27]=&povBlurSamples;

    CPluginContainer::extRenderer(sim_message_eventcallback_extrenderer_start,data);
    return(retVal);
}

void CCamera::_extRenderer_prepareLights()
{   // Set-up the lights:
    for (unsigned int li=0;li<App::ct->objCont->lightList.size();li++)
    {
        CLight* light=App::ct->objCont->getLight(App::ct->objCont->lightList[li]);
        if (light->getLightActive())
        {
            void* data[20];
            int lightType=light->getLightType();
            data[0]=&lightType;
            float cutoffAngle=light->getSpotCutoffAngle();
            data[1]=&cutoffAngle;
            int spotExponent=light->getSpotExponent();
            data[2]=&spotExponent;
            data[3]=light->getColor(true)->colors;
            float constAttenuation=light->getAttenuationFactor(CONSTANT_ATTENUATION);
            data[4]=&constAttenuation;
            float linAttenuation=light->getAttenuationFactor(LINEAR_ATTENUATION);
            data[5]=&linAttenuation;
            float quadAttenuation=light->getAttenuationFactor(QUADRATIC_ATTENUATION);
            data[6]=&quadAttenuation;
            C7Vector tr(light->getCumulativeTransformation_forDisplay(true));
            data[7]=tr.X.data;
            data[8]=tr.Q.data;
            float lightSize=light->getLightSize();
            data[9]=&lightSize;
            bool lightIsVisible=light->getShouldObjectBeDisplayed(_objectHandle,0);
            data[11]=&lightIsVisible;
            int lightHandle=light->getObjectHandle();
            data[13]=&lightHandle;

            // Following actually free since V-REP 3.3.0
            // But the older PovRay plugin version crash without this:
            float povFadeXDist=0.0;
            bool povNoShadow=false;
            data[10]=&povFadeXDist;
            data[12]=&povNoShadow;

            CPluginContainer::extRenderer(sim_message_eventcallback_extrenderer_light,data);
        }
    }
}

void CCamera::_extRenderer_prepareMirrors()
{
    for (unsigned int li=0;li<App::ct->objCont->mirrorList.size();li++)
    {
        CMirror* mirror=App::ct->objCont->getMirror(App::ct->objCont->mirrorList[li]);
        bool visible=mirror->getShouldObjectBeDisplayed(_objectHandle,_attributesForRendering);
        if (mirror->getIsMirror()&&visible)
        {
            bool active=mirror->getActive()&&(!App::ct->mainSettings->mirrorsDisabled);
            C7Vector tr=mirror->getCumulativeTransformationPart1_forDisplay(true);
            float w_=mirror->getMirrorWidth()/2.0f;
            float h_=mirror->getMirrorHeight()/2.0f;
            float vertices[18]={w_,-h_,0.0005f,w_,h_,0.0005f,-w_,-h_,0.0005f,-w_,-h_,0.0005f,w_,h_,0.0005f,-w_,h_,0.0005f};
            int verticesCnt=6;
            float normals[18]={0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f};
            for (int i=0;i<6;i++)
            {
                C3Vector v(vertices+i*3);
                v*=tr;
                C3Vector n(normals+i*3);
                n=tr.Q*n;
                vertices[i*3+0]=v(0);
                vertices[i*3+1]=v(1);
                vertices[i*3+2]=v(2);
                normals[i*3+0]=n(0);
                normals[i*3+1]=n(1);
                normals[i*3+2]=n(2);
            }
            void* data[20];
            data[0]=vertices;
            data[1]=&verticesCnt;
            data[2]=normals;
            float colors[15];
            colors[0]=mirror->mirrorColor[0];
            colors[1]=mirror->mirrorColor[1];
            colors[2]=mirror->mirrorColor[2];
            colors[6]=0.0f;
            colors[7]=0.0f;
            colors[8]=0.0f;
            colors[9]=0.0f;
            colors[10]=0.0f;
            colors[11]=0.0f;
            colors[12]=0.0f;
            colors[13]=0.0f;
            colors[14]=0.0f;
            data[3]=colors;
            bool translucid=false;
            data[4]=&translucid;
            float opacityFactor=1.0f;
            data[5]=&opacityFactor;
            const char* povMaterial={"mirror"};
            data[6]=(char*)povMaterial;
            data[7]=&active;
            CPluginContainer::extRenderer(sim_message_eventcallback_extrenderer_triangles,data);
            C3Vector shift=tr.Q.getMatrix().axis[2]*(-0.001f);
            for (int i=0;i<6;i++)
            {
                C3Vector v(vertices+i*3);
                v+=shift;
                vertices[i*3+0]=v(0);
                vertices[i*3+1]=v(1);
                vertices[i*3+2]=v(2);
            }
            active=false;
            CPluginContainer::extRenderer(sim_message_eventcallback_extrenderer_triangles,data);
        }
    }
}

void CCamera::_extRenderer_retrieveImage(char* rgbBuffer)
{   // Fetch the finished image:
    void* data[20];
    data[0]=rgbBuffer;
    data[1]=nullptr;
    bool readRgb=true;
    data[2]=&readRgb;
    bool readDepth=false;
    data[3]=&readDepth;
    CPluginContainer::extRenderer(sim_message_eventcallback_extrenderer_stop,data);
}

void CCamera::_drawObjects(int renderingMode,int pass,int currentWinSize[2],CSView* subView,bool mirrored)
{
    FUNCTION_DEBUG;

    int displayAttrib=_attributesForRendering;
    if (pass==DEPTHPASS)
        displayAttrib=sim_displayattribute_depthpass|sim_displayattribute_forbidedges;
    if (mirrored)
        displayAttrib=sim_displayattribute_mirror;
    if (pass==PICKPASS)
        displayAttrib=sim_displayattribute_pickpass|sim_displayattribute_forbidedges;
    if (pass==COLORCODEDPICKPASS)
        displayAttrib=sim_displayattribute_colorcodedpickpass|sim_displayattribute_forbidedges;
    if (((renderingMode==RENDERING_MODE_WIREFRAME_EDGES)||(renderingMode==RENDERING_MODE_WIREFRAME_TRIANGLES))&&(pass==RENDERPASS))
    {
        displayAttrib|=sim_displayattribute_forcewireframe;
        if (renderingMode==RENDERING_MODE_WIREFRAME_TRIANGLES)
            displayAttrib|=sim_displayattribute_trianglewireframe;
    }

    if (App::ct->simulation->getDynamicContentVisualizationOnly())
        displayAttrib|=sim_displayattribute_dynamiccontentonly;

    int viewIndex=-1;
    if (subView!=nullptr)
    {
        if ((!subView->getShowEdges())||CEnvironment::getShapeEdgesTemporarilyDisabled())
            displayAttrib|=sim_displayattribute_forbidedges;
        if (subView->getThickEdges())
            displayAttrib|=sim_displayattribute_thickEdges;
        if ( subView->getVisualizeOnlyInertias()&&(App::getEditModeType()==NO_EDIT_MODE)&&(!App::ct->simulation->getDynamicContentVisualizationOnly()) )
            displayAttrib|=sim_displayattribute_inertiaonly|sim_displayattribute_forbidedges;
        viewIndex=subView->getViewIndex();
    }

    bool shapeEditMode=((App::getEditModeType()&SHAPE_EDIT_MODE)!=0);
    bool multiShapeEditMode=((App::getEditModeType()&MULTISHAPE_EDIT_MODE)!=0);
    bool shapeEditModeAndPicking=( shapeEditMode&&((pass==PICKPASS)||(pass==COLORCODEDPICKPASS)) );

    // If selection size is bigger than 10, we set up a fast index:
    std::vector<unsigned char>* selMap=nullptr; // An arry which will show which object is selected
    if (App::ct->objCont->getSelSize()>10)
    {
        selMap=new std::vector<unsigned char>(App::ct->objCont->getHighestObjectHandle()+1,0);
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            selMap->at(App::ct->objCont->getSelID(i))=1;
    }
    int lastSel=-1;
    if (App::ct->objCont->getSelSize()>0)
        lastSel=App::ct->objCont->getSelID(App::ct->objCont->getSelSize()-1);

    std::vector<C3DObject*> toRender;
    C3DObject* viewBoxObject=_getInfoOfWhatNeedsToBeRendered(toRender);

    if (viewBoxObject!=nullptr)
    { // we set the same position as the camera, but we keep the initial orientation
        // If the camera is in ortho view mode, we additionally shift it along the viewing axis
        // to be sure we don't cover anything visible with the far side of the box (the near side is clipped by model settings)
        C4Vector rel(viewBoxObject->getLocalTransformationPart1().Q);
        C7Vector cam(getCumulativeTransformation());
        if (!_currentPerspective)
        {
            C3Vector minV,maxV;
            bool first=true;
            viewBoxObject->getGlobalMarkingBoundingBox(getCumulativeTransformation().getInverse(),minV,maxV,first,true,true);
            float shift=ORTHO_CAMERA_FAR_CLIPPING_PLANE-0.505f*(maxV(2)-minV(2)); // just a bit more than half!
            cam.X+=cam.Q.getMatrix().axis[2]*shift;
        }
        C7Vector newLocal(viewBoxObject->getParentCumulativeTransformation().getInverse()*cam);
        newLocal.Q=rel;
        viewBoxObject->setLocalTransformation(newLocal);
        viewBoxObject->bufferMainDisplayStateVariables();
        viewBoxObject->bufferedMainDisplayStateVariablesToDisplay();
    }

    // For those special drawing routines that require the window size and other info:
    float verticalViewSizeOrAngle;
    float ratio=(float)(currentWinSize[0]/(float)currentWinSize[1]);
    if (_currentPerspective)
    {
        if (ratio>1.0f)
            verticalViewSizeOrAngle=2.0f*(float)atan(tan(_viewAngle/2.0f)/ratio);
        else
            verticalViewSizeOrAngle=_viewAngle;
    }
    else
    {
        if (ratio>1.0f)
            verticalViewSizeOrAngle=_orthoViewSize/ratio;
        else
            verticalViewSizeOrAngle=_orthoViewSize;
    }

    if (!multiShapeEditMode)
    {
        if (getInternalRendering())
        {
            glInitNames();
            glPushName(-1);
            glShadeModel(GL_SMOOTH);

            if ((displayAttrib&(sim_displayattribute_colorcoded|sim_displayattribute_colorcodedpickpass))==0)
                glEnable(GL_DITHER);
            else
                glDisable(GL_DITHER);

            _prepareAuxClippingPlanes();
            _enableAuxClippingPlanes(-1);
        }



        if (!shapeEditModeAndPicking)
        {
            if (getInternalRendering())
            {
                if ((displayAttrib&sim_displayattribute_noopenglcallbacks)==0)
                    CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_opengl,0,displayAttrib,_objectHandle,viewIndex);
            }

            if (getInternalRendering()) // following not yet implemented for ext. rendering
                App::ct->renderYourGeneralObject3DStuff_beforeRegularObjects(this,displayAttrib,currentWinSize,verticalViewSizeOrAngle,_currentPerspective); // those objects are not supposed to be translucid!

            if (getInternalRendering())
            {
                if ((displayAttrib&sim_displayattribute_noopenglcallbacks)==0)
                    CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_opengl,1,displayAttrib,_objectHandle,viewIndex);
            }
        }

        if (getInternalRendering())
            _disableAuxClippingPlanes();

        if (!shapeEditModeAndPicking)
        {
            for (int rp=0;rp<int(toRender.size());rp++)
            {
                C3DObject* it=toRender[rp];
                int atr=displayAttrib;
                if (((it->getCumulativeObjectProperty()&sim_objectproperty_depthinvisible)==0)||(pass!=DEPTHPASS))
                {
                    if (selMap!=nullptr)
                    {
                        if (selMap->at(it->getObjectHandle())!=0)
                            atr|=sim_displayattribute_selected;
                        if (it->getObjectHandle()==lastSel)
                            atr|=sim_displayattribute_mainselection;
                    }
                    else
                    {
                        for (int i=0;i<App::ct->objCont->getSelSize();i++)
                        {
                            if (it->getObjectHandle()==App::ct->objCont->getSelID(i))
                            {
                                atr|=sim_displayattribute_selected;
                                if (it->getObjectHandle()==lastSel)
                                    atr|=sim_displayattribute_mainselection;
                                break;
                            }
                        }
                    }
                    // Draw everything except for the camera you look through (unless we look through the mirror) and the object which is being edited!
                    if  ( (it->getObjectHandle()!=getObjectHandle())||mirrored )
                    {
                        if (App::ct->collections->isObjectInMarkedCollection(it->getObjectHandle()))
                            atr|=sim_displayattribute_groupselection;

                        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
                        {
                            if (getInternalRendering())
                                it->display(this,atr);
                            else
                            {
                                if (it->getObjectType()==sim_object_shape_type)
                                    ((CShape*)it)->display_extRenderer(this,atr);
                            }
                        }
                        else if (it->getObjectHandle()!=App::mainWindow->editModeContainer->getEditModeObjectID())
                            it->display(this,atr);
                    }
                }
            }
        }

        if (selMap!=nullptr)
        {
            delete selMap;
            selMap=nullptr;
        }

        if (getInternalRendering())
            _enableAuxClippingPlanes(-1);

        if (!shapeEditModeAndPicking)
        {
            if (getInternalRendering())
            {
                if ((displayAttrib&sim_displayattribute_noopenglcallbacks)==0)
                    CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_opengl,2,displayAttrib,_objectHandle,viewIndex);
            }

            if (getInternalRendering()) // following not yet implemented for ext. rendering
                App::ct->renderYourGeneralObject3DStuff_afterRegularObjects(this,displayAttrib,currentWinSize,verticalViewSizeOrAngle,_currentPerspective); // those objects can be translucid

            if (getInternalRendering())
            {
                if ((displayAttrib&sim_displayattribute_noopenglcallbacks)==0)
                    CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_opengl,3,displayAttrib,_objectHandle,viewIndex);
            }

            if (getInternalRendering()) // following not yet implemented for ext. rendering
                App::ct->renderYourGeneralObject3DStuff_onTopOfRegularObjects(this,displayAttrib,currentWinSize,verticalViewSizeOrAngle,_currentPerspective); // those objects are overlay

            if (getInternalRendering())
            {
                if ((displayAttrib&sim_displayattribute_noopenglcallbacks)==0)
                    CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_opengl,4,displayAttrib,_objectHandle,viewIndex);
            }

            if (getInternalRendering())
            {   //Now we display all graphs' 3D curves that should appear on top of everything:
                for (int i=0;i<int(App::ct->objCont->graphList.size());i++)
                {
                    CGraph* it=App::ct->objCont->getGraph(App::ct->objCont->graphList[i]);
                    if (it!=nullptr)
                    {
                        it->setJustDrawCurves(true);
                        it->display(this,displayAttrib);
                        it->setJustDrawCurves(false);
                    }
                }
            }
        }

        if (getInternalRendering())
            _disableAuxClippingPlanes();

        // We render the object being edited last (the vertices appear above everything)
        if (getInternalRendering())
        {
            if (App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)
            {
                C3DObject* it=App::mainWindow->editModeContainer->getEditModeObject();
                if (it!=nullptr)
                    it->display(this,displayAttrib);
            }
        }

        if (getInternalRendering())
        {
            glLoadName(-1);

            if (pass==RENDERPASS)
            {
                // Wireless communication activities:
                if ((displayAttrib&sim_displayattribute_dynamiccontentonly)==0)
                    App::ct->luaScriptContainer->broadcastDataContainer.visualizeCommunications(VDateTime::getTimeInMs());
            }

            if (!shapeEditModeAndPicking)
            {
                if ((displayAttrib&sim_displayattribute_noopenglcallbacks)==0)
                    CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_opengl,5,displayAttrib,_objectHandle,viewIndex);
            }
        }
    }
    else
    { // Multishape edit mode:
        CShape* it=App::mainWindow->editModeContainer->getEditModeShape();
        if (it!=nullptr)
            it->display(this,displayAttrib);
    }

    SModelThumbnailInfo* info=App::mainWindow->openglWidget->getModelDragAndDropInfo();
    if ((pass==RENDERPASS)&&(info!=nullptr))
    {
        float ss=info->modelNonDefaultTranslationStepSize;
        if (ss==0.0)
            ss=App::userSettings->getTranslationStepSize();
        float x=info->desiredDropPos(0)-fmod(info->desiredDropPos(0),ss);
        float y=info->desiredDropPos(1)-fmod(info->desiredDropPos(1),ss);
        const float pink[3]={1.0,1.0,0.0};
        ogl::setMaterialColor(pink,ogl::colorBlack,ogl::colorBlack);
        ogl::setBlending(false);
        glPushMatrix();
        glTranslatef(x+info->modelTr.X(0),y+info->modelTr.X(1),info->modelTr.X(2));
        C4Vector axis=info->modelTr.Q.getAngleAndAxis();
        glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
        ogl::drawBox(info->modelBoundingBoxSize(0),info->modelBoundingBoxSize(1),info->modelBoundingBoxSize(2),true,nullptr);
        glPopMatrix();
        ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,pink);
        glPushMatrix();
        glTranslatef(x,y,0.002f);
        glLineWidth(2.0f);
        ogl::drawCircle(0.125f,32);
        ogl::drawCircle(0.25f,32);
        ogl::drawCircle(0.5,32);
        ogl::drawCircle(1.0,32);
        ogl::drawCircle(2.0,32);
        glPopMatrix();
        glLineStipple(1,0x0F0F);
        glEnable(GL_LINE_STIPPLE);
        ogl::buffer.clear();
        ogl::addBuffer3DPoints(x,y,0.0);
        ogl::addBuffer3DPoints(x,y,info->modelTr.X(2));
        ogl::drawRandom3dLines(&ogl::buffer[0],int(ogl::buffer.size()/3),false,nullptr);
        glDisable(GL_LINE_STIPPLE);
        glLineWidth(1.0f);
    }
}

C3DObject* CCamera::_getInfoOfWhatNeedsToBeRendered(std::vector<C3DObject*>& toRender)
{
    std::vector<int> transparentObjects;
    std::vector<float> transparentObjectDist;
    C7Vector camTrInv(getCumulativeTransformationPart1().getInverse());
    C3DObject* viewBoxObject=nullptr;
    for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
        if (it->getObjectType()==sim_object_shape_type)
        {
            CShape* sh=(CShape*)it;
            if (sh->getContainsTransparentComponent())
            {
                C7Vector obj(it->getCumulativeTransformationPart1());
                transparentObjectDist.push_back(-(camTrInv*obj).X(2)-it->getTransparentObjectDistanceOffset());
                transparentObjects.push_back(it->getObjectHandle());
            }
            else
                toRender.push_back(it);
        }
        else
        {
            if (it->getObjectType()==sim_object_mirror_type)
            {
                CMirror* mir=(CMirror*)it;
                if (mir->getContainsTransparentComponent())
                {
                    C7Vector obj(it->getCumulativeTransformationPart1());
                    transparentObjectDist.push_back(-(camTrInv*obj).X(2)-it->getTransparentObjectDistanceOffset());
                    transparentObjects.push_back(it->getObjectHandle());
                }
                else
                    toRender.push_back(it);
            }
            else
                toRender.push_back(it);
        }
        if (it->getObjectName()==IDSOGL_SKYBOX_DO_NOT_RENAME)
            viewBoxObject=it;
    }

    tt::orderAscending(transparentObjectDist,transparentObjects);
    for (int i=0;i<int(transparentObjects.size());i++)
        toRender.push_back(App::ct->objCont->getObjectFromHandle(transparentObjects[i]));

    return(viewBoxObject);
}

void CCamera::performDepthPerception(CSView* subView,bool isPerspective)
{
    FUNCTION_DEBUG;
    if (subView==nullptr)
        return;
    int mouseMode=App::getMouseMode();
    int windowSize[2];
    subView->getViewSize(windowSize);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    GLfloat pixel[1];
    float farDivFarMinusNear=_farClippingPlane/(_farClippingPlane-_nearClippingPlane);
    if (!isPerspective)
        farDivFarMinusNear=ORTHO_CAMERA_FAR_CLIPPING_PLANE/(ORTHO_CAMERA_FAR_CLIPPING_PLANE-ORTHO_CAMERA_NEAR_CLIPPING_PLANE);
    float m[4][4];
    getCumulativeTransformationMatrixPart1(m);

    int mouseRelativePosition[2];
    subView->getMouseRelativePosition(mouseRelativePosition);

    glReadPixels(mouseRelativePosition[0]+viewport[0],mouseRelativePosition[1]+viewport[1],1,1,GL_DEPTH_COMPONENT,GL_FLOAT,pixel);
    if (isPerspective)
        subView->setMousePositionDepth(_nearClippingPlane*_farClippingPlane/((_farClippingPlane-_nearClippingPlane)*(farDivFarMinusNear-pixel[0])));
    else
        subView->setMousePositionDepth(ORTHO_CAMERA_NEAR_CLIPPING_PLANE+pixel[0]*(ORTHO_CAMERA_FAR_CLIPPING_PLANE-ORTHO_CAMERA_NEAR_CLIPPING_PLANE));

    float clippFar=_farClippingPlane;
    float clippNear=_nearClippingPlane;
    if (!isPerspective)
    {
        clippFar=ORTHO_CAMERA_FAR_CLIPPING_PLANE;
        clippNear=ORTHO_CAMERA_NEAR_CLIPPING_PLANE;
    }

    if ( (pixel[0]>=(1.0f-2.0f*std::numeric_limits<float>::epsilon())) || ((mouseMode&0x00ff)==sim_navigation_cameratilt) )
    { // The cursor hit the far clipping plane or we are in a tilting mode:
        subView->setMousePositionDepth(clippNear);
        float p[3];
        p[0]=m[0][3]+m[0][2]*clippNear;
        p[1]=m[1][3]+m[1][2]*clippNear;
        p[2]=m[2][3]+m[2][2]*clippNear;
        subView->setCenterPosition(p);
    }
    else
    {
        float yShift;
        float xShift;
        float ratio=(float)(windowSize[0]/(float)windowSize[1]);
        int mouseDownRelPos[2];
        subView->getMouseDownRelativePosition(mouseDownRelPos);
        if (isPerspective)
        {
            if (ratio>1.0f)
            {
                float tmp=(windowSize[0]/2)/tan(_viewAngle*0.5f);
                xShift=(-mouseDownRelPos[0]+(windowSize[0]/2))*subView->getMousePositionDepth()/tmp;
                float angle2=2.0f*(float)atan(tan(_viewAngle/2.0f)/ratio)*radToDeg_f;
                tmp=(windowSize[1]/2)/tan(angle2*degToRad_f*0.5f);
                yShift=(mouseDownRelPos[1]-(windowSize[1]/2))*subView->getMousePositionDepth()/tmp;
            }
            else
            {
                float tmp=(windowSize[1]/2)/tan(_viewAngle*0.5f);
                yShift=(mouseDownRelPos[1]-(windowSize[1]/2))*subView->getMousePositionDepth()/tmp;
                float angle2=2.0f*(float)atan(tan(_viewAngle/2.0f)*ratio)*radToDeg_f;
                tmp=(windowSize[0]/2)/tan(angle2*degToRad_f*0.5f);
                xShift=(-mouseDownRelPos[0]+(windowSize[0]/2))*subView->getMousePositionDepth()/tmp;
            }
        }
        else
        {
            if (ratio>1.0f)
            {
                float tmp=windowSize[0]/_orthoViewSize;
                xShift=(-mouseDownRelPos[0]+(windowSize[0]/2))/tmp;
                tmp=windowSize[1]/(_orthoViewSize/ratio);
                yShift=(+mouseDownRelPos[1]-(windowSize[1]/2))/tmp;
            }
            else
            {
                float tmp=windowSize[1]/_orthoViewSize;
                yShift=(mouseDownRelPos[1]-(windowSize[1]/2))/tmp;
                tmp=windowSize[0]/(_orthoViewSize*ratio);
                xShift=(-mouseDownRelPos[0]+(windowSize[0]/2))/tmp;
            }
        }
        float p[3];
        p[0]=m[0][3]+m[0][2]*subView->getMousePositionDepth()+m[0][1]*yShift+m[0][0]*xShift;
        p[1]=m[1][3]+m[1][2]*subView->getMousePositionDepth()+m[1][1]*yShift+m[1][0]*xShift;
        p[2]=m[2][3]+m[2][2]*subView->getMousePositionDepth()+m[2][1]*yShift+m[2][0]*xShift;
        subView->setCenterPosition(p);
    }
}

void CCamera::_drawOverlay(bool passiveView,bool drawText,bool displ_ref,int windowSize[2],CSView* subView)
{
    FUNCTION_DEBUG;
    int navigationMode=sim_navigation_passive;
    int selectionMode=NOSELECTION;
    if (subView!=nullptr)
    {
        navigationMode=App::getMouseMode()&0x00ff;
        selectionMode=subView->getSelectionStatus();
    }
    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,windowSize[0],0,windowSize[1],-100,100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity ();

    glDisable(GL_DEPTH_TEST);

    // Draw the selection square
    if ( (selectionMode==SHIFTSELECTION)&&(subView!=nullptr)&&subView->isMouseDown() )
    {
        int mouseDownRelPos[2];
        int mouseRelPos[2];
        subView->getMouseDownRelativePosition(mouseDownRelPos);
        subView->getMouseRelativePosition(mouseRelPos);
        int x1=mouseDownRelPos[0];
        int y1=mouseDownRelPos[1];
        int x2=mouseRelPos[0];
        int y2=mouseRelPos[1];
        tt::limitValue(0,windowSize[0],x1);
        tt::limitValue(0,windowSize[1],y1);
        tt::limitValue(0,windowSize[0],x2);
        tt::limitValue(0,windowSize[1],y2);

        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorYellow);
        ogl::setAlpha(0.2f);
        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
            glVertex3i(x1,y1,0);
            glVertex3i(x1,y2,0);
            glVertex3i(x2,y2,0);
            glVertex3i(x2,y1,0);
            glVertex3i(x1,y1,0);
        glEnd();
        ogl::setBlending(false);
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorWhite);
        glBegin(GL_LINE_STRIP);
            glVertex3i(x1,y1,0);
            glVertex3i(x1,y2,0);
            glVertex3i(x2,y2,0);
            glVertex3i(x2,y1,0);
            glVertex3i(x1,y1,0);
        glEnd();
        glDisable(GL_LINE_STIPPLE);
    }

    if (App::userSettings->displayWorldReference&&displ_ref)
    {
        glTranslatef(float(windowSize[0]-60.0f*App::sc),40.0f*App::sc,0.0f);
        C7Vector tr2(getCumulativeTransformation());
        tr2.inverse();
        C4X4Matrix m1;
        m1.buildYRotation(piValue_f);
        C7Vector tr0(m1.getTransformation()*tr2);
        float refSize=30.0f*App::sc;

        C3Vector euler(tr0.Q.getEulerAngles());
        glPushMatrix();
        glRotatef(euler(0)*radToDeg_f,1.0,0.0,0.0);
        glRotatef(euler(1)*radToDeg_f,0.0,1.0,0.0);
        glRotatef(euler(2)*radToDeg_f,0.0,0.0,1.0);

        glLineWidth(App::sc);
        ogl::drawReference(refSize,true,true,true,nullptr);
        glLineWidth(1.0f);

        glPopMatrix();
        ogl::setTextColor(ogl::colorWhite);
        glTranslatef(0.2f*refSize,0.2f*refSize,0.0f);
        C3X3Matrix m0(tr0.Q.getMatrix());
        ogl::drawBitmapTextIntoScene(m0(0,0)*refSize,m0(1,0)*refSize,0.0f,IDSOGL_X);
        ogl::drawBitmapTextIntoScene(m0(0,1)*refSize,m0(1,1)*refSize,0.0f,IDSOGL_Y);
        ogl::drawBitmapTextIntoScene(m0(0,2)*refSize,m0(1,2)*refSize,0.0f,IDSOGL_Z);
    }
    glEnable(GL_DEPTH_TEST);
}

int CCamera::getSingleHit(int hits,unsigned int selectBuff[],bool ignoreDepthBuffer,int& hitThatIgnoresTheSelectableFlag)
{ // this routine is called in non-edit mode, but also while in an edit mode!!
    FUNCTION_DEBUG;
    hitThatIgnoresTheSelectableFlag=-1;
    if (hits!=0)
    {
        unsigned int nearestObj=0;
        unsigned int distance=selectBuff[1];
        for (int i=0;i<hits;i++)
        {
            if (i==0)
            { // the first hit. We always take this one into account
                distance=selectBuff[4*i+1];
                nearestObj=selectBuff[4*i+3];
                hitThatIgnoresTheSelectableFlag=nearestObj;
                if ((nearestObj<=SIM_IDEND_3DOBJECT)&&(App::getEditModeType()==NO_EDIT_MODE))
                { // since 3/6/2013 we check for the selectable flag here instead of in the object display routine
                    C3DObject* theObj=App::ct->objCont->getObjectFromHandle(nearestObj);
                    if ((theObj!=nullptr)&&((theObj->getCumulativeObjectProperty()&sim_objectproperty_selectable)==0))
                        nearestObj=(unsigned int)-1;
                }
            }
            else
            {
                if (!ignoreDepthBuffer)
                {
                    bool shapeEditAndNoHitYet=((App::getEditModeType()&SHAPE_EDIT_MODE)&&(nearestObj==(unsigned int)-1));
                    if ((selectBuff[4*i+1]<=distance)||shapeEditAndNoHitYet)
                    { // this hit is closer
                        distance=selectBuff[4*i+1];
                        nearestObj=selectBuff[4*i+3];
                        hitThatIgnoresTheSelectableFlag=nearestObj;
                        if ((nearestObj<=SIM_IDEND_3DOBJECT)&&(App::getEditModeType()==NO_EDIT_MODE))
                        { // since 3/6/2013 we check for the selectable flag here instead of in the object display routine
                            C3DObject* theObj=App::ct->objCont->getObjectFromHandle(nearestObj);
                            if ((theObj!=nullptr)&&((theObj->getCumulativeObjectProperty()&sim_objectproperty_selectable)==0))
                                nearestObj=(unsigned int)-1;
                        }
                    }
                }
                else
                {
                    if (nearestObj==(unsigned int)-1)
                    { // we take this hit since we don't yet have a hit
                        distance=selectBuff[4*i+1];
                        nearestObj=selectBuff[4*i+3];
                        hitThatIgnoresTheSelectableFlag=nearestObj;
                        if ((nearestObj<=SIM_IDEND_3DOBJECT)&&(App::getEditModeType()==NO_EDIT_MODE))
                        { // since 3/6/2013 we check for the selectable flag here instead of in the object display routine
                            C3DObject* theObj=App::ct->objCont->getObjectFromHandle(nearestObj);
                            if ((theObj!=nullptr)&&((theObj->getCumulativeObjectProperty()&sim_objectproperty_selectable)==0))
                                nearestObj=(unsigned int)-1;
                        }
                    }
                }
            }
        }

        if (nearestObj==(unsigned int)-1)
            return(-1); // we didn't hit anything
        else
            return((int)nearestObj); // we hit this ID
    }
    else
        return(-1); // we didn't hit anything
}

int CCamera::getSeveralHits(int hits,unsigned int selectBuff[],std::vector<int>& hitList)
{ // this routine is called in non-edit mode, but also while in an edit mode!!
    FUNCTION_DEBUG;
    hitList.clear();
    if (hits!=0)
    {
        int cnt=0;
        for (int i=0;i<hits;i++)
        {
            unsigned int theHit=selectBuff[4*i+3];
            if (theHit!=((unsigned int)-1))
            {
                if ((theHit<=SIM_IDEND_3DOBJECT)&&(App::getEditModeType()==NO_EDIT_MODE))
                { // since 3/6/2013 we check for the selectable flag here instead of in the object display routine
                    C3DObject* theObj=App::ct->objCont->getObjectFromHandle(theHit);
                    if ((theObj!=nullptr)&&((theObj->getCumulativeObjectProperty()&sim_objectproperty_selectable)==0))
                        theHit=(unsigned int)-1;
                }
                if (int(theHit)!=-1)
                {
                    cnt++;
                    hitList.push_back((int)theHit);
                }
            }
        }
        return(cnt);
    }
    else
        return(0); // we didn't hit anything
}

void CCamera::handleMouseUpHit(int hitId)
{
    FUNCTION_DEBUG;
    if (hitId==-1)
    {
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
            App::ct->objCont->deselectObjects();
        else
            App::mainWindow->editModeContainer->deselectEditModeBuffer();
    }
    else
    {
        if ( (hitId<SIM_IDEND_3DOBJECT)||(hitId>=NON_OBJECT_PICKING_ID_PATH_PTS_START) ) // We need the NON_OBJECT_PICKING_ID_PATH_PTS_START start here to select individual path points when not in path edit mode!!!!!!!
        {
            if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
            {
                App::ct->objCont->deselectObjects();
                App::ct->objCont->addObjectToSelection(hitId);
            }
            else
            {
                App::mainWindow->editModeContainer->deselectEditModeBuffer();
                App::mainWindow->editModeContainer->addItemToEditModeBuffer(hitId,false);
            }
        }
        if ( (hitId>=NON_OBJECT_PICKING_ID_BANNER_START)&&(hitId<NON_OBJECT_PICKING_ID_BANNER_END) )
            _handleBannerClick(hitId-NON_OBJECT_PICKING_ID_BANNER_START);
    }
}

int CCamera::handleHits(int hits,unsigned int selectBuff[])
{ // -2 means: handle no mouse up hits. Otherwise, handle mouse up hits!
    FUNCTION_DEBUG;
    if (App::mainWindow==nullptr)
        return(-2);
    if (App::mainWindow->getMouseButtonState()&4) // added on 2011/01/12 because this routine is now also called when not in click-select mode, etc. We need to make sure we don't have a "virtual" left mouse button clicked triggered by the right mouse button
        return(-2);
    if (App::mainWindow->getKeyDownState()&2)
    {
        std::vector<int> hitList;
        if (getSeveralHits(hits,selectBuff,hitList)>0)
        {
            for (int i=0;i<int(hitList.size());i++)
            {
                if ( (hitList[i]<SIM_IDEND_3DOBJECT)||(hitList[i]>=NON_OBJECT_PICKING_ID_PATH_PTS_START) ) // We need the NON_OBJECT_PICKING_ID_PATH_PTS_START start here to select individual path points when not in path edit mode!!!!!!!
                {
                    if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
                        App::ct->objCont->addObjectToSelection(hitList[i]);
                    else
                        App::mainWindow->editModeContainer->addItemToEditModeBuffer(hitList[i],true);
                }
                if ( (hitList[i]>=NON_OBJECT_PICKING_ID_BANNER_START)&&(hitList[i]<NON_OBJECT_PICKING_ID_BANNER_END) )
                    _handleBannerClick(hitList[i]-NON_OBJECT_PICKING_ID_BANNER_START);
            }
        }
        return(-2);
    }
    else
    {
        if (App::mainWindow->getKeyDownState()&1)
        {
            int dummy;
            bool ignoreDepth=((App::getEditModeType()&VERTEX_EDIT_MODE)||(App::getEditModeType()&EDGE_EDIT_MODE))&&App::mainWindow->editModeContainer->getShapeEditMode()->getShowHiddenVerticeAndEdges();
            int hitId=getSingleHit(hits,selectBuff,ignoreDepth,dummy);
            if (hitId==-1)
            {
                if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
                    App::ct->objCont->deselectObjects();
                else
                    App::mainWindow->editModeContainer->deselectEditModeBuffer();
            }
            else
            {
                if ( (hitId<SIM_IDEND_3DOBJECT)||(hitId>=NON_OBJECT_PICKING_ID_PATH_PTS_START) ) // We need the NON_OBJECT_PICKING_ID_PATH_PTS_START start here to select individual path points when not in path edit mode!!!!!!!
                {
                    if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
                        App::ct->objCont->xorAddObjectToSelection(hitId);
                    else
                        App::mainWindow->editModeContainer->xorAddItemToEditModeBuffer(hitId,false);
                }
                if ( (hitId>=NON_OBJECT_PICKING_ID_BANNER_START)&&(hitId<NON_OBJECT_PICKING_ID_BANNER_END) )
                    _handleBannerClick(hitId-NON_OBJECT_PICKING_ID_BANNER_START);
            }
            return(-2);
        }
        else
        { // no ctrl or shift key down here. We simply return the hit for processing at a later stage
            int hitThatIgnoresTheSelectableFlag;
            bool ignoreDepth=((App::getEditModeType()&VERTEX_EDIT_MODE)||(App::getEditModeType()&EDGE_EDIT_MODE))&&App::mainWindow->editModeContainer->getShapeEditMode()->getShowHiddenVerticeAndEdges();
            int hitId=getSingleHit(hits,selectBuff,ignoreDepth,hitThatIgnoresTheSelectableFlag);

            // Now generate a plugin callback:
            int data[4]={hitThatIgnoresTheSelectableFlag,0,0,0};
            void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_pickselectdown,data,nullptr,nullptr);
            delete[] (char*)retVal;
            // Now generate a script message:
            App::ct->outsideCommandQueue->addCommand(sim_message_pick_select_down,hitThatIgnoresTheSelectableFlag,0,0,0,nullptr,0);

            return(hitId);
        }
    }
    return(-2);
}



void CCamera::_handleBannerClick(int bannerID)
{
    FUNCTION_DEBUG;
    if (App::getEditModeType()!=NO_EDIT_MODE)
        return;
    CBannerObject* it=App::ct->bannerCont->getObject(bannerID);
    if ( (it!=nullptr)&&it->isVisible() )
    {
        if (it->getOptions()&sim_banner_clickselectsparent)
        {
            if (it->getParentObjectHandle()>=0)
                App::ct->objCont->addObjectToSelection(it->getParentObjectHandle());
        }
        if (it->getOptions()&sim_banner_clicktriggersevent)
        {
            int auxVals[4]={bannerID,0,0,0};
            void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_bannerclicked,auxVals,nullptr,nullptr);
            delete[] (char*)returnVal;
            auxVals[0]=bannerID;
            returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_bannerclicked,auxVals,nullptr,nullptr); // for backward compatibility
            delete[] (char*)returnVal;
            App::ct->outsideCommandQueue->addCommand(sim_message_bannerclicked,bannerID,0,0,0,nullptr,0);
        }
    }
}

void CCamera::setAttributesForRendering(int attr)
{
    _attributesForRendering=attr;
}

int CCamera::getAttributesForRendering() const
{
    return(_attributesForRendering);
}

void CCamera::setRenderMode(int mode,bool duringSimulation,bool duringRecording)
{
    _renderMode=mode;
    _renderModeDuringSimulation=duringSimulation;
    _renderModeDuringRecording=duringRecording;
}

int CCamera::getRenderMode(bool* duringSimulation,bool* duringRecording) const
{
    if (duringSimulation!=nullptr)
        duringSimulation[0]=_renderModeDuringSimulation;
    if (duringRecording!=nullptr)
        duringRecording[0]=_renderModeDuringRecording;
    return(_renderMode);
}

bool CCamera::getInternalRendering() const
{
    if (_renderMode!=sim_rendermode_opengl)
    {
        if (App::getEditModeType()==NO_EDIT_MODE)
        {
            if ( (_renderMode==sim_rendermode_povray)||(_renderMode==sim_rendermode_extrenderer)||(_renderMode==sim_rendermode_opengl3) )
            {
                if ( App::ct->simulation->isSimulationRunning()||(!_renderModeDuringSimulation) )
                    return(App::mainWindow->simulationRecorder->getIsRecording()!=_renderModeDuringRecording);
            }
        }
    }
    return(true);
}
#endif
