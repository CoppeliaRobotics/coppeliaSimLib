
#include "vrepMainHeader.h" 
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "path.h"
#include "tt.h"
#include "meshManip.h"
#include "gV.h"
#include "meshRoutines.h"
#include "v_repStrings.h"
#include "ttUtil.h"
#include "easyLock.h"
#include "app.h"
#include "pathRendering.h"

CPath::CPath()
{
    setObjectType(sim_object_path_type);
    pathContainer=nullptr;
    layer=PATH_LAYER;
    _explicitHandling=false;
    _shapingEnabled=false;
    _initialValuesInitialized=false;
    _shapingScaling=1.0f;
    _shapingFollowFullOrientation=false;
    _shapingConvexHull=false;
    _shapingSectionClosed=true;
    _shapingElementMaxLength=0.005f;
    shapingColor.setDefaultValues();
    shapingColor.setColor(0.1f,0.9f,0.1f,sim_colorcomponent_ambient_diffuse);
    _localObjectSpecialProperty=sim_objectspecialproperty_renderable;
    _pathModifID=0;
    pathContainer=new CPathCont();
    setShapingType(1);
    _objectName=IDSOGL_PATH;
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName);
}

CPath::~CPath()
{
    delete pathContainer;
}

bool CPath::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    int c=pathContainer->getSimplePathPointCount();
    if (c==0)
        return(false);
    for (int i=0;i<c;i++)
    {
        C3Vector v(pathContainer->getSimplePathPoint(i)->getTransformation().X);
        if (i==0)
        {
            minV=v;
            maxV=v;
        }
        else
        {
            minV.keepMin(v);
            maxV.keepMax(v);
        }
    }
    float ssd2=pathContainer->getSquareSize()/2.0f;
    minV(0)-=ssd2;
    minV(1)-=ssd2;
    minV(2)-=ssd2;
    maxV(0)+=ssd2;
    maxV(1)+=ssd2;
    maxV(2)+=ssd2;
    return(true);
}

bool CPath::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    float ssd2=pathContainer->getSquareSize()/2.0f;
    minV(0)=-ssd2;
    minV(1)=-ssd2;
    minV(2)=-ssd2;
    maxV(0)=ssd2;
    maxV(1)=ssd2;
    maxV(2)=ssd2;
    return(true);
}

bool CPath::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    return(false); // for now
}

void CPath::scaleObject(float scalingFactor)
{
    if (pathContainer!=nullptr)
        pathContainer->scaleObject(scalingFactor);
    _shapingScaling*=scalingFactor;
    setShapingElementMaxLength(getShapingElementMaxLength()*scalingFactor);
    scaleObjectMain(scalingFactor);
}

void CPath::scaleObjectNonIsometrically(float x,float y,float z)
{
    if (pathContainer!=nullptr)
        pathContainer->scaleObjectNonIsometrically(x,y,z);
    _shapingScaling*=cbrt(x*y*z);
    setShapingElementMaxLength(getShapingElementMaxLength()*cbrt(x*y*z));
    scaleObjectNonIsometricallyMain(x,y,z);
}


void CPath::setExplicitHandling(bool explicitHandl)
{
    _explicitHandling=explicitHandl;
}

bool CPath::getExplicitHandling()
{
    return(_explicitHandling);
}

void CPath::setShapingEnabled(bool isEnabled)
{
    _shapingEnabled=isEnabled;
}

bool CPath::getShapingEnabled()
{
    return(_shapingEnabled);
}

void CPath::setShapingScaling(float s)
{
    tt::limitValue(0.01f,100.0f,s);
    _shapingScaling=s;
    _generatePathShape();
}

float CPath::getShapingScaling() const
{
    return(_shapingScaling);
}

CVisualParam* CPath::getShapingColor()
{
    return(&shapingColor);
}

void CPath::setShapingFollowFullOrientation(bool full)
{
    _shapingFollowFullOrientation=full;
    _generatePathShape();
}

bool CPath::getShapingFollowFullOrientation()
{
    return(_shapingFollowFullOrientation);
}

void CPath::setShapingSectionClosed(bool closed)
{
    _shapingSectionClosed=closed;
    _generatePathShape();
}

bool CPath::getShapingSectionClosed()
{
    return(_shapingSectionClosed);
}

void CPath::setShapingElementMaxLength(float l)
{
    tt::limitValue(0.00005f,1.0f,l);
    _shapingElementMaxLength=l;
    _generatePathShape();
}

float CPath::getShapingElementMaxLength()
{
    return(_shapingElementMaxLength);
}

void CPath::setShapingType(int theType)
{ // 0=user, 1=circle, 2=square, 3=line horiz, 4=line vert
    const float s=pathContainer->getSquareSize()/2.0f;
    if (theType>0)
    {
        shapingCoordinates.clear();
        if (theType==4)
        {
            shapingCoordinates.push_back(+s);
            shapingCoordinates.push_back(0.0f);
            shapingCoordinates.push_back(-s);
            shapingCoordinates.push_back(0.0f);
            _shapingSectionClosed=false;
        }
        if (theType==3)
        {
            shapingCoordinates.push_back(0.0f);
            shapingCoordinates.push_back(+s);
            shapingCoordinates.push_back(0.0f);
            shapingCoordinates.push_back(-s);
            _shapingSectionClosed=false;
        }
        if (theType==2)
        {
            shapingCoordinates.push_back(-s);
            shapingCoordinates.push_back(-s);
            shapingCoordinates.push_back(+s);
            shapingCoordinates.push_back(-s);
            shapingCoordinates.push_back(+s);
            shapingCoordinates.push_back(+s);
            shapingCoordinates.push_back(-s);
            shapingCoordinates.push_back(+s);
            _shapingSectionClosed=true;
        }
        if (theType==1)
        {
            float da=piValTimes2_f/16.0f;
            float a=0.0f;
            for (int i=0;i<16;i++)
            {
                shapingCoordinates.push_back(s*cos(a));
                shapingCoordinates.push_back(s*sin(a));
                a+=da;
            }
            _shapingSectionClosed=true;
        }
    }
    _shapingType=theType;
    
    _generatePathShape();
}

int CPath::getShapingType()
{
    return(_shapingType);
}

void CPath::setShapingThroughConvexHull(bool c)
{
    _shapingConvexHull=c;
    _generatePathShape();
}

bool CPath::getShapingThroughConvexHull()
{
    return(_shapingConvexHull);
}

void CPath::resetPath()
{
    if (pathContainer!=nullptr)
        pathContainer->resetPath(this);
}

void CPath::handlePath(float deltaTime)
{
    if (pathContainer!=nullptr)
        pathContainer->handlePath(this,deltaTime);
}

void CPath::removeSceneDependencies()
{
    removeSceneDependenciesMain();
}

C3DObject* CPath::copyYourself()
{   
    CPath* newPath=(CPath*)copyYourselfMain();

    newPath->setExplicitHandling(_explicitHandling);
    newPath->_shapingEnabled=_shapingEnabled;

    newPath->_shapingFollowFullOrientation=_shapingFollowFullOrientation;
    newPath->_shapingConvexHull=_shapingConvexHull;
    newPath->_shapingSectionClosed=_shapingSectionClosed;
    newPath->_shapingElementMaxLength=_shapingElementMaxLength;
    newPath->_shapingType=_shapingType;
    newPath->shapingCoordinates.clear();
    for (int i=0;i<int(shapingCoordinates.size());i++)
        newPath->shapingCoordinates.push_back(shapingCoordinates[i]);
    shapingColor.copyYourselfInto(&newPath->shapingColor);
    newPath->_shapingScaling=_shapingScaling;

    delete newPath->pathContainer;
    newPath->pathContainer=pathContainer->copyYourself();

    newPath->_initialValuesInitialized=_initialValuesInitialized;
    newPath->_initialExplicitHandling=_initialExplicitHandling;
    newPath->_generatePathShape();

    return(newPath);
}

bool CPath::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'CObjCont::getObject(objectHandle)'-call or similar
    // Return value true means 'this' has to be erased too!
    bool retVal=announceObjectWillBeErasedMain(objectHandle,copyBuffer);
    return(retVal);
}

void CPath::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollectionWillBeErasedMain(groupID,copyBuffer);
}
void CPath::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollisionWillBeErasedMain(collisionID,copyBuffer);
}
void CPath::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceDistanceWillBeErasedMain(distanceID,copyBuffer);
}
void CPath::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceIkObjectWillBeErasedMain(ikGroupID,copyBuffer);
}
void CPath::announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceGcsObjectWillBeErasedMain(gcsObjectID,copyBuffer);
}
void CPath::performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    performObjectLoadingMappingMain(map,loadingAmodel);
}
void CPath::performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollectionLoadingMappingMain(map,loadingAmodel);
}
void CPath::performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollisionLoadingMappingMain(map,loadingAmodel);
}
void CPath::performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performDistanceLoadingMappingMain(map,loadingAmodel);
}
void CPath::performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
    performIkLoadingMappingMain(map,loadingAmodel);
}
void CPath::performGcsLoadingMapping(std::vector<int>* map)
{
    performGcsLoadingMappingMain(map);
}

void CPath::performTextureObjectLoadingMapping(std::vector<int>* map)
{
    performTextureObjectLoadingMappingMain(map);
}

void CPath::performDynMaterialObjectLoadingMapping(std::vector<int>* map)
{
    performDynMaterialObjectLoadingMappingMain(map);
}

void CPath::bufferMainDisplayStateVariables()
{
    bufferMainDisplayStateVariablesMain();
}

void CPath::bufferedMainDisplayStateVariablesToDisplay()
{
    bufferedMainDisplayStateVariablesToDisplayMain();
}

std::string CPath::getObjectTypeInfo() const
{
    return(IDSOGL_PATH);
}

std::string CPath::getObjectTypeInfoExtended() const
{
    std::string retVal;
    retVal=IDSOGL_PATH_BEZIER_CURVE_POINT_COUNT;
    retVal+=tt::getIString(false,pathContainer->getBezierPathPointCount())+", ";
    retVal+=IDSOGL_TOTAL_LENGTH;
    retVal+=tt::getFString(false,pathContainer->getBezierVirtualPathLength(),3)+", p=";
    retVal+=tt::getFString(true,float(pathContainer->getPosition()),3)+")";
    return(retVal);
}
bool CPath::isPotentiallyCollidable() const
{
    return(false);
}
bool CPath::isPotentiallyMeasurable() const
{
    return(false);
}
bool CPath::isPotentiallyDetectable() const
{
    return(false);
}
bool CPath::isPotentiallyRenderable() const
{
    return(true);
}
bool CPath::isPotentiallyCuttable() const
{
    return(false);
}

void CPath::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    initializeInitialValuesMain(simulationIsRunning);
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        _initialExplicitHandling=_explicitHandling;
    }
    if (pathContainer!=nullptr)
        pathContainer->initializeInitialValues(simulationIsRunning);
}

void CPath::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CPath::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
    {
        _explicitHandling=_initialExplicitHandling;
    }
    _initialValuesInitialized=false;
    if (pathContainer!=nullptr)
        pathContainer->simulationEnded();
    simulationEndedMain();
}

void CPath::_generatePathShape()
{
    _pathShapeVertices.clear();
    _pathShapeIndices.clear();
    _pathShapeNormals.clear();
    if (!_shapingEnabled)
        return;
    int pts=(int)shapingCoordinates.size()/2;
    float l=pathContainer->getBezierNormalPathLength();
    if (l==0.0f)
        return;
    int elements=int(l/_shapingElementMaxLength)+1;
    float dl=l/float(elements);
    float pos=0.0f;
    C3Vector upVect;

    for (int i=0;i<elements+1;i++)
    {
        if ( (i==elements)&&(pathContainer->getAttributes()&sim_pathproperty_closed_path) )
            break;
        int index;
        float t;
        pathContainer->getPointOnBezierCurveAtNormalDistance(pos,index,t);
        C4X4Matrix m(pathContainer->_getInterpolatedBezierCurvePoint(index,t));
        if (i==0)
            upVect=m.M.axis[0];
        else
        {
            if (!_shapingFollowFullOrientation)
            {
                if (m.M.axis[2].getAngle(upVect)<1.0f*degToRad_f)
                    upVect=m.M.axis[0]; // This becomes the new up vector
                else
                {
                    m.M.axis[1]=(m.M.axis[2]^upVect).getNormalized();
                    m.M.axis[0]=(m.M.axis[1]^m.M.axis[2]).getNormalized();;
                }
            }
        }
        for (int j=0;j<int(shapingCoordinates.size())/2;j++)
        {
            C3Vector v(shapingCoordinates[2*j+0]*_shapingScaling,shapingCoordinates[2*j+1]*_shapingScaling,0.0f);
            v=m*v;
            _pathShapeVertices.push_back(v(0));
            _pathShapeVertices.push_back(v(1));
            _pathShapeVertices.push_back(v(2));
        }
        pos+=dl;
    }

    bool convexSuccess=false;
    if (_shapingConvexHull)
    { // Using the element-wise convex hull for generation
        std::vector<float> cop(_pathShapeVertices);
        for (int i=0;i<elements;i++)
        {
            std::vector<float> tmpVert;
            for (int k=0;k<2;k++)
            {
                for (int j=0;j<pts;j++)
                {
                    if ( (i==elements-1)&&(k==1)&&(pathContainer->getAttributes()&sim_pathproperty_closed_path) )
                    { // This is the closing part (closed path)
                        tmpVert.push_back(cop[3*((0)*pts+j)+0]);
                        tmpVert.push_back(cop[3*((0)*pts+j)+1]);
                        tmpVert.push_back(cop[3*((0)*pts+j)+2]);
                    }
                    else
                    {
                        tmpVert.push_back(cop[3*((i+k)*pts+j)+0]);
                        tmpVert.push_back(cop[3*((i+k)*pts+j)+1]);
                        tmpVert.push_back(cop[3*((i+k)*pts+j)+2]);
                    }
                }
            }
            std::vector<float> vertOut;
            std::vector<int> indOut;
            convexSuccess=CMeshRoutines::getConvexHull(&tmpVert,&vertOut,&indOut);
            if (!convexSuccess)
                break;
            if (i==0)
            {
                _pathShapeVertices.clear();
                _pathShapeIndices.clear();
            }

            // We remove the degenerate triangles:
            CMeshManip::checkVerticesIndicesNormalsTexCoords(vertOut,indOut,nullptr,nullptr,true,0.00001f,true);

            // we merge the individual elements
            CMeshManip::mergeWith(&_pathShapeVertices,&_pathShapeIndices,nullptr,&vertOut,&indOut,nullptr);
        }

        if (convexSuccess)
        { // We make a final check of the resulting shape:
            CMeshManip::checkVerticesIndicesNormalsTexCoords(_pathShapeVertices,_pathShapeIndices,nullptr,nullptr,true,0.00001f,true);
        }
    }

    if (!convexSuccess)
    { // Not using the element-wise convex hull
        for (int i=0;i<elements;i++)
        {
            for (int j=0;j<pts;j++)
            {
                int ind[4]={i*pts+j+0,i*pts+j+1,(i+1)*pts+j+1,(i+1)*pts+j+0};
                if ( (i==elements-1)&&(pathContainer->getAttributes()&sim_pathproperty_closed_path) )
                {
                    ind[2]=j+1;
                    ind[3]=j+0;
                }
                if (j==pts-1)
                {
                    if (!_shapingSectionClosed)
                        break;
                    ind[0]=i*pts+j;
                    ind[1]=i*pts+0;
                    ind[2]=(i+1)*pts+0;
                    ind[3]=(i+1)*pts+j;
                    if ( (i==elements-1)&&(pathContainer->getAttributes()&sim_pathproperty_closed_path) )
                    {
                        ind[2]=0;
                        ind[3]=j;
                    }
                }
                _pathShapeIndices.push_back(ind[0]);
                _pathShapeIndices.push_back(ind[1]);
                _pathShapeIndices.push_back(ind[3]);

                _pathShapeIndices.push_back(ind[1]);
                _pathShapeIndices.push_back(ind[2]);
                _pathShapeIndices.push_back(ind[3]);
            }
        }
    }

    CMeshManip::getNormals(&_pathShapeVertices,&_pathShapeIndices,&_pathShapeNormals);
}

bool CPath::getShape(CGeomProxy* geomObj[1],CShape* shapeObj[1])
{
    bool retVal=false;
    if (_shapingEnabled&&(_pathShapeVertices.size()!=0))
    {
        std::vector<float> vert(_pathShapeVertices);
        C7Vector tr(getCumulativeTransformation());
        for (int i=0;i<int(vert.size()/3);i++)
        {
            C3Vector v(&vert[3*i]);
            v=tr*v;
            vert[3*i+0]=v(0);
            vert[3*i+1]=v(1);
            vert[3*i+2]=v(2);
        }
        std::vector<int> ind(_pathShapeIndices);
        geomObj[0]=new CGeomProxy(nullptr,vert,ind,nullptr,nullptr);
        shapeObj[0]=new CShape();
        shapeObj[0]->setLocalTransformation(geomObj[0]->getCreationTransformation());
        geomObj[0]->setCreationTransformation(C7Vector::identityTransformation);
        retVal=true;
    }
    return(retVal);
}

void CPath::serialize(CSer& ar)
{
    serializeMain(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Var");
            unsigned char dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,_shapingEnabled);
            SIM_SET_CLEAR_BIT(dummy,1,_shapingFollowFullOrientation);
            SIM_SET_CLEAR_BIT(dummy,2,_shapingSectionClosed);
            SIM_SET_CLEAR_BIT(dummy,3,_shapingConvexHull);
    //FREE
            SIM_SET_CLEAR_BIT(dummy,5,_explicitHandling);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Pss");
            ar << _shapingScaling;
            ar.flush();

            ar.storeDataName("Sha");
            ar << _shapingElementMaxLength << _shapingType;
            ar.flush();

            ar.storeDataName("Ntp");
            ar.setCountingMode();
            pathContainer->serialize(ar);
            if (ar.setWritingMode())
                pathContainer->serialize(ar);

            ar.storeDataName("Sec");
            ar << int(shapingCoordinates.size());
            for (int i=0;i<int(shapingCoordinates.size());i++)
                ar << shapingCoordinates[i];
            ar.flush();

            ar.storeDataName("Scl");
            ar.setCountingMode();
            shapingColor.serialize(ar,0);
            if (ar.setWritingMode())
                shapingColor.serialize(ar,0);

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;

                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _shapingEnabled=SIM_IS_BIT_SET(dummy,0);
                        _shapingFollowFullOrientation=SIM_IS_BIT_SET(dummy,1);
                        _shapingSectionClosed=SIM_IS_BIT_SET(dummy,2);
                        _shapingConvexHull=SIM_IS_BIT_SET(dummy,3);
    //FREE
                        _explicitHandling=SIM_IS_BIT_SET(dummy,5);
                    }

                    if (theName.compare("Pss")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _shapingScaling;
                    }

                    if (theName.compare("Sha")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _shapingElementMaxLength >> _shapingType;
                    }
                    if (theName.compare("Ntp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        pathContainer->serialize(ar);
                    }
                    if (theName.compare("Sec")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int l;
                        ar >> l;
                        float c;
                        shapingCoordinates.clear();
                        for (int i=0;i<l;i++)
                        {
                            ar >> c;
                            shapingCoordinates.push_back(c);
                        }
                    }
                    if (theName.compare("Scl")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        shapingColor.serialize(ar,0);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }

            if (ar.getSerializationVersionThatWroteThisFile()<17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                CTTUtil::scaleColorUp_(shapingColor.colors);
            }
            _generatePathShape();
        }
    }
}

void CPath::serializeWExtIk(CExtIkSer& ar)
{
    serializeWExtIkMain(ar);
    CDummy::serializeWExtIkStatic(ar);
}

void CPath::display(CViewableBase* renderingObject,int displayAttrib)
{
    FUNCTION_INSIDE_DEBUG("CPath::display");
    EASYLOCK(_objectMutex);
    displayPath(this,renderingObject,displayAttrib);
}

#ifdef SIM_WITH_GUI
bool CPath::transformSelectedPathPoints(const C4X4Matrix& cameraAbsConf,const C3Vector& clicked3DPoint,float prevPos[2],float pos[2],float screenHalfSizes[2],float halfSizes[2],bool perspective,int eventID)
{
    C3Vector pointCenter;
    pointCenter.clear();
    CPathCont* pc;
    std::vector<int> selectedPathPoints;
    if (App::getEditModeType()&PATH_EDIT_MODE)
    {
        pc=App::mainWindow->editModeContainer->getEditModePathContainer();
        selectedPathPoints.assign(App::mainWindow->editModeContainer->getEditModeBuffer()->begin(),App::mainWindow->editModeContainer->getEditModeBuffer()->end());
    }
    else
    {
        pc=pathContainer;
        selectedPathPoints.assign(App::mainWindow->editModeContainer->pathPointManipulation->getPointerToSelectedPathPointIndices_nonEditMode()->begin(),App::mainWindow->editModeContainer->pathPointManipulation->getPointerToSelectedPathPointIndices_nonEditMode()->end());
    }
    for (int i=0;i<int(selectedPathPoints.size());i++)
    {
        CSimplePathPoint* aPt=pc->getSimplePathPoint(selectedPathPoints[i]);
        if (aPt!=nullptr)
        {
            pointCenter+=(getCumulativeTransformationPart1()*aPt->getTransformation()).X;
        }
    }
    if (selectedPathPoints.size()!=0)
        pointCenter/=float(selectedPathPoints.size());

    C4X4Matrix objAbs;
    objAbs.X=pointCenter;
        objAbs.M=getCumulativeTransformationPart1().getMatrix().M;
    bool ctrlKeyDown=((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&1));
    if (eventID!=_objectManipulationModeEventId)
        _objectManipulationModeRelativePositionOfClickedPoint=clicked3DPoint-objAbs.X;
    if ( (eventID!=_objectManipulationModeEventId)||(ctrlKeyDown!=_objectManipulationModePermissionsPreviousCtrlKeyDown) )
    {
        _objectManipulationModeSubTranslation.clear();
        _objectManipulationModeSubRotation=0.0f;
        _objectManipulationModeEventId=eventID;
        _objectManipulationModeTotalTranslation.clear();
        _objectManipulationModeTotalRotation=0.0f;
        // Let's first see on which plane we wanna translate:
        bool specialMode=false;
        if (ctrlKeyDown)
            specialMode=true;
        _objectManipulationModeAxisIndex=2; // x/y plane
        if (specialMode&&((pc->getAttributes()&sim_pathproperty_flat_path)==0))
            _objectManipulationModeAxisIndex+=3;
    }

    _objectManipulationModePermissionsPreviousCtrlKeyDown=ctrlKeyDown;

    C4X4Matrix originalPlane(objAbs); // x-y plane
    originalPlane.X+=_objectManipulationModeRelativePositionOfClickedPoint;
    bool projectOntoXAxis=false;
    if (_objectManipulationModeAxisIndex==5)
    { // z axis
        projectOntoXAxis=true;
        C3X3Matrix rot;
        rot.buildYRotation(piValD2_f);
        originalPlane.M*=rot;
    }

    C4X4Matrix plane(originalPlane);
    C3Vector p[2]; // previous and current point on the plane
    float d=-(plane.X*plane.M.axis[2]);
    float screenP[2]={prevPos[0],prevPos[1]};
    C4X4Matrix cam(cameraAbsConf);
    bool singularityProblem=false;

    for (int pass=0;pass<2;pass++)
    {
        float tt[2];
        for (int i=0;i<2;i++)
        {
            if (i==1)
            {
                screenP[0]=pos[0];
                screenP[1]=pos[1];
            }
            C3Vector pp(cam.X);
            if (!perspective)
            {
                if (fabs(cam.M.axis[2]*plane.M.axis[2])<0.05f)
                {
                    singularityProblem=true;
                    break;
                }
                pp-=cam.M.axis[0]*halfSizes[0]*(screenP[0]/screenHalfSizes[0]);
                pp+=cam.M.axis[1]*halfSizes[1]*(screenP[1]/screenHalfSizes[1]);
                float t=(-d-(plane.M.axis[2]*pp))/(cam.M.axis[2]*plane.M.axis[2]);
                p[i]=pp+cam.M.axis[2]*t;
            }
            else
            {
                C3Vector v(cam.M.axis[2]+cam.M.axis[0]*tan(-screenP[0])+cam.M.axis[1]*tan(screenP[1]));
                v.normalize();
                pp+=v;
                if (fabs(v*plane.M.axis[2])<0.05f)
                {
                    singularityProblem=true;
                    break;
                }
                float t=(-d-(plane.M.axis[2]*pp))/(v*plane.M.axis[2]);
                tt[i]=t;
                p[i]=pp+v*t;
            }
        }
        if (!singularityProblem)
        {
            if ((!perspective)||(tt[0]*tt[1]>0.0f))
                break;
            singularityProblem=true;
        }
        plane.M=cam.M;
    }
    if (projectOntoXAxis)
    {
        C4X4Matrix inv(originalPlane.getInverse());
        p[0]*=inv;
        p[1]*=inv;
        p[0](1)=0.0f;
        p[0](2)=0.0f;
        p[1](1)=0.0f;
        p[1](2)=0.0f;
        p[0]*=originalPlane;
        p[1]*=originalPlane;
    }
    else
    {
        if (singularityProblem)
        { // we have to project the coordinates onto the original plane:
            C4X4Matrix inv(originalPlane.getInverse());
            p[0]*=inv;
            p[1]*=inv;
            p[0](2)=0.0f;
            p[1](2)=0.0f;
            p[0]*=originalPlane;
            p[1]*=originalPlane;
        }
    }

    // We snap the translation!
    C3Vector v(p[1]-p[0]);
    v=objAbs.getInverse().M*v;
    _objectManipulationModeSubTranslation+=v;
    for (int i=0;i<3;i++)
    {
        float ss=getNonDefaultTranslationStepSize();
        if (ss==0.0f)
            ss=App::userSettings->getTranslationStepSize();
        if ((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&2))
            ss=0.001f;
        float w=fmod(_objectManipulationModeSubTranslation(i),ss);
        v(i)=_objectManipulationModeSubTranslation(i)-w;
        _objectManipulationModeTotalTranslation(i)+=_objectManipulationModeSubTranslation(i)-w;
        _objectManipulationModeSubTranslation(i)=w;
    }
    v=objAbs.M*v;
    for (int i=0;i<int(selectedPathPoints.size());i++)
    {
        CSimplePathPoint* aPt=pc->getSimplePathPoint(selectedPathPoints[i]);
        if (aPt!=nullptr)
        {
            C4X4Matrix m(getCumulativeTransformationPart1()*aPt->getTransformation());
            m.X+=v;
            aPt->setTransformation(getCumulativeTransformationPart1().getInverse().getMatrix()*m,pc->getAttributes());
        }
    }
    _objectManipulationMode_flaggedForGridOverlay=_objectManipulationModeAxisIndex+16;
    pc->actualizePath();
    return(true);
}
#endif
