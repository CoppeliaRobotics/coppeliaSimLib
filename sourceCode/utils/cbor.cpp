#include <cbor.h>
#include <app.h>
#include <string.h>
#include <utils.h>

#define USE_TAGGED_ARRAYS (true)
#define NO_DATAFIELD_HANDLE(code) \
do { \
        (_handleDataFieldDisableLevel)++; \
        do { code } while(0); \
        (_handleDataFieldDisableLevel)--; \
} while(0)

std::set<std::string> CCbor::allEVentFieldNames;

bool CCbor::isText(const char* v, size_t l)
{
    for (size_t i = 0; i < l; i++)
    {
        if (v[i] >= 127)
            return (false);
        if ((v[i] <= 31) && (v[i] != 9) && (v[i] != 13))
            return (false);
    }
    return true;
}

CCbor::CCbor(const std::string* initBuff /*=nullptr*/, int options /*=0*/)
{
    clear();
    _options = options;
    if (initBuff != nullptr)
        _buff.assign(initBuff->begin(), initBuff->end());
}

CCbor::~CCbor()
{
}

void CCbor::swapWithEmptyBuffer(std::vector<unsigned char>* emptyBuff)
{
    _buff.swap(emptyBuff[0]);
    clear();
    _buff.clear();
}

void CCbor::appendInt64(int64_t v)
{
    _handleDataField();
    unsigned char add = 0;
    if (v < 0)
    {
        v = -v;
        v = v - 1;
        add = 32;
    }
    _appendItemTypeAndLength(add, v);
}

void CCbor::appendHandle(int64_t h)
{
    if ((h >= 0) && (_createdObjects_events.find(h) == _createdObjects_events.end()) && (_eventInfos.size() > 0))
    { // referencing an object not yet created...
        SEventInf* inf = &_eventInfos[_eventInfos.size() - 1];
        inf->unknownObjects.insert(h);
    }

    _handleDataField();
    _buff.push_back(0xDB); // Tag header (219)
    int64_t w = 4294999999; // Type info (handle)
    _buff.push_back(((unsigned char*)&w)[7]);
    _buff.push_back(((unsigned char*)&w)[6]);
    _buff.push_back(((unsigned char*)&w)[5]);
    _buff.push_back(((unsigned char*)&w)[4]);
    _buff.push_back(((unsigned char*)&w)[3]);
    _buff.push_back(((unsigned char*)&w)[2]);
    _buff.push_back(((unsigned char*)&w)[1]);
    _buff.push_back(((unsigned char*)&w)[0]);
    NO_DATAFIELD_HANDLE({
        appendInt64(h);
    });
}

void CCbor::appendUint8Array(const unsigned char* v, size_t cnt)
{
    _handleDataField();
    if (USE_TAGGED_ARRAYS)
    {
        _buff.push_back(0xD8); // Tag header
        _buff.push_back(0x40); // 64
        _appendItemTypeAndLength(0x40, cnt);
        if (cnt > 0)
            _buff.insert(_buff.end(), v, v + cnt);
    }
    else
    {
        NO_DATAFIELD_HANDLE({
            openArray(cnt);
            for (size_t i = 0; i < cnt; i++)
                appendInt64(v[i]);
            closeArrayOrMap();
        });
    }
}

void CCbor::appendInt32Array(const int* v, size_t cnt)
{
    _handleDataField();
    if (USE_TAGGED_ARRAYS)
    {
        _buff.push_back(0xD8); // Tag header
        _buff.push_back(0x4e); // 78
        _appendItemTypeAndLength(0x40, cnt * sizeof(int));
        if (cnt > 0)
            _buff.insert(_buff.end(), (unsigned char*)v, ((unsigned char*)v) + cnt * sizeof(int));
    }
    else
    {
        NO_DATAFIELD_HANDLE({
            openArray(cnt);
            for (size_t i = 0; i < cnt; i++)
                appendInt64(v[i]);
            closeArrayOrMap();
        });
    }
}

void CCbor::appendUint32Array(const unsigned int* v, size_t cnt)
{
    _handleDataField();
    if (USE_TAGGED_ARRAYS)
    {
        _buff.push_back(0xD8); // Tag header
        _buff.push_back(0x46); // 70
        _appendItemTypeAndLength(0x40, cnt * sizeof(unsigned int));
        if (cnt > 0)
            _buff.insert(_buff.end(), (unsigned char*)v, ((unsigned char*)v) + cnt * sizeof(unsigned int));
    }
    else
    {
        NO_DATAFIELD_HANDLE({
            openArray(cnt);
            for (size_t i = 0; i < cnt; i++)
                appendInt64(v[i]);
            closeArrayOrMap();
        });
    }
}

void CCbor::appendInt64Array(const int64_t* v, size_t cnt)
{
    _handleDataField();
    if (USE_TAGGED_ARRAYS)
    {
        _buff.push_back(0xD8); // Tag header
        _buff.push_back(0x4f); // 79
        _appendItemTypeAndLength(0x40, cnt * sizeof(int64_t));
        if (cnt > 0)
            _buff.insert(_buff.end(), (unsigned char*)v, ((unsigned char*)v) + cnt * sizeof(int64_t));
    }
    else
    {
        NO_DATAFIELD_HANDLE({
            openArray(cnt);
            for (size_t i = 0; i < cnt; i++)
                appendInt64(v[i]);
            closeArrayOrMap();
        });
    }
}

void CCbor::appendHandleArray(const int64_t* h, size_t cnt)
{
    if ((cnt > 0) && (_eventInfos.size() > 0))
    {
        SEventInf* inf = &_eventInfos[_eventInfos.size() - 1];
        for (size_t i = 0; i < cnt; i++)
        {
            int64_t hh = h[i];
            if ((hh >= 0) && (_createdObjects_events.find(hh) == _createdObjects_events.end()))
                inf->unknownObjects.insert(hh); // referencing an object not yet created...
        }
    }

    _handleDataField();
    _buff.push_back(0xDB); // Tag header (219)
    int64_t w = 4294999998; // Type info (handle array)
    _buff.push_back(((unsigned char*)&w)[7]);
    _buff.push_back(((unsigned char*)&w)[6]);
    _buff.push_back(((unsigned char*)&w)[5]);
    _buff.push_back(((unsigned char*)&w)[4]);
    _buff.push_back(((unsigned char*)&w)[3]);
    _buff.push_back(((unsigned char*)&w)[2]);
    _buff.push_back(((unsigned char*)&w)[1]);
    _buff.push_back(((unsigned char*)&w)[0]);
    NO_DATAFIELD_HANDLE({
        openArray(int(cnt));
        for (size_t i = 0; i < cnt; i++)
            appendInt64(h[i]);
        closeArrayOrMap();
    });
}

void CCbor::appendHandleArray(const std::vector<CSceneObject*>& h)
{
    _handleDataField();
    std::vector<int64_t> arr;
    arr.resize(h.size());
    for (size_t i = 0; i < h.size(); i++)
        arr[i] = h[i]->getObjectHandle();
    NO_DATAFIELD_HANDLE({
        appendHandleArray(arr.data(), arr.size());
    });
}

void CCbor::appendHandleArray(const int* h, size_t cnt)
{
    _handleDataField();
    std::vector<int64_t> arr;
    arr.resize(cnt);
    for (size_t i = 0; i < cnt; i++)
        arr[i] = h[i];
    NO_DATAFIELD_HANDLE({
        appendHandleArray(arr.data(), cnt);
    });
}

void CCbor::appendFloat(float v)
{
    _handleDataField();
    _buff.push_back(128 + 64 + 32 + 26);
    _buff.push_back(((unsigned char*)&v)[3]);
    _buff.push_back(((unsigned char*)&v)[2]);
    _buff.push_back(((unsigned char*)&v)[1]);
    _buff.push_back(((unsigned char*)&v)[0]);
}

void CCbor::appendFloatArray(const float* v, size_t cnt)
{
    _handleDataField();
    if (USE_TAGGED_ARRAYS)
    {
        _buff.push_back(0xD8); // Tag header
        _buff.push_back(0x55); // 85
        _appendItemTypeAndLength(0x40, cnt * sizeof(float));
        if (cnt > 0)
            _buff.insert(_buff.end(), (unsigned char*)v, ((unsigned char*)v) + cnt * sizeof(float));
    }
    else
    {
        NO_DATAFIELD_HANDLE({
            openArray(int(cnt));
            for (size_t i = 0; i < cnt; i++)
                appendFloat(v[i]);
            closeArrayOrMap();
        });
    }
}

void CCbor::appendDouble(double v)
{
    if ((_options & 1) == 0)
        appendFloat(float(v)); // treat doubles as floats
    else
    {
        _handleDataField();
        _buff.push_back(128 + 64 + 32 + 27);
        _buff.push_back(((unsigned char*)&v)[7]);
        _buff.push_back(((unsigned char*)&v)[6]);
        _buff.push_back(((unsigned char*)&v)[5]);
        _buff.push_back(((unsigned char*)&v)[4]);
        _buff.push_back(((unsigned char*)&v)[3]);
        _buff.push_back(((unsigned char*)&v)[2]);
        _buff.push_back(((unsigned char*)&v)[1]);
        _buff.push_back(((unsigned char*)&v)[0]);
    }
}

void CCbor::appendDoubleArray(const double* v, size_t cnt)
{
    _handleDataField();
    if (USE_TAGGED_ARRAYS)
    {
        _buff.push_back(0xD8); // Tag header (216)
        _buff.push_back(0x56); // 86
        _appendItemTypeAndLength(0x40, cnt * sizeof(double));
        if (cnt > 0)
            _buff.insert(_buff.end(), (unsigned char*)v, ((unsigned char*)v) + cnt * sizeof(double));
    }
    else
    {
        NO_DATAFIELD_HANDLE({
            openArray(int(cnt));
            for (size_t i = 0; i < cnt; i++)
                appendDouble(v[i]);
            closeArrayOrMap();
        });
    }
}

void CCbor::appendMatrix(const float* v, size_t rows, size_t cols, bool dataIsRowMajor /*= true*/)
{
    _handleDataField();
    _buff.push_back(0xD8); // major type 6, tag header (216)
    _buff.push_back(40); // tag 40 for matrices
    _buff.push_back(0x82); // array of 2 values (dims + data)
    _buff.push_back(0x82); // array of 2 values (rows and cols)
    _appendItemTypeAndLength(0, rows);
    _appendItemTypeAndLength(0, cols);
    NO_DATAFIELD_HANDLE({
        openArray(int(rows * cols));
        if (dataIsRowMajor)
        {
            for (size_t i = 0; i < rows * cols; i++)
                appendFloat(v[i]);
        }
        else
        {
            for (size_t r = 0; r < rows; r++)
            {
                for (size_t c = 0; c < cols; c++)
                    appendFloat(v[c * rows + r]);
            }
        }
        closeArrayOrMap();
    });
}

void CCbor::appendMatrix(const double* v, size_t rows, size_t cols, bool dataIsRowMajor /*= true*/)
{
    _handleDataField();
    _buff.push_back(0xD8); // major type 6, tag header (216)
    _buff.push_back(40); // tag 40 for matrices
    _buff.push_back(0x82); // array of 2 values (dims + data)
    _buff.push_back(0x82); // array of 2 values (rows and cols)
    _appendItemTypeAndLength(0, rows);
    _appendItemTypeAndLength(0, cols);
    NO_DATAFIELD_HANDLE({
        openArray(int(rows * cols));
        if (dataIsRowMajor)
        {
            for (size_t i = 0; i < rows * cols; i++)
                appendDouble(v[i]);
        }
        else
        {
            for (size_t r = 0; r < rows; r++)
            {
                for (size_t c = 0; c < cols; c++)
                    appendDouble(v[c * rows + r]);
            }
        }
        closeArrayOrMap();
    });
}

void CCbor::appendMatrix(const C3X3Matrix& m)
{
    double v[9];
    m.getData(v);
    appendMatrix(v, 3, 3);
}

void CCbor::appendMatrix(const C4X4Matrix& m)
{
    double v[16];
    m.getData(v);
    appendMatrix(v, 4, 4);
}

void CCbor::appendMatrix(const CMatrix& m)
{
    appendMatrix(m.data.data(), m.rows, m.cols);
}

void CCbor::appendVector2(const double* v)
{
    appendMatrix(v, 2, 1);
}

void CCbor::appendVector3(const double* v)
{
    appendMatrix(v, 3, 1);
}

void CCbor::appendVector3(const C3Vector& v)
{
    appendMatrix(v.data, 3, 1);
}

void CCbor::appendQuaternion(const double* v, bool xyzwLayout /*= false*/)
{
    _handleDataField();
    _buff.push_back(0xDB); // Tag header (219)
    int64_t w = 4294980000; // Type info (quaternion)
    _buff.push_back(((unsigned char*)&w)[7]);
    _buff.push_back(((unsigned char*)&w)[6]);
    _buff.push_back(((unsigned char*)&w)[5]);
    _buff.push_back(((unsigned char*)&w)[4]);
    _buff.push_back(((unsigned char*)&w)[3]);
    _buff.push_back(((unsigned char*)&w)[2]);
    _buff.push_back(((unsigned char*)&w)[1]);
    _buff.push_back(((unsigned char*)&w)[0]);

    NO_DATAFIELD_HANDLE({
        openArray(4);
        if (xyzwLayout)
        {
            appendDouble(v[0]);
            appendDouble(v[1]);
            appendDouble(v[2]);
            appendDouble(v[3]);
        }
        else
        {
            appendDouble(v[1]);
            appendDouble(v[2]);
            appendDouble(v[3]);
            appendDouble(v[0]);
        }
        closeArrayOrMap();
    });
}

void CCbor::appendQuaternion(const CQuaternion& q)
{
    appendQuaternion(q.data, false);
}

void CCbor::appendPose(const double* v, bool xyzqxqyqzqwLayout /*= false*/)
{
    _handleDataField();
    _buff.push_back(0xDB); // Tag header (219)
    int64_t w = 4294980500; // Type info (pose)
    _buff.push_back(((unsigned char*)&w)[7]);
    _buff.push_back(((unsigned char*)&w)[6]);
    _buff.push_back(((unsigned char*)&w)[5]);
    _buff.push_back(((unsigned char*)&w)[4]);
    _buff.push_back(((unsigned char*)&w)[3]);
    _buff.push_back(((unsigned char*)&w)[2]);
    _buff.push_back(((unsigned char*)&w)[1]);
    _buff.push_back(((unsigned char*)&w)[0]);
    NO_DATAFIELD_HANDLE({
        openArray(7);
        if (xyzqxqyqzqwLayout)
        {
            appendDouble(v[0]);
            appendDouble(v[1]);
            appendDouble(v[2]);
            appendDouble(v[3]);
            appendDouble(v[4]);
            appendDouble(v[5]);
            appendDouble(v[6]);
        }
        else
        {
            appendDouble(v[0]);
            appendDouble(v[1]);
            appendDouble(v[2]);
            appendDouble(v[4]);
            appendDouble(v[5]);
            appendDouble(v[6]);
            appendDouble(v[3]);
        }
        closeArrayOrMap();
    });
}

void CCbor::appendPose(const CPose& p)
{
    double w[7] = {p.X(0), p.X(1), p.X(2), p.Q(1), p.Q(2), p.Q(3), p.Q(0)};
    appendPose(w, true);
}

void CCbor::appendColor3(const float c[3])
{
    float cc[4] = {c[0], c[1], c[2], 1.0f};
    appendColor(cc);
}

/*
void CCbor::appendColor(const float c[4])
{
    _handleDataField();

    _buff.push_back(0xDB); // Tag header (219)
    int64_t w = 4294970000; // Type info (color)
    _buff.push_back(((unsigned char*)&w)[7]);
    _buff.push_back(((unsigned char*)&w)[6]);
    _buff.push_back(((unsigned char*)&w)[5]);
    _buff.push_back(((unsigned char*)&w)[4]);
    _buff.push_back(((unsigned char*)&w)[3]);
    _buff.push_back(((unsigned char*)&w)[2]);
    _buff.push_back(((unsigned char*)&w)[1]);
    _buff.push_back(((unsigned char*)&w)[0]);
    NO_DATAFIELD_HANDLE({
        openArray(4);
        appendDouble(c[0]);
        appendDouble(c[1]);
        appendDouble(c[2]);
        appendDouble(c[3]);
        closeArrayOrMap();
    });
}
*/
void CCbor::appendColor(const float c[4])
{
    _handleDataField();

    _buff.push_back(0xDB); // Tag header (219)
    int64_t w = 4294970000; // Type info (color)
    _buff.push_back(((unsigned char*)&w)[7]);
    _buff.push_back(((unsigned char*)&w)[6]);
    _buff.push_back(((unsigned char*)&w)[5]);
    _buff.push_back(((unsigned char*)&w)[4]);
    _buff.push_back(((unsigned char*)&w)[3]);
    _buff.push_back(((unsigned char*)&w)[2]);
    _buff.push_back(((unsigned char*)&w)[1]);
    _buff.push_back(((unsigned char*)&w)[0]);

    _buff.push_back((uint8_t)(0x44));

    _buff.push_back((uint8_t)(c[0] * 255.1f));
    _buff.push_back((uint8_t)(c[1] * 255.1f));
    _buff.push_back((uint8_t)(c[2] * 255.1f));
    _buff.push_back((uint8_t)(c[3] * 255.1f));
}

void CCbor::appendNull()
{
    _handleDataField();
    _buff.push_back(128 + 64 + 32 + 22);
}

void CCbor::appendBool(bool v)
{
    _handleDataField();
    if (v)
        _buff.push_back(128 + 64 + 32 + 21);
    else
        _buff.push_back(128 + 64 + 32 + 20);
}

void CCbor::_appendItemTypeAndLength(unsigned char t, int64_t l)
{
    if (l < 24)
        _buff.push_back(t + (unsigned char)l);
    else if (l <= 0xff)
    {
        _buff.push_back(t + 24);
        _buff.push_back((unsigned char)l);
    }
    else if (l <= 0xffff)
    {
        _buff.push_back(t + 25);
        _buff.push_back(((unsigned char*)&l)[1]);
        _buff.push_back(((unsigned char*)&l)[0]);
    }
    else if (l <= 0xffffffff)
    {
        _buff.push_back(t + 26);
        _buff.push_back(((unsigned char*)&l)[3]);
        _buff.push_back(((unsigned char*)&l)[2]);
        _buff.push_back(((unsigned char*)&l)[1]);
        _buff.push_back(((unsigned char*)&l)[0]);
    }
    else
    {
        _buff.push_back(t + 27);
        _buff.push_back(((unsigned char*)&l)[7]);
        _buff.push_back(((unsigned char*)&l)[6]);
        _buff.push_back(((unsigned char*)&l)[5]);
        _buff.push_back(((unsigned char*)&l)[4]);
        _buff.push_back(((unsigned char*)&l)[3]);
        _buff.push_back(((unsigned char*)&l)[2]);
        _buff.push_back(((unsigned char*)&l)[1]);
        _buff.push_back(((unsigned char*)&l)[0]);
    }
}

void CCbor::appendBuff(const unsigned char* v, size_t l)
{
    _handleDataField();
    _appendItemTypeAndLength(0x40, l);
    for (size_t i = 0; i < l; i++)
        _buff.push_back(v[i]);
}

void CCbor::appendText(const char* v, int l /*=-1*/)
{
    if (l < 0)
        l = int(strlen(v));
    if (utils::isValidUtf8(v, l))
    {
        _handleDataField(v);
        _appendItemTypeAndLength(64 + 32, size_t(l));
        for (size_t i = 0; i < size_t(l); i++)
            _buff.push_back(v[i]);
    }
    else
    {
        std::string str = utils::toWellFormedUtf8(v, l);
        appendText(str.data(), str.size());
    }
}

void CCbor::appendTextArray(const std::vector<std::string>& txtArr)
{
    _handleDataField();
    NO_DATAFIELD_HANDLE({
        openArray(int(txtArr.size())); // _handleDataField() called in there
        for (size_t i = 0; i < txtArr.size(); i++)
            appendText(txtArr[i].c_str());
        closeArrayOrMap();
    });
}

void CCbor::appendRaw(const unsigned char* v, size_t l)
{
    _handleDataField();
    _buff.insert(_buff.end(), v, v + l);
}

void CCbor::appendLuaString(const std::string& v, bool isBuffer, bool isText)
{
    std::string suff;
    if (v.size() >= 6)
        suff.assign(v.begin() + v.size() - 6, v.end());
    if (suff == "@:txt:")
        appendText(v.c_str(), int(v.size()) - 6);
    else if (suff == "@:dat:")
        appendBuff((unsigned char*)v.c_str(), v.size() - 6);
    else
    { // following modified on 12.03.2024 (buffer/string/text differentiation)
        if (isBuffer)
            appendBuff((unsigned char*)v.c_str(), v.size());
        else
        {
            if (isText)
                appendText(v.c_str(), int(v.size()));
            else
            { // we have a binary string (could contain text chars only):
                if (CCbor::isText(v.c_str(), int(v.size())))
                    appendText(v.c_str(), int(v.size()));
                else
                    appendBuff((unsigned char*)v.c_str(), v.size());
            }
        }
        /*
        if (isText(v.c_str(), int(v.size())))
            appendString(v.c_str(), int(v.size()));
        else
            appendBuff((unsigned char *)v.c_str(), v.size());
            */
    }
}

void CCbor::openArray(int sizedArray /*= -1*/)
{
    _handleDataField();
    _eventDepth++;
    if (sizedArray == -1)
        _buff.push_back(128 + 31); // array + use a break char
    else
        _appendItemTypeAndLength(128, (size_t)sizedArray);
    _sizedArrayInfo.push_back(sizedArray != -1);
}

void CCbor::openMap(int sizedMap /*= -1*/)
{
    _handleDataField();
    _eventDepth++;
    if (sizedMap == -1)
        _buff.push_back(128 + 32 + 31); // map + use a break char
    else
        _appendItemTypeAndLength(128 + 32, (size_t)sizedMap);
    _sizedArrayInfo.push_back(sizedMap != -1);
}

void CCbor::closeArrayOrMap()
{
    if ((_eventDepth == 2) && _inDataField)
    { // we close the data field
        _inDataField = false;
        SEventInf* inf = &_eventInfos[_eventInfos.size() - 1];
        // for last key-value pair:
        if (inf->fieldPositions.size() > 0)
            inf->fieldSizes.push_back(_buff.size() - inf->fieldPositions[inf->fieldPositions.size() - 1]);
    }
    _eventDepth--;
    if (!_sizedArrayInfo.back())
        _buff.push_back(255); // break char
    _sizedArrayInfo.pop_back();
}

void CCbor::clear()
{
    // do not clear _buff in here! _buff.clear();
    _eventInfos.clear();
    _eventInfos_forReorder.clear();
    _eventInfos_forPopRepush.clear();
    _infoIndex_forPopRepush = -1;
    _buff_forReorder.clear();
    _buff_forPopRepush.clear();
    _eventDepth = 0;
    _eventOpen = false;
    _nextIsKeyInData = true;
    _inDataField = false;
    _handleDataFieldDisableLevel = 0;
}

std::string CCbor::getBuff() const
{
    std::string retVal;
    retVal.assign(_buff.begin(), _buff.end());
    return (retVal);
}

const unsigned char* CCbor::getBuff(size_t& l) const
{
    l = _buff.size();
    return (_buff.data());
}

size_t CCbor::getEventDepth() const
{
    return (_eventDepth);
}

void CCbor::createEvent(const char* event, const char* fieldName, const char* objType, int64_t handle, int64_t uid, bool mergeable, bool openDataField /*=true*/)
{
    if (strcmp(event, EVENTTYPE_OBJECTADDED) == 0)
        _createdObjects_events.insert(handle);
    else if (strcmp(event, EVENTTYPE_OBJECTREMOVED) == 0)
        _createdObjects_events.erase(handle);
    else if (strcmp(event, EVENTTYPE_GENESISBEGIN) == 0)
        _createdObjects_events.clear();

    if (_eventOpen)
    {
        printf("[CoppeliaSim:error] creating an event where an event push is expected.\n");
        App::logMsg(sim_verbosity_errors, "creating an event where an event push is expected.");
    }
    _eventOpen = true;

    SEventInf inf;
    inf.pos = _buff.size();
    inf.target = handle;
    inf.event = event;
    if (mergeable)
    {
        inf.eventId = event;
        if (fieldName != nullptr)
            inf.eventId += fieldName;
        if (objType != nullptr)
            inf.eventId += objType;
        if (uid != -1)
            inf.eventId += std::to_string(uid);
    }
    _eventInfos.push_back(inf);

    openMap(); // holding the event
    appendKeyText("event", event);
    if (uid != -1)
        appendKeyInt64("uid", uid);
    if (handle != -1)
        appendKeyInt64("handle", handle);
    if (openDataField)
    {
        appendText("data");
        openMap(); // holding the data
        _inDataField = true;
    }
    // Do not open any other map or array below here
}

void CCbor::pushEvent()
{
    if (_eventOpen)
    {
        while (_eventDepth > 1)
            closeArrayOrMap(); // make sure to close the current event's arrays/maps, except for the one holding the event
        _eventDepth = 0;       // yes, we intentionally forget to close the last array/map, but we anyways reset the depth to zero
        _eventOpen = false;
        _nextIsKeyInData = true;
    }
    else
        App::logMsg(sim_verbosity_errors, "pushing an event that doesn't exist.");

    SEventInf* inf = &_eventInfos[_eventInfos.size() - 1];
    inf->size = _buff.size() - inf->pos;

    if (_allowEventsReordering == 2)
    {
        _eventInfos_forReorder.push_back(inf[0]);
        std::vector<unsigned char> v(_buff.begin() + inf->pos, _buff.end());
        _buff_forReorder.push_back(v);
        _buff.resize(inf->pos);
        _eventInfos.pop_back();
    }
    else if (_allowEventsReordering == 1)
    {
        for (size_t i = 0; i < _eventInfos_forReorder.size(); i++)
        {
            auto& delayed = _eventInfos_forReorder[i];
            size_t newPos = _buff.size();
            size_t oldPos = delayed.pos;
            size_t delta = newPos - oldPos;
            for (size_t& fp : delayed.fieldPositions)
                fp += delta;
            delayed.pos = newPos;
            _eventInfos.push_back(delayed);
            _buff.insert(_buff.end(), _buff_forReorder[i].begin(), _buff_forReorder[i].end());
        }
        _buff_forReorder.clear();
        _eventInfos_forReorder.clear();
        _allowEventsReordering = 0;
    }
    else
    {
        if (!inf->unknownObjects.empty())
        {
            std::string txt;
            txt += "Event '";
            txt += inf->event + "' with handle ";
            txt += std::to_string(inf->target) + " references following unknown object(s): ";
            int cnt = 0;
            for (int x : inf->unknownObjects)
            {
                if (cnt != 0)
                    txt += ", ";
                txt += std::to_string(x);
                cnt++;
            }
            /*
            txt += "\n Known objects: ";
            cnt = 0;
            for (int x : _createdObjects_events)
            {
                if (cnt != 0)
                    txt += ", ";
                txt += std::to_string(x);
                cnt++;
            }
            */
            App::logMsg(sim_verbosity_errors, txt.c_str());
            App::logScriptMsg(nullptr, sim_verbosity_scripterrors, txt.c_str());
        }
    }
    /*
    if (!inf->unknownObjects.empty())
    {
        _eventInfos_forReorder.push_back(inf[0]);
        std::vector<unsigned char> v(_buff.begin() + inf->pos, _buff.end());
        _buff_forReorder.push_back(v);
        _buff.resize(inf->pos);
        _eventInfos.pop_back();
    }
    else if ((inf->event.compare(EVENTTYPE_OBJECTADDED) == 0) && (_eventInfos_forReorder.size() > 0))
    {
        std::vector<int64_t> addedObjList;
        addedObjList.push_back(inf->target);
        while (addedObjList.size() > 0)
        {
            int64_t addedHandle = addedObjList[0];
            addedObjList.erase(addedObjList.begin());
            for (int i = 0; i < int(_eventInfos_forReorder.size()); i++)
            {
                auto& delayed = _eventInfos_forReorder[i];
                if (delayed.unknownObjects.find(addedHandle) != delayed.unknownObjects.end())
                {
                    delayed.unknownObjects.erase(addedHandle);
                    if (delayed.unknownObjects.empty())
                    {
                        if ((delayed.event.compare(EVENTTYPE_OBJECTADDED) == 0) && (_eventInfos_forReorder.size() > 1))
                            addedObjList.push_back(delayed.target); // the inserted event could trigger other insertions too
                        size_t newPos = _buff.size();
                        size_t oldPos = delayed.pos;
                        size_t delta = newPos - oldPos;
                        for (size_t& fp : delayed.fieldPositions)
                            fp += delta;
                        delayed.pos = newPos;
                        _eventInfos.push_back(delayed);

                        _buff.insert(_buff.end(), _buff_forReorder[i].begin(), _buff_forReorder[i].end());
                        _buff_forReorder.erase(_buff_forReorder.begin() + i);
                        _eventInfos_forReorder.erase(_eventInfos_forReorder.begin() + i);
                        i--; // reprocess this position
                    }
                }
            }
        }
    }
    */
}

void CCbor::mark()
{
    _infoIndex_forPopRepush = _eventInfos.size();
}

void CCbor::popAfterMark()
{
    _buff_forPopRepush.clear();
    _eventInfos_forPopRepush.clear();
    if (_infoIndex_forPopRepush < _eventInfos.size())
    {
        size_t start = 0;
        while (_infoIndex_forPopRepush < _eventInfos.size())
        {
            size_t p = _eventInfos[_infoIndex_forPopRepush].pos;
            if (_buff_forPopRepush.size() == 0)
                start = p;
            size_t s = _eventInfos[_infoIndex_forPopRepush].size;
            _buff_forPopRepush.push_back(std::vector<unsigned char>(_buff.begin() + p, _buff.begin() + p + s));
            _eventInfos_forPopRepush.push_back(_eventInfos[_infoIndex_forPopRepush]);
            _eventInfos.erase(_eventInfos.begin() + _infoIndex_forPopRepush);
        }
        _buff.erase(_buff.begin() + start, _buff.end());
    }
}

void CCbor::repush()
{
    for (size_t i = 0; i < _eventInfos_forPopRepush.size(); i++)
    {
        auto& delayed = _eventInfos_forPopRepush[i];
        size_t newPos = _buff.size();
        size_t oldPos = delayed.pos;
        size_t delta = newPos - oldPos;
        for (size_t& fp : delayed.fieldPositions)
            fp += delta;
        delayed.pos = newPos;
        _eventInfos.push_back(delayed);
        _buff.insert(_buff.end(), _buff_forPopRepush[i].begin(), _buff_forPopRepush[i].end());
    }
    _buff_forPopRepush.clear();
    _eventInfos_forPopRepush.clear();
}

/*
void CCbor::popEvent(std::vector<unsigned char>& data, SEventInf& eventInfo)
{
    if (!_eventInfos.empty())
    {
        SEventInf* inf = &_eventInfos[_eventInfos.size() - 1];
        eventInfo = _eventInfos.back();
        data.assign(_buff.begin() + inf->pos, _buff.end());
        _buff.resize(inf->pos);
        _eventInfos.pop_back();
    }
}

void CCbor::pushEvent(const std::vector<unsigned char>& data, const SEventInf& eventInfo)
{
    _eventInfos.push_back(eventInfo);
    SEventInf* inf = &_eventInfos[_eventInfos.size() - 1];
    inf->pos = _buff.size();
    _buff.insert(_buff.end(), data.begin(), data.end());
}
*/
void CCbor::allowEventsReordering(bool allow)
{
    if ((allow && (_allowEventsReordering == 2)) || ((!allow) && (_allowEventsReordering != 2)))
    {
        std::string txt("allowEventsReordering called in a cascaded manner.");
        App::logMsg(sim_verbosity_errors, txt.c_str());
        App::logScriptMsg(nullptr, sim_verbosity_scripterrors, txt.c_str());
    }
    if (allow)
        _allowEventsReordering = 2;
    else
        _allowEventsReordering = 1;
}

int64_t CCbor::finalizeEvents(int64_t nextSeq, bool seqChanges, std::vector<SEventInf>* inf /*= nullptr*/)
{
    if (_eventOpen)
        App::logMsg(sim_verbosity_errors, "finalizing events where an event push is expected.");
    int discardableEventCnt = 0;
    std::map<std::string, int> mergeInfo;
    for (size_t i = 0; i < _eventInfos.size(); i++)
    {
        if (_eventInfos[i].eventId.size() > 0)
        {
            if (mergeInfo.find(_eventInfos[i].eventId) != mergeInfo.end())
                discardableEventCnt++;
            mergeInfo[_eventInfos[i].eventId] = i;
        }
    }
    if (!seqChanges)
        nextSeq = nextSeq - _eventInfos.size() + discardableEventCnt;
    std::vector<unsigned char> events;
    _buff.swap(events);
    openArray(); // holding all events
    for (size_t i = 0; i < _eventInfos.size(); i++)
    {
        if ((_eventInfos[i].eventId.size() == 0) || (mergeInfo.find(_eventInfos[i].eventId)->second == i))
        {
            SEventInf n;
            n.target = _eventInfos[i].target;
            n.pos = _buff.size();
            if (i < _eventInfos.size() - 1)
                _buff.insert(_buff.end(), events.begin() + _eventInfos[i].pos, events.begin() + _eventInfos[i + 1].pos);
            else
                _buff.insert(_buff.end(), events.begin() + _eventInfos[i].pos, events.end());
            appendKeyInt64("seq", nextSeq++);
            closeArrayOrMap(); // to close the event
            n.size = _buff.size() - n.pos;
            if (inf != nullptr)
            {
                for (size_t j = 0; j < _eventInfos[i].fieldNames.size(); j++)
                {
                    n.fieldNames.push_back(_eventInfos[i].fieldNames[j]);
                    n.fieldPositions.push_back(n.pos + _eventInfos[i].fieldPositions[j] - _eventInfos[i].pos);
                    n.fieldSizes.push_back(_eventInfos[i].fieldSizes[j]);
                }
                inf->push_back(n);
            }
        }
    }
    closeArrayOrMap(); // to close the array holding all events

    clear();
    return nextSeq;
}

size_t CCbor::getEventCnt() const
{
    return _eventInfos.size();
}

std::string CCbor::getEventName() const
{ // last created event (maybe not yet pushed)
    std::string eventNm;
    if (_eventInfos.size() > 0)
        eventNm = _eventInfos[_eventInfos.size() - 1].event;
    return eventNm;
}

void CCbor::appendKeyInt64(const char* key, int64_t v)
{
    appendText(key);
    appendInt64(v);
}

void CCbor::appendKeyHandle(const char* key, int64_t h)
{
    appendText(key);
    appendHandle(h);
}

void CCbor::appendKeyUint8Array(const char* key, const unsigned char* v, size_t cnt)
{
    appendText(key);
    appendUint8Array(v, cnt);
}

void CCbor::appendKeyInt32Array(const char* key, const int* v, size_t cnt)
{
    appendText(key);
    appendInt32Array(v, cnt);
}

void CCbor::appendKeyUint32Array(const char* key, const unsigned int* v, size_t cnt)
{
    appendText(key);
    appendUint32Array(v, cnt);
}

void CCbor::appendKeyInt64Array(const char* key, const int64_t* v, size_t cnt)
{
    appendText(key);
    appendInt64Array(v, cnt);
}

void CCbor::appendKeyHandleArray(const char* key, const int64_t* h, size_t cnt)
{
    appendText(key);
    appendHandleArray(h, cnt);
}

void CCbor::appendKeyHandleArray(const char* key, const int* h, size_t cnt)
{
    appendText(key);
    appendHandleArray(h, cnt);
}

void CCbor::appendKeyHandleArray(const char* key, const std::vector<CSceneObject*>& h)
{
    appendText(key);
    appendHandleArray(h);
}

void CCbor::appendKeyFloat(const char* key, float v)
{
    appendText(key);
    appendFloat(v);
}

void CCbor::appendKeyFloatArray(const char* key, const float* v, size_t cnt)
{
    appendText(key);
    appendFloatArray(v, cnt);
}

void CCbor::appendKeyDouble(const char* key, double v)
{
    appendText(key);
    appendDouble(v);
}

void CCbor::appendKeyDoubleArray(const char* key, const double* v, size_t cnt)
{
    appendText(key);
    appendDoubleArray(v, cnt);
}

void CCbor::appendKeyTextArray(const char* key, const std::vector<std::string>& txtArr)
{
    appendText(key);
    appendTextArray(txtArr);
}

void CCbor::appendKeyNull(const char* key)
{
    appendText(key);
    appendNull();
}

void CCbor::appendKeyBool(const char* key, bool v)
{
    appendText(key);
    appendBool(v);
}

void CCbor::appendKeyMatrix(const char* key, const float* v, size_t rows, size_t cols, bool dataIsRowMajor /*= true*/)
{
    appendText(key);
    appendMatrix(v, rows, cols, dataIsRowMajor);
}

void CCbor::appendKeyMatrix(const char* key, const double* v, size_t rows, size_t cols, bool dataIsRowMajor /*= true*/)
{
    appendText(key);
    appendMatrix(v, rows, cols, dataIsRowMajor);
}

void CCbor::appendKeyMatrix(const char* key, const CMatrix& m)
{
    appendText(key);
    appendMatrix(m);
}

void CCbor::appendKeyVector2(const char* key, const double* v)
{
    appendText(key);
    appendVector2(v);
}

void CCbor::appendKeyVector3(const char* key, const double* v)
{
    appendText(key);
    appendVector3(v);
}

void CCbor::appendKeyVector3(const char* key, const C3Vector& v)
{
    appendText(key);
    appendVector3(v);
}

void CCbor::appendKeyQuaternion(const char* key, const double* v, bool xyzwLayout /*= false*/)
{
    appendText(key);
    appendQuaternion(v, xyzwLayout);
}

void CCbor::appendKeyQuaternion(const char* key, const CQuaternion& q)
{
    appendText(key);
    appendQuaternion(q);
}

void CCbor::appendKeyPose(const char* key, const double* v, bool xyzqxqyqzqwLayout /*= false*/)
{
    appendText(key);
    appendPose(v, xyzqxqyqzqwLayout);
}

void CCbor::appendKeyPose(const char* key, const CPose& p)
{
    appendText(key);
    appendPose(p);
}

void CCbor::appendKeyColor3(const char* key, const float* c)
{
    appendText(key);
    appendColor3(c);
}

void CCbor::appendKeyColor(const char* key, const float* c)
{
    appendText(key);
    appendColor(c);
}

void CCbor::appendKeyBuff(const char* key, const unsigned char* v, size_t l)
{
    appendText(key);
    appendBuff(v, l);
}

void CCbor::appendKeyText(const char* key, const char* v, int l /*=-1*/)
{
    appendText(key);
    appendText(v, l);
}

void CCbor::openKeyArray(const char* key, int sizedArray /*= -1*/)
{
    appendText(key);
    openArray(sizedArray);
}

void CCbor::openKeyMap(const char* key, int sizedMap /*= -1*/)
{
    appendText(key);
    openMap(sizedMap);
}

void CCbor::_handleDataField(const char* key /*= nullptr*/)
{
    if ((_eventDepth == 2) && _inDataField)
    {
        if (_handleDataFieldDisableLevel == 0)
        {
            if (_nextIsKeyInData)
            {
                if (_eventInfos.size() > 0)
                {
                    SEventInf* inf = &_eventInfos[_eventInfos.size() - 1];
                    // for previous key-value pair:
                    if (inf->fieldPositions.size() > 0)
                        inf->fieldSizes.push_back(_buff.size() - inf->fieldPositions[inf->fieldPositions.size() - 1]);
                    // For current key-value pair:
                    inf->fieldPositions.push_back(_buff.size());
                    if (key != nullptr)
                    {
                        inf->fieldNames.push_back(key);
                        allEVentFieldNames.insert(key);
                    }
                    else
                        inf->fieldNames.push_back("");
                }
            }
            _nextIsKeyInData = !_nextIsKeyInData;
        }
    }
}
