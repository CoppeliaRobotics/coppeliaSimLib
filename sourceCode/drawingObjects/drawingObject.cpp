#include <simInternal.h>
#include <drawingObject.h>
#include <app.h>
#include <tt.h>
#include <utils.h>
#ifdef SIM_WITH_GUI
#include <drawingObjectRendering.h>
#endif

static std::string OBJECT_TYPE = "drawingObject";
static std::string OBJECT_META_INFO = R"(
{
    "superclass": "object",
    "namespaces": {
    },
    "methods": {
        )" DRAWINGOBJECT_META_METHODS R"(,
        )" OBJECT_META_METHODS R"(
    }
}
)";

double CDrawingObject::getSize() const
{
    return (_size);
}

int CDrawingObject::getMaxItemCount() const
{
    return (_maxItemCount);
}

int CDrawingObject::getStartItem() const
{
    return (_startItem);
}

int CDrawingObject::getExpectedFloatsPerItem() const
{
    int retVal = floatsPerItem;
    int tmp = _objectType & 0x001f;
    bool usingNormals_old = ((tmp == sim_drawing_trianglepoints) || (tmp == sim_drawing_quadpoints) ||
                             (tmp == sim_drawing_discpoints) || (tmp == sim_drawing_cubepoints));
    if (usingNormals_old)
        retVal--;
    return (retVal);
}

std::vector<double>* CDrawingObject::getDataPtr()
{
    return (&_data);
}

CDrawingObject::CDrawingObject(int theObjectType, double size, double duplicateTolerance, int sceneObjId, int maxItemCount, int creatorHandle)
{
    _rebuildRemoteItems = true;
    _creatorHandle = creatorHandle;
    double tr = 0.0;
    if (theObjectType & sim_drawing_50percenttransparency)
        tr += 0.5;
    if (theObjectType & sim_drawing_25percenttransparency)
        tr += 0.25;
    if (theObjectType & sim_drawing_12percenttransparency)
        tr += 0.125;
    color.setDefaultValues();

    if (tr != 0.0)
    {
        color.setTranslucid(true);
        color.setOpacity(1.0 - tr);
    }

    _objectId = -1;
    _sceneObjectId = -1;
    _sceneObjectUid = -1;

    size = tt::getLimitedFloat(0.0001, 100.0, size);
    _size = size;
    if (maxItemCount == 0)
        maxItemCount = 1000;
    maxItemCount = tt::getLimitedInt(1, 1000000, maxItemCount);
    _maxItemCount = maxItemCount;
    _startItem = 0;
    int tmp = theObjectType & 0x001f;
    if (theObjectType & sim_drawing_vertexcolors)
    {
        if ((tmp != sim_drawing_lines) && (tmp != sim_drawing_triangles) && (tmp != sim_drawing_linestrip))
            theObjectType -= sim_drawing_vertexcolors;
        if ((theObjectType & sim_drawing_itemcolors) && (theObjectType & sim_drawing_vertexcolors))
            theObjectType -= sim_drawing_vertexcolors;
        if (theObjectType & sim_drawing_itemtransparency)
            theObjectType -= sim_drawing_itemtransparency;
    }
    if (theObjectType & sim_drawing_itemsizes)
    {
        if (tmp == sim_drawing_triangles)
            theObjectType -= sim_drawing_itemsizes;
    }
    if (duplicateTolerance < 0.0)
        duplicateTolerance = 0.0;
    _duplicateTolerance = duplicateTolerance;
    _objectType = theObjectType;
    _setItemSizes();

    if (sceneObjId != -1)
    {
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(sceneObjId);
        if (it != nullptr)
        {
            _sceneObjectId = sceneObjId;
            _sceneObjectUid = it->getObjectUid();
        }
    }
}

CDrawingObject::~CDrawingObject()
{
}

int CDrawingObject::getObjectType() const
{
    return (_objectType);
}

int CDrawingObject::getSceneObjectId() const
{
    return (_sceneObjectId);
}

void CDrawingObject::setObjectId(int newId)
{
    _objectId = newId;
}

int CDrawingObject::getObjectId() const
{
    return _objectId;
}

void CDrawingObject::setItems(const double* itemData, size_t itemCnt)
{
    addItem(nullptr);
    addItems(itemData, itemCnt);
}

void CDrawingObject::addItems(const double* itemData, size_t itemCnt)
{
    size_t off = size_t(verticesPerItem * 3 + quaternionsPerItem * 4 + colorsPerItem * 3 + otherFloatsPerItem);
    for (size_t i = 0; i < itemCnt; i++)
        addItem(itemData + off * i);
}

bool CDrawingObject::addItem(const double* itemData)
{
    if (itemData == nullptr)
    {
        _data.clear();
        _bufferedEventData.clear();
        _startItem = 0;

        if ((otherFloatsPerItem == 0) && App::worldContainer->getEventsEnabled())
        {
#if SIM_EVENT_PROTOCOL_VERSION >= 3
            {
                CCbor* ev = App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED, _objectId, _objectId, nullptr, false);
                ev->appendKeyFloatArray("points", nullptr, 0);
                ev->appendKeyFloatArray("quaternions", nullptr, 0);
                ev->appendKeyFloatArray("colors", nullptr, 0);
                App::worldContainer->pushEvent();
            }
#endif
#if SIM_EVENT_PROTOCOL_VERSION <= 3
            // For backw. compatibility
            {
                CCbor* ev = App::worldContainer->createEvent("drawingObjectChanged", _objectId, _objectId, nullptr, false);
                ev->appendKeyBool("clearPoints", true);
                App::worldContainer->pushEvent();
            }
#endif
        }

        return (false);
    }

    C7Vector trInv;
    trInv.setIdentity();
    if ((_sceneObjectId >= 0) && ((_objectType & sim_drawing_local) == 0))
    {
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectId);
        if (it == nullptr)
            _sceneObjectId = -2; // should normally never happen!
        else
            trInv = it->getCumulativeTransformation().getInverse();
    }

    if ((_duplicateTolerance > 0.0) && (verticesPerItem == 1))
    { // Check for duplicates
        C3Vector v(itemData);
        v *= trInv;
        for (int i = 0; i < int(_data.size()) / floatsPerItem; i++)
        {
            C3Vector w(&_data[floatsPerItem * i + 0]);
            if ((w - v).getLength() <= _duplicateTolerance)
                return (false); // point already there!
        }
    }

    int newPos = _startItem;
    if (int(_data.size()) / floatsPerItem >= _maxItemCount)
    { // the buffer is full
        if (_objectType & sim_drawing_cyclic)
        {
            _startItem++;
            if (_startItem >= _maxItemCount)
                _startItem = 0;
        }
        else
            return (false); // saturated
    }

    int tmp = _objectType & 0x001f;
    bool usingNormals_old = ((tmp == sim_drawing_trianglepoints) || (tmp == sim_drawing_quadpoints) ||
                             (tmp == sim_drawing_discpoints) || (tmp == sim_drawing_cubepoints));

    if (int(_data.size()) / floatsPerItem < _maxItemCount)
    { // The buffer is not yet full!
        newPos = int(_data.size()) / floatsPerItem;
        for (int i = 0; i < floatsPerItem; i++)
            _data.push_back(0.0);
    }

    if (_sceneObjectId != -2)
    {
        int off1 = 0;
        int off2 = 0;
        for (int i = 0; i < verticesPerItem; i++)
        {
            C3Vector v(itemData + off2);
            v *= trInv;
            _data[newPos * floatsPerItem + off1 + 0] = v(0);
            _data[newPos * floatsPerItem + off1 + 1] = v(1);
            _data[newPos * floatsPerItem + off1 + 2] = v(2);
            if ((otherFloatsPerItem == 0) && App::worldContainer->getEventsEnabled())
            {
                _bufferedEventData.push_back(v(0));
                _bufferedEventData.push_back(v(1));
                _bufferedEventData.push_back(v(2));
            }
            off1 += 3;
            off2 += 3;
        }
        if (usingNormals_old)
        {
            for (int i = 0; i < quaternionsPerItem; i++)
            {
                C3X3Matrix m;
                m.axis[2] = C3Vector(itemData + off2);
                if (m.axis[2](0) < 0.1)
                {
                    C3Vector v(1.0, 0.0, 0.0);
                    m.axis[1] = (m.axis[2] ^ v).getNormalized();
                    m.axis[0] = m.axis[1] ^ m.axis[2];
                }
                else
                {
                    C3Vector v(0.0, 1.0, 0.0);
                    m.axis[0] = (v ^ m.axis[2]).getNormalized();
                    m.axis[1] = m.axis[2] ^ m.axis[0];
                }
                C4Vector q(m.getQuaternion());
                q = trInv.Q * q; // no translational part!
                _data[newPos * floatsPerItem + off1 + 0] = q(0);
                _data[newPos * floatsPerItem + off1 + 1] = q(1);
                _data[newPos * floatsPerItem + off1 + 2] = q(2);
                _data[newPos * floatsPerItem + off1 + 3] = q(3);
                if ((otherFloatsPerItem == 0) && App::worldContainer->getEventsEnabled())
                {
                    _bufferedEventData.push_back(q(0));
                    _bufferedEventData.push_back(q(1));
                    _bufferedEventData.push_back(q(2));
                    _bufferedEventData.push_back(q(3));
                }
                off1 += 4;
                off2 += 3;
            }
        }
        else
        {
            for (int i = 0; i < quaternionsPerItem; i++)
            {
                C4Vector q(itemData + off2, true);
                q = trInv.Q * q; // no translational part!
                _data[newPos * floatsPerItem + off1 + 0] = q(0);
                _data[newPos * floatsPerItem + off1 + 1] = q(1);
                _data[newPos * floatsPerItem + off1 + 2] = q(2);
                _data[newPos * floatsPerItem + off1 + 3] = q(3);
                if ((otherFloatsPerItem == 0) && App::worldContainer->getEventsEnabled())
                {
                    _bufferedEventData.push_back(q(0));
                    _bufferedEventData.push_back(q(1));
                    _bufferedEventData.push_back(q(2));
                    _bufferedEventData.push_back(q(3));
                }
                off1 += 4;
                off2 += 4;
            }
        }
        for (int i = 0; i < colorsPerItem * 3 + otherFloatsPerItem; i++)
        {
            _data[newPos * floatsPerItem + off1 + i] = itemData[off2 + i];
            if ((otherFloatsPerItem == 0) && App::worldContainer->getEventsEnabled())
                _bufferedEventData.push_back(itemData[off2 + i]);
        }
    }
    return (true);
}

void CDrawingObject::_setItemSizes()
{
    verticesPerItem = 0;
    quaternionsPerItem = 0;
    colorsPerItem = 0;
    otherFloatsPerItem = 0;
    int tmp = _objectType & 0x001f;
    if ((tmp == sim_drawing_points) || (tmp == sim_drawing_trianglepoints) || (tmp == sim_drawing_quadpoints) ||
        (tmp == sim_drawing_discpoints) || (tmp == sim_drawing_cubepoints) || (tmp == sim_drawing_spherepts) ||
        (tmp == sim_drawing_trianglepts) || (tmp == sim_drawing_quadpts) || (tmp == sim_drawing_discpts) ||
        (tmp == sim_drawing_cubepts))
        verticesPerItem = 1;
    if (tmp == sim_drawing_lines)
        verticesPerItem = 2;
    if (tmp == sim_drawing_linestrip)
        verticesPerItem = 1;
    if (tmp == sim_drawing_triangles)
        verticesPerItem = 3;

    if ((tmp == sim_drawing_trianglepts) || (tmp == sim_drawing_quadpts) || (tmp == sim_drawing_discpts) ||
        (tmp == sim_drawing_cubepts) || (tmp == sim_drawing_trianglepoints) || (tmp == sim_drawing_quadpoints) ||
        (tmp == sim_drawing_discpoints) || (tmp == sim_drawing_cubepoints))
    {
        if ((_objectType & sim_drawing_facingcamera) == 0)
            quaternionsPerItem = 1;
    }

    if (_objectType & sim_drawing_itemcolors)
        colorsPerItem += 1;
    if (_objectType & sim_drawing_vertexcolors)
    {
        if (tmp == sim_drawing_linestrip)
            colorsPerItem += 1;
        if (tmp == sim_drawing_lines)
            colorsPerItem += 2;
        if (tmp == sim_drawing_triangles)
            colorsPerItem += 3;
    }
    if (_objectType & sim_drawing_itemsizes)
        otherFloatsPerItem += 1;
    if (_objectType & sim_drawing_itemtransparency)
        otherFloatsPerItem += 1;

    floatsPerItem = 3 * verticesPerItem + 4 * quaternionsPerItem + 3 * colorsPerItem + otherFloatsPerItem;
}

bool CDrawingObject::announceObjectWillBeErased(const CSceneObject* object)
{
    return (_sceneObjectId == object->getObjectHandle());
}

bool CDrawingObject::announceScriptStateWillBeErased(int scriptHandle, bool simulationScript,
                                                     bool sceneSwitchPersistentScript)
{
    return ((!sceneSwitchPersistentScript) && (_creatorHandle == scriptHandle));
}

void CDrawingObject::_initBufferedEventData()
{
    _bufferedEventData.resize(_data.size());
    int index = _startItem;
    for (size_t itemCnt = 0; itemCnt < _data.size() / size_t(floatsPerItem); itemCnt++)
    {
        for (size_t i = 0; i < size_t(floatsPerItem); i++)
            _bufferedEventData[itemCnt * floatsPerItem + i] = _data[index * floatsPerItem + i];

        index++;
        if (index >= _maxItemCount)
            index -= _maxItemCount;
    }
    _rebuildRemoteItems = true;
}

void CDrawingObject::_getEventData(std::vector<float>& vertices, std::vector<float>& quaternions,
                                   std::vector<float>& colors) const
{
    size_t w = 0;
    if (_objectType & sim_drawing_itemcolors)
    {
        int tmp = _objectType & 0x001f;
        if (tmp == sim_drawing_lines)
            w = 2;
        if (tmp == sim_drawing_triangles)
            w = 3;
    }

    for (size_t itemCnt = 0; itemCnt < _bufferedEventData.size() / size_t(floatsPerItem); itemCnt++)
    {
        size_t t = itemCnt * floatsPerItem;

        for (int i = 0; i < verticesPerItem; i++)
        {
            vertices.push_back((float)_bufferedEventData[t + 0]);
            vertices.push_back((float)_bufferedEventData[t + 1]);
            vertices.push_back((float)_bufferedEventData[t + 2]);
            t += 3;
        }
        for (int i = 0; i < quaternionsPerItem; i++)
        {
            quaternions.push_back((float)_bufferedEventData[t + 1]);
            quaternions.push_back((float)_bufferedEventData[t + 2]);
            quaternions.push_back((float)_bufferedEventData[t + 3]);
            quaternions.push_back((float)_bufferedEventData[t + 0]);
            t += 4;
        }
        if (w == 0)
        {
            for (int i = 0; i < colorsPerItem; i++)
            {
                colors.push_back(_bufferedEventData[t + 0]);
                colors.push_back(_bufferedEventData[t + 1]);
                colors.push_back(_bufferedEventData[t + 2]);
                t += 3;
            }
        }
        else
        {
            if (colorsPerItem > 0)
            {
                for (size_t i = 0; i < w; i++)
                {
                    colors.push_back(_bufferedEventData[t + 0]);
                    colors.push_back(_bufferedEventData[t + 1]);
                    colors.push_back(_bufferedEventData[t + 2]);
                }
            }
        }
    }
    if (colorsPerItem == 0)
    { // this is the regular case when sim.drawing_itemcolors is not specified:
        int t = _objectType & 0x001f;
        w = 1;
        if (t == sim_drawing_lines)
            w = 2;
        if (t == sim_drawing_triangles)
            w = 3;
        const float* c = color.getColorsPtr();
        for (size_t itemCnt = 0; itemCnt < _bufferedEventData.size() / size_t(floatsPerItem); itemCnt++)
        {
            for (size_t j = 0; j < w; j++)
            {
                colors.push_back(c[0]);
                colors.push_back(c[1]);
                colors.push_back(c[2]);
            }
        }
    }
}

void CDrawingObject::pushAddEvent()
{
    if ((otherFloatsPerItem == 0) && App::worldContainer->getEventsEnabled())
    {
#if SIM_EVENT_PROTOCOL_VERSION >= 3
        {
            CCbor* ev = App::worldContainer->createEvent(EVENTTYPE_OBJECTADDED, _objectId, _objectId, nullptr, false);
            std::string tp;
            switch (_objectType & 0x001f)
            {
            case sim_drawing_points:
                tp = "point";
                break;
            case sim_drawing_lines:
                tp = "line";
                break;
            case sim_drawing_linestrip:
                tp = "lineStrip";
                break;
            case sim_drawing_triangles:
                tp = "triangle";
                break;
            case sim_drawing_trianglepts:
                tp = "trianglePoint";
                break;
            case sim_drawing_quadpts:
                tp = "quadPoint";
                break;
            case sim_drawing_discpts:
                tp = "discPoint";
                break;
            case sim_drawing_cubepts:
                tp = "cubePoint";
                break;
            case sim_drawing_spherepts:
                tp = "spherePoint";
                break;
            }
            ev->appendKeyText(propDrawingObj_objectType.name, OBJECT_TYPE.c_str());
            ev->appendKeyInt64(propDrawingObj_handle.name, _objectId);
#if SIM_EVENT_PROTOCOL_VERSION <= 3
            ev->appendKeyInt64(propDrawingObj_parent.name, _sceneObjectId);
#else
            ev->appendKeyHandle(propDrawingObj_parent.name, _sceneObjectId);
#endif
            ev->appendKeyText("type", tp.c_str());
            ev->appendKeyInt64("maxCnt", _maxItemCount);
            ev->appendKeyDouble("size", _size);
            ev->appendKeyInt64("parentUid", _sceneObjectUid);
            ev->appendKeyBool("cyclic", (_objectType & sim_drawing_cyclic) != 0);
            ev->appendKeyBool("overlay", _objectType & sim_drawing_overlay);
            App::worldContainer->pushEvent();
        }
#endif
#if SIM_EVENT_PROTOCOL_VERSION <= 3
        // For backw. compatibility
        {
            CCbor* ev = App::worldContainer->createEvent("drawingObjectAdded", _objectId, _objectId, nullptr, false);
            std::string tp;
            switch (_objectType & 0x001f)
            {
            case sim_drawing_points:
                tp = "point";
                break;
            case sim_drawing_lines:
                tp = "line";
                break;
            case sim_drawing_linestrip:
                tp = "lineStrip";
                break;
            case sim_drawing_triangles:
                tp = "triangle";
                break;
            case sim_drawing_trianglepts:
                tp = "trianglePoint";
                break;
            case sim_drawing_quadpts:
                tp = "quadPoint";
                break;
            case sim_drawing_discpts:
                tp = "discPoint";
                break;
            case sim_drawing_cubepts:
                tp = "cubePoint";
                break;
            case sim_drawing_spherepts:
                tp = "spherePoint";
                break;
            }
            ev->appendKeyText(propDrawingObj_objectType.name, OBJECT_TYPE.c_str());
            ev->appendKeyText("type", tp.c_str());
            ev->appendKeyInt64("maxCnt", _maxItemCount);
            ev->appendKeyDouble("size", _size);
            ev->appendKeyInt64("parentUid", _sceneObjectUid);
            ev->appendKeyBool("cyclic", (_objectType & sim_drawing_cyclic) != 0);
            ev->appendKeyBool("clearPoints", true);
            ev->appendKeyBool("overlay", _objectType & sim_drawing_overlay);
            App::worldContainer->pushEvent();
        }
#endif

        _initBufferedEventData();
    }
}

void CDrawingObject::pushAppendNewPointEvent()
{
    if ((_bufferedEventData.size() > 0) && App::worldContainer->getEventsEnabled())
    {
        std::vector<float> points;
        std::vector<float> quaternions;
        std::vector<float> colors;
        _getEventData(points, quaternions, colors);

#if SIM_EVENT_PROTOCOL_VERSION >= 3
        {
            CCbor* ev = App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED, _objectId, _objectId, nullptr, false);
            if (_rebuildRemoteItems)
            {
                ev->appendKeyFloatArray("points", points.data(), points.size());
                ev->appendKeyFloatArray("quaternions", quaternions.data(), quaternions.size());
                ev->appendKeyFloatArray("colors", colors.data(), colors.size());
            }
            else
            {
                ev->appendKeyFloatArray("appendPoints", points.data(), points.size());
                ev->appendKeyFloatArray("appendQuaternions", quaternions.data(), quaternions.size());
                ev->appendKeyFloatArray("appendColors", colors.data(), colors.size());
            }
            App::worldContainer->pushEvent();
        }
#endif
#if SIM_EVENT_PROTOCOL_VERSION <= 3
        // For backw. compatibility
        {
            CCbor* ev = App::worldContainer->createEvent("drawingObjectChanged", _objectId, _objectId, nullptr, false);
            ev->appendKeyFloatArray("points", points.data(), points.size());
            ev->appendKeyFloatArray("quaternions", quaternions.data(), quaternions.size());
            ev->appendKeyFloatArray("colors", colors.data(), colors.size());
            ev->appendKeyBool("clearPoints", _rebuildRemoteItems);
            App::worldContainer->pushEvent();
        }
#endif

        _bufferedEventData.clear();
        _rebuildRemoteItems = false;
    }
}

int CDrawingObject::getLongProperty(const char* ppName, long long int& pState) const
{
    std::string _pName(ppName);
    int retVal = -1;

    if (_pName == propDrawingObj_handle.name)
    {
        retVal = 1;
        pState = _objectId;
    }
 
    return retVal;
}

int CDrawingObject::getHandleProperty(const char* ppName, long long int& pState) const
{
    std::string _pName(ppName);
    int retVal = -1;

    if (_pName == propDrawingObj_parent.name)
    {
        retVal = 1;
        pState = _sceneObjectId;
    }

    return retVal;
}

int CDrawingObject::getStringProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(ppName);
    int retVal = -1;

    if (_pName == propDrawingObj_objectType.name)
    {
        retVal = 1;
        pState = OBJECT_TYPE;
    }
    else if (_pName == propDrawingObj_objectMetaInfo.name)
    {
        retVal = 1;
        pState = OBJECT_META_INFO;
    }

    return retVal;
}

int CDrawingObject::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_drawingObj.size(); i++)
    {
        if ((pName.size() == 0) || utils::startsWith(allProps_collection[i].name, pName.c_str()))
        {
            if ((allProps_drawingObj[i].flags & excludeFlags) == 0)
            {
                index--;
                if (index == -1)
                {
                    pName = allProps_drawingObj[i].name;
                    retVal = 1;
                    break;
                }
            }
        }
    }
    return retVal;
}

int CDrawingObject::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt)
{
    const char* pName = ppName;
    int retVal = -1;
    for (size_t i = 0; i < allProps_drawingObj.size(); i++)
    {
        if (strcmp(allProps_drawingObj[i].name, pName) == 0)
        {
            retVal = allProps_drawingObj[i].type;
            info = allProps_drawingObj[i].flags;
            if (infoTxt == "j")
                infoTxt = allProps_drawingObj[i].shortInfoTxt;
            else
            {
                auto w = QJsonDocument::fromJson(allProps_drawingObj[i].shortInfoTxt.c_str()).object();
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
    return retVal;
}

#ifdef SIM_WITH_GUI
void CDrawingObject::draw(bool overlay, bool transparentObject, int displayAttrib, const C4X4Matrix& cameraCTM)
{
    if (displayAttrib & sim_displayattribute_colorcoded)
        return;

    if (_objectType & sim_drawing_overlay)
    {
        if (!overlay)
            return;
    }
    else
    {
        if (overlay)
            return;
    }

    if (!overlay)
    {
        if (_objectType & (sim_drawing_50percenttransparency + sim_drawing_25percenttransparency +
                           sim_drawing_12percenttransparency + sim_drawing_itemtransparency))
        {
            if (!transparentObject)
                return;
        }
        else
        {
            if (transparentObject)
                return;
        }
    }

    C7Vector tr;
    tr.setIdentity();

    if (_sceneObjectId >= 0)
    {
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(_sceneObjectId);
        if (it == nullptr)
            _sceneObjectId = -2; // should normally never happen
        else
        {
            tr = it->getCumulativeTransformation();
            if (_objectType & sim_drawing_followparentvisibility)
            {
                if (((App::currentWorld->environment->getActiveLayers() & it->getVisibilityLayer()) == 0) &&
                    ((displayAttrib & sim_displayattribute_ignorelayer) == 0))
                    return; // not visible
                if (it->isObjectPartOfInvisibleModel())
                    return; // not visible
                if (((_objectType & sim_drawing_painttag) == 0) &&
                    (displayAttrib & sim_displayattribute_forvisionsensor))
                    return; // not visible
            }
            else
            {
                if (((_objectType & sim_drawing_painttag) == 0) &&
                    (displayAttrib & sim_displayattribute_forvisionsensor))
                    return; // not visible
            }
        }
    }
    if (_sceneObjectId == -2)
        return;

    displayDrawingObject(this, tr, overlay, transparentObject, displayAttrib, cameraCTM);
}
#endif
