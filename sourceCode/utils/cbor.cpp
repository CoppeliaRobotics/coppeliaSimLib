#include <cbor.h>
#include <app.h>
#include <string.h>

bool CCbor::isText(const char *v, size_t l)
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

CCbor::CCbor(const std::string *initBuff /*=nullptr*/, int options /*=0*/)
{
    _options = options;
    _eventDepth = 0;
    _eventOpen = false;
    _discardableEventCnt = 0;
    if (initBuff != nullptr)
        _buff.assign(initBuff->begin(), initBuff->end());
}

CCbor::~CCbor()
{
}

void CCbor::swapWithEmptyBuffer(std::vector<unsigned char> *emptyBuff)
{
    _buff.swap(emptyBuff[0]);
    clear();
}

void CCbor::appendInt(long long int v)
{
    unsigned char add = 0;
    if (v < 0)
    {
        v = -v;
        v = v - 1;
        add = 32;
    }
    _appendItemTypeAndLength(add, v);
}

void CCbor::appendUCharArray(const unsigned char *v, size_t cnt)
{
    openArray();

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
}

void CCbor::appendIntArray(const int *v, size_t cnt)
{
    openArray();

    unsigned char *w = (unsigned char *)v;
    for (size_t i = 0; i < cnt; i++)
    {
        if (v[i] < 0)
        {
            int x = -v[i] - 1;
            _buff.push_back(26 + 32);
            unsigned char *y = (unsigned char *)&x;
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
}

void CCbor::appendIntArray(const long long int *v, size_t cnt)
{
    openArray();

    unsigned char *w = (unsigned char *)v;
    for (size_t i = 0; i < cnt; i++)
    {
        if (v[i] < 0)
        {
            long long int x = -v[i] - 1;
            _buff.push_back(27 + 32);
            unsigned char *y = (unsigned char *)&x;
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
}

void CCbor::appendFloat(float v)
{
    _buff.push_back(128 + 64 + 32 + 26);
    _buff.push_back(((unsigned char *)&v)[3]);
    _buff.push_back(((unsigned char *)&v)[2]);
    _buff.push_back(((unsigned char *)&v)[1]);
    _buff.push_back(((unsigned char *)&v)[0]);
}

void CCbor::appendFloatArray(const float *v, size_t cnt)
{
    openArray();

    const unsigned char *w = (const unsigned char *)v;
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
}

void CCbor::appendDouble(double v)
{
    if ((_options & 1) == 0)
        appendFloat(float(v)); // treat doubles as floats
    else
    {
        _buff.push_back(128 + 64 + 32 + 27);
        _buff.push_back(((unsigned char *)&v)[7]);
        _buff.push_back(((unsigned char *)&v)[6]);
        _buff.push_back(((unsigned char *)&v)[5]);
        _buff.push_back(((unsigned char *)&v)[4]);
        _buff.push_back(((unsigned char *)&v)[3]);
        _buff.push_back(((unsigned char *)&v)[2]);
        _buff.push_back(((unsigned char *)&v)[1]);
        _buff.push_back(((unsigned char *)&v)[0]);
    }
}

void CCbor::appendDoubleArray(const double *v, size_t cnt)
{
    openArray();

    if ((_options & 1) == 0)
    { // treat doubles as floats
        for (size_t i = 0; i < cnt; i++)
        {
            float ww = float(v[i]);
            const unsigned char *w = (const unsigned char *)&ww;
            _buff.push_back(128 + 64 + 32 + 26);
            _buff.push_back(w[3]);
            _buff.push_back(w[2]);
            _buff.push_back(w[1]);
            _buff.push_back(w[0]);
        }
    }
    else
    {
        const unsigned char *w = (const unsigned char *)v;
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
}

void CCbor::appendNull()
{
    _buff.push_back(128 + 64 + 32 + 22);
}

void CCbor::appendBool(bool v)
{
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
        _buff.push_back(((unsigned char *)&l)[1]);
        _buff.push_back(((unsigned char *)&l)[0]);
    }
    else if (l <= 0xffffffff)
    {
        _buff.push_back(t + 26);
        _buff.push_back(((unsigned char *)&l)[3]);
        _buff.push_back(((unsigned char *)&l)[2]);
        _buff.push_back(((unsigned char *)&l)[1]);
        _buff.push_back(((unsigned char *)&l)[0]);
    }
    else
    {
        _buff.push_back(t + 27);
        _buff.push_back(((unsigned char *)&l)[7]);
        _buff.push_back(((unsigned char *)&l)[6]);
        _buff.push_back(((unsigned char *)&l)[5]);
        _buff.push_back(((unsigned char *)&l)[4]);
        _buff.push_back(((unsigned char *)&l)[3]);
        _buff.push_back(((unsigned char *)&l)[2]);
        _buff.push_back(((unsigned char *)&l)[1]);
        _buff.push_back(((unsigned char *)&l)[0]);
    }
}

void CCbor::appendBuff(const unsigned char *v, size_t l)
{
    _appendItemTypeAndLength(64, l);
    for (size_t i = 0; i < l; i++)
        _buff.push_back(v[i]);
}

void CCbor::appendString(const char *v, int l /*=-1*/)
{
    if (l < 0)
        l = int(strlen(v));
    _appendItemTypeAndLength(64 + 32, size_t(l));
    for (size_t i = 0; i < size_t(l); i++)
        _buff.push_back(v[i]);
}

void CCbor::appendRaw(const unsigned char *v, size_t l)
{
    _buff.insert(_buff.end(), v, v + l);
}

void CCbor::appendLuaString(const std::string &v, bool isBuffer, bool isText)
{
    std::string suff;
    if (v.size() >= 6)
        suff.assign(v.begin() + v.size() - 6, v.end());
    if (suff == "@:txt:")
        appendString(v.c_str(), int(v.size()) - 6);
    else if (suff == "@:dat:")
        appendBuff((unsigned char *)v.c_str(), v.size() - 6);
    else
    { // following modified on 12.03.2024 (buffer/string/text differentiation)
        if (isBuffer)
            appendBuff((unsigned char *)v.c_str(), v.size());
        else
        {
            if (isText)
                appendString(v.c_str(), int(v.size()));
            else
            { // we have a binary string (could contain text chars only):
                if (CCbor::isText(v.c_str(), int(v.size())))
                    appendString(v.c_str(), int(v.size()));
                else
                    appendBuff((unsigned char *)v.c_str(), v.size());
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
    _eventDepth++;
    _buff.push_back(128 + 31); // array + use a break char
}

void CCbor::openMap()
{
    _eventDepth++;
    _buff.push_back(128 + 32 + 31); // map + use a break char
}

void CCbor::closeArrayOrMap()
{
    _eventDepth--;
    _buff.push_back(255); // break char
}

void CCbor::clear()
{
    _buff.clear();
    _eventInfos.clear();
    _mergeableEventIds.clear();
    _discardableEventCnt = 0;
    _eventDepth = 0;
    _eventOpen = false;
}

std::string CCbor::getBuff() const
{
    std::string retVal;
    retVal.assign(_buff.begin(), _buff.end());
    return (retVal);
}

const unsigned char *CCbor::getBuff(size_t &l) const
{
    l = _buff.size();
    return (_buff.data());
}

size_t CCbor::getEventDepth() const
{
    return (_eventDepth);
}

void CCbor::createEvent(const char *event, const char *fieldName, const char *objType, long long int uid, int handle,
                        bool mergeable, bool openDataField /*=true*/)
{
    if (_eventOpen)
        App::logMsg(sim_verbosity_errors, "creating an event where an event push is expected.");
    _eventOpen = true;

    SEventInf inf;
    inf.pos = _buff.size();
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
    appendKeyString("event", event);
    if (uid != -1)
        appendKeyInt("uid", uid);
    if (handle != -1)
        appendKeyInt("handle", handle);
    if (openDataField)
    {
        appendString("data");
        openMap(); // holding the data
#if SIM_EVENT_PROTOCOL_VERSION == 2
        if (objType != nullptr)
        {
            appendString(objType);
            openMap(); // holding the scene object's data specific to the object type
        }
#endif
    }
}

void CCbor::pushEvent()
{
    if (_eventOpen)
    {
        while (_eventDepth > 1)
            closeArrayOrMap(); // make sure to close the current event's arrays/maps, except for the one holding the event
        _eventDepth = 0; // yes, we intentionally forget to close the last array/map, but we anyways reset the depth to zero
        _eventOpen = false;
    }
    else
        App::logMsg(sim_verbosity_errors, "pushing an event unexisting event.");
}

long long int CCbor::finalizeEvents(long long int nextSeq, bool seqChanges)
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
            if (i < _eventInfos.size() - 1)
                _buff.insert(_buff.end(), events.begin() + _eventInfos[i].pos, events.begin() + _eventInfos[i + 1].pos);
            else
                _buff.insert(_buff.end(), events.begin() + _eventInfos[i].pos, events.end());
            appendKeyInt("seq", nextSeq++);
            closeArrayOrMap(); // to close the event
        }
    }
    closeArrayOrMap(); // to close the array holding all events
    _eventInfos.clear();
    _mergeableEventIds.clear();
    _discardableEventCnt = 0;
    _eventDepth = 0;
    _eventOpen = false;
    return (nextSeq);
}

size_t CCbor::getEventCnt() const
{
    return (_eventInfos.size());
}

void CCbor::appendKeyInt(const char *key, long long int v)
{
    appendString(key);
    appendInt(v);
}

void CCbor::appendKeyUCharArray(const char *key, const unsigned char *v, size_t cnt)
{
    appendString(key);
    appendUCharArray(v, cnt);
}

void CCbor::appendKeyIntArray(const char *key, const int *v, size_t cnt)
{
    appendString(key);
    appendIntArray(v, cnt);
}

void CCbor::appendKeyIntArray(const char *key, const long long int *v, size_t cnt)
{
    appendString(key);
    appendIntArray(v, cnt);
}

void CCbor::appendKeyFloat(const char *key, float v)
{
    appendString(key);
    appendFloat(v);
}

void CCbor::appendKeyFloatArray(const char *key, const float *v, size_t cnt)
{
    appendString(key);
    appendFloatArray(v, cnt);
}

void CCbor::appendKeyDouble(const char *key, double v)
{
    appendString(key);
    appendDouble(v);
}

void CCbor::appendKeyDoubleArray(const char *key, const double *v, size_t cnt)
{
    appendString(key);
    appendDoubleArray(v, cnt);
}

void CCbor::appendKeyNull(const char *key)
{
    appendString(key);
    appendNull();
}

void CCbor::appendKeyBool(const char *key, bool v)
{
    appendString(key);
    appendBool(v);
}

void CCbor::appendKeyBuff(const char *key, const unsigned char *v, size_t l)
{
    appendString(key);
    appendBuff(v, l);
}

void CCbor::appendKeyString(const char *key, const char *v, int l /*=-1*/)
{
    appendString(key);
    appendString(v, l);
}

void CCbor::openKeyArray(const char *key)
{
    appendString(key);
    openArray();
}

void CCbor::openKeyMap(const char *key)
{
    appendString(key);
    openMap();
}
