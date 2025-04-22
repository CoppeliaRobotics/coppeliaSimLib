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
#include <meshRoutines.h>

CMeshWrapper::CMeshWrapper()
{
    _commonInit();
}

void CMeshWrapper::_commonInit()
{
    _parentObjectHandle = -1;
    _mass = 1.0;
    _name = "sub__0";
    _dynMaterialId_old = -1; // not used anymore since V3.4.0
    _convex_OLD = false;     // not used anymore since V4.5
    _iFrame.setIdentity();
    _bbFrame.setIdentity();
    _bbSize.clear();

    _com.clear();
    _iMatrix.clear();
    _iMatrix(0, 0) = 0.01;
    _iMatrix(1, 1) = 0.01;
    _iMatrix(2, 2) = 0.01;
    getPMIFromInertia(_iMatrix, _pmiRotFrame, _pmi);
}

CMeshWrapper::~CMeshWrapper()
{
    for (size_t i = 0; i < childList.size(); i++)
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
    std::vector<double> vert;
    std::vector<int> ind;
    getCumulativeMeshes(C7Vector::identityTransformation, vert, &ind, nullptr);
    _bbFrame = CAlgos::getMeshBoundingBox(vert, ind, false, &_bbSize);
    _computeInertiaFromComposingInertias();
}

void CMeshWrapper::_computeInertiaFromComposingInertias()
{
    double mass = 0.0;
    C3Vector com(C3Vector::zeroVector);
    for (size_t i = 0; i < childList.size(); i++)
    {
        CMeshWrapper* mesh = childList[i];
        mass += mesh->getMass();
        com = com + mesh->getIFrame() * mesh->getCOM() * mesh->getMass();
    }
    setMass(mass);
    setCOM(com / _mass);
    C3X3Matrix imatrix;
    imatrix.clear();
    for (size_t i = 0; i < childList.size(); i++)
    {
        CMeshWrapper* mesh = childList[i];
        C3X3Matrix m(getInertiaInNewFrame(mesh->getIFrame().Q, mesh->getInertia(), C4Vector::identityRotation));
        m = m * mesh->getMass();
        C3Vector d((mesh->getIFrame() * mesh->getCOM()) - _com);
        C3X3Matrix I;
        I.setIdentity();
        C3X3Matrix matr(m + I * (d * d) * mesh->getMass());
        C3X3Matrix ddt;
        for (size_t j = 0; j < 3; j++)
        {
            for (size_t k = 0; k < 3; k++)
                ddt(j, k) = d(j) * d(k);
        }
        matr = matr - (ddt * mesh->getMass());
        imatrix = imatrix + matr;
    }
    imatrix = imatrix / _mass;
    setInertiaAndComputePMI(imatrix);
}

void CMeshWrapper::display(const C7Vector& cumulIFrameTr, CShape* geomData, int displayAttrib,
                           CColorObject* collisionColor, int dynObjFlag_forVisualization, int transparencyHandling,
                           bool multishapeEditSelected)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->display(cumulIFrameTr * _iFrame, geomData, displayAttrib, collisionColor,
                              dynObjFlag_forVisualization, transparencyHandling, multishapeEditSelected);
}

void CMeshWrapper::display_extRenderer(const C7Vector& cumulIFrameTr, CShape* geomData, int displayAttrib,
                                       const C7Vector& tr, int shapeHandle, int& componentIndex)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->display_extRenderer(cumulIFrameTr * _iFrame, geomData, displayAttrib, tr, shapeHandle,
                                          componentIndex);
}

void CMeshWrapper::display_colorCoded(const C7Vector& cumulIFrameTr, CShape* geomData, int objectId, int displayAttrib)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->display_colorCoded(cumulIFrameTr * _iFrame, geomData, objectId, displayAttrib);
}

void CMeshWrapper::displayGhost(const C7Vector& cumulIFrameTr, CShape* geomData, int displayAttrib, bool originalColors,
                                bool backfaceCulling, double transparency, const float* newColors)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->displayGhost(cumulIFrameTr * _iFrame, geomData, displayAttrib, originalColors, backfaceCulling,
                                   transparency, newColors);
}

void CMeshWrapper::performSceneObjectLoadingMapping(const std::map<int, int>* map)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->performSceneObjectLoadingMapping(map);
}

void CMeshWrapper::performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->performTextureObjectLoadingMapping(map, opType);
}

void CMeshWrapper::performDynMaterialObjectLoadingMapping(const std::map<int, int>* map)
{
    _dynMaterialId_old = CWorld::getLoadingMapping(map, _dynMaterialId_old);
}

void CMeshWrapper::announceSceneObjectWillBeErased(const CSceneObject* object)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->announceSceneObjectWillBeErased(object);
}

void CMeshWrapper::setTextureDependencies(int shapeID)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->setTextureDependencies(shapeID);
}

int CMeshWrapper::getTextureCount() const
{ // function has virtual/non-virtual counterpart!
    int retVal = 0;
    for (size_t i = 0; i < childList.size(); i++)
        retVal += childList[i]->getTextureCount();
    return (retVal);
}

bool CMeshWrapper::hasTextureThatUsesFixedTextureCoordinates() const
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
    {
        if (childList[i]->hasTextureThatUsesFixedTextureCoordinates())
            return (true);
    }
    return (false);
}

void CMeshWrapper::removeAllTextures()
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->removeAllTextures();
}

void CMeshWrapper::getColorStrings(std::string& colorStrings, bool onlyNamed) const
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->getColorStrings(colorStrings, onlyNamed);
}

bool CMeshWrapper::getContainsTransparentComponents() const
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
    {
        if (childList[i]->getContainsTransparentComponents())
            return (true);
    }
    return (false);
}

double CMeshWrapper::getShadingAngle() const
{                                             // function has virtual/non-virtual counterpart!
    return (childList[0]->getShadingAngle()); // we just return the first angle we encounter! Normally never used
}

void CMeshWrapper::setShadingAngle(double angle)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->setShadingAngle(angle);
}

void CMeshWrapper::setCulling(bool c)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->setCulling(c);
}

void CMeshWrapper::setVisibleEdges(bool v)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->setVisibleEdges(v);
}

double CMeshWrapper::getEdgeThresholdAngle() const
{                                                   // function has virtual/non-virtual counterpart!
    return (childList[0]->getEdgeThresholdAngle()); // we just return the first angle we encounter! Normally never used
}

void CMeshWrapper::setEdgeThresholdAngle(double angle)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->setEdgeThresholdAngle(angle);
}

void CMeshWrapper::setHideEdgeBorders_OLD(bool v)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->setHideEdgeBorders_OLD(v);
}

bool CMeshWrapper::getHideEdgeBorders_OLD() const
{ // function has virtual/non-virtual counterpart!
    return (
        childList[0]->getHideEdgeBorders_OLD()); // we just return the first setting we encounter! Normally never used
}

CMeshWrapper* CMeshWrapper::copyYourself()
{ // function has virtual/non-virtual counterpart!
    CMeshWrapper* newIt = new CMeshWrapper();
    copyWrapperData(newIt);
    return (newIt);
}

void CMeshWrapper::copyAttributesTo(CMeshWrapper* target)
{
    target->_mass = _mass;
    target->_name = _name;
    target->_iFrame = _iFrame;
    target->_com = _com;
    target->_iMatrix = _iMatrix;
    target->_pmi = _pmi;
    target->_pmiRotFrame = _pmiRotFrame;
    target->_dynMaterialId_old = _dynMaterialId_old;
    if (childList.size() == 0)
        target->takeVisualAttributesFrom((CMesh*)this);
    else
        target->takeVisualAttributesFrom(getFirstMesh());
}

void CMeshWrapper::copyWrapperData(CMeshWrapper* target)
{
    target->_mass = _mass;
    target->_name = _name;

    target->_iFrame = _iFrame;
    target->_com = _com;
    target->_iMatrix = _iMatrix;
    target->_pmi = _pmi;
    target->_pmiRotFrame = _pmiRotFrame;
    target->_bbFrame = _bbFrame;
    target->_bbSize = _bbSize;

    target->_dynMaterialId_old = _dynMaterialId_old;

    for (size_t i = 0; i < childList.size(); i++)
        target->childList.push_back(childList[i]->copyYourself());
}

void CMeshWrapper::setMass(double m)
{
    m = tt::getLimitedFloat(0.000000001, 100000.0, m);
    if (m != _mass)
    {
        _mass = m;
        if ((_parentObjectHandle >= 0) && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propMeshWrap_mass.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(_parentObjectHandle, false, cmd, true);
            ev->appendKeyDouble(cmd, _mass);
            App::worldContainer->pushEvent();
        }
    }
}

double CMeshWrapper::getMass() const
{
    return _mass;
}

void CMeshWrapper::setName(std::string newName)
{
    _name = newName;
}

std::string CMeshWrapper::getName() const
{
    return (_name);
}

int CMeshWrapper::getDynMaterialId_old() const
{
    return (_dynMaterialId_old);
}

void CMeshWrapper::setDynMaterialId_old(int id)
{
    _dynMaterialId_old = id;
}

C7Vector CMeshWrapper::getDiagonalInertiaInfo(C3Vector& diagMasslessI) const
{
    C7Vector retVal;
    retVal.X = _com;
    retVal.Q = _pmiRotFrame;
    diagMasslessI = _pmi;
    return (retVal);
}

C7Vector CMeshWrapper::getBB(C3Vector* optBBSize) const
{
    if (optBBSize != nullptr)
        optBBSize[0] = _bbSize;
    return (_bbFrame);
}

void CMeshWrapper::setBBFrame(const C7Vector& bbFrame)
{ // function has virtual/non-virtual counterpart!
    _bbFrame = bbFrame;
}

bool CMeshWrapper::reorientBB(const C4Vector* rot)
{ // function has virtual/non-virtual counterpart!
    bool retVal = false;
    if ((!isMesh()) || (!isPure()))
    {
        std::vector<double> vertices;
        if (rot != nullptr)
        {
            C7Vector tr;
            tr.X.clear();
            tr.Q = rot[0];
            getCumulativeMeshes(tr.getInverse(), vertices, nullptr, nullptr);
            C3Vector mmin(DBL_MAX, DBL_MAX, DBL_MAX);
            C3Vector mmax(-DBL_MAX, -DBL_MAX, -DBL_MAX);
            for (size_t i = 0; i < vertices.size() / 3; i++)
            {
                C3Vector v(vertices.data() + 3 * i);
                mmin.keepMin(v);
                mmax.keepMax(v);
            }
            _bbSize = mmax - mmin;
            C7Vector bbc;
            bbc.Q.setIdentity();
            bbc.X = (mmax + mmin) * 0.5;
            _bbFrame = tr * bbc;
        }
        else
        {
            std::vector<int> indices;
            getCumulativeMeshes(C7Vector::identityTransformation, vertices, &indices, nullptr);
            _bbFrame = CAlgos::getMeshBoundingBox(vertices, indices, true);
            C7Vector inv(_bbFrame.getInverse());

            C3Vector mmin(DBL_MAX, DBL_MAX, DBL_MAX);
            C3Vector mmax(-DBL_MAX, -DBL_MAX, -DBL_MAX);
            for (size_t i = 0; i < vertices.size() / 3; i++)
            {
                C3Vector v(vertices.data() + 3 * i);
                v = inv * v;
                mmin.keepMin(v);
                mmax.keepMax(v);
            }
            _bbSize = mmax - mmin;
        }
        retVal = true;
    }
    return (retVal);
}

bool CMeshWrapper::getShapeRelIFrame(const C7Vector& parentCumulTr, const CMeshWrapper* wrapper,
                                     C7Vector& shapeRelIFrame) const
{
    bool retVal = false;
    if (wrapper == this)
    {
        retVal = true;
        shapeRelIFrame = parentCumulTr * _iFrame;
    }
    else
    {
        for (size_t i = 0; i < childList.size(); i++)
        {
            if (childList[i]->getShapeRelIFrame(parentCumulTr * _iFrame, wrapper, shapeRelIFrame))
            {
                retVal = true;
                break;
            }
        }
    }
    return (retVal);
}

bool CMeshWrapper::getShapeRelBB(const C7Vector& parentCumulTr, const CMeshWrapper* wrapper, C7Vector& shapeRelBB,
                                 C3Vector* optBBSize) const
{
    C7Vector ifr;
    bool retVal = getShapeRelIFrame(parentCumulTr, wrapper, ifr);
    if (retVal)
        shapeRelBB = ifr * wrapper->getBB(optBBSize);
    return (retVal);
}

C3Vector CMeshWrapper::getCOM() const
{
    return _com;
}

C7Vector CMeshWrapper::getIFrame() const
{
    return _iFrame;
}

void CMeshWrapper::setIFrame(const C7Vector& iframe)
{
    _iFrame = iframe;
}

void CMeshWrapper::setCOM(const C3Vector& com)
{
    if (_com != com)
    {
        _com = com;
        if ((_parentObjectHandle >= 0) && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propMeshWrap_com.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(_parentObjectHandle, false, cmd, true);
            ev->appendKeyDoubleArray(cmd, _com.data, 3);
            App::worldContainer->pushEvent();
        }
    }
}

C3X3Matrix CMeshWrapper::getInertia() const
{
    return _iMatrix;
}

void CMeshWrapper::setInertia(const C3X3Matrix& im, int modifItemRow /*=-1*/, int modifItemCol /*=-1*/)
{
    C3X3Matrix imatrix(im);
    if ((modifItemRow != -1) && (modifItemCol != -1))
        imatrix(modifItemCol, modifItemRow) = imatrix(modifItemRow, modifItemCol);
    setInertiaAndComputePMI(imatrix);
}

void CMeshWrapper::setInertiaAndComputePMI(const C3X3Matrix& inertia, bool forcePMICalc /*= false*/)
{
    C3X3Matrix _in(inertia);
    for (size_t i = 0; i < 3; i++)
    {
        // Make sure we are symmetric;
        for (size_t j = 0; j < 3; j++)
            _in(i, j) = _in(j, i);
        // Make sure diagonals are positive and above a certain threshold:
        if (_in(i, i) < 1e-8)
            _in(i, i) = 1e-8;
    }
    if (_in != _iMatrix)
    {
        _iMatrix = _in;
        getPMIFromInertia(_iMatrix, _pmiRotFrame, _pmi);
        if ((_parentObjectHandle >= 0) && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propMeshWrap_inertia.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(_parentObjectHandle, false, cmd, true);
            double dat[9];
            _in *= _mass;
            _in.getData(dat);
            ev->appendKeyDoubleArray(cmd, dat, 9);
            C3Vector pmi(_pmi * _mass);
            ev->appendKeyDoubleArray(propMeshWrap_pmi.name, pmi.data, 3);
            _pmiRotFrame.getData(dat, true);
            ev->appendKeyDoubleArray(propMeshWrap_pmiQuaternion.name, dat, 4);
            App::worldContainer->pushEvent();
        }
    }
    else if (forcePMICalc)
        getPMIFromInertia(_iMatrix, _pmiRotFrame, _pmi);
}

C3Vector CMeshWrapper::getPMI() const
{
    return _pmi;
}

void CMeshWrapper::setPMI(const C3Vector& pmi)
{
    C3X3Matrix im;
    im.clear();
    im(0, 0) = pmi(0);
    im(1, 1) = pmi(1);
    im(2, 2) = pmi(2);
    setInertia(im);
}

void CMeshWrapper::prepareVerticesIndicesNormalsAndEdgesForSerialization()
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->prepareVerticesIndicesNormalsAndEdgesForSerialization();
}

void CMeshWrapper::scaleMassAndInertia(double s)
{
    setMass(_mass * s * s * s);
    setInertiaAndComputePMI(_iMatrix * s * s);
}

void CMeshWrapper::scale(double isoVal)
{ // function has virtual/non-virtual counterpart
    scaleMassAndInertia(isoVal);
    setCOM(_com * isoVal);
    _iFrame.X *= isoVal;
    _bbFrame.X *= isoVal;
    _bbSize *= isoVal;

    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->scale(isoVal);
}

void CMeshWrapper::setPurePrimitiveType(int theType, double xOrDiameter, double y, double zOrHeight)
{ // function has virtual/non-virtual counterpart!
    // Following added on 14/03/2011 because a compound shape composed by pure and non pure shapes would decompose as
    // pure shapes with wrong orientation!
    if (theType == sim_primitiveshape_none)
    {
        for (size_t i = 0; i < childList.size(); i++)
            childList[i]->setPurePrimitiveType(theType, xOrDiameter, y, zOrHeight);
    }
}

int CMeshWrapper::getPurePrimitiveType() const
{                                                  // function has virtual/non-virtual counterpart!
    return (childList[0]->getPurePrimitiveType()); // we just return the first type we encounter! Normally never used
}

bool CMeshWrapper::isMesh() const
{ // function has virtual/non-virtual counterpart!
    return (false);
}

bool CMeshWrapper::isPure() const
{ // function has virtual/non-virtual counterpart!
    return (childList[0]->isPure());
}

bool CMeshWrapper::isConvex() const
{ // function has virtual/non-virtual counterpart!
    bool retVal = true;
    for (size_t i = 0; i < childList.size(); i++)
        retVal = retVal && childList[i]->isConvex();
    return (retVal);
}

CMesh* CMeshWrapper::getFirstMesh()
{ // function has virtual/non-virtual counterpart!
    return (childList[0]->getFirstMesh());
}

CMesh* CMeshWrapper::getMeshFromUid(long long int meshUid, const C7Vector& parentCumulTr, C7Vector& shapeRelTr)
{
    CMesh* retVal = nullptr;
    for (size_t i = 0; i < childList.size(); i++)
    {
        retVal = childList[i]->getMeshFromUid(meshUid, parentCumulTr * _iFrame, shapeRelTr);
        if (retVal != nullptr)
            break;
    }
    return retVal;
}

void CMeshWrapper::takeVisualAttributesFrom(CMesh* origin)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->takeVisualAttributesFrom(origin);
}

int CMeshWrapper::countTriangles() const
{ // function has virtual/non-virtual counterpart!
    int retVal = 0;
    for (size_t i = 0; i < childList.size(); i++)
        retVal += childList[i]->countTriangles();
    return (retVal);
}

void CMeshWrapper::getCumulativeMeshes(const C7Vector& parentCumulTr, std::vector<double>& vertices,
                                       std::vector<int>* indices, std::vector<double>* normals)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->getCumulativeMeshes(parentCumulTr * _iFrame, vertices, indices, normals);
}

void CMeshWrapper::getCumulativeMeshes(const C7Vector& parentCumulTr, const CMeshWrapper* wrapper,
                                       std::vector<double>& vertices, std::vector<int>* indices,
                                       std::vector<double>* normals)
{ // function has virtual/non-virtual counterpart!
    const CMeshWrapper* w = wrapper;
    if (wrapper == this)
        w = nullptr;
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->getCumulativeMeshes(parentCumulTr * _iFrame, w, vertices, indices, normals);
}

void CMeshWrapper::setColor(int colorComponent, const float* rgbData)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->setColor(colorComponent, rgbData);
}

void CMeshWrapper::setColor(const CShape* shape, int& elementIndex, const char* colorName, int colorComponent,
                            const float* rgbData, int& rgbDataOffset)
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->setColor(shape, elementIndex, colorName, colorComponent, rgbData, rgbDataOffset);
}

bool CMeshWrapper::getColor(const char* colorName, int colorComponent, float* rgbData, int& rgbDataOffset) const
{ // function has virtual/non-virtual counterpart!
    bool retVal = false;
    for (size_t i = 0; i < childList.size(); i++)
        retVal = childList[i]->getColor(colorName, colorComponent, rgbData, rgbDataOffset) || retVal;
    return (retVal);
}

void CMeshWrapper::getAllMeshComponentsCumulative(const C7Vector& parentCumulTr,
                                                  std::vector<CMesh*>& shapeComponentList,
                                                  std::vector<C7Vector>* OptParentCumulTrList /*=nullptr*/)
{ // function has virtual/non-virtual counterpart!
    // needed by the dynamics routine. We return ALL shape components!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->getAllMeshComponentsCumulative(parentCumulTr * _iFrame, shapeComponentList, OptParentCumulTrList);
}

CMesh* CMeshWrapper::getMeshComponentAtIndex(const C7Vector& parentCumulTr, int& index,
                                             C7Vector* optParentCumulTrOut /*=nullptr*/)
{ // function has virtual/non-virtual counterpart!
    CMesh* retVal = nullptr;
    if (index >= 0)
    {
        for (size_t i = 0; i < childList.size(); i++)
        {
            retVal = childList[i]->getMeshComponentAtIndex(parentCumulTr * _iFrame, index, optParentCumulTrOut);
            if (retVal != nullptr)
                break;
        }
    }
    return (retVal);
}

int CMeshWrapper::getComponentCount() const
{ // function has virtual/non-virtual counterpart!
    int retVal = 0;
    for (size_t i = 0; i < childList.size(); i++)
        retVal += childList[i]->getComponentCount();
    return (retVal);
}

void CMeshWrapper::flipFaces()
{ // function has virtual/non-virtual counterpart!
    for (size_t i = 0; i < childList.size(); i++)
        childList[i]->flipFaces();
}

bool CMeshWrapper::serialize(CSer& ar, const char* shapeName, const C7Vector& parentCumulIFrame, bool rootLevel)
{ // function has virtual/non-virtual counterpart!
    bool isNewTypeOfShapeFormat = false;
    if (rootLevel)
        _iFrame.setIdentity();
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Nme");
            ar << _name;
            ar.flush();

            ar.storeDataName("_as");
            ar << _mass;
            ar.flush();

            ar.storeDataName("Dmi");
            ar << _dynMaterialId_old;
            ar.flush();

            ar.storeDataName("_ne"); // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
            C7Vector w(parentCumulIFrame * _iFrame);
            w.X += _com;
            w.Q = w.Q * _pmiRotFrame;
            ar << w(0) << w(1) << w(2);
            ar << w(3) << w(4) << w(5) << w(6);
            ar << _pmi(0) << _pmi(1) << _pmi(2);
            ar.flush();

            ar.storeDataName("_tb"); // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
            w = parentCumulIFrame * _iFrame;
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
            for (size_t i = 0; i < 3; i++)
            {
                for (size_t j = 0; j < 3; j++)
                    ar << _iMatrix(i, j);
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
            unsigned char nothing = 0;
            SIM_SET_CLEAR_BIT(nothing, 2, isConvex()); // for backward compatibility
            SIM_SET_CLEAR_BIT(nothing, 5, true);
            ar << nothing;
            ar.flush();

            for (size_t i = 0; i < childList.size(); i++)
            {
                if (childList[i]->isMesh())
                    ar.storeDataName("Geo");
                else
                    ar.storeDataName("Wrp");
                ar.setCountingMode();
                childList[i]->serialize(ar, shapeName, parentCumulIFrame * _iFrame, false);
                if (ar.setWritingMode())
                    childList[i]->serialize(ar, shapeName, parentCumulIFrame * _iFrame, false);
            }

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            C3Vector principalMomentsOfInertia_OLD;
            C7Vector localInertiaFrame_OLD;
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Nme") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _name;
                    }
                    if (theName.compare("Mas") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _mass = (double)bla;
                        if (_mass == 0.0) // to catch an old bug
                            _mass = 0.001;
                    }
                    if (theName.compare("_as") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _mass;
                        if (_mass == 0.0) // to catch an old bug
                            _mass = 0.001;
                    }

                    if (theName.compare("Dmi") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _dynMaterialId_old;
                    }
                    if (theName.compare("Ine") == 0) // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                    {                                // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        for (size_t i = 0; i < 7; i++)
                        {
                            ar >> bla;
                            localInertiaFrame_OLD(i) = (double)bla;
                        }
                        localInertiaFrame_OLD.Q.normalize(); // we read from float. Make sure we are perfectly normalized!
                        for (size_t i = 0; i < 3; i++)
                        {
                            ar >> bla;
                            principalMomentsOfInertia_OLD(i) = (double)bla;
                        }
                    }

                    if (theName.compare("_ne") == 0) // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> localInertiaFrame_OLD(0) >> localInertiaFrame_OLD(1) >> localInertiaFrame_OLD(2) >>
                            localInertiaFrame_OLD(3);
                        ar >> localInertiaFrame_OLD(4) >> localInertiaFrame_OLD(5) >> localInertiaFrame_OLD(6);
                        localInertiaFrame_OLD.Q.normalize();
                        ar >> principalMomentsOfInertia_OLD(0) >> principalMomentsOfInertia_OLD(1) >>
                            principalMomentsOfInertia_OLD(2);
                    }

                    if (theName.compare("Vtb") == 0) // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                    {                                // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        C7Vector transformationsSinceGrouping_OLD;
                        float bla;
                        for (size_t i = 0; i < 7; i++)
                        {
                            ar >> bla;
                            transformationsSinceGrouping_OLD(i) = (double)bla;
                        }
                        transformationsSinceGrouping_OLD.Q
                            .normalize(); // we read from float. Make sure we are perfectly normalized!
                        if (rootLevel)
                            transformationsSinceGrouping_OLD.setIdentity();
                        _iFrame = parentCumulIFrame.getInverse() * transformationsSinceGrouping_OLD;
                        C7Vector inf(transformationsSinceGrouping_OLD.getInverse() * localInertiaFrame_OLD);
                        _com = inf.X;
                        inf.X.clear();
                        _iMatrix = getInertiaFromPMI(principalMomentsOfInertia_OLD, inf);
                        setInertiaAndComputePMI(_iMatrix, true);
                    }

                    if (theName.compare("_tb") == 0) // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                    {
                        noHit = false;
                        C7Vector transformationsSinceGrouping_OLD;
                        ar >> byteQuantity;
                        ar >> transformationsSinceGrouping_OLD(0) >> transformationsSinceGrouping_OLD(1) >>
                            transformationsSinceGrouping_OLD(2) >> transformationsSinceGrouping_OLD(3);
                        ar >> transformationsSinceGrouping_OLD(4) >> transformationsSinceGrouping_OLD(5) >>
                            transformationsSinceGrouping_OLD(6);
                        transformationsSinceGrouping_OLD.Q.normalize();
                        if (rootLevel)
                            transformationsSinceGrouping_OLD.setIdentity();
                        _iFrame = parentCumulIFrame.getInverse() * transformationsSinceGrouping_OLD;
                        C7Vector inf(transformationsSinceGrouping_OLD.getInverse() * localInertiaFrame_OLD);
                        _com = inf.X;
                        inf.X.clear();
                        _iMatrix = getInertiaFromPMI(principalMomentsOfInertia_OLD, inf);
                        setInertiaAndComputePMI(_iMatrix, true);
                    }

                    if (theName.compare("Ifr") == 0)
                    {
                        isNewTypeOfShapeFormat = true;
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _iFrame(0) >> _iFrame(1) >> _iFrame(2);
                        ar >> _iFrame(3) >> _iFrame(4) >> _iFrame(5) >> _iFrame(6);
                        if (rootLevel)
                            _iFrame.setIdentity();
                    }
                    if (theName.compare("Com") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _com(0) >> _com(1) >> _com(2);
                    }
                    if (theName.compare("Imx") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        for (size_t i = 0; i < 3; i++)
                        {
                            for (size_t j = 0; j < 3; j++)
                                ar >> _iMatrix(i, j);
                        }
                        setInertiaAndComputePMI(_iMatrix, true);
                    }
                    if (theName.compare("Bbf") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _bbFrame(0) >> _bbFrame(1) >> _bbFrame(2);
                        ar >> _bbFrame(3) >> _bbFrame(4) >> _bbFrame(5) >> _bbFrame(6);
                    }
                    if (theName.compare("Bbs") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _bbSize(0) >> _bbSize(1) >> _bbSize(2);
                    }

                    if (theName == "Var")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _convex_OLD = SIM_IS_BIT_SET(nothing, 2);
                    }
                    if (theName.compare("Geo") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        CMesh* it = new CMesh();
                        it->serialize(ar, shapeName, parentCumulIFrame * _iFrame, false);
                        childList.push_back(it);
                    }
                    if (theName.compare("Wrp") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        CMeshWrapper* it = new CMeshWrapper();
                        it->serialize(ar, shapeName, parentCumulIFrame * _iFrame, false);
                        childList.push_back(it);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if ((!isNewTypeOfShapeFormat) && (childList.size() != 0))
            {
                std::vector<double> vert;
                std::vector<int> ind;
                getCumulativeMeshes(C7Vector::identityTransformation, vert, &ind, nullptr);
                _bbFrame = CAlgos::getMeshBoundingBox(vert, ind, false, &_bbSize);
            }
        }
    }
    else
    {
        if (ar.isStoring())
        {
            ar.xmlPushNewNode("common");

            ar.xmlAddNode_string("name", _name.c_str());

            ar.xmlAddNode_comment(" 'transformationSinceGrouping' tag: deprecated, for backward compatibility ", false);
            ar.xmlPushNewNode("transformationSinceGrouping"); // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
            C7Vector w(parentCumulIFrame * _iFrame);
            ar.xmlAddNode_floats("position", w.X.data, 3);
            ar.xmlAddNode_floats("quaternion", w.Q.data, 4);
            ar.xmlPopNode();

            ar.xmlPushNewNode("dynamics");

            ar.xmlAddNode_float("mass", _mass);

            ar.xmlAddNode_comment(" 'localInertiaFrame' tag: deprecated, for backward compatibility ", false);
            ar.xmlPushNewNode("localInertiaFrame"); // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
            w = parentCumulIFrame * _iFrame;
            w.X += _com;
            w.Q = w.Q * _pmiRotFrame;
            ar.xmlAddNode_floats("position", w.X.data, 3);
            ar.xmlAddNode_floats("quaternion", w.Q.data, 4);
            ar.xmlAddNode_floats("principalMomentsOfInertia", _pmi.data, 3);
            ar.xmlPopNode();

            ar.xmlPushNewNode("inertiaFrame");
            ar.xmlAddNode_floats("position", _iFrame.X.data, 3);
            ar.xmlAddNode_floats("quaternion", _iFrame.Q.data, 4);
            ar.xmlPopNode();

            ar.xmlAddNode_floats("centerOfMass", _com.data, 3);

            double im[9];
            for (size_t i = 0; i < 3; i++)
            {
                for (size_t j = 0; j < 3; j++)
                    im[i * 3 + j] = _iMatrix(i, j);
            }
            ar.xmlAddNode_floats("inertia", im, 9);

            ar.xmlPushNewNode("bbFrame");
            ar.xmlAddNode_floats("position", _bbFrame.X.data, 3);
            ar.xmlAddNode_floats("quaternion", _bbFrame.Q.data, 4);
            ar.xmlPopNode();

            ar.xmlAddNode_floats("bbSize", _bbSize.data, 3);
            ar.xmlPopNode(); // "dynamics" node

            for (size_t i = 0; i < childList.size(); i++)
            {
                ar.xmlPushNewNode("child");
                if (childList[i]->isMesh())
                    ar.xmlPushNewNode("mesh");
                else
                    ar.xmlPushNewNode("compound");
                childList[i]->serialize(ar, shapeName, parentCumulIFrame * _iFrame, false);
                ar.xmlPopNode();
                ar.xmlPopNode();
            }
            ar.xmlPopNode();
        }
        else
        {
            if (ar.xmlPushChildNode("common"))
            {
                ar.xmlGetNode_string("name", _name);

                C7Vector transformationsSinceGrouping_OLD;
                if (ar.xmlPushChildNode("transformationSinceGrouping",
                                        false)) // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                {
                    ar.xmlGetNode_floats("position", transformationsSinceGrouping_OLD.X.data, 3);
                    ar.xmlGetNode_floats("quaternion", transformationsSinceGrouping_OLD.Q.data, 4);
                    if (rootLevel)
                        transformationsSinceGrouping_OLD.setIdentity();
                    transformationsSinceGrouping_OLD.Q.normalize(); // just in case
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("dynamics"))
                {
                    ar.xmlGetNode_float("mass", _mass);

                    if (ar.xmlPushChildNode("localInertiaFrame",
                                            false)) // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                    {
                        C7Vector localInertiaFrame_OLD;
                        ar.xmlGetNode_floats("position", localInertiaFrame_OLD.X.data, 3);
                        ar.xmlGetNode_floats("quaternion", localInertiaFrame_OLD.Q.data, 4);
                        localInertiaFrame_OLD.Q.normalize(); // just in case
                        C3Vector principalMomentsOfInertia_OLD;
                        ar.xmlGetNode_floats("principalMomentsOfInertia", principalMomentsOfInertia_OLD.data, 3);
                        ar.xmlPopNode();
                        _iFrame = parentCumulIFrame.getInverse() * transformationsSinceGrouping_OLD;
                        C7Vector inf(transformationsSinceGrouping_OLD.getInverse() * localInertiaFrame_OLD);
                        _com = inf.X;
                        inf.X.clear();
                        _iMatrix = getInertiaFromPMI(principalMomentsOfInertia_OLD, inf);
                        setInertiaAndComputePMI(_iMatrix, true);
                    }

                    if (ar.xmlPushChildNode("inertiaFrame"))
                    {
                        isNewTypeOfShapeFormat = true;
                        ar.xmlGetNode_floats("position", _iFrame.X.data, 3);
                        ar.xmlGetNode_floats("quaternion", _iFrame.Q.data, 4);
                        _iFrame.Q.normalize(); // just in case
                        if (rootLevel)
                            _iFrame.setIdentity();
                        ar.xmlPopNode();
                    }

                    ar.xmlGetNode_floats("centerOfMass", _com.data, 3);

                    double im[9];
                    if (ar.xmlGetNode_floats("inertia", im, 9))
                    {
                        for (size_t i = 0; i < 3; i++)
                        {
                            for (size_t j = 0; j < 3; j++)
                                _iMatrix(i, j) = im[i * 3 + j];
                        }
                        setInertiaAndComputePMI(_iMatrix, true);
                    }

                    if (ar.xmlPushChildNode("bbFrame"))
                    {
                        ar.xmlGetNode_floats("position", _bbFrame.X.data, 3);
                        ar.xmlGetNode_floats("quaternion", _bbFrame.Q.data, 4);
                        _bbFrame.Q.normalize(); // just in case
                        ar.xmlPopNode();
                    }

                    ar.xmlGetNode_floats("bbSize", _bbSize.data, 3);

                    ar.xmlPopNode(); // "dynamics" node
                }

                if (ar.xmlPushChildNode("switches", false)) // deprecated, old shapes (prior to CoppeliaSim V4.5 rev2)
                {
                    ar.xmlGetNode_bool("convex", _convex_OLD);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("child", false))
                {
                    while (true)
                    {
                        if (ar.xmlPushChildNode("mesh", false))
                        {
                            CMesh* it = new CMesh();
                            it->serialize(ar, shapeName, parentCumulIFrame * _iFrame, false);
                            childList.push_back(it);
                            ar.xmlPopNode();
                        }
                        else
                        {
                            if (ar.xmlPushChildNode("compound"))
                            {
                                CMeshWrapper* it = new CMeshWrapper();
                                it->serialize(ar, shapeName, parentCumulIFrame * _iFrame, false);
                                childList.push_back(it);
                                ar.xmlPopNode();
                            }
                        }
                        if (!ar.xmlPushSiblingNode("child", false))
                            break;
                    }
                    ar.xmlPopNode();
                }
                ar.xmlPopNode();
            }
            if ((!isNewTypeOfShapeFormat) && (childList.size() != 0))
            {
                std::vector<double> vert;
                std::vector<int> ind;
                getCumulativeMeshes(C7Vector::identityTransformation, vert, &ind, nullptr);
                _bbFrame = CAlgos::getMeshBoundingBox(vert, ind, false, &_bbSize);
            }
        }
    }
    return (isNewTypeOfShapeFormat);
}

bool CMeshWrapper::getPMIFromInertia(const C3X3Matrix& tensor, C4Vector& rotation, C3Vector& principalMoments)
{ // tensor --> PMI + rotation (tensor and PMI are mass-less)
    Eigen::Matrix3d m;
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t j = 0; j < 3; j++)
            m(i, j) = tensor(j, i);
    }
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eigensolver(m);
    eigensolver.compute(m);
    Eigen::Vector3d a = eigensolver.eigenvalues();
    Eigen::Matrix3d v = eigensolver.eigenvectors();
    int idx[3] = {0, 1, 2};
    for (size_t i = 0; i < 3; i++)
        principalMoments(i) = a(i);
    principalMoments.sort(idx);
    C3X3Matrix r;
    for (size_t i = 0; i < 3; i++)
    {
        r.axis[0](i) = v.col(idx[0]).normalized()(i);
        r.axis[1](i) = v.col(idx[1]).normalized()(i);
    }
    r.axis[2] = r.axis[0] ^ r.axis[1];
    rotation = r.getQuaternion();
    return ((principalMoments(0) > 0.0) && (principalMoments(1) > 0.0) && (principalMoments(2) > 0.0));
}

C3X3Matrix CMeshWrapper::getInertiaFromPMI(const C3Vector& principalMoments, const C7Vector& newFrame)
{ // PMI + transf --> tensor (tensor and PMI are mass-less)
    C3X3Matrix tensor;
    tensor.clear();
    tensor.axis[0](0) = principalMoments(0);
    tensor.axis[1](1) = principalMoments(1);
    tensor.axis[2](2) = principalMoments(2);
    // 1. reorient the frame:
    tensor = getInertiaInNewFrame(newFrame.Q, tensor, C4Vector::identityRotation);
    // 2. shift the frame:
    C3X3Matrix D;
    D.setIdentity();
    D *= (newFrame.X * newFrame.X);
    D.axis[0] -= newFrame.X * newFrame.X(0);
    D.axis[1] -= newFrame.X * newFrame.X(1);
    D.axis[2] -= newFrame.X * newFrame.X(2);
    tensor += D;
    return (tensor);
}

C3X3Matrix CMeshWrapper::getInertiaInNewFrame(const C4Vector& oldFrame, const C3X3Matrix& oldMatrix,
                                              const C4Vector& newFrame)
{
    C3X3Matrix rot(oldFrame.getMatrix().getTranspose() * newFrame.getMatrix());
    C3X3Matrix retVal(rot.getTranspose() * oldMatrix * rot);
    return (retVal);
}

std::string CMeshWrapper::getInertiaErrorString() const
{
    return (getInertiaErrorString(_iMatrix));
}

std::string CMeshWrapper::getInertiaErrorString(const C3X3Matrix& matrix)
{
    std::string retVal = "";
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
            if (matrix(i, j) != matrix(j, i))
            {
                retVal = "Invalid inertia matrix: not symmetric";
                break;
            }
        }
    }
    if (retVal == "")
    {
        if ((matrix(0, 0) > 1e-8) && (matrix(1, 1) > 1e-8) && (matrix(2, 2) > 1e-8))
        {
            C4Vector dummyRot;
            C3Vector pmi;
            if (getPMIFromInertia(matrix, dummyRot, pmi))
            { // positive definite: eigenvals are positive
                if ((pmi(0) + pmi(1) < pmi(2)) || (pmi(0) + pmi(2) < pmi(1)) || (pmi(1) + pmi(2) < pmi(0)))
                    retVal = "Invalid/unstable inertia matrix: A+B<C";
            }
            else
                retVal = "Invalid inertia matrix: not positive definite";
        }
        else
            retVal = "Invalid inertia matrix: negative or too small diagonal elements";
    }
    return (retVal);
}

void CMeshWrapper::addSpecializedObjectEventData(int parentObjectHandle, CCbor* ev)
{
    _parentObjectHandle = parentObjectHandle;

    if (_parentObjectHandle >= 0)
    {
        ev->appendKeyDouble(propMeshWrap_mass.name, _mass);
        ev->appendKeyDoubleArray(propMeshWrap_com.name, _com.data, 3);
        C3X3Matrix inertia(_iMatrix * _mass);
        double dat[9];
        inertia.getData(dat);
        ev->appendKeyDoubleArray(propMeshWrap_inertia.name, dat, 9);
        C3Vector pmi(_pmi * _mass);
        ev->appendKeyDoubleArray(propMeshWrap_pmi.name, pmi.data, 3);
        _pmiRotFrame.getData(dat, true);
        ev->appendKeyDoubleArray(propMeshWrap_pmiQuaternion.name, dat, 4);
    }
}

int CMeshWrapper::setFloatProperty_wrapper(const char* pName, double pState)
{
    int retVal = -1;

    if (strcmp(propMeshWrap_mass.name, pName) == 0)
    {
        retVal = 1;
        setMass(pState);
    }

    return retVal;
}

int CMeshWrapper::getFloatProperty_wrapper(const char* pName, double& pState) const
{
    int retVal = -1;

    if (strcmp(propMeshWrap_mass.name, pName) == 0)
    {
        retVal = 1;
        pState = _mass;
    }

    return retVal;
}

int CMeshWrapper::setVector3Property_wrapper(const char* pName, const C3Vector& pState)
{
    int retVal = -1;

    if (strcmp(propMeshWrap_com.name, pName) == 0)
    {
        retVal = 1;
        setCOM(pState);
    }

    return retVal;
}

int CMeshWrapper::getVector3Property_wrapper(const char* pName, C3Vector& pState) const
{
    int retVal = -1;

    if (strcmp(propMeshWrap_com.name, pName) == 0)
    {
        retVal = 1;
        pState = _com;
    }

    return retVal;
}

int CMeshWrapper::setQuaternionProperty_wrapper(const char* pName, const C4Vector& pState)
{
    int retVal = -1;

    return retVal;
}

int CMeshWrapper::getQuaternionProperty_wrapper(const char* pName, C4Vector& pState) const
{
    int retVal = -1;

    if (strcmp(propMeshWrap_pmiQuaternion.name, pName) == 0)
    {
        retVal = 1;
        pState = _pmiRotFrame;
    }

    return retVal;
}

int CMeshWrapper::setFloatArrayProperty_wrapper(const char* pName, const double* v, int vL)
{
    int retVal = -1;

    if (strcmp(propMeshWrap_inertia.name, pName) == 0)
    {
        if (vL >= 9)
        {
            retVal = 1;
            C3X3Matrix m;
            m.setData(v);
            setInertiaAndComputePMI(m);
        }
        else
            retVal = 0;
    }

    return retVal;
}

int CMeshWrapper::getFloatArrayProperty_wrapper(const char* pName, std::vector<double>& pState) const
{
    int retVal = -1;
    pState.clear();

    if (strcmp(propMeshWrap_inertia.name, pName) == 0)
    {
        pState.resize(9, 0.0);
        _iMatrix.getData(pState.data());
        retVal = 1;
    }
    else if (strcmp(propMeshWrap_pmi.name, pName) == 0)
    {
        retVal = 1;
        pState.push_back(_pmi(0));
        pState.push_back(_pmi(1));
        pState.push_back(_pmi(2));
    }

    return retVal;
}

int CMeshWrapper::getPropertyName_wrapper(int& index, std::string& pName) const
{
    int retVal = getPropertyName_static_wrapper(index, pName);
    return retVal;
}

int CMeshWrapper::getPropertyName_static_wrapper(int& index, std::string& pName)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_meshWrap.size(); i++)
    {
        if ((pName.size() == 0) || utils::startsWith(allProps_meshWrap[i].name, pName.c_str()))
        {
            index--;
            if (index == -1)
            {
                pName = allProps_meshWrap[i].name;
                retVal = 1;
                break;
            }
        }
    }
    return retVal;
}

int CMeshWrapper::getPropertyInfo_wrapper(const char* pName, int& info, std::string& infoTxt) const
{
    int retVal = getPropertyInfo_static_wrapper(pName, info, infoTxt);
    return retVal;
}

int CMeshWrapper::getPropertyInfo_static_wrapper(const char* pName, int& info, std::string& infoTxt)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_meshWrap.size(); i++)
    {
        if (strcmp(allProps_meshWrap[i].name, pName) == 0)
        {
            retVal = allProps_meshWrap[i].type;
            info = allProps_meshWrap[i].flags;
            if ((infoTxt == "") && (strcmp(allProps_meshWrap[i].infoTxt, "") != 0))
                infoTxt = allProps_meshWrap[i].infoTxt;
            else
                infoTxt = allProps_meshWrap[i].shortInfoTxt;
            break;
        }
    }
    return retVal;
}
