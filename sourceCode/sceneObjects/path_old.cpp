#include "simInternal.h"
#include "path_old.h"
#include "tt.h"
#include "meshManip.h"
#include "gV.h"
#include "meshRoutines.h"
#include "simStrings.h"
#include "ttUtil.h"
#include "easyLock.h"
#include "app.h"
#include "pathRendering.h"

CPath_old::CPath_old()
{
    setObjectType(sim_object_path_type);
    pathContainer=nullptr;
    _visibilityLayer=PATH_LAYER;
    _explicitHandling=false;
    _shapingEnabled=false;
    _shapingScaling=1.0f;
    _shapingFollowFullOrientation=false;
    _shapingConvexHull=false;
    _shapingSectionClosed=true;
    _shapingElementMaxLength=0.005f;
    shapingColor.setDefaultValues();
    shapingColor.setColor(0.1f,0.9f,0.1f,sim_colorcomponent_ambient_diffuse);
    _localObjectSpecialProperty=sim_objectspecialproperty_renderable;
    _pathModifID=0;
    pathContainer=new CPathCont_old();
    setShapingType(1);
    _objectName=IDSOGL_PATH;
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName.c_str());
}

CPath_old::~CPath_old()
{
    delete pathContainer;
}

bool CPath_old::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
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

bool CPath_old::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
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

bool CPath_old::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    return(false); // for now
}

void CPath_old::scaleObject(float scalingFactor)
{
    if (pathContainer!=nullptr)
        pathContainer->scaleObject(scalingFactor);
    _shapingScaling*=scalingFactor;
    setShapingElementMaxLength(getShapingElementMaxLength()*scalingFactor);
    CSceneObject::scaleObject(scalingFactor);
}

void CPath_old::scaleObjectNonIsometrically(float x,float y,float z)
{
    if (pathContainer!=nullptr)
        pathContainer->scaleObjectNonIsometrically(x,y,z);
    _shapingScaling*=cbrt(x*y*z);
    setShapingElementMaxLength(getShapingElementMaxLength()*cbrt(x*y*z));
    CSceneObject::scaleObjectNonIsometrically(x,y,z);
}


void CPath_old::setExplicitHandling(bool explicitHandl)
{
    _explicitHandling=explicitHandl;
}

bool CPath_old::getExplicitHandling()
{
    return(_explicitHandling);
}

void CPath_old::setShapingEnabled(bool isEnabled)
{
    _shapingEnabled=isEnabled;
}

bool CPath_old::getShapingEnabled()
{
    return(_shapingEnabled);
}

void CPath_old::setShapingScaling(float s)
{
    tt::limitValue(0.01f,100.0f,s);
    _shapingScaling=s;
    _generatePathShape();
}

float CPath_old::getShapingScaling() const
{
    return(_shapingScaling);
}

CColorObject* CPath_old::getShapingColor()
{
    return(&shapingColor);
}

void CPath_old::setShapingFollowFullOrientation(bool full)
{
    _shapingFollowFullOrientation=full;
    _generatePathShape();
}

bool CPath_old::getShapingFollowFullOrientation()
{
    return(_shapingFollowFullOrientation);
}

void CPath_old::setShapingSectionClosed(bool closed)
{
    _shapingSectionClosed=closed;
    _generatePathShape();
}

bool CPath_old::getShapingSectionClosed()
{
    return(_shapingSectionClosed);
}

void CPath_old::setShapingElementMaxLength(float l)
{
    tt::limitValue(0.00005f,1.0f,l);
    _shapingElementMaxLength=l;
    _generatePathShape();
}

float CPath_old::getShapingElementMaxLength()
{
    return(_shapingElementMaxLength);
}

void CPath_old::setShapingType(int theType)
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

int CPath_old::getShapingType()
{
    return(_shapingType);
}

void CPath_old::setShapingThroughConvexHull(bool c)
{
    _shapingConvexHull=c;
    _generatePathShape();
}

bool CPath_old::getShapingThroughConvexHull()
{
    return(_shapingConvexHull);
}

void CPath_old::resetPath()
{
    if (pathContainer!=nullptr)
        pathContainer->resetPath(this);
}

void CPath_old::handlePath(float deltaTime)
{
    if (pathContainer!=nullptr)
        pathContainer->handlePath(this,deltaTime);
}

void CPath_old::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

CSceneObject* CPath_old::copyYourself()
{   
    CPath_old* newPath=(CPath_old*)CSceneObject::copyYourself();

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

bool CPath_old::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'CSceneObjectContainer::getObject(objectHandle)'-call or similar
    // Return value true means 'this' has to be erased too!
    bool retVal=CSceneObject::announceObjectWillBeErased(objectHandle,copyBuffer);
    return(retVal);
}

void CPath_old::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID,copyBuffer);
}
void CPath_old::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID,copyBuffer);
}
void CPath_old::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID,copyBuffer);
}
void CPath_old::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID,copyBuffer);
}

void CPath_old::performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    CSceneObject::performObjectLoadingMapping(map,loadingAmodel);
}
void CPath_old::performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performCollectionLoadingMapping(map,loadingAmodel);
}
void CPath_old::performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performCollisionLoadingMapping(map,loadingAmodel);
}
void CPath_old::performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performDistanceLoadingMapping(map,loadingAmodel);
}
void CPath_old::performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map,loadingAmodel);
}

void CPath_old::performTextureObjectLoadingMapping(const std::vector<int>* map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
}

void CPath_old::performDynMaterialObjectLoadingMapping(const std::vector<int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

std::string CPath_old::getObjectTypeInfo() const
{
    return(IDSOGL_PATH);
}

std::string CPath_old::getObjectTypeInfoExtended() const
{
    std::string retVal;
    retVal=IDSOGL_PATH_BEZIER_CURVE_POINT_COUNT;
    retVal+=tt::getIString(false,pathContainer->getBezierPathPointCount())+", ";
    retVal+=IDSOGL_TOTAL_LENGTH;
    retVal+=tt::getFString(false,pathContainer->getBezierVirtualPathLength(),3)+", p=";
    retVal+=tt::getFString(true,float(pathContainer->getPosition()),3)+")";
    return(retVal);
}
bool CPath_old::isPotentiallyCollidable() const
{
    return(false);
}
bool CPath_old::isPotentiallyMeasurable() const
{
    return(false);
}
bool CPath_old::isPotentiallyDetectable() const
{
    return(false);
}
bool CPath_old::isPotentiallyRenderable() const
{
    return(false);
}

void CPath_old::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    _initialExplicitHandling=_explicitHandling;
    if (pathContainer!=nullptr)
        pathContainer->initializeInitialValues(simulationAlreadyRunning);
}

void CPath_old::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CPath_old::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
        {
            _explicitHandling=_initialExplicitHandling;
        }
    }
    if (pathContainer!=nullptr)
        pathContainer->simulationEnded();
    CSceneObject::simulationEnded();
}

void CPath_old::_generatePathShape()
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

CShape* CPath_old::getShape() const
{
    CShape* retVal=nullptr;
    if (_shapingEnabled&&(_pathShapeVertices.size()!=0))
    {
        std::vector<float> vert(_pathShapeVertices);
        C7Vector tr(getFullCumulativeTransformation());
        for (size_t i=0;i<vert.size()/3;i++)
        {
            C3Vector v(&vert[3*i]);
            v=tr*v;
            vert[3*i+0]=v(0);
            vert[3*i+1]=v(1);
            vert[3*i+2]=v(2);
        }
        std::vector<int> ind(_pathShapeIndices);
        retVal=new CShape(nullptr,vert,ind,nullptr,nullptr);
    }
    return(retVal);
}

void CPath_old::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
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
                CTTUtil::scaleColorUp_(shapingColor.getColorsPtr());
            }
            _generatePathShape();
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("shaping");
                ar.xmlAddNode_int("type",_shapingType);
                ar.xmlAddNode_float("elementMaxLength",_shapingElementMaxLength);
                ar.xmlAddNode_float("scaling",_shapingScaling);
                ar.xmlAddNode_floats("coordinates",shapingCoordinates);
                ar.xmlPushNewNode("switches");
                ar.xmlAddNode_bool("enabled",_shapingEnabled);
                ar.xmlAddNode_bool("followFullOrientation",_shapingFollowFullOrientation);
                ar.xmlAddNode_bool("sectionClosed",_shapingSectionClosed);
                ar.xmlAddNode_bool("convexHull",_shapingConvexHull);
                ar.xmlPopNode();
                ar.xmlPushNewNode("color");
                shapingColor.serialize(ar,0);
                ar.xmlPopNode();
                ar.xmlPopNode();

                ar.xmlPushNewNode("pathContainer");
                pathContainer->serialize(ar);
                ar.xmlPopNode();
            }
            else
                pathContainer->serialize(ar);
        }
        else
        {
            if (exhaustiveXml&&ar.xmlPushChildNode("shaping"))
            {
                ar.xmlGetNode_int("type",_shapingType);
                ar.xmlGetNode_float("elementMaxLength",_shapingElementMaxLength);
                ar.xmlGetNode_float("scaling",_shapingScaling);
                shapingCoordinates.clear();
                ar.xmlGetNode_floats("coordinates",shapingCoordinates);
                if (ar.xmlPushChildNode("switches"))
                {
                    ar.xmlGetNode_bool("enabled",_shapingEnabled);
                    ar.xmlGetNode_bool("followFullOrientation",_shapingFollowFullOrientation);
                    ar.xmlGetNode_bool("sectionClosed",_shapingSectionClosed);
                    ar.xmlGetNode_bool("convexHull",_shapingConvexHull);
                    ar.xmlPopNode();
                }
                if (ar.xmlPushChildNode("color"))
                {
                    shapingColor.serialize(ar,0);
                    ar.xmlPopNode();
                }
                ar.xmlPopNode();
            }
            _explicitHandling=true;
            if (exhaustiveXml)
            {
                if (ar.xmlPushChildNode("pathContainer"))
                {
                    pathContainer->serialize(ar);
                    ar.xmlPopNode();
                }
                _generatePathShape();
            }
            else
                pathContainer->serialize(ar);
        }
    }
}

void CPath_old::serializeWExtIk(CExtIkSer& ar)
{
    CSceneObject::serializeWExtIk(ar);
    CDummy::serializeWExtIkStatic(ar);
}

void CPath_old::display(CViewableBase* renderingObject,int displayAttrib)
{
    EASYLOCK(_objectMutex);
    displayPath(this,renderingObject,displayAttrib);
}

#ifdef SIM_WITH_GUI
bool CPath_old::transformSelectedPathPoints(const C4X4Matrix& cameraAbsConf,const C3Vector& clicked3DPoint,float prevPos[2],float pos[2],float screenHalfSizes[2],float halfSizes[2],bool perspective,int eventID)
{
    C3Vector pointCenter;
    pointCenter.clear();
    CPathCont_old* pc;
    std::vector<int> selectedPathPoints;
    if (App::getEditModeType()&PATH_EDIT_MODE_OLD)
    {
        pc=App::mainWindow->editModeContainer->getEditModePathContainer_old();
        selectedPathPoints.assign(App::mainWindow->editModeContainer->getEditModeBuffer()->begin(),App::mainWindow->editModeContainer->getEditModeBuffer()->end());
    }
    else
    {
        pc=pathContainer;
        selectedPathPoints.assign(App::mainWindow->editModeContainer->pathPointManipulation->getPointerToSelectedPathPointIndices_nonEditMode()->begin(),App::mainWindow->editModeContainer->pathPointManipulation->getPointerToSelectedPathPointIndices_nonEditMode()->end());
    }
    for (int i=0;i<int(selectedPathPoints.size());i++)
    {
        CSimplePathPoint_old* aPt=pc->getSimplePathPoint(selectedPathPoints[i]);
        if (aPt!=nullptr)
        {
            pointCenter+=(getCumulativeTransformation()*aPt->getTransformation()).X;
        }
    }
    if (selectedPathPoints.size()!=0)
        pointCenter/=float(selectedPathPoints.size());

    C4X4Matrix objAbs;
    objAbs.X=pointCenter;
        objAbs.M=getCumulativeTransformation().getMatrix().M;
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
        CSimplePathPoint_old* aPt=pc->getSimplePathPoint(selectedPathPoints[i]);
        if (aPt!=nullptr)
        {
            C4X4Matrix m(getCumulativeTransformation()*aPt->getTransformation());
            m.X+=v;
            aPt->setTransformation(getCumulativeTransformation().getInverse().getMatrix()*m,pc->getAttributes());
        }
    }
    _objectManipulationMode_flaggedForGridOverlay=_objectManipulationModeAxisIndex+16;
    pc->actualizePath();
    return(true);
}
#endif
