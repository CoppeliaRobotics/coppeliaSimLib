#include <simInternal.h>
#include <simStrings.h>
#include <utils.h>
#include <marker.h>
#include <global.h>
#include <app.h>
#include <tt.h>
#ifdef SIM_WITH_GUI
#include <markerRendering.h>
#include <guiApp.h>
#endif

#define MARKER_MAX_ID 1000000

static std::string OBJECT_META_INFO = R"(
{
    "superclass": "sceneObject",
    "namespaces": {
        "refs": {"newPropertyForcedType": "sim.propertytype_handlearray"},
        "origRefs": {"newPropertyForcedType": "sim.propertytype_handlearray"},
        "customData": {},
        "signal": {}
    },
    "methods": {
        )" MARKER_META_METHODS R"(,
        )" SCENEOBJECT_META_METHODS R"(
    }
}
)";

CMarker::CMarker(int type /*= sim_markertype_points*/, unsigned char col[3] /*= nullptr*/, double size[3] /*= nullptr*/, int maxCnt /*= 0*/, int options /*= 0*/, float duplicateTol /*= 0.0f*/)
{
    _objectType = sim_sceneobject_marker;
    _localObjectSpecialProperty = 0;
    _objectProperty |= sim_objectproperty_dontshowasinsidemodel;
    _markerSize = 0.01;

    _visibilityLayer = MARKER_LAYER;
    _objectAlias = getObjectTypeInfo();
    _objectName_old = getObjectTypeInfo();
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());

    _markerColor.setDefaultValues();
    _markerColor.setColor(1.0f, 1.0f, 1.0f, sim_colorcomponent_ambient_diffuse);

    _itemType = type;
    _itemMaxCnt = maxCnt;
    _itemOptions = options;
    _itemDuplicateTol = duplicateTol;
    _itemCol[0] = 255;
    _itemCol[1] = 255;
    _itemCol[2] = 0;
    _itemCol[3] = 255;
    if (col != nullptr)
    {
        for (size_t i = 0; i < 3; i++)
            _itemCol[i] = col[i];
    }
    _itemSize[0] = 0.005;
    _itemSize[1] = 0.005;
    _itemSize[2] = 0.005;
    if (size != nullptr)
    {
        for (size_t i = 0; i < 3; i++)
            _itemSize[i] = size[i];
    }
    _initialize();
}

void CMarker::_initialize()
{
    _itemPointCnt = 1;
    if (_itemType == sim_markertype_lines)
        _itemPointCnt = 2;
    else if (_itemType == sim_markertype_triangles)
        _itemPointCnt = 3;
    _nextId = 0;
    _newItemsCnt = 0;
    _sendFullEvent = true;
    _remIds.clear();
    _ids.clear();

    std::vector<float> ppts;
    ppts.swap(_pts);
    std::vector<float> qqats;
    qqats.swap(_quats);
    std::vector<unsigned char> rrgba;
    rrgba.swap(_rgba);
    std::vector<float> ssizes;
    ssizes.swap(_sizes);

    addItems(&ppts, &qqats, &rrgba, &ssizes, false);

    computeBoundingBox();
}

void CMarker::remItems(int itemCnt, bool triggerEvent /*= true*/)
{
    int items = _pts.size() / (3 * _itemPointCnt);
    if ((items <= itemCnt) || (itemCnt == 0))
    {
        _pts.clear();
        _quats.clear();
        _rgba.clear();
        _sizes.clear();
        _ids.clear();
        _remIds.clear();
        _newItemsCnt = 0;
        _sendFullEvent = true;
        _nextId = 0;
    }
    else
    {
        _remIds.insert(_remIds.end(), _ids.begin(), _ids.begin() + itemCnt);
        _pts.erase(_pts.begin(), _pts.begin() + itemCnt * 3 * _itemPointCnt);
        _rgba.erase(_rgba.begin(), _rgba.begin() + itemCnt * 4 * _itemPointCnt);
        _ids.erase(_ids.begin(), _ids.begin() + itemCnt);
        if ( (_itemType != sim_markertype_points) && (_itemType != sim_markertype_lines) && (_itemType != sim_markertype_triangles) )
        {
            _quats.erase(_quats.begin(), _quats.begin() + itemCnt * 4 * _itemPointCnt);
            _sizes.erase(_sizes.begin(), _sizes.begin() + itemCnt * 3 * _itemPointCnt);
        }
        int tot = int(_pts.size()) / (3 * _itemPointCnt);
        if (tot < _newItemsCnt)
            _newItemsCnt = tot;
    }
    if (triggerEvent)
        _updateMarkerEvent(true);
}

void CMarker::addItems(const std::vector<float>* pts, const std::vector<float>* quats, const std::vector<unsigned char>* rgbas, const std::vector<float>* sizes, bool transform /*= true*/)
{
    int ptsCnt = int(pts->size()) / 3;
    if (quats != nullptr)
    {
        if (quats->size() != ptsCnt * 4)
            quats = nullptr;
    }
    if (rgbas != nullptr)
    {
        if (rgbas->size() != ptsCnt * 4)
            rgbas = nullptr;
    }
    if (sizes != nullptr)
    {
        if (sizes->size() != ptsCnt * 3)
            sizes = nullptr;
    }
    int totItemCnt = int(_pts.size()) / (3 * _itemPointCnt);
    int itemCnt = ptsCnt / _itemPointCnt;

    if ((_itemMaxCnt != 0) && (totItemCnt + itemCnt > _itemMaxCnt) && ((_itemOptions & sim_markeropts_cyclic) == 0))
        itemCnt = _itemMaxCnt - totItemCnt;

    // handle coords
    C7Vector tr(getFullCumulativeTransformation());
    C7Vector trInv(tr.getInverse());
    std::vector<C7Vector> trs;
    if (transform)
    {
        if ((_itemOptions & sim_markeropts_local) != 0)
        {
            if ((_itemOptions & sim_markeropts_fixed) != 0)
            {
                for (int i = 0; i < ptsCnt; i++)
                {
                    C7Vector t;
                    t.X.setData(pts->data() + i * 3);
                    if (quats != nullptr)
                        t.Q.setData(quats->data() + i * 4, true);
                    else
                        t.Q.setIdentity();
                    trs.push_back(tr * t);
                }
            }
            else
            {
                for (int i = 0; i < ptsCnt; i++)
                {
                    C7Vector t;
                    t.X.setData(pts->data() + i * 3);
                    if (quats != nullptr)
                        t.Q.setData(quats->data() + i * 4, true);
                    else
                        t.Q.setIdentity();
                    trs.push_back(t);
                }
            }
        }
        else
        {
            if ((_itemOptions & sim_markeropts_fixed) != 0)
            {
                for (int i = 0; i < ptsCnt; i++)
                {
                    C7Vector t;
                    t.X.setData(pts->data() + i * 3);
                    if (quats != nullptr)
                        t.Q.setData(quats->data() + i * 4, true);
                    else
                        t.Q.setIdentity();
                    trs.push_back(t);
                }
            }
            else
            {
                for (int i = 0; i < ptsCnt; i++)
                {
                    C7Vector t;
                    t.X.setData(pts->data() + i * 3);
                    if (quats != nullptr)
                        t.Q.setData(quats->data() + i * 4, true);
                    else
                        t.Q.setIdentity();
                    trs.push_back(trInv * t);
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < ptsCnt; i++)
        {
            C7Vector t;
            t.X.setData(pts->data() + i * 3);
            if (quats != nullptr)
                t.Q.setData(quats->data() + i * 4, true);
            else
                t.Q.setIdentity();
            trs.push_back(t);
        }
    }

    bool hasDim = ((_itemType != sim_markertype_points) && (_itemType != sim_markertype_lines) && (_itemType != sim_markertype_triangles));
    for (int i = 0; i < itemCnt; i++)
    {
        bool skip = false;
        if ((_itemDuplicateTol > 0.0f) && (_itemPointCnt == 1))
        { // optimize later
            float minDD = 1000000000.0f;
            for (int k = 0; k < _pts.size() / 3; k++)
            {
                float dxx[3] = {((float)trs[i].X(0)) - _pts[k * 3 + 0], ((float)trs[i].X(1)) - _pts[k * 3 + 1], ((float)trs[i].X(2)) - _pts[k * 3 + 2]};
                float dd = dxx[0] * dxx[0] + dxx[1] * dxx[1] + dxx[2] * dxx[2];
                if (dd < minDD)
                    minDD = dd;
            }
            skip = (minDD < _itemDuplicateTol * _itemDuplicateTol);
        }
        if (!skip)
        {
            for (int j = 0; j < _itemPointCnt; j++)
            {
                _pts.push_back((float)trs[i * _itemPointCnt + j].X(0));
                _pts.push_back((float)trs[i * _itemPointCnt + j].X(1));
                _pts.push_back((float)trs[i * _itemPointCnt + j].X(2));
            }
            if (rgbas != nullptr)
            {
                for (int j = 0; j < 4 * _itemPointCnt; j++)
                    _rgba.push_back(rgbas->at(i * 4 * _itemPointCnt + j));
            }
            else
            {
                for (int j = 0; j < _itemPointCnt; j++)
                {
                    _rgba.push_back(_itemCol[0]);
                    _rgba.push_back(_itemCol[1]);
                    _rgba.push_back(_itemCol[2]);
                    _rgba.push_back(_itemCol[3]);
                }
            }
            if (hasDim)
            {
                _quats.push_back((float)trs[i].Q(1));
                _quats.push_back((float)trs[i].Q(2));
                _quats.push_back((float)trs[i].Q(3));
                _quats.push_back((float)trs[i].Q(0));
                if (sizes != nullptr)
                {
                    for (int j = 0; j < 3 ; j++)
                        _sizes.push_back(sizes->at(i * 3 + j));
                }
                else
                {
                    for (int j = 0; j < 3 ; j++)
                        _sizes.push_back(_itemSize[j]);
                }
            }
            _ids.push_back(_nextId);

            if (_nextId < MARKER_MAX_ID)
                _nextId++;
        }
    }

    totItemCnt = int(_pts.size()) / (3 * _itemPointCnt);
    if (_nextId > MARKER_MAX_ID)
    {
        _nextId = 0;
        _ids.resize(totItemCnt);
        for (size_t i = 0; i < totItemCnt; i++)
            _ids[i] = _nextId++;
        _newItemsCnt = totItemCnt;
        _remIds.clear();
        _sendFullEvent = true;
    }
    else
    {
        _newItemsCnt += itemCnt;
        if ((totItemCnt > _itemMaxCnt) && (_itemMaxCnt != 0))
            remItems(totItemCnt - _itemMaxCnt, false);
    }
    _updateMarkerEvent(true);
}

CMarker::~CMarker()
{
}

void CMarker::setIsInScene(bool s)
{
    CSceneObject::setIsInScene(s);
    if (s)
        _markerColor.setEventParams(true, _objectHandle);
    else
        _markerColor.setEventParams(true, -1);
}

std::string CMarker::getObjectTypeInfo() const
{
    return "marker";
}

std::string CMarker::getObjectTypeInfoExtended() const
{
    return getObjectTypeInfo();
}

void CMarker::computeBoundingBox()
{
    _setBB(C7Vector::identityTransformation, C3Vector(1.05, 1.05, 1.05) * _markerSize * 0.5);
}

void CMarker::scaleObject(double scalingFactor)
{
    setMarkerSize(_markerSize * scalingFactor);
    //TODO
    CSceneObject::scaleObject(scalingFactor);
}

void CMarker::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void CMarker::addSpecializedObjectEventData(CCbor* ev)
{
    _markerColor.addGenesisEventData(ev);
    ev->appendKeyDouble(propScript_size.name, _markerSize);
    ev->appendKeyInt(propMarker_itemType.name, _itemType);
    ev->appendKeyInt(propMarker_options.name, _itemOptions);
    _updateMarkerEvent(false, ev);
}

CSceneObject* CMarker::copyYourself()
{
    CMarker* newMarker = (CMarker*)CSceneObject::copyYourself();

    _markerColor.copyYourselfInto(&newMarker->_markerColor);
    newMarker->_markerSize = _markerSize;

    newMarker->_itemType = _itemType;
    newMarker->_itemMaxCnt = _itemMaxCnt;
    newMarker->_itemOptions = _itemOptions;
    newMarker->_itemDuplicateTol = _itemDuplicateTol;
    newMarker->_itemPointCnt = _itemPointCnt;
    for (size_t i = 0; i < 4; i++)
        newMarker->_itemCol[i] =_itemCol[i];
    for (size_t i = 0; i < 3; i++)
        newMarker->_itemSize[i] =_itemSize[i];

    newMarker->_pts.assign(_pts.begin(), _pts.end());
    newMarker->_quats.assign(_quats.begin(), _quats.end());
    newMarker->_rgba.assign(_rgba.begin(), _rgba.end());
    newMarker->_sizes.assign(_sizes.begin(), _sizes.end());
    newMarker->_ids.assign(_ids.begin(), _ids.end());

    newMarker->_sendFullEvent = true;
    newMarker->_nextId = _nextId;

    return newMarker;
}

void CMarker::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    //TODO
}

void CMarker::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
    //TODO
}

void CMarker::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0)
        {
        }
    }
    //TODO
    CSceneObject::simulationEnded();
}

void CMarker::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Sos");
            ar << _markerSize;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy = 0;
            // SIM_SET_CLEAR_BIT(dummy, 0, _resetAfterSimError);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Set");
            ar << _itemType;
            ar << _itemMaxCnt;
            ar << _itemOptions;
            ar << _itemDuplicateTol;
            for (size_t i = 0; i < 4; i++)
                ar << _itemCol[i];
            for (size_t i = 0; i < 3; i++)
                ar << _itemSize[i];
            ar.flush();

            ar.storeDataName("Pts");
            ar << _pts.size();
            for (size_t i = 0; i < _pts.size(); i++)
                ar << _pts[i];
            ar.flush();

            ar.storeDataName("Qts");
            ar << _quats.size();
            for (size_t i = 0; i < _quats.size(); i++)
                ar << _quats[i];
            ar.flush();

            ar.storeDataName("Cls");
            ar << _rgba.size();
            for (size_t i = 0; i < _rgba.size(); i++)
                ar << _rgba[i];
            ar.flush();

            ar.storeDataName("Siz");
            ar << _sizes.size();
            for (size_t i = 0; i < _sizes.size(); i++)
                ar << _sizes[i];
            ar.flush();

            ar.storeDataName("Soc");
            ar.setCountingMode();
            _markerColor.serialize(ar, 0);
            if (ar.setWritingMode())
                _markerColor.serialize(ar, 0);

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Sos") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _markerSize;
                    }

                    if (theName.compare("Var") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        // _resetAfterSimError = SIM_IS_BIT_SET(dummy, 0);
                    }

                    if (theName.compare("Set") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _itemType;
                        ar >> _itemMaxCnt;
                        ar >> _itemOptions;
                        ar >> _itemDuplicateTol;
                        for (size_t i = 0; i < 4; i++)
                            ar >> _itemCol[i];
                        for (size_t i = 0; i < 3; i++)
                            ar >> _itemSize[i];
                    }

                    if (theName.compare("Pts") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        size_t cnt;
                        ar >> cnt;
                        _pts.resize(cnt);
                        for (size_t i = 0; i < cnt; i++)
                            ar >> _pts[i];
                    }

                    if (theName.compare("Qts") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        size_t cnt;
                        ar >> cnt;
                        _quats.resize(cnt);
                        for (size_t i = 0; i < cnt; i++)
                            ar >> _quats[i];
                    }

                    if (theName.compare("Cls") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        size_t cnt;
                        ar >> cnt;
                        _rgba.resize(cnt);
                        for (size_t i = 0; i < cnt; i++)
                            ar >> _rgba[i];
                    }

                    if (theName.compare("Siz") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        size_t cnt;
                        ar >> cnt;
                        _sizes.resize(cnt);
                        for (size_t i = 0; i < cnt; i++)
                            ar >> _sizes[i];
                    }

                    if (theName.compare("Soc") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        _markerColor.serialize(ar, 0);
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            _initialize();
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        {
            ar.xmlAddNode_comment(" 'type' tag: can be 'points', 'lines', 'triangles', 'spheres', 'squares', 'discs' or 'cubes' ", exhaustiveXml);
            ar.xmlAddNode_enum("type", _itemType, sim_markertype_points, "points", sim_markertype_lines, "lines", sim_markertype_triangles, "triangles", sim_markertype_spheres, "spheres", sim_markertype_squares, "squares", sim_markertype_discs, "discs", sim_markertype_cubes, "cubes");

            ar.xmlAddNode_float("size", _markerSize);
            ar.xmlAddNode_int("maxCnt", _itemMaxCnt);
            ar.xmlAddNode_int("options", _itemOptions);
            ar.xmlAddNode_float("duplicateTolerance", _itemDuplicateTol);

            int rgba[4];
            for (size_t i = 0; i< 4; i++)
                rgba[i] = _itemCol[i];
            ar.xmlAddNode_ints("defaultColor", rgba, 4);
            ar.xmlAddNode_floats("defaultSize", _itemSize, 3);

            ar.xmlAddNode_floats("points", _pts.data(), _pts.size());
            ar.xmlAddNode_floats("quaternions", _quats.data(), _quats.size());
            std::vector<int> cols;
            for (size_t i = 0; i < _rgba.size(); i++)
                cols.push_back(_rgba[i]);
            ar.xmlAddNode_ints("colors", cols.data(), cols.size());
            ar.xmlAddNode_floats("sizes", _sizes.data(), _sizes.size());

            ar.xmlPushNewNode("color");
            if (exhaustiveXml)
                _markerColor.serialize(ar, 0);
            else
            {
                int rgb[3];
                for (size_t l = 0; l < 3; l++)
                    rgb[l] = int(_markerColor.getColorsPtr()[l] * 255.1);
                ar.xmlAddNode_ints("object", rgb, 3);
            }
            ar.xmlPopNode();
        }
        else
        {
            ar.xmlGetNode_enum("type", _itemType, exhaustiveXml, "points", sim_markertype_points, "lines", sim_markertype_lines, "triangles", sim_markertype_triangles, "spheres", sim_markertype_spheres, "squares", sim_markertype_squares, "discs", sim_markertype_discs, "cubes", sim_markertype_cubes);

            ar.xmlGetNode_float("size", _markerSize, exhaustiveXml);
            if (_markerSize < 0.001)
                _markerSize = 0.001;
            ar.xmlGetNode_int("maxCnt", _itemMaxCnt, exhaustiveXml);
            if (_itemMaxCnt < 0)
                _itemMaxCnt = 0;
            ar.xmlGetNode_int("options", _itemOptions, exhaustiveXml);
            ar.xmlGetNode_float("duplicateTolerance", _itemDuplicateTol, exhaustiveXml);
            if (_itemDuplicateTol < 0.0)
                _itemDuplicateTol = 0.0;

            int rgba[4];
            ar.xmlGetNode_ints("defaultColor", rgba, 4, exhaustiveXml);
            for (size_t i = 0; i< 4; i++)
                _rgba[i] = (unsigned char)rgba[i];
            ar.xmlGetNode_floats("defaultSize", _itemSize, 3, exhaustiveXml);

            ar.xmlGetNode_floats("points", _pts, exhaustiveXml);
            ar.xmlGetNode_floats("quaternions", _quats, exhaustiveXml);
            std::vector<int> cols;
            ar.xmlGetNode_ints("colors", cols, exhaustiveXml);
            for (size_t i = 0; i < cols.size(); i++)
                _rgba.push_back((unsigned char)cols[i]);
            ar.xmlGetNode_floats("sizes", _sizes, exhaustiveXml);

            if (ar.xmlPushChildNode("color", exhaustiveXml))
            {
                if (exhaustiveXml)
                    _markerColor.serialize(ar, 0);
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("object", rgb, 3, exhaustiveXml))
                        _markerColor.setColor(float(rgb[0]) / 255.1, float(rgb[1]) / 255.1, float(rgb[2]) / 255.1, sim_colorcomponent_ambient_diffuse);
                }
                ar.xmlPopNode();
            }
            _initialize();
        }
    }
}

double CMarker::getMarkerSize() const
{
    return (_markerSize);
}

int CMarker::getMarkerOptions() const
{
    return _itemOptions;
}

CColorObject* CMarker::getMarkerColor()
{
    return (&_markerColor);
}

void CMarker::setMarkerSize(double s)
{
    bool diff = (_markerSize != s);
    if (diff)
    {
        _markerSize = s;
        computeBoundingBox();
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propMarker_size.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _markerSize);
            App::worldContainer->pushEvent();
        }
    }
}

#ifdef SIM_WITH_GUI
void CMarker::display(CViewableBase* renderingObject, int displayAttrib)
{
    displayMarker(this, renderingObject, displayAttrib, false);
}

void CMarker::displayOverlay(CViewableBase* renderingObject, int displayAttrib)
{
    displayMarker(this, renderingObject, displayAttrib, true);
}

void CMarker::_drawPoints(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const
{
    glPointSize(2.0f);
    ogl::buffer.clear();
    std::vector<float> glCols;
    for (size_t i = 0; i < _pts.size() / 3; i++)
    {
        glCols.push_back(float(_rgba[i * 4 + 0]) / 255.0);
        glCols.push_back(float(_rgba[i * 4 + 1]) / 255.0);
        glCols.push_back(float(_rgba[i * 4 + 2]) / 255.0);
        glCols.push_back(float(_rgba[i * 4 + 3]) / 255.0);
        ogl::addBuffer3DPoints(_pts[i * 3 + 0], _pts[i * 3 + 1], _pts[i * 3 + 2]);
    }
    if (ogl::buffer.size() != 0)
    {
        if ((displayAttrib & sim_displayattribute_colorcoded) == 0)
            ogl::drawRandom3dPointsEx(&ogl::buffer[0], (int)ogl::buffer.size() / 3, nullptr, &glCols[0], nullptr, false, normalVectorForLinesAndPoints);
        else
            ogl::drawRandom3dPointsEx(&ogl::buffer[0], (int)ogl::buffer.size() / 3, nullptr, nullptr, nullptr, false, normalVectorForLinesAndPoints);
    }
    ogl::buffer.clear();
    glPointSize(1.0f);
}

void CMarker::_drawLines(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const
{
    glLineWidth(2.0f);
    if ((displayAttrib & sim_displayattribute_colorcoded) == 0)
    {
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_SPECULAR);
        glColor3f(0.0, 0.0, 0.0);
        glColorMaterial(GL_FRONT_AND_BACK, GL_EMISSION);
        glColor3f(0.0, 0.0, 0.0);
        glColorMaterial(GL_FRONT_AND_BACK, GL_SHININESS);
        glColor3f(0.0, 0.0, 0.0);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glColor3f(0.0, 0.0, 0.0);
    }
    for (size_t i = 0; i < _pts.size() / 6; i++)
    {
        glBegin(GL_LINES);
        glNormal3dv(normalVectorForLinesAndPoints);
        if ((displayAttrib & sim_displayattribute_colorcoded) == 0)
            glColor4ubv((GLubyte*)_rgba.data() + i * 8 + 0);
        glVertex3fv(_pts.data() + i * 6 + 0);
        if ((displayAttrib & sim_displayattribute_colorcoded) == 0)
            glColor4ubv((GLubyte*)_rgba.data() + i * 8 + 4);
        glVertex3fv(_pts.data() + i * 6 + 3);
        glEnd();
    }
    if ((displayAttrib & sim_displayattribute_colorcoded) == 0)
        glDisable(GL_COLOR_MATERIAL);
    glLineWidth(1.0f);
}

void CMarker::_drawTriangles(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const
{
    if ((displayAttrib & sim_displayattribute_colorcoded) == 0)
    {
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_SPECULAR);
        glColor3f(0.0, 0.0, 0.0);
        glColorMaterial(GL_FRONT_AND_BACK, GL_EMISSION);
        glColor3f(0.0, 0.0, 0.0);
        glColorMaterial(GL_FRONT_AND_BACK, GL_SHININESS);
        glColor3f(0.0, 0.0, 0.0);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glColor3f(0.0, 0.0, 0.0);
    }
    for (size_t i = 0; i < _pts.size() / 9; i++)
    {
        const float* v0 = _pts.data() + i * 9 + 0;
        const float* v1 = _pts.data() + i * 9 + 3;
        const float* v2 = _pts.data() + i * 9 + 6;

        float edge1[3] = {v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]};
        float edge2[3] = {v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]};

        float normal[3] =
        {
            edge1[1] * edge2[2] - edge1[2] * edge2[1],
            edge1[2] * edge2[0] - edge1[0] * edge2[2],
            edge1[0] * edge2[1] - edge1[1] * edge2[0]
        };

        float len = sqrtf(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);
        if (len > 0.0f)
        {
            normal[0] /= len;
            normal[1] /= len;
            normal[2] /= len;
        }

        glBegin(GL_TRIANGLES);
        glNormal3fv(normal);
        if ((displayAttrib & sim_displayattribute_colorcoded) == 0)
            glColor4ubv((GLubyte*)_rgba.data() + i * 12 + 0);
        glVertex3fv(v0);
        glNormal3fv(normal);
        if ((displayAttrib & sim_displayattribute_colorcoded) == 0)
            glColor4ubv((GLubyte*)_rgba.data() + i * 12 + 4);
        glVertex3fv(v1);
        glNormal3fv(normal);
        if ((displayAttrib & sim_displayattribute_colorcoded) == 0)
            glColor4ubv((GLubyte*)_rgba.data() + i * 12 + 8);
        glVertex3fv(v2);
        glEnd();
    }
    if ((displayAttrib & sim_displayattribute_colorcoded) == 0)
        glDisable(GL_COLOR_MATERIAL);
}

void CMarker::_drawQuadPoints(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const
{
    for (size_t i = 0; i < _pts.size() / 3; i++)
    {
        glPushMatrix();
        glTranslatef(_pts[3 * i + 0], _pts[3 * i + 1], _pts[3 * i + 2]);
        const float* q = _quats.data() + 4 * i;
        float angle = 2.0f * acosf(q[3]);
        float s_inv = 1.0f / sqrtf(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]);
        if (s_inv != INFINITY && angle != 0.0f)
        {
            float ax = q[0] * s_inv;
            float ay = q[1] * s_inv;
            float az = q[2] * s_inv;
            glRotatef(angle * 180.0f / piValue, ax, ay, az);
        }
        glScalef(_sizes[3 * i + 0], _sizes[3 * i + 1], _sizes[3 * i + 2]);

        if ((displayAttrib & sim_displayattribute_colorcoded) == 0)
        {
            float col[3] = {float(_rgba[4 * i + 0]) / 255.0f, float(_rgba[4 * i + 1]) / 255.0f, float(_rgba[4 * i + 2]) / 255.0f};
            ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse, col);
        }

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-0.5f, -0.5f, 0.0f);
        glVertex3f(0.5f, -0.5f, 0.0f);
        glVertex3f(0.5f, 0.5f, 0.0f);
        glVertex3f(-0.5f, 0.5f, 0.0f);
        glEnd();
        glPopMatrix();
    }
}

void CMarker::_drawDiscPoints(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const
{
    for (size_t i = 0; i < _pts.size() / 3; i++)
    {
        glPushMatrix();
        glTranslatef(_pts[3 * i + 0], _pts[3 * i + 1], _pts[3 * i + 2]);
        const float* q = _quats.data() + 4 * i;
        float angle = 2.0f * acosf(q[3]);
        float s_inv = 1.0f / sqrtf(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]);
        if (s_inv != INFINITY && angle != 0.0f)
        {
            float ax = q[0] * s_inv;
            float ay = q[1] * s_inv;
            float az = q[2] * s_inv;
            glRotatef(angle * 180.0f / piValue, ax, ay, az);
        }
        glScalef(_sizes[3 * i + 0], _sizes[3 * i + 1], _sizes[3 * i + 2]);

        if ((displayAttrib & sim_displayattribute_colorcoded) == 0)
        {
            float col[3] = {float(_rgba[4 * i + 0]) / 255.0f, float(_rgba[4 * i + 1]) / 255.0f, float(_rgba[4 * i + 2]) / 255.0f};
            ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse, col);
        }

        glBegin(GL_TRIANGLE_FAN); // GL_POLYGON is problematic on certain graphic cards!
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.5f, 0.0f, 0.0f);
        glVertex3f(0.433f, 0.25f, 0.0f);
        glVertex3f(0.25f, 0.433f, 0.0f);
        glVertex3f(0.0f, 0.5f, 0.0f);
        glVertex3f(-0.25f, 0.433f, 0.0f);
        glVertex3f(-0.433f, 0.25f, 0.0f);
        glVertex3f(-0.5f, 0.0f, 0.0f);
        glVertex3f(-0.433f, -0.25f, 0.0f);
        glVertex3f(-0.25f, -0.433f, 0.0f);
        glVertex3f(0.0f, -0.5f, 0.0f);
        glVertex3f(0.25f, -0.433f, 0.0f);
        glVertex3f(0.433f, -0.25f, 0.0f);
        glVertex3f(0.5f, 0.0f, 0.0f);
        glEnd();
        glPopMatrix();
    }
}

void CMarker::_drawCubePoints(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const
{
    for (size_t i = 0; i < _pts.size() / 3; i++)
    {
        glPushMatrix();
        glTranslatef(_pts[3 * i + 0], _pts[3 * i + 1], _pts[3 * i + 2]);
        const float* q = _quats.data() + 4 * i;
        float angle = 2.0f * acosf(q[3]);
        float s_inv = 1.0f / sqrtf(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]);
        if (s_inv != INFINITY && angle != 0.0f)
        {
            float ax = q[0] * s_inv;
            float ay = q[1] * s_inv;
            float az = q[2] * s_inv;
            glRotatef(angle * 180.0f / piValue, ax, ay, az);
        }
        glScalef(_sizes[3 * i + 0], _sizes[3 * i + 1], _sizes[3 * i + 2]);

        if ((displayAttrib & sim_displayattribute_colorcoded) == 0)
        {
            float col[3] = {float(_rgba[4 * i + 0]) / 255.0f, float(_rgba[4 * i + 1]) / 255.0f, float(_rgba[4 * i + 2]) / 255.0f};
            ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse, col);
        }

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f( 0.5f, -0.5f, 0.5f);
        glVertex3f( 0.5f,  0.5f, 0.5f);
        glVertex3f(-0.5f,  0.5f, 0.5f);
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f,  0.5f, -0.5f);
        glVertex3f( 0.5f,  0.5f, -0.5f);
        glVertex3f( 0.5f, -0.5f, -0.5f);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f,  0.5f);
        glVertex3f( 0.5f, 0.5f,  0.5f);
        glVertex3f( 0.5f, 0.5f, -0.5f);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f( 0.5f, -0.5f, -0.5f);
        glVertex3f( 0.5f, -0.5f,  0.5f);
        glVertex3f(-0.5f, -0.5f,  0.5f);
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f,  0.5f, -0.5f);
        glVertex3f(0.5f,  0.5f,  0.5f);
        glVertex3f(0.5f, -0.5f,  0.5f);
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f,  0.5f);
        glVertex3f(-0.5f,  0.5f,  0.5f);
        glVertex3f(-0.5f,  0.5f, -0.5f);
        glEnd();
        glPopMatrix();
    }
}

void CMarker::_drawSpherePoints(int displayAttrib, const double normalVectorForLinesAndPoints[3]) const
{
    for (size_t i = 0; i < _pts.size() / 3; i++)
    {
        glPushMatrix();
        glTranslatef(_pts[3 * i + 0], _pts[3 * i + 1], _pts[3 * i + 2]);
        const float* q = _quats.data() + 4 * i;
        float angle = 2.0f * acosf(q[3]);
        float s_inv = 1.0f / sqrtf(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]);
        if (s_inv != INFINITY && angle != 0.0f)
        {
            float ax = q[0] * s_inv;
            float ay = q[1] * s_inv;
            float az = q[2] * s_inv;
            glRotatef(angle * 180.0f / piValue, ax, ay, az);
        }
        glScalef(_sizes[3 * i + 0], _sizes[3 * i + 1], _sizes[3 * i + 2]);

        if ((displayAttrib & sim_displayattribute_colorcoded) == 0)
        {
            float col[3] = {float(_rgba[4 * i + 0]) / 255.0f, float(_rgba[4 * i + 1]) / 255.0f, float(_rgba[4 * i + 2]) / 255.0f};
            ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse, col);
        }

        int slices = 12;  // Longitude divisions
        int stacks = 12;  // Latitude divisions
        for (int k = 0; k < stacks; k++)
        {
            float lat0 = piValue * (-0.5f + (float)k / stacks);
            float z0 = 0.5f * sinf(lat0);
            float r0 = 0.5f * cosf(lat0);

            float lat1 = piValue * (-0.5f + (float)(k + 1) / stacks);
            float z1 = 0.5f * sinf(lat1);
            float r1 = 0.5f * cosf(lat1);

            glBegin(GL_QUAD_STRIP);
            for (int j = 0; j <= slices; j++)
            {
                float lng = 2.0f * piValue * (float)j / slices;
                float x = cosf(lng);
                float y = sinf(lng);

                // Normal and vertex for upper latitude
                glNormal3f(x * r1 / 0.5f, y * r1 / 0.5f, z1 / 0.5f);
                glVertex3f(x * r1, y * r1, z1);

                // Normal and vertex for lower latitude
                glNormal3f(x * r0 / 0.5f, y * r0 / 0.5f, z0 / 0.5f);
                glVertex3f(x * r0, y * r0, z0);
            }
            glEnd();
        }
        glPopMatrix();
    }
}

/*
void _drawTriangles(CDrawingObject* drawingObject, int displayAttrib)
{
    bool auxCmp = (displayAttrib & sim_displayattribute_useauxcomponent) != 0;
    int _objectType = drawingObject->getObjectType();
    // double _size=drawingObject->getSize();
    int _maxItemCount = drawingObject->getMaxItemCount();
    int _startItem = drawingObject->getStartItem();
    std::vector<double>& _data = drawingObject->getDataPtr()[0];
    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ((_objectType & sim_drawing_itemtransparency) && (!auxCmp))
        ogl::setBlending(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // We turn blending on!

    glBegin(GL_TRIANGLES);
    C3Vector v, w, x, n;
    int off = 0;
    for (int i = 0; i < int(_data.size()) / drawingObject->floatsPerItem; i++)
    {
        int p = _startItem + i;
        if (p >= _maxItemCount)
            p -= _maxItemCount;
        v.setData(&_data[drawingObject->floatsPerItem * p + 0]);
        w.setData(&_data[drawingObject->floatsPerItem * p + 3]);
        x.setData(&_data[drawingObject->floatsPerItem * p + 6]);
        if ((_objectType & (sim_drawing_itemcolors | sim_drawing_vertexcolors)) &&
            ((!auxCmp) || (_objectType & sim_drawing_auxchannelcolor2)))
        {
            float ab[3] = {(float)_data[drawingObject->floatsPerItem * p + 9],
                           (float)_data[drawingObject->floatsPerItem * p + 10],
                           (float)_data[drawingObject->floatsPerItem * p + 11]};
            if (_objectType & (sim_drawing_emissioncolor | sim_drawing_auxchannelcolor2))
                ogl::setMaterialColor(sim_colorcomponent_emission, ab);
            else
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse, ab);
        }
        if (_objectType & (sim_drawing_itemcolors | sim_drawing_vertexcolors))
            off += 3;
        float* vertex2Col = nullptr;
        float* vertex3Col = nullptr;
        float vertex2Col_[3];
        float vertex3Col_[3];
        if (_objectType & sim_drawing_vertexcolors)
        {
            vertex2Col_[0] = _data[drawingObject->floatsPerItem * p + 9 + off + 0];
            vertex2Col_[1] = _data[drawingObject->floatsPerItem * p + 9 + off + 1];
            vertex2Col_[2] = _data[drawingObject->floatsPerItem * p + 9 + off + 2];
            vertex2Col = vertex2Col_;
            off += 3;
            vertex3Col_[0] = _data[drawingObject->floatsPerItem * p + 9 + off + 0];
            vertex3Col_[1] = _data[drawingObject->floatsPerItem * p + 9 + off + 1];
            vertex3Col_[2] = _data[drawingObject->floatsPerItem * p + 9 + off + 2];
            vertex3Col = vertex3Col_;
            off += 3;
        }

        if ((_objectType & sim_drawing_itemtransparency) && (!auxCmp))
        { // Following part new since introduction of sim_drawing_itemtransparency:
            ogl::setAlpha(1.0 - _data[drawingObject->floatsPerItem * p + 9 + off]);
            off++;
        }

        n = (w - v) ^ (x - v);
        double l = n.getLength();
        if (l != 0.0)
            n /= l;
        glNormal3dv(n.data);
        glVertex3dv(v.data);
        if ((_objectType & sim_drawing_vertexcolors) && ((!auxCmp) || (_objectType & sim_drawing_auxchannelcolor2)))
        {
            if (_objectType & (sim_drawing_emissioncolor | sim_drawing_auxchannelcolor2))
                ogl::setMaterialColor(sim_colorcomponent_emission, vertex2Col);
            else
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse, vertex2Col);
        }
        glVertex3dv(w.data);
        if ((_objectType & sim_drawing_vertexcolors) && ((!auxCmp) || (_objectType & sim_drawing_auxchannelcolor2)))
        {
            if (_objectType & (sim_drawing_emissioncolor | sim_drawing_auxchannelcolor2))
                ogl::setMaterialColor(sim_colorcomponent_emission, vertex3Col);
            else
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse, vertex3Col);
        }
        glVertex3dv(x.data);
    }
    glEnd();

           // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ((_objectType & sim_drawing_itemtransparency) && (!auxCmp))
        ogl::setBlending(false); // make sure we turn blending off!
}
*/
void CMarker::drawItems(int displayAttrib, const double normalVectorForLinesAndPoints[3], bool overlay) const
{
    if (_itemOptions & sim_markeropts_overlay)
    {
        if (!overlay)
            return;
    }
    else
    {
        if (overlay)
            return;
    }

    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);

    if ((displayAttrib & sim_displayattribute_colorcoded) != 0)
    {
        ogl::setMaterialColor(ogl::colorBlack, ogl::colorBlack, ogl::colorBlack); // probably not needed
        glColor3ub(_objectHandle & 255, (_objectHandle >> 8) & 255, (_objectHandle >> 16) & 255);
        glLoadName(-1);
    }

    bool wire = ( (displayAttrib & sim_displayattribute_forcewireframe) && ((displayAttrib & sim_displayattribute_depthpass) == 0));
    if (displayAttrib & sim_displayattribute_forbidwireframe)
        wire = false;
    if (wire)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (_itemOptions & sim_markeropts_overlay)
        glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    if (_itemType == sim_markertype_points)
        _drawPoints(displayAttrib, normalVectorForLinesAndPoints);
    else if (_itemType == sim_markertype_lines)
        _drawLines(displayAttrib, normalVectorForLinesAndPoints);
    else if (_itemType == sim_markertype_triangles)
        _drawTriangles(displayAttrib, normalVectorForLinesAndPoints);
    else if (_itemType == sim_markertype_squares)
        _drawQuadPoints(displayAttrib, normalVectorForLinesAndPoints);
    else if (_itemType == sim_markertype_discs)
        _drawDiscPoints(displayAttrib, normalVectorForLinesAndPoints);
    else if (_itemType == sim_markertype_cubes)
        _drawCubePoints(displayAttrib, normalVectorForLinesAndPoints);
    else if (_itemType == sim_markertype_spheres)
        _drawSpherePoints(displayAttrib, normalVectorForLinesAndPoints);
    glDisable(GL_NORMALIZE);
    glDisable(GL_CULL_FACE);
    glPopAttrib();
    glPopMatrix();


}
#endif

int CMarker::setBoolProperty(const char* ppName, bool pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setBoolProperty(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CMarker::getBoolProperty(const char* ppName, bool& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getBoolProperty(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CMarker::setIntProperty(const char* ppName, int pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setIntProperty(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CMarker::getIntProperty(const char* ppName, int& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getIntProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propMarker_itemType.name)
        {
            pState = _itemType;
            retVal = 1;
        }
        else if (_pName == propMarker_options.name)
        {
            pState = _itemOptions;
            retVal = 1;
        }
    }

    return retVal;
}

int CMarker::setLongProperty(const char* ppName, long long int pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setLongProperty(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CMarker::getLongProperty(const char* ppName, long long int& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getLongProperty(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CMarker::getHandleProperty(const char* ppName, long long int& pState) const
{
    int retVal = CSceneObject::getHandleProperty(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CMarker::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setFloatProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propMarker_size.name)
        {
            setMarkerSize(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int CMarker::getFloatProperty(const char* ppName, double& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getFloatProperty(ppName, pState);
    if (retVal == -1)
        retVal = _markerColor.getFloatProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propMarker_size.name)
        {
            pState = _markerSize;
            retVal = 1;
        }
    }

    return retVal;
}

int CMarker::setStringProperty(const char* ppName, const char* pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setStringProperty(ppName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CMarker::getStringProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getStringProperty(ppName, pState);
    if (retVal == -1)
    {
        if (_pName == propMarker_objectMetaInfo.name)
        {
            pState = OBJECT_META_INFO;
            retVal = 1;
        }
    }

    return retVal;
}

int CMarker::getBufferProperty(const char* pName, std::string& pState) const
{
    int retVal = CSceneObject::getBufferProperty(pName, pState);
    if (retVal == -1)
    {
        if (strcmp(pName, propMarker_colors.name) == 0)
        {
            pState.assign(_rgba.begin(), _rgba.end());
            retVal = 1;
        }
    }

    return retVal;
}

int CMarker::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::setColorProperty(ppName, pState);
    if (retVal == -1)
        retVal = _markerColor.setColorProperty(ppName, pState);
    if (retVal == -1)
    {
    }
    return retVal;
}

int CMarker::getColorProperty(const char* ppName, float* pState) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getColorProperty(ppName, pState);
    if (retVal == -1)
        retVal = _markerColor.getColorProperty(ppName, pState);
    if (retVal == -1)
    {
    }
    return retVal;
}

int CMarker::getFloatArrayProperty(const char* pName, std::vector<double>& pState) const
{
    int retVal = CSceneObject::getFloatArrayProperty(pName, pState);
    if (retVal == -1)
    {
        if (strcmp(pName, propMarker_points.name) == 0)
        {
            pState.resize(_pts.size());
            for (size_t i = 0; i < _pts.size(); i++)
                pState[i] = (double)_pts[i];
            retVal = 1;
        }
        else if (strcmp(pName, propMarker_quaternions.name) == 0)
        {
            pState.resize(_quats.size());
            for (size_t i = 0; i < _quats.size(); i++)
                pState[i] = (double)_quats[i];
            retVal = 1;
        }
        else if (strcmp(pName, propMarker_sizes.name) == 0)
        {
            pState.resize(_sizes.size());
            for (size_t i = 0; i < _sizes.size(); i++)
                pState[i] = (double)_sizes[i];
            retVal = 1;
        }
    }
    return retVal;
}

int CMarker::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance, excludeFlags);
    if (retVal == -1)
    {
        appartenance = "marker";
        retVal = _markerColor.getPropertyName(index, pName, excludeFlags);
    }
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_marker.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_marker[i].name, pName.c_str()))
            {
                if ((allProps_marker[i].flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_marker[i].name;
                        retVal = 1;
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

int CMarker::getPropertyName_static(int& index, std::string& pName, std::string& appartenance, int excludeFlags)
{
    int retVal = CSceneObject::getPropertyName_bstatic(index, pName, appartenance, excludeFlags);
    if (retVal == -1)
    {
        appartenance = "marker";
        retVal = CColorObject::getPropertyName_static(index, pName, 1 + 4 + 8, "", excludeFlags);
    }
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_marker.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_marker[i].name, pName.c_str()))
            {
                if ((allProps_marker[i].flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_marker[i].name;
                        retVal = 1;
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

int CMarker::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt) const
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getPropertyInfo(ppName, info, infoTxt);
    if (retVal == -1)
        retVal = _markerColor.getPropertyInfo(ppName, info, infoTxt);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_marker.size(); i++)
        {
            if (strcmp(allProps_marker[i].name, ppName) == 0)
            {
                retVal = allProps_marker[i].type;
                info = allProps_marker[i].flags;
                if (infoTxt == "j")
                    infoTxt = allProps_marker[i].shortInfoTxt;
                else
                {
                    auto w = QJsonDocument::fromJson(allProps_marker[i].shortInfoTxt.c_str()).object();
                    std::string descr = w["description"].toString().toStdString();
                    std::string label = w["label"].toString().toStdString();
                    if ( (infoTxt == "s") || (descr == "") )
                        infoTxt = label;
                    else
                        infoTxt = descr;
                }
                break;
            }
        }
    }
    return retVal;
}

int CMarker::getPropertyInfo_static(const char* ppName, int& info, std::string& infoTxt)
{
    std::string _pName(ppName);
    int retVal = CSceneObject::getPropertyInfo_bstatic(ppName, info, infoTxt);
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(ppName, info, infoTxt, 1 + 4 + 8, "");
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_marker.size(); i++)
        {
            if (strcmp(allProps_marker[i].name, ppName) == 0)
            {
                retVal = allProps_marker[i].type;
                info = allProps_marker[i].flags;
                if (infoTxt == "j")
                    infoTxt = allProps_marker[i].shortInfoTxt;
                else
                {
                    auto w = QJsonDocument::fromJson(allProps_marker[i].shortInfoTxt.c_str()).object();
                    std::string descr = w["description"].toString().toStdString();
                    std::string label = w["label"].toString().toStdString();
                    if ( (infoTxt == "s") || (descr == "") )
                        infoTxt = label;
                    else
                        infoTxt = descr;
                }
                break;
            }
        }
    }
    return retVal;
}

void CMarker::_updateMarkerEvent(bool incremental, CCbor* evv /*= nullptr*/)
{
    CCbor* ev = evv;
    if ((evv != nullptr) || (_isInScene && App::worldContainer->getEventsEnabled()))
    {
        if (!incremental)
            _sendFullEvent = true;
        if (_sendFullEvent)
        {
            _sendFullEvent = false;
            if (evv == nullptr)
                ev = App::worldContainer->createSceneObjectChangedEvent(this, false, "set", true);
            ev->openKeyMap("set");
            ev->appendKeyBuff("pts", (unsigned char*)_pts.data(), _pts.size() * sizeof(float));
            ev->appendKeyBuff("quats", (unsigned char*)_quats.data(), _quats.size() * sizeof(float));
            ev->appendKeyBuff("rgba", _rgba.data(), _rgba.size());
            ev->appendKeyBuff("ids", (unsigned char*)_ids.data(), _ids.size());
            ev->closeArrayOrMap();
            if (evv == nullptr)
                App::worldContainer->pushEvent();
        }
        else
        {
            if ( (_newItemsCnt > 0) || (_remIds.size() > 0) )
            {
                if (evv == nullptr)
                    ev = App::worldContainer->createSceneObjectChangedEvent(this, false, "addRemove", true);
                if (_newItemsCnt > 0)
                {
                    ev->openKeyMap("add");
                    ev->appendKeyBuff("pts", (unsigned char*)(_pts.data() + _pts.size() - (_newItemsCnt * 3 * _itemPointCnt)), (_newItemsCnt * 3 * _itemPointCnt) * sizeof(float));
                    if (_quats.size() > 0)
                        ev->appendKeyBuff("quats", (unsigned char*)(_quats.data() + _quats.size() - (_newItemsCnt * 4 * _itemPointCnt)), (_newItemsCnt * 4 * _itemPointCnt) * sizeof(float));
                    else
                        ev->appendKeyBuff("quats", nullptr, 0);
                    ev->appendKeyBuff("rgba", (unsigned char*)(_rgba.data() + _rgba.size() - (_newItemsCnt * 4 * _itemPointCnt)), _newItemsCnt * 4 * _itemPointCnt);
                    ev->appendKeyBuff("ids", (unsigned char*)(_ids.data() + _ids.size() - _newItemsCnt), _newItemsCnt * sizeof(unsigned int));
                    ev->closeArrayOrMap();
                }
                if (_remIds.size() > 0)
                {
                    ev->openKeyMap("rem");
                    ev->appendKeyBuff("ids", (unsigned char*)_remIds.data(), _remIds.size() * sizeof(unsigned int));
                    ev->closeArrayOrMap();
                }
                if (evv == nullptr)
                    App::worldContainer->pushEvent();
            }
        }
        _newItemsCnt = 0;
        _remIds.clear();
    }
}

