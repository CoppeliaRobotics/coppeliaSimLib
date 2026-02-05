#include <cbor.h>
#include <app.h>
#include <string.h>

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

void CCbor::appendInt(long long int v)
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

void CCbor::appendUCharArray(const unsigned char* v, size_t cnt)
{
    /*
    openArray(); // _handleDataField() called in there

    for (size_t i = 0; i < cnt; i++)
    {
        if (v[i] <= 23)
            _buff.push_back(v[i]);
        else
        {
            _buff.push_back(24);
            _buff.push_back(v[i]);
        }
    }

    closeArrayOrMap();
    */
    _buff.push_back(0xD8); // Tag header
    _buff.push_back(0x40); // 64
    _appendItemTypeAndLength(0x40, cnt);
    _buff.insert(_buff.end(), v, v + cnt);
}

void CCbor::appendIntArray(const int* v, size_t cnt)
{
    /*
    openArray(); // _handleDataField() called in there

    unsigned char* w = (unsigned char*)v;
    for (size_t i = 0; i < cnt; i++)
    {
        if (v[i] < 0)
        {
            int x = -v[i] - 1;
            _buff.push_back(26 + 32);
            unsigned char* y = (unsigned char*)&x;
            _buff.push_back(y[3]);
            _buff.push_back(y[2]);
            _buff.push_back(y[1]);
            _buff.push_back(y[0]);
        }
        else
        {
            _buff.push_back(26);
            _buff.push_back(w[3]);
            _buff.push_back(w[2]);
            _buff.push_back(w[1]);
            _buff.push_back(w[0]);
        }
        w += 4;
    }

    closeArrayOrMap();
*/
    _buff.push_back(0xD8); // Tag header
    _buff.push_back(0x4e); // 78
    _appendItemTypeAndLength(0x40, cnt * sizeof(int));
    _buff.insert(_buff.end(), (unsigned char*)v, ((unsigned char*)v) + cnt * sizeof(int));
}

void CCbor::appendIntArray(const long long int* v, size_t cnt)
{
/*
    openArray(); // _handleDataField() called in there

    unsigned char* w = (unsigned char*)v;
    for (size_t i = 0; i < cnt; i++)
    {
        if (v[i] < 0)
        {
            long long int x = -v[i] - 1;
            _buff.push_back(27 + 32);
            unsigned char* y = (unsigned char*)&x;
            _buff.push_back(y[7]);
            _buff.push_back(y[6]);
            _buff.push_back(y[5]);
            _buff.push_back(y[4]);
            _buff.push_back(y[3]);
            _buff.push_back(y[2]);
            _buff.push_back(y[1]);
            _buff.push_back(y[0]);
        }
        else
        {
            _buff.push_back(27);
            _buff.push_back(w[7]);
            _buff.push_back(w[6]);
            _buff.push_back(w[5]);
            _buff.push_back(w[4]);
            _buff.push_back(w[3]);
            _buff.push_back(w[2]);
            _buff.push_back(w[1]);
            _buff.push_back(w[0]);
        }
        w += 8;
    }

    closeArrayOrMap();
    */
    _buff.push_back(0xD8); // Tag header
    _buff.push_back(0x4f); // 79
    _appendItemTypeAndLength(0x40, cnt * sizeof(long long int));
    _buff.insert(_buff.end(), (unsigned char*)v, ((unsigned char*)v) + cnt * sizeof(long long int));
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
    /*
    openArray(); // _handleDataField() called in there

    const unsigned char* w = (const unsigned char*)v;
    for (size_t i = 0; i < cnt; i++)
    {
        _buff.push_back(128 + 64 + 32 + 26);
        _buff.push_back(w[3]);
        _buff.push_back(w[2]);
        _buff.push_back(w[1]);
        _buff.push_back(w[0]);
        w += 4;
    }

    closeArrayOrMap();
    */
    size_t d = _buff.size();
    _buff.push_back(0xD8); // Tag header
    _buff.push_back(0x55); // 85
    _appendItemTypeAndLength(0x40, cnt * sizeof(float));
    _buff.insert(_buff.end(), (unsigned char*)v, ((unsigned char*)v) + cnt * sizeof(float));
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
    /*
    openArray(); // _handleDataField() called in there

    if ((_options & 1) == 0)
    { // treat doubles as floats
        for (size_t i = 0; i < cnt; i++)
        {
            float ww = float(v[i]);
            const unsigned char* w = (const unsigned char*)&ww;
            _buff.push_back(128 + 64 + 32 + 26);
            _buff.push_back(w[3]);
            _buff.push_back(w[2]);
            _buff.push_back(w[1]);
            _buff.push_back(w[0]);
        }
    }
    else
    {
        const unsigned char* w = (const unsigned char*)v;
        for (size_t i = 0; i < cnt; i++)
        {
            _buff.push_back(128 + 64 + 32 + 27);
            _buff.push_back(w[7]);
            _buff.push_back(w[6]);
            _buff.push_back(w[5]);
            _buff.push_back(w[4]);
            _buff.push_back(w[3]);
            _buff.push_back(w[2]);
            _buff.push_back(w[1]);
            _buff.push_back(w[0]);
            w += 8;
        }
    }

    closeArrayOrMap();
*/
    _buff.push_back(0xD8); // Tag header
    _buff.push_back(0x56); // 86
    _appendItemTypeAndLength(0x40, cnt * sizeof(double));
    _buff.insert(_buff.end(), (unsigned char*)v, ((unsigned char*)v) + cnt * sizeof(double));
}

void CCbor::appendMatrix(const double* v, size_t rows, size_t cols)
{
    _handleDataField();
    _buff.push_back(216); // major type 6, tag
    _buff.push_back(40); // tag 40 for matrices
    _buff.push_back(82); // array of 2 values (dims + data)
    _buff.push_back(82); // array of 2 values (rows and cols)
    _appendItemTypeAndLength(0, rows);
    _appendItemTypeAndLength(0, cols);
    appendDoubleArray(v, rows * cols);
}

void CCbor::appendQuaternion(const double* v)
{
    appendDoubleArray(v, 4);
}

void CCbor::appendPose(const double* v)
{
    appendDoubleArray(v, 7);
}

void CCbor::appendColor(const float c[3])
{
    appendFloatArray(c, 3);
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

void CCbor::_appendItemTypeAndLength(unsigned char t, long long int l)
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
    _handleDataField(v);
    if (l < 0)
        l = int(strlen(v));
    _appendItemTypeAndLength(64 + 32, size_t(l));
    for (size_t i = 0; i < size_t(l); i++)
        _buff.push_back(v[i]);
}

void CCbor::appendTextArray(const std::vector<std::string>& txtArr)
{
    openArray(); // _handleDataField() called in there
    for (size_t i = 0; i < txtArr.size(); i++)
        appendText(txtArr[i].c_str());
    closeArrayOrMap();
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

void CCbor::openArray()
{
    _handleDataField();
    _eventDepth++;
    _buff.push_back(128 + 31); // array + use a break char
}

void CCbor::openMap()
{
    _handleDataField();
    _eventDepth++;
    _buff.push_back(128 + 32 + 31); // map + use a break char
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
    _buff.push_back(255); // break char
}

void CCbor::clear()
{
    // do not clear _buff in here! _buff.clear();
    _eventInfos.clear();
    _mergeableEventIds.clear();
    _discardableEventCnt = 0;
    _eventDepth = 0;
    _eventOpen = false;
    _nextIsKeyInData = true;
    _inDataField = false;
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

void CCbor::createEvent(const char* event, const char* fieldName, const char* objType, long long int handle, long long int uid, bool mergeable, bool openDataField /*=true*/)
{
    if (_eventOpen)
    {
        printf("[CoppeliaSim:error] creating an event where an event push is expected.\n");
        App::logMsg(sim_verbosity_errors, "creating an event where an event push is expected.");
    }
    _eventOpen = true;

    SEventInf inf;
    inf.pos = _buff.size();
    inf.target = handle;
    if (mergeable)
    {
        std::string eventId(event);
        if (fieldName != nullptr)
            eventId += fieldName;
        if (objType != nullptr)
            eventId += objType;
        if (uid != -1)
            eventId += std::to_string(uid);
        inf.eventId = eventId;
        if (_mergeableEventIds.find(eventId) != _mergeableEventIds.end())
            _discardableEventCnt++;
        _mergeableEventIds[eventId] = _eventInfos.size();
    }
    _eventInfos.push_back(inf);

    openMap(); // holding the event
    appendKeyText("event", event);
    if (uid != -1)
        appendKeyInt("uid", uid);
    if (handle != -1)
        appendKeyInt("handle", handle);
    if (openDataField)
    {
        appendText("data");
        openMap(); // holding the data
        _inDataField = true;
#if SIM_EVENT_PROTOCOL_VERSION == 2
        if (objType != nullptr)
        {
            appendString(objType);
            openMap(); // holding the scene object's data specific to the object type
        }
#endif
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
        App::logMsg(sim_verbosity_errors, "pushing an event unexisting event.");

    SEventInf* inf = &_eventInfos[_eventInfos.size() - 1];
    inf->size = _buff.size() - inf->pos;
}

long long int CCbor::finalizeEvents(long long int nextSeq, bool seqChanges, std::vector<SEventInf>* inf /*= nullptr*/)
{
    if (_eventOpen)
        App::logMsg(sim_verbosity_errors, "finalizing events where an event push is expected.");

    if (!seqChanges)
        nextSeq = nextSeq - _eventInfos.size() + _discardableEventCnt;
    std::vector<unsigned char> events;
    _buff.swap(events);
    openArray(); // holding all events
    for (size_t i = 0; i < _eventInfos.size(); i++)
    {
        if ((_eventInfos[i].eventId.size() == 0) || (_mergeableEventIds.find(_eventInfos[i].eventId)->second == i))
        {
            SEventInf n;
            n.target = _eventInfos[i].target;
            n.pos = _buff.size();
            if (i < _eventInfos.size() - 1)
                _buff.insert(_buff.end(), events.begin() + _eventInfos[i].pos, events.begin() + _eventInfos[i + 1].pos);
            else
                _buff.insert(_buff.end(), events.begin() + _eventInfos[i].pos, events.end());
            appendKeyInt("seq", nextSeq++);
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
    return (_eventInfos.size());
}

void CCbor::appendKeyInt(const char* key, long long int v)
{
    appendText(key);
    appendInt(v);
}

void CCbor::appendKeyUCharArray(const char* key, const unsigned char* v, size_t cnt)
{
    appendText(key);
    appendUCharArray(v, cnt);
}

void CCbor::appendKeyIntArray(const char* key, const int* v, size_t cnt)
{
    appendText(key);
    appendIntArray(v, cnt);
}

void CCbor::appendKeyIntArray(const char* key, const long long int* v, size_t cnt)
{
    appendText(key);
    appendIntArray(v, cnt);
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

void CCbor::openKeyArray(const char* key)
{
    appendText(key);
    openArray();
}

void CCbor::openKeyMap(const char* key)
{
    appendText(key);
    openMap();
}

void CCbor::_handleDataField(const char* key /*= nullptr*/)
{
    if ((_eventDepth == 2) && _inDataField)
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
