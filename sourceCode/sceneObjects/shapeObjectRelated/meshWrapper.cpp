#include <simInternal.h>
#include <meshWrapper.h>
#include <mesh.h>
#include <tt.h>
#include <utils.h>
#include <global.h>
#include <viewableBase.h>
#include <app.h>
#include <Eigen/Eigenvalues>
#include <iostream>
#include <algos.h>

CMeshWrapper::CMeshWrapper()
{
    _commonInit();
}

void CMeshWrapper::_commonInit()
{
    _mass=1.0;
    _name="sub__0";
    _dynMaterialId_old=-1; // not used anymore since V3.4.0
    _convex=false;
    _iFrame.setIdentity();
    _bbFrame.setIdentity();
    _bbSize.clear();
    setDefaultInertiaParams();
    _transformationsSinceGrouping.setIdentity();
}

CMeshWrapper::~CMeshWrapper()
{
    for (size_t i=0;i<childList.size();i++)
        delete childList[i];
}

void CMeshWrapper::detachItems()
{
    childList.clear();
    _commonInit();
}

void CMeshWrapper::addItem(CMeshWrapper* m)
{
    childList.push_back(m);
    _convex=true;
    for (size_t i=0;i<childList.size();i++)
    {
        if (!childList[i]->isConvex())
        {
            _convex=false;
            break;
        }
    }
    std::vector<double> vert;
    std::vector<int> ind;
    getCumulativeMeshes(C7Vector::identityTransformation,vert,&ind,nullptr);
    _bbFrame=CAlgos::getMeshBoundingBox(vert,ind,false,&_bbSize);
    _computeInertiaFromChildren();
}

void CMeshWrapper::_computeInertiaFromChildren()
{
    _mass=0.0;
    _com.clear();
    _iMatrix.clear();
    for (size_t i=0;i<childList.size();i++)
    {
        CMeshWrapper* mesh=childList[i];
        _mass+=mesh->getMass();
        _com=_com+mesh->getIFrame()*mesh->getCOM()*mesh->getMass();
        C3X3Matrix matr(getMasslesInertiaMatrixInNewFrame(mesh->getIFrame().Q,mesh->getMasslessInertiaMatrix(),C4Vector::identityRotation));
        matr=matr*mesh->getMass();
        _iMatrix=_iMatrix+matr;
    }
    _com=_com/_mass;
    _iMatrix=_iMatrix/_mass;
    fixInertiaMatrixAndComputePMI();
}

C7Vector CMeshWrapper::getTransformationsSinceGrouping() const
{
    return(_transformationsSinceGrouping);
}

void CMeshWrapper::setTransformationsSinceGrouping(const C7Vector& tr)
{
    _transformationsSinceGrouping=tr;
}

void CMeshWrapper::display(const C7Vector& cumulIFrameTr,CShape* geomData,int displayAttrib,CColorObject* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->display(cumulIFrameTr*_iFrame,geomData,displayAttrib,collisionColor,dynObjFlag_forVisualization,transparencyHandling,multishapeEditSelected);
}

void CMeshWrapper::display_extRenderer(const C7Vector& cumulIFrameTr,CShape* geomData,int displayAttrib,const C7Vector& tr,int shapeHandle,int& componentIndex)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->display_extRenderer(cumulIFrameTr*_iFrame,geomData,displayAttrib,tr,shapeHandle,componentIndex);
}

void CMeshWrapper::display_colorCoded(const C7Vector& cumulIFrameTr,CShape* geomData,int objectId,int displayAttrib)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->display_colorCoded(cumulIFrameTr*_iFrame,geomData,objectId,displayAttrib);
}

void CMeshWrapper::displayGhost(const C7Vector& cumulIFrameTr,CShape* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,double transparency,const float* newColors)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->displayGhost(cumulIFrameTr*_iFrame,geomData,displayAttrib,originalColors,backfaceCulling,transparency,newColors);
}

void CMeshWrapper::performSceneObjectLoadingMapping(const std::map<int,int>* map)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->performSceneObjectLoadingMapping(map);
}

void CMeshWrapper::performTextureObjectLoadingMapping(const std::map<int,int>* map)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->performTextureObjectLoadingMapping(map);
}

void CMeshWrapper::performDynMaterialObjectLoadingMapping(const std::map<int,int>* map)
{
    _dynMaterialId_old=CWorld::getLoadingMapping(map,_dynMaterialId_old);
}

void CMeshWrapper::announceSceneObjectWillBeErased(const CSceneObject* object)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->announceSceneObjectWillBeErased(object);
}

void CMeshWrapper::setTextureDependencies(int shapeID)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->setTextureDependencies(shapeID);
}

int CMeshWrapper::getTextureCount() const
{ // function has virtual/non-virtual counterpart!
    int retVal=0;
    for (size_t i=0;i<childList.size();i++)
        retVal+=childList[i]->getTextureCount();
    return(retVal);
}

bool CMeshWrapper::hasTextureThatUsesFixedTextureCoordinates() const
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
    {
        if (childList[i]->hasTextureThatUsesFixedTextureCoordinates())
            return(true);
    }
    return(false);
}

void CMeshWrapper::removeAllTextures()
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->removeAllTextures();
}

void CMeshWrapper::getColorStrings(std::string& colorStrings) const
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->getColorStrings(colorStrings);
}

bool CMeshWrapper::getContainsTransparentComponents() const
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
    {
        if (childList[i]->getContainsTransparentComponents())
            return(true);
    }
    return(false);
}

double CMeshWrapper::getShadingAngle() const
{ // function has virtual/non-virtual counterpart!
    return(childList[0]->getShadingAngle()); // we just return the first angle we encounter! Normally never used
}

void CMeshWrapper::setShadingAngle(double angle)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->setShadingAngle(angle);
}

double CMeshWrapper::getEdgeThresholdAngle() const
{ // function has virtual/non-virtual counterpart!
    return(childList[0]->getEdgeThresholdAngle()); // we just return the first angle we encounter! Normally never used
}

void CMeshWrapper::setEdgeThresholdAngle(double angle)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->setEdgeThresholdAngle(angle);
}

void CMeshWrapper::setHideEdgeBorders_OLD(bool v)
{  // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->setHideEdgeBorders_OLD(v);
}

bool CMeshWrapper::getHideEdgeBorders_OLD() const
{  // function has virtual/non-virtual counterpart!
    return(childList[0]->getHideEdgeBorders_OLD()); // we just return the first setting we encounter! Normally never used
}

CMeshWrapper* CMeshWrapper::copyYourself()
{ // function has virtual/non-virtual counterpart!
    CMeshWrapper* newIt=new CMeshWrapper();
    copyWrapperData(newIt);
    return(newIt);
}

void CMeshWrapper::copyAttributesTo(CMeshWrapper* target)
{
    target->_mass=_mass;
    target->_name=_name;
    target->_localInertiaFrame=_localInertiaFrame;
    target->_principalMomentsOfInertia=_principalMomentsOfInertia;
    target->_transformationsSinceGrouping=_transformationsSinceGrouping;
    target->_iFrame=_iFrame;
    target->_com=_com;
    target->_iMatrix=_iMatrix;
    target->_pmi=_pmi;
    target->_pmiRotFrame=_pmiRotFrame;
    target->_dynMaterialId_old=_dynMaterialId_old;
    if (childList.size()==0)
        target->takeVisualAttributesFrom((CMesh*)this);
}

void CMeshWrapper::copyWrapperData(CMeshWrapper* target)
{
    target->_mass=_mass;
    target->_name=_name;
    target->_convex=_convex;

    target->_localInertiaFrame=_localInertiaFrame;
    target->_principalMomentsOfInertia=_principalMomentsOfInertia;
    target->_transformationsSinceGrouping=_transformationsSinceGrouping;
    target->_iFrame=_iFrame;
    target->_com=_com;
    target->_iMatrix=_iMatrix;
    target->_pmi=_pmi;
    target->_pmiRotFrame=_pmiRotFrame;
    target->_bbFrame=_bbFrame;
    target->_bbSize=_bbSize;


    target->_dynMaterialId_old=_dynMaterialId_old;

    for (size_t i=0;i<childList.size();i++)
        target->childList.push_back(childList[i]->copyYourself());
}

void CMeshWrapper::setMass(double m)
{
    _mass=tt::getLimitedFloat(0.000000001,100000.0,m);
}

double CMeshWrapper::getMass() const
{
    return(_mass);
}

void CMeshWrapper::setDefaultInertiaParams()
{
    _localInertiaFrame.setIdentity();
    _principalMomentsOfInertia(0)=0.001;
    _principalMomentsOfInertia(1)=0.001;
    _principalMomentsOfInertia(2)=0.001;
    _com.clear();
    _iMatrix.clear();
    _iMatrix(0,0)=0.01;
    _iMatrix(1,1)=0.01;
    _iMatrix(2,2)=0.01;
    fixInertiaMatrixAndComputePMI();
}

void CMeshWrapper::setName(std::string newName)
{
    _name=newName;
}

std::string CMeshWrapper::getName() const
{
    return(_name);
}

int CMeshWrapper::getDynMaterialId_old() const
{
    return(_dynMaterialId_old);
}

void CMeshWrapper::setDynMaterialId_old(int id)
{
    _dynMaterialId_old=id;
}

C7Vector CMeshWrapper::getDiagonalInertiaInfo(C3Vector& diagMasslessI) const
{
    C7Vector retVal;
    retVal.X=_com;
    retVal.Q=_pmiRotFrame;
    diagMasslessI=_pmi;
    return(retVal);
}

C7Vector CMeshWrapper::getBB(C3Vector* optBBSize) const
{
    if (optBBSize!=nullptr)
        optBBSize[0]=_bbSize;
    return(_bbFrame);
}

bool CMeshWrapper::getShapeRelIFrame(const C7Vector& parentCumulTr,const CMeshWrapper* wrapper,C7Vector& shapeRelIFrame) const
{
    bool retVal=false;
    if (wrapper==this)
    {
        retVal=true;
        shapeRelIFrame=parentCumulTr*_iFrame;
    }
    else
    {
        for (size_t i=0;i<childList.size();i++)
        {
            if (childList[i]->getShapeRelIFrame(parentCumulTr*_iFrame,wrapper,shapeRelIFrame))
            {
                retVal=true;
                break;
            }
        }
    }
    return(retVal);
}

bool CMeshWrapper::getShapeRelBB(const C7Vector& parentCumulTr,const CMeshWrapper* wrapper,C7Vector& shapeRelBB,C3Vector* optBBSize) const
{
    C7Vector ifr;
    bool retVal=getShapeRelIFrame(parentCumulTr,wrapper,ifr);
    if (retVal)
        shapeRelBB=ifr*wrapper->getBB(optBBSize);
    return(retVal);
}

C3Vector CMeshWrapper::getCOM() const
{
    return(_com);
}

C7Vector CMeshWrapper::getIFrame() const
{
    return(_iFrame);
}

void CMeshWrapper::setIFrame(const C7Vector& iframe)
{
    _iFrame=iframe;
}

void CMeshWrapper::setCOM(const C3Vector& com)
{
    _com=com;
}

C3X3Matrix CMeshWrapper::getMasslessInertiaMatrix() const
{
    return(_iMatrix);
}

void CMeshWrapper::setMasslessInertiaMatrix(const C3X3Matrix& im,int modifItemRow/*=-1*/,int modifItemCol/*=-1*/)
{
    _iMatrix=im;
    if ( (modifItemRow!=-1)&&(modifItemCol!=-1) )
        _iMatrix(modifItemCol,modifItemRow)=_iMatrix(modifItemRow,modifItemCol);
    fixInertiaMatrixAndComputePMI();
}

void CMeshWrapper::fixInertiaMatrixAndComputePMI()
{
    for (size_t i=0;i<3;i++)
    {
        // Make sure we are symmetric;
        for (size_t j=0;j<3;j++)
            _iMatrix(i,j)=_iMatrix(j,i);
        // Make sure diagonals are positive and above a certain threshold:
        if (_iMatrix(i,i)<1e-8)
            _iMatrix(i,i)=1e-8;
    }
    getPMIFromMasslessTensor(_iMatrix,_pmiRotFrame,_pmi);
}

C3Vector CMeshWrapper::getPMI() const
{
    return(_pmi);
}

void CMeshWrapper::setPMI(const C3Vector& pmi)
{
    C3X3Matrix im;
    im.clear();
    im(0,0)=pmi(0);
    im(1,1)=pmi(1);
    im(2,2)=pmi(2);
    setMasslessInertiaMatrix(im);
}

C7Vector CMeshWrapper::getLocalInertiaFrame() const
{
    return (_localInertiaFrame);
}

void CMeshWrapper::setLocalInertiaFrame(const C7Vector& li)
{
    _localInertiaFrame=li;
}

C3Vector CMeshWrapper::getPrincipalMomentsOfInertia() const
{
    return (_principalMomentsOfInertia);
}

void CMeshWrapper::setPrincipalMomentsOfInertia(const C3Vector& inertia)
{ // massless inertia
    _principalMomentsOfInertia=inertia;
    _principalMomentsOfInertia(0)=tt::getLimitedFloat(0.0,10000.0,_principalMomentsOfInertia(0));
    _principalMomentsOfInertia(1)=tt::getLimitedFloat(0.0,10000.0,_principalMomentsOfInertia(1));
    _principalMomentsOfInertia(2)=tt::getLimitedFloat(0.0,10000.0,_principalMomentsOfInertia(2));
    if (_principalMomentsOfInertia.getLength()==0.0)
        _principalMomentsOfInertia(0)=0.001; // make sure we don't have a zero vector (problems with Bullet? and CoppeliaSim!)
}

void CMeshWrapper::prepareVerticesIndicesNormalsAndEdgesForSerialization()
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->prepareVerticesIndicesNormalsAndEdgesForSerialization();
}

void CMeshWrapper::scaleMassAndInertia(double s)
{
    _mass*=s*s*s;
    _iMatrix*=s*s;
    fixInertiaMatrixAndComputePMI();
}

void CMeshWrapper::scale(double isoVal)
{ // function has virtual/non-virtual counterpart
    scaleMassAndInertia(isoVal);
    _com*=isoVal;
    _iFrame.X*=isoVal;
    _bbFrame.X*=isoVal;
    _bbSize*=isoVal;

    for (size_t i=0;i<childList.size();i++)
        childList[i]->scale(isoVal);
    if (isoVal<0.0) // that effectively flips faces!
        checkIfConvex();
}

void CMeshWrapper::setPurePrimitiveType(int theType,double xOrDiameter,double y,double zOrHeight)
{ // function has virtual/non-virtual counterpart!
    // Following added on 14/03/2011 because a compound shape composed by pure and non pure shapes would decompose as pure shapes with wrong orientation!
    if (theType==sim_primitiveshape_none)
    {
        for (size_t i=0;i<childList.size();i++)
            childList[i]->setPurePrimitiveType(theType,xOrDiameter,y,zOrHeight);
        // _convex=false;  NO!!!
    }
}

int CMeshWrapper::getPurePrimitiveType() const
{ // function has virtual/non-virtual counterpart!
    return(childList[0]->getPurePrimitiveType()); // we just return the first type we encounter! Normally never used
}

bool CMeshWrapper::isMesh() const
{ // function has virtual/non-virtual counterpart!
    return(false);
}

bool CMeshWrapper::isPure() const
{ // function has virtual/non-virtual counterpart!
    return(childList[0]->isPure());
}

bool CMeshWrapper::isConvex() const
{ // function has virtual/non-virtual counterpart!
    return(_convex);
}

bool CMeshWrapper::containsOnlyPureConvexShapes()
{ // function has virtual/non-virtual counterpart!
    bool retVal=true;
    for (size_t i=0;i<childList.size();i++)
        retVal=retVal&&childList[i]->containsOnlyPureConvexShapes();
    if (retVal)
        _convex=retVal; // needed since there was a bug where pure planes and pure discs were considered as convex
    return(retVal);
}

bool CMeshWrapper::checkIfConvex()
{ // function has virtual/non-virtual counterpart!
    _convex=true;
    for (size_t i=0;i<childList.size();i++)
        _convex=_convex&&childList[i]->checkIfConvex();
    setConvex(_convex);
    return(_convex);
}

void CMeshWrapper::setConvex(bool convex)
{ // function has virtual/non-virtual counterpart!
    _convex=convex;
}

void CMeshWrapper::takeVisualAttributesFrom(CMesh* origin)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->takeVisualAttributesFrom(origin);
}

void CMeshWrapper::getCumulativeMeshes(const C7Vector& parentCumulTr,std::vector<double>& vertices,std::vector<int>* indices,std::vector<double>* normals)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->getCumulativeMeshes(parentCumulTr*_iFrame,vertices,indices,normals);
}

void CMeshWrapper::getCumulativeMeshes(const C7Vector& parentCumulTr,const CMeshWrapper* wrapper,std::vector<double>& vertices,std::vector<int>* indices,std::vector<double>* normals)
{ // function has virtual/non-virtual counterpart!
    const CMeshWrapper* w=wrapper;
    if (wrapper==this)
        w=nullptr;
    for (size_t i=0;i<childList.size();i++)
        childList[i]->getCumulativeMeshes(parentCumulTr*_iFrame,w,vertices,indices,normals);
}

void CMeshWrapper::setColor(const CShape* shape,int& elementIndex,const char* colorName,int colorComponent,const float* rgbData,int& rgbDataOffset)
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->setColor(shape,elementIndex,colorName,colorComponent,rgbData,rgbDataOffset);
}

bool CMeshWrapper::getColor(const char* colorName,int colorComponent,float* rgbData,int& rgbDataOffset) const
{ // function has virtual/non-virtual counterpart!
    bool retVal=false;
    for (size_t i=0;i<childList.size();i++)
        retVal=childList[i]->getColor(colorName,colorComponent,rgbData,rgbDataOffset)||retVal;
    return(retVal);
}

void CMeshWrapper::getAllShapeComponentsCumulative(const C7Vector& parentCumulTr,std::vector<CMesh*>& shapeComponentList,std::vector<C7Vector>* OptParentCumulTrList/*=nullptr*/)
{   // function has virtual/non-virtual counterpart!
    // needed by the dynamics routine. We return ALL shape components!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->getAllShapeComponentsCumulative(parentCumulTr*_iFrame,shapeComponentList,OptParentCumulTrList);
}

CMesh* CMeshWrapper::getShapeComponentAtIndex(const C7Vector& parentCumulTr,int& index,C7Vector* optParentCumulTrOut/*=nullptr*/)
{ // function has virtual/non-virtual counterpart!
    CMesh* retVal=nullptr;
    if (index>=0)
    {
        for (size_t i=0;i<childList.size();i++)
        {
            retVal=childList[i]->getShapeComponentAtIndex(parentCumulTr*_iFrame,index,optParentCumulTrOut);
            if (retVal!=nullptr)
                break;
        }
    }
    return(retVal);
}

int CMeshWrapper::getComponentCount() const
{ // function has virtual/non-virtual counterpart!
    int retVal=0;
    for (size_t i=0;i<childList.size();i++)
        retVal+=childList[i]->getComponentCount();
    return(retVal);
}

void CMeshWrapper::flipFaces()
{ // function has virtual/non-virtual counterpart!
    for (size_t i=0;i<childList.size();i++)
        childList[i]->flipFaces();
    checkIfConvex();
}

bool CMeshWrapper::serialize(CSer& ar,const char* shapeName,const C7Vector& parentCumulIFrame,bool rootLevel)
{ // function has virtual/non-virtual counterpart!
    bool isNewTypeOfShapeFormat=false;
    if (rootLevel)
    {
        _iFrame.setIdentity();
        _transformationsSinceGrouping.setIdentity();
    }
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Nme");
            ar << _name;
            ar.flush();

#ifdef TMPOPERATION
            ar.storeDataName("Mas");
            ar << (float)_mass;
            ar.flush();
#endif

            ar.storeDataName("_as");
            ar << _mass;
            ar.flush();


            ar.storeDataName("Dmi");
            ar << _dynMaterialId_old;
            ar.flush();

#ifdef TMPOPERATION
            ar.storeDataName("Ine"); // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
            C7Vector w(parentCumulIFrame*_iFrame);
            w.X+=_com;
            w.Q=w.Q*_pmiRotFrame;
            ar << (float)w(0) << (float)w(1) << (float)w(2);
            ar << (float)w(3) << (float)w(4) << (float)w(5) << (float)w(6);
            ar << (float)_pmi(0) << (float)_pmi(1) << (float)_pmi(2);
            ar.flush();
#endif

            ar.storeDataName("_ne"); // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
            w=parentCumulIFrame*_iFrame;
            w.X+=_com;
            w.Q=w.Q*_pmiRotFrame;
            ar << w(0) << w(1) << w(2);
            ar << w(3) << w(4) << w(5) << w(6);
            ar << _pmi(0) << _pmi(1) << _pmi(2);
            ar.flush();

#ifdef TMPOPERATION
            ar.storeDataName("Vtb"); // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
            w=parentCumulIFrame*_iFrame;
            ar << (float)w(0) << (float)w(1) << (float)w(2) << (float)w(3);
            ar << (float)w(4) << (float)w(5) << (float)w(6);
            ar.flush();
#endif

            ar.storeDataName("_tb"); // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
            w=parentCumulIFrame*_iFrame;
            ar << w(0) << w(1) << w(2) << w(3);
            ar << w(4) << w(5) << w(6);
            ar.flush();

            ar.storeDataName("Ifr");
            ar << _iFrame(0) << _iFrame(1) << _iFrame(2);
            ar << _iFrame(3) << _iFrame(4) << _iFrame(5) << _iFrame(6);
            ar.flush();

            ar.storeDataName("Com");
            ar << _com(0) << _com(1) << _com(2);
            ar.flush();

            ar.storeDataName("Imx");
            for (size_t i=0;i<3;i++)
            {
                for (size_t j=0;j<3;j++)
                    ar << _iMatrix(i,j);
            }
            ar.flush();

            ar.storeDataName("Bbf");
            ar << _bbFrame(0) << _bbFrame(1) << _bbFrame(2);
            ar << _bbFrame(3) << _bbFrame(4) << _bbFrame(5) << _bbFrame(6);
            ar.flush();

            ar.storeDataName("Bbs");
            ar << _bbSize(0) << _bbSize(1) << _bbSize(2);
            ar.flush();

            ar.storeDataName("Var");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,2,_convex);
            SIM_SET_CLEAR_BIT(nothing,5,true); // means: we do not have to make the convectivity test for this shape (was already done). Added this on 28/1/2013
            ar << nothing;
            ar.flush();

            for (size_t i=0;i<childList.size();i++)
            {
                if (childList[i]->isMesh())
                    ar.storeDataName("Geo");
                else
                    ar.storeDataName("Wrp");
                ar.setCountingMode();
                childList[i]->serialize(ar,shapeName,parentCumulIFrame*_iFrame,false);
                if (ar.setWritingMode())
                    childList[i]->serialize(ar,shapeName,parentCumulIFrame*_iFrame,false);
            }

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Nme")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _name;
                    }
                    if (theName.compare("Mas")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _mass=(double)bla;
                        if (_mass==0.0) // to catch an old bug
                            _mass=0.001;
                    }

                    if (theName.compare("_as")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _mass;
                        if (_mass==0.0) // to catch an old bug
                            _mass=0.001;
                    }

                    if (theName.compare("Dmi")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynMaterialId_old;
                    }
                    if (theName.compare("Ine")==0) // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        float bla;
                        for (size_t i=0;i<7;i++)
                        {
                            ar >> bla;
                            _localInertiaFrame(i)=(double)bla;
                        }
                        for (size_t i=0;i<3;i++)
                        {
                            ar >> bla;
                            _principalMomentsOfInertia(i)=(double)bla;
                        }
                    }

                    if (theName.compare("_ne")==0) // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _localInertiaFrame(0) >> _localInertiaFrame(1) >> _localInertiaFrame(2) >> _localInertiaFrame(3);
                        ar >> _localInertiaFrame(4) >> _localInertiaFrame(5) >> _localInertiaFrame(6);
                        ar >> _principalMomentsOfInertia(0) >> _principalMomentsOfInertia(1) >> _principalMomentsOfInertia(2);
                    }

                    if (theName.compare("Vtb")==0) // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        float bla;
                        for (size_t i=0;i<7;i++)
                        {
                            ar >> bla;
                            _transformationsSinceGrouping(i)=(double)bla;
                        }
                        if (rootLevel)
                            _transformationsSinceGrouping.setIdentity();
                        _iFrame=parentCumulIFrame.getInverse()*_transformationsSinceGrouping;
                        C7Vector inf(_transformationsSinceGrouping.getInverse()*_localInertiaFrame);
                        _com=inf.X;
                        inf.X.clear();
                        _iMatrix=getMasslessTensorFromPMI(_principalMomentsOfInertia,inf);
                        fixInertiaMatrixAndComputePMI();
                    }

                    if (theName.compare("_tb")==0) // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _transformationsSinceGrouping(0) >> _transformationsSinceGrouping(1) >> _transformationsSinceGrouping(2) >> _transformationsSinceGrouping(3);
                        ar >> _transformationsSinceGrouping(4) >> _transformationsSinceGrouping(5) >> _transformationsSinceGrouping(6);
                        if (rootLevel)
                            _transformationsSinceGrouping.setIdentity();
                        _iFrame=parentCumulIFrame.getInverse()*_transformationsSinceGrouping;
                        C7Vector inf(_transformationsSinceGrouping.getInverse()*_localInertiaFrame);
                        _com=inf.X;
                        inf.X.clear();
                        _iMatrix=getMasslessTensorFromPMI(_principalMomentsOfInertia,inf);
                        fixInertiaMatrixAndComputePMI();
                    }

                    if (theName.compare("Ifr")==0)
                    {
                        isNewTypeOfShapeFormat=true;
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _iFrame(0) >> _iFrame(1) >> _iFrame(2);
                        ar >> _iFrame(3) >> _iFrame(4) >> _iFrame(5) >> _iFrame(6);
                        if (rootLevel)
                            _iFrame.setIdentity();
                    }
                    if (theName.compare("Com")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _com(0) >> _com(1) >> _com(2);
                    }
                    if (theName.compare("Imx")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        for (size_t i=0;i<3;i++)
                        {
                            for (size_t j=0;j<3;j++)
                                ar >> _iMatrix(i,j);
                        }
                        fixInertiaMatrixAndComputePMI();
                    }
                    if (theName.compare("Bbf")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _bbFrame(0) >> _bbFrame(1) >> _bbFrame(2);
                        ar >> _bbFrame(3) >> _bbFrame(4) >> _bbFrame(5) >> _bbFrame(6);
                    }
                    if (theName.compare("Bbs")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _bbSize(0) >> _bbSize(1) >> _bbSize(2);
                    }

                    if (theName=="Var")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _convex=SIM_IS_BIT_SET(nothing,2);
                    }
                    if (theName.compare("Geo")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; 
                        CMesh* it=new CMesh();
                        it->serialize(ar,shapeName,parentCumulIFrame*_iFrame,false);
                        childList.push_back(it);
                    }
                    if (theName.compare("Wrp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; 
                        CMeshWrapper* it=new CMeshWrapper();
                        it->serialize(ar,shapeName,parentCumulIFrame*_iFrame,false);
                        childList.push_back(it);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if ( (!isNewTypeOfShapeFormat)&&(childList.size()!=0) )
            {
                std::vector<double> vert;
                std::vector<int> ind;
                getCumulativeMeshes(C7Vector::identityTransformation,vert,&ind,nullptr);
                _bbFrame=CAlgos::getMeshBoundingBox(vert,ind,false,&_bbSize);
            }
        }
    }
    else
    {
        if (ar.isStoring())
        {
            ar.xmlPushNewNode("common");

            ar.xmlAddNode_string("name",_name.c_str());

            ar.xmlAddNode_comment(" 'transformationSinceGrouping' tag: deprecated, for backward compatibility ",false);
            ar.xmlPushNewNode("transformationSinceGrouping"); // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
            C7Vector w(parentCumulIFrame*_iFrame);
            ar.xmlAddNode_floats("position",w.X.data,3);
            ar.xmlAddNode_floats("quaternion",w.Q.data,4);
            ar.xmlPopNode();

            ar.xmlPushNewNode("dynamics");

            ar.xmlAddNode_float("mass",_mass);

            ar.xmlAddNode_comment(" 'localInertiaFrame' tag: deprecated, for backward compatibility ",false);
            ar.xmlPushNewNode("localInertiaFrame"); // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
            w=parentCumulIFrame*_iFrame;
            w.X+=_com;
            w.Q=w.Q*_pmiRotFrame;
            ar.xmlAddNode_floats("position",w.X.data,3);
            ar.xmlAddNode_floats("quaternion",w.Q.data,4);
            ar.xmlAddNode_floats("principalMomentsOfInertia",_pmi.data,3);
            ar.xmlPopNode();

            ar.xmlPushNewNode("inertiaFrame");
            ar.xmlAddNode_floats("position",_iFrame.X.data,3);
            ar.xmlAddNode_floats("quaternion",_iFrame.Q.data,4);
            ar.xmlPopNode();

            ar.xmlAddNode_floats("centerOfMass",_com.data,3);

            double im[9];
            for (size_t i=0;i<3;i++)
            {
                for (size_t j=0;j<3;j++)
                    im[i*3+j]=_iMatrix(i,j);
            }
            ar.xmlAddNode_floats("masslessInertiaMatrix",im,9);

            ar.xmlPushNewNode("bbFrame");
            ar.xmlAddNode_floats("position",_bbFrame.X.data,3);
            ar.xmlAddNode_floats("quaternion",_bbFrame.Q.data,4);
            ar.xmlPopNode();

            ar.xmlAddNode_floats("bbSize",_bbSize.data,3);
            ar.xmlPopNode(); // "dynamics" node

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("convex",_convex);
            ar.xmlPopNode();

            for (size_t i=0;i<childList.size();i++)
            {
                ar.xmlPushNewNode("child");
                if (childList[i]->isMesh())
                    ar.xmlPushNewNode("mesh");
                else
                    ar.xmlPushNewNode("compound");
                childList[i]->serialize(ar,shapeName,parentCumulIFrame*_iFrame,false);
                ar.xmlPopNode();
                ar.xmlPopNode();
            }
            ar.xmlPopNode();
        }
        else
        {
            if (ar.xmlPushChildNode("common"))
            {
                ar.xmlGetNode_string("name",_name);

                if (ar.xmlPushChildNode("transformationSinceGrouping"))  // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                {
                    ar.xmlGetNode_floats("position",_transformationsSinceGrouping.X.data,3);
                    ar.xmlGetNode_floats("quaternion",_transformationsSinceGrouping.Q.data,4);
                    if (rootLevel)
                        _transformationsSinceGrouping.setIdentity();
                    _transformationsSinceGrouping.Q.normalize(); // just in case
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("dynamics"))
                {
                    ar.xmlGetNode_float("mass",_mass);

                    if (ar.xmlPushChildNode("localInertiaFrame"))  // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                    {
                        ar.xmlGetNode_floats("position",_localInertiaFrame.X.data,3);
                        ar.xmlGetNode_floats("quaternion",_localInertiaFrame.Q.data,4);
                        _localInertiaFrame.Q.normalize(); // just in case
                        ar.xmlGetNode_floats("principalMomentsOfInertia",_principalMomentsOfInertia.data,3);
                        ar.xmlPopNode();
                        _iFrame=parentCumulIFrame.getInverse()*_transformationsSinceGrouping;
                        C7Vector inf(_transformationsSinceGrouping.getInverse()*_localInertiaFrame);
                        _com=inf.X;
                        inf.X.clear();
                        _iMatrix=getMasslessTensorFromPMI(_principalMomentsOfInertia,inf);
                        fixInertiaMatrixAndComputePMI();
                    }

                    if (ar.xmlPushChildNode("inertiaFrame"))
                    {
                        isNewTypeOfShapeFormat=true;
                        ar.xmlGetNode_floats("position",_iFrame.X.data,3);
                        ar.xmlGetNode_floats("quaternion",_iFrame.Q.data,4);
                        _iFrame.Q.normalize(); // just in case
                        if (rootLevel)
                            _iFrame.setIdentity();
                        ar.xmlPopNode();
                    }

                    ar.xmlGetNode_floats("centerOfMass",_com.data,3);

                    double im[9];
                    ar.xmlGetNode_floats("masslessInertiaMatrix",im,9);
                    for (size_t i=0;i<3;i++)
                    {
                        for (size_t j=0;j<3;j++)
                            _iMatrix(i,j)=im[i*3+j];
                    }
                    fixInertiaMatrixAndComputePMI();

                    if (ar.xmlPushChildNode("bbFrame"))
                    {
                        ar.xmlGetNode_floats("position",_bbFrame.X.data,3);
                        ar.xmlGetNode_floats("quaternion",_bbFrame.Q.data,4);
                        _bbFrame.Q.normalize(); // just in case
                        ar.xmlPopNode();
                    }

                    ar.xmlGetNode_floats("bbSize",_bbSize.data,3);

                    ar.xmlPopNode(); // "dynamics" node
                }

                if (ar.xmlPushChildNode("switches"))
                {
                    ar.xmlGetNode_bool("convex",_convex);
                    ar.xmlPopNode();
                }
                if (ar.xmlPushChildNode("child",false))
                {
                    while (true)
                    {
                        if (ar.xmlPushChildNode("mesh",false))
                        {
                            CMesh* it=new CMesh();
                            it->serialize(ar,shapeName,parentCumulIFrame*_iFrame,false);
                            childList.push_back(it);
                            ar.xmlPopNode();
                        }
                        else
                        {
                            if (ar.xmlPushChildNode("compound"))
                            {
                                CMeshWrapper* it=new CMeshWrapper();
                                it->serialize(ar,shapeName,parentCumulIFrame*_iFrame,false);
                                childList.push_back(it);
                                ar.xmlPopNode();
                            }
                        }
                        if (!ar.xmlPushSiblingNode("child",false))
                            break;
                    }
                    ar.xmlPopNode();
                }
                ar.xmlPopNode();
            }
            if ( (!isNewTypeOfShapeFormat)&&(childList.size()!=0) )
            {
                std::vector<double> vert;
                std::vector<int> ind;
                getCumulativeMeshes(C7Vector::identityTransformation,vert,&ind,nullptr);
                _bbFrame=CAlgos::getMeshBoundingBox(vert,ind,false,&_bbSize);
            }
        }
    }
    return(isNewTypeOfShapeFormat);
}


bool CMeshWrapper::getPMIFromMasslessTensor(const C3X3Matrix& tensor,C4Vector& rotation,C3Vector& principalMoments)
{ // tensor --> PMI + rotation (tensor and PMI are mass-less)
    Eigen::Matrix3f m;
    m(0,0)=tensor(0,0);
    m(1,1)=tensor(1,1);
    m(2,2)=tensor(2,2);
    m(0,1)=tensor(0,1);
    m(1,0)=tensor(0,1);
    m(0,2)=tensor(0,2);
    m(2,0)=tensor(0,2);
    m(1,2)=tensor(1,2);
    m(2,1)=tensor(1,2);
    Eigen::EigenSolver<Eigen::Matrix3f> es(m);
    Eigen::Vector3cf eigenVals=es.eigenvalues();
    principalMoments.setData(eigenVals(0).real(),eigenVals(1).real(),eigenVals(2).real());
    Eigen::Vector3cf eigenVect1=es.eigenvectors().col(0);
    C3Vector eVect1(eigenVect1(0).real(),eigenVect1(1).real(),eigenVect1(2).real());
    Eigen::Vector3cf eigenVect2=es.eigenvectors().col(1);
    C3Vector eVect2(eigenVect2(0).real(),eigenVect2(1).real(),eigenVect2(2).real());
    Eigen::Vector3cf eigenVect3=es.eigenvectors().col(2);
    C3Vector eVect3(eigenVect3(0).real(),eigenVect3(1).real(),eigenVect3(2).real());
    C3X3Matrix matr;
    matr.axis[0]=eVect1;
    matr.axis[1]=eVect2;
    matr.axis[2]=eVect3;
    rotation=matr.getQuaternion();
    return((eigenVals(0).real()>0.0)&&(eigenVals(1).real()>0.0)&&(eigenVals(2).real()>0.0));
}

C3X3Matrix CMeshWrapper::getMasslessTensorFromPMI(const C3Vector& principalMoments,const C7Vector& newFrame)
{ // PMI + transf --> tensor (tensor and PMI are mass-less)
    C3X3Matrix tensor;
    tensor.clear();
    tensor.axis[0](0)=principalMoments(0);
    tensor.axis[1](1)=principalMoments(1);
    tensor.axis[2](2)=principalMoments(2);
    // 1. reorient the frame:
    tensor=getMasslesInertiaMatrixInNewFrame(newFrame.Q,tensor,C4Vector::identityRotation);
//    tensor=newFrame.Q.getMatrix()*tensor*newFrame.Q.getInverse().getMatrix();
    // 2. shift the frame:
    C3X3Matrix D;
    D.setIdentity();
    D*=(newFrame.X*newFrame.X);
    D.axis[0]-=newFrame.X*newFrame.X(0);
    D.axis[1]-=newFrame.X*newFrame.X(1);
    D.axis[2]-=newFrame.X*newFrame.X(2);
    tensor+=D;
    return(tensor);
}

C3X3Matrix CMeshWrapper::getMasslesInertiaMatrixInNewFrame(const C4Vector& oldFrame,const C3X3Matrix& oldMatrix,const C4Vector& newFrame)
{
    C3X3Matrix rot(oldFrame.getMatrix().getTranspose()*newFrame.getMatrix());
    C3X3Matrix retVal(rot.getTranspose()*oldMatrix*rot);
    return(retVal);
}

std::string CMeshWrapper::getInertiaMatrixErrorString() const
{
    return(getInertiaMatrixErrorString(_iMatrix));
}

std::string CMeshWrapper::getInertiaMatrixErrorString(const C3X3Matrix& matrix)
{
    std::string retVal="";
    for (size_t i=0;i<3;i++)
    {
        for (size_t j=0;j<3;j++)
        {
            if (matrix(i,j)!=matrix(j,i))
            {
                retVal="Invalid inertia matrix: not symmetric";
                break;
            }
        }
    }
    if (retVal=="")
    {
        if ( (matrix(0,0)>1e-8)&&(matrix(1,1)>1e-8)&&(matrix(2,2)>1e-8) )
        {
            C4Vector dummyRot;
            C3Vector pmi;
            if (getPMIFromMasslessTensor(matrix,dummyRot,pmi))
            { // positive definite: eigenvals are positive
                if ((pmi(0)+pmi(1)<pmi(2))||(pmi(0)+pmi(2)<pmi(1))||(pmi(1)+pmi(2)<pmi(0)))
                    retVal="Invalid/unstable inertia matrix: A+B<C";
            }
            else
                retVal="Invalid inertia matrix: not positive definite";
        }
        else
            retVal="Invalid inertia matrix: negative or too small diagonal elements";
    }
    return(retVal);
}
