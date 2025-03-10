#include <app.h>
#include <graphDataStream.h>
#include <simLib/simConst.h>
#include <tt.h>

CGraphDataStream::CGraphDataStream()
{
    _nextValueToInsertIsValid = false;
    _scriptHandle = -1;
    _transformationType = sim_stream_transf_raw;
    _transformationMult = 1.0;
    _transformationOff = 0.0;
    _movingAveragePeriod = 1;
    _uid = App::getFreshUniqueId(-1);
}

CGraphDataStream::CGraphDataStream(const char* streamName, const char* unitStr, int options, const float* color,
                                   double cyclicRange, int scriptHandle)
{
    _streamName = streamName;
    _transformationType = sim_stream_transf_raw;
    _transformationMult = 1.0;
    _transformationOff = 0.0;
    _movingAveragePeriod = 1;
    setBasics(unitStr, options, color, cyclicRange, scriptHandle);
    setTransformation(sim_stream_transf_raw, 1.0, 0.0, 1);
    _nextValueToInsertIsValid = false;
    _uid = App::getFreshUniqueId(-1);
}

CGraphDataStream::~CGraphDataStream()
{
}

void CGraphDataStream::setBasics(const char* unitStr, int options, const float* color, double cyclicRange,
                                 int scriptHandle)
{
    _unitStr.clear();
    if (unitStr != nullptr)
        _unitStr = unitStr;
    _scriptHandle = scriptHandle;
    _visible = (options & 1) == 0;
    _showLabel = (options & 2) == 0;
    _linkPoints = (options & 4) == 0;
    _cyclicRange = cyclicRange;
    _static = false;
    if (color != nullptr)
    {
        _color[0] = color[0];
        _color[1] = color[1];
        _color[2] = color[2];
    }
    else
    {
        _color[0] = 1.0;
        _color[1] = 0.0;
        _color[2] = 0.0;
    }
}

bool CGraphDataStream::setTransformation(int trType, double mult, double off, int movAvgPeriod)
{
    bool retVal = (_transformationType != trType);
    retVal = retVal || (_transformationMult != mult);
    retVal = retVal || (_transformationOff != off);
    retVal = retVal || (_movingAveragePeriod != movAvgPeriod);
    _transformationType = trType;
    _transformationMult = mult;
    _transformationOff = off;
    _movingAveragePeriod = movAvgPeriod;
    return (retVal);
}

void CGraphDataStream::reset(size_t bufferSize)
{
    if (!_static)
    {
        _values.clear();
        _values.resize(bufferSize, 0.0);
        _transformedValues.clear();
        _transformedValues.resize(bufferSize, 0.0);
        _valuesValidityFlags.clear();
        _valuesValidityFlags.resize(1 + bufferSize / 8, 0);
        _transformedValuesValidityFlags.clear();
        _transformedValuesValidityFlags.resize(1 + bufferSize / 8, 0);
    }
    _nextValueToInsertIsValid = false;
}

void CGraphDataStream::setId(int id)
{
    _id = id;
}

int CGraphDataStream::getId() const
{
    return _id;
}

long long int CGraphDataStream::getUid() const
{
    return _uid;
}

void CGraphDataStream::setStreamName(const char* nm)
{
    _streamName = nm;
}

std::string CGraphDataStream::getStreamName() const
{
    return (_streamName);
}

std::string CGraphDataStream::getUnitStr() const
{
    return (_unitStr);
}

int CGraphDataStream::getOptions() const
{
    int retVal = 0;
    if (!_visible)
        retVal |= 1;
    if (!_showLabel)
        retVal |= 2;
    if (!_linkPoints)
        retVal |= 4;
    return (retVal);
}

bool CGraphDataStream::getIsStatic() const
{
    return (_static);
}

double CGraphDataStream::getCyclicRange() const
{
    return (_cyclicRange);
}

const float* CGraphDataStream::getColorPtr() const
{
    return (_color);
}

int CGraphDataStream::getScriptHandle() const
{
    return (_scriptHandle);
}

void CGraphDataStream::setNextValueToInsert(double v)
{
    if (!_static)
    {
        _nextValueToInsert = v;
        _nextValueToInsertIsValid = true;
    }
}

void CGraphDataStream::insertNextValue(int absIndex, bool firstValue, const std::vector<double>& times)
{
    if (!_static)
    {
        if (_nextValueToInsertIsValid)
        {
            _values[absIndex] = _nextValueToInsert;
            _valuesValidityFlags[absIndex / 8] |= (1 << (absIndex & 7)); // valid data
            if (firstValue)
            { // this is the very first point
                if (_transformationType == sim_stream_transf_raw)
                {
                    _transformedValues[absIndex] = _values[absIndex];
                    _transformedValuesValidityFlags[absIndex / 8] |= (1 << (absIndex & 7)); // valid data
                }
                if (_transformationType == sim_stream_transf_derivative)
                { // invalid data
                    _transformedValues[absIndex] = 0.0;
                    _transformedValuesValidityFlags[absIndex / 8] &= 255 - (1 << (absIndex & 7)); // invalid data
                }
                if (_transformationType == sim_stream_transf_integral)
                {
                    _transformedValues[absIndex] = 0.0;
                    _transformedValuesValidityFlags[absIndex / 8] |= (1 << (absIndex & 7)); // valid data
                }
                if (_transformationType == sim_stream_transf_cumulative)
                {
                    _transformedValues[absIndex] = _values[absIndex];
                    _transformedValuesValidityFlags[absIndex / 8] |= (1 << (absIndex & 7)); // valid data
                }
            }
            else
            { // this is not the first point
                // We get the index of previous data:
                size_t prevIndex;
                if (absIndex - 1 < 0)
                    prevIndex = _values.size() - 1;
                else
                    prevIndex = absIndex - 1;
                double dt = (times[absIndex] - times[prevIndex]);
                if (_transformationType == sim_stream_transf_raw)
                {
                    _transformedValues[absIndex] = _values[absIndex];
                    _transformedValuesValidityFlags[absIndex / 8] |= (1 << (absIndex & 7)); // valid data
                }
                if (_transformationType == sim_stream_transf_derivative)
                {
                    if (dt == 0.0)
                    { // invalid data
                        _transformedValues[absIndex] = 0.0;
                        _transformedValuesValidityFlags[absIndex / 8] &= 255 - (1 << (absIndex & 7)); // invalid data
                    }
                    else
                    {
                        if ((_valuesValidityFlags[prevIndex / 8] & (1 << (prevIndex & 7))) != 0)
                        { // previous data was valid
                            if (_cyclicRange == 0.0)
                                _transformedValues[absIndex] = (_values[absIndex] - _values[prevIndex]) / dt;
                            else
                                _transformedValues[absIndex] =
                                    (tt::getAngleMinusAlpha_range(_values[absIndex], _values[prevIndex],
                                                                  _cyclicRange)) /
                                    dt;
                            _transformedValuesValidityFlags[absIndex / 8] |= (1 << (absIndex & 7)); // valid data
                        }
                        else
                        { // previous data was invalid
                            _transformedValues[absIndex] = 0.0;
                            _transformedValuesValidityFlags[absIndex / 8] &=
                                255 - (1 << (absIndex & 7)); // invalid data
                        }
                    }
                }
                if (_transformationType == sim_stream_transf_integral)
                {
                    if ((_valuesValidityFlags[prevIndex / 8] & (1 << (prevIndex & 7))) != 0)
                    { // previous data was valid
                        if ((_transformedValuesValidityFlags[prevIndex / 8] & (1 << (prevIndex & 7))) != 0)
                        { // previous transformed data was valid
                            if (_cyclicRange == 0.0)
                                _transformedValues[absIndex] =
                                    _transformedValues[prevIndex] + (_values[prevIndex] + _values[absIndex]) * 0.5 * dt;
                            else
                                _transformedValues[absIndex] =
                                    _transformedValues[prevIndex] +
                                    (_values[prevIndex] +
                                     tt::getAngleMinusAlpha_range(_values[absIndex], _values[prevIndex], _cyclicRange) *
                                         0.5) *
                                        dt;
                        }
                        else
                            _transformedValues[absIndex] = (_values[prevIndex] + _values[absIndex]) * 0.5 *
                                                           dt; // previous transformed data was invalid
                    }
                    else
                        _transformedValues[absIndex] = 0.0;                                 // previous data was invalid
                    _transformedValuesValidityFlags[absIndex / 8] |= (1 << (absIndex & 7)); // valid transformed data
                }
                if (_transformationType == sim_stream_transf_cumulative)
                {
                    if ((_valuesValidityFlags[prevIndex / 8] & (1 << (prevIndex & 7))) != 0)
                    { // previous data was valid
                        if ((_transformedValuesValidityFlags[prevIndex / 8] & (1 << (prevIndex & 7))) != 0)
                        { // previous transformed data was valid
                            if (_cyclicRange == 0.0)
                                _transformedValues[absIndex] = _transformedValues[prevIndex] + _values[absIndex];
                            else
                                _transformedValues[absIndex] =
                                    _transformedValues[prevIndex] +
                                    (_values[prevIndex] +
                                     tt::getAngleMinusAlpha_range(_values[absIndex], _values[prevIndex], _cyclicRange));
                        }
                        else
                            _transformedValues[absIndex] =
                                _values[prevIndex] + _values[absIndex]; // previous transformed data was invalid
                    }
                    else
                        _transformedValues[absIndex] = _values[absIndex];                   // previous data was invalid
                    _transformedValuesValidityFlags[absIndex / 8] |= (1 << (absIndex & 7)); // valid transformed data
                }
            }
        }
        else
        {
            _values[absIndex] = 0.0;
            _transformedValues[absIndex] = 0.0;
            _valuesValidityFlags[absIndex / 8] &= 255 - (1 << (absIndex & 7));            // invalid data
            _transformedValuesValidityFlags[absIndex / 8] &= 255 - (1 << (absIndex & 7)); // invalid data
        }
    }
    _nextValueToInsertIsValid = false;
}

bool CGraphDataStream::getTransformedValue(int startPt, int pos, double& retVal) const
{
    if (_static)
        return (false);
    double cumulativeValue = 0.0;
    int cumulativeValueCount = 0;
    for (int i = 0; i < _movingAveragePeriod; i++)
    {
        if ((_transformedValuesValidityFlags[pos / 8] & (1 << (pos & 7))) != 0)
        {
            cumulativeValue += (_transformedValues[pos] * _transformationMult + _transformationOff);
            cumulativeValueCount++;
        }
        if ((i == 0) && (cumulativeValueCount == 0))
            return (false); // not valid
        if (i != _movingAveragePeriod - 1)
        {
            if (pos == startPt)
                return (false); // not enough values from current point
            pos--;
            if (pos < 0)
                pos += int(_transformedValues.size()); // i.e. bufferSize
        }
    }
    if (cumulativeValueCount > 0)
        retVal = cumulativeValue / double(cumulativeValueCount);
    return (cumulativeValueCount > 0);
}

bool CGraphDataStream::getCurveData(bool staticCurve, int* index, int startPt, int ptCnt,
                                    const std::vector<double>& times, std::string* label, std::vector<double>& xVals,
                                    std::vector<double>& yVals, int* curveType, float col[3], double minMax[6]) const
{
    if (_visible && (staticCurve == _static))
    {
        if ((index == nullptr) || (index[0] == 0))
        {
            if (label != nullptr)
            {
                label[0] = _streamName;
                if (_unitStr.size() > 0)
                    label[0] += " (" + _unitStr + ")";
            }
            if (curveType != nullptr)
            {
                if (_linkPoints)
                    curveType[0] = 0;
                else
                    curveType[0] = 1;
                if (!_showLabel)
                    curveType[0] += 4;
            }
            if (col != nullptr)
            {
                col[0] = _color[0];
                col[1] = _color[1];
                col[2] = _color[2];
            }
            if (!_static)
            {
                int cnt = 0;
                for (int cnt = 0; cnt < ptCnt; cnt++)
                {
                    int absIndex = startPt + cnt;
                    if (absIndex >= int(_values.size())) // i.e. bufferSize
                        absIndex -= int(_values.size());
                    double yVal;
                    if (getTransformedValue(startPt, absIndex, yVal))
                    {
                        double xVal = times[absIndex];
                        xVals.push_back(xVal);
                        yVals.push_back(yVal);
                        if (minMax != nullptr)
                        {
                            if (xVals.size() == 1)
                            {
                                minMax[0] = xVal;
                                minMax[1] = xVal;
                                minMax[2] = yVal;
                                minMax[3] = yVal;
                            }
                            else
                            {
                                if (xVal < minMax[0])
                                    minMax[0] = xVal;
                                if (xVal > minMax[1])
                                    minMax[1] = xVal;
                                if (yVal < minMax[2])
                                    minMax[2] = yVal;
                                if (yVal > minMax[3])
                                    minMax[3] = yVal;
                            }
                        }
                    }
                }
            }
            else
            { // static
                if (curveType != nullptr)
                    curveType[0] += 2; // static
                for (size_t i = 0; i < _staticCurveValues.size() / 2; i++)
                {
                    double xVal = _staticCurveValues[2 * i + 0];
                    double yVal = _staticCurveValues[2 * i + 1];
                    xVals.push_back(xVal);
                    yVals.push_back(yVal);
                    if (minMax != nullptr)
                    {
                        if (xVals.size() == 1)
                        {
                            minMax[0] = xVal;
                            minMax[1] = xVal;
                            minMax[2] = yVal;
                            minMax[3] = yVal;
                        }
                        else
                        {
                            if (xVal < minMax[0])
                                minMax[0] = xVal;
                            if (xVal > minMax[1])
                                minMax[1] = xVal;
                            if (yVal < minMax[2])
                                minMax[2] = yVal;
                            if (yVal > minMax[3])
                                minMax[3] = yVal;
                        }
                    }
                }
            }
            return (true);
        }
        if (index != nullptr)
            index[0]--;
    }
    return (false);
}

bool CGraphDataStream::getExportValue(int startPt, int relPos, double* val, std::string* label) const
{ // only for non-static curves!
    if (label != nullptr)
    {
        label[0] = _streamName;
        if (_unitStr.size() > 0)
            label[0] += " (" + _unitStr + ")";
    }

    if (val != nullptr)
    {
        int absIndex = startPt + relPos;
        if (absIndex >= int(_values.size())) // i.e. bufferSize
            absIndex -= int(_values.size());
        return getTransformedValue(startPt, absIndex, val[0]);
    }
    return (true);
}

void CGraphDataStream::makeStatic(int startPt, int ptCnt, const std::vector<double>& times)
{
    if (!_static)
    {
        std::vector<double> xVals;
        std::vector<double> yVals;
        std::vector<double> zVals;
        _staticCurveValues.clear();
        getCurveData(false, nullptr, startPt, ptCnt, times, nullptr, xVals, yVals, nullptr, nullptr, nullptr);
        for (size_t i = 0; i < xVals.size(); i++)
        {
            _staticCurveValues.push_back(xVals[i]);
            _staticCurveValues.push_back(yVals[i]);
        }
        _values.clear();
        _transformedValues.clear();
        _valuesValidityFlags.clear();
        _transformedValuesValidityFlags.clear();
        _static = true;
    }
}

void CGraphDataStream::serialize(CSer& ar, int startPt, int ptCnt, int bufferSize)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Nme");
            ar << _streamName << _unitStr;
            ar.flush();

            ar.storeDataName("Oid");
            ar << _id;
            ar.flush();

            ar.storeDataName("Sch");
            ar << _scriptHandle;
            ar.flush();

            ar.storeDataName("Col");
            ar << _color[0] << _color[1] << _color[2];
            ar.flush();

            ar.storeDataName("_ar");
            ar << _transformationType;
            ar << _transformationMult << _transformationOff;
            ar << _movingAveragePeriod;
            ar << _cyclicRange;
            ar.flush();

            ar.storeDataName("Pa0");
            unsigned char nothing = 0;
            SIM_SET_CLEAR_BIT(nothing, 0, _visible);
            SIM_SET_CLEAR_BIT(nothing, 1, _showLabel);
            SIM_SET_CLEAR_BIT(nothing, 2, _linkPoints);

            SIM_SET_CLEAR_BIT(nothing, 4, _static);
            ar << nothing;
            ar.flush();

            if (!_static)
            {
                ar.storeDataName("_ts");
                for (int i = 0; i < ptCnt; i++)
                {
                    int absIndex = startPt + i;
                    if (absIndex >= bufferSize)
                        absIndex -= bufferSize;
                    ar << _values[absIndex] << _transformedValues[absIndex];
                    if ((_valuesValidityFlags[absIndex / 8] & (1 << (absIndex & 7))) != 0)
                        ar << (unsigned char)1;
                    else
                        ar << (unsigned char)0;
                    if ((_transformedValuesValidityFlags[absIndex / 8] & (1 << (absIndex & 7))) != 0)
                        ar << (unsigned char)1;
                    else
                        ar << (unsigned char)0;
                }
                ar.flush();
            }
            else
            {
                ar.storeDataName("_ps");
                ar << int(_staticCurveValues.size());
                for (size_t i = 0; i < _staticCurveValues.size(); i++)
                    ar << _staticCurveValues[i];
                ar.flush();
            }

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
                    if (theName.compare("Nme") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _streamName >> _unitStr;
                    }
                    if (theName.compare("Oid") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _id;
                    }
                    if (theName.compare("Sch") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _scriptHandle;
                    }
                    if (theName.compare("Col") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _color[0] >> _color[1] >> _color[2];
                    }
                    if (theName.compare("Var") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _transformationType;
                        float bla, bli;
                        ar >> bla >> bli;
                        _transformationMult = (double)bla;
                        _transformationOff = (double)bli;
                        ar >> _movingAveragePeriod;
                        ar >> bla;
                        _cyclicRange = (double)bla;
                        ;
                    }

                    if (theName.compare("_ar") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _transformationType;
                        ar >> _transformationMult >> _transformationOff;
                        ar >> _movingAveragePeriod;
                        ar >> _cyclicRange;
                    }

                    if (theName == "Pa0")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _visible = SIM_IS_BIT_SET(nothing, 0);
                        _showLabel = SIM_IS_BIT_SET(nothing, 1);
                        _linkPoints = SIM_IS_BIT_SET(nothing, 2);

                        _static = SIM_IS_BIT_SET(nothing, 4);
                    }
                    if (theName.compare("Pts") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        _values.resize(bufferSize, 0.0);
                        _transformedValues.resize(bufferSize, 0.0);
                        _valuesValidityFlags.resize(1 + bufferSize / 8, 0);
                        _transformedValuesValidityFlags.resize(1 + bufferSize / 8, 0);
                        for (int i = 0; i < ptCnt; i++)
                        {
                            float bla, bli;
                            ar >> bla >> bli;
                            _values[i] = (double)bla;
                            _transformedValues[i] = (double)bli;
                            unsigned char b;
                            ar >> b;
                            if (b != 0)
                                _valuesValidityFlags[i / 8] |= (1 << (i & 7));
                            ar >> b;
                            if (b != 0)
                                _transformedValuesValidityFlags[i / 8] |= (1 << (i & 7));
                        }
                    }

                    if (theName.compare("_ts") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        _values.resize(bufferSize, 0.0);
                        _transformedValues.resize(bufferSize, 0.0);
                        _valuesValidityFlags.resize(1 + bufferSize / 8, 0);
                        _transformedValuesValidityFlags.resize(1 + bufferSize / 8, 0);
                        for (int i = 0; i < ptCnt; i++)
                        {
                            ar >> _values[i] >> _transformedValues[i];
                            unsigned char b;
                            ar >> b;
                            if (b != 0)
                                _valuesValidityFlags[i / 8] |= (1 << (i & 7));
                            ar >> b;
                            if (b != 0)
                                _transformedValuesValidityFlags[i / 8] |= (1 << (i & 7));
                        }
                    }

                    if (theName.compare("Sps") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        _staticCurveValues.resize(cnt);
                        float bla;
                        for (int i = 0; i < cnt; i++)
                        {
                            ar >> bla;
                            _staticCurveValues[i] = (double)bla;
                        }
                    }

                    if (theName.compare("_ps") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        _staticCurveValues.resize(cnt);
                        for (int i = 0; i < cnt; i++)
                            ar >> _staticCurveValues[i];
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        if (ar.isStoring())
        {
            ar.xmlAddNode_string("name", _streamName.c_str());
            ar.xmlAddNode_string("unitStr", _unitStr.c_str());
            ar.xmlAddNode_int("id", _id);
            ar.xmlAddNode_int("scriptHandle", _scriptHandle);
            ar.xmlAddNode_floats("color", _color, 3);
            ar.xmlAddNode_float("cyclicRange", _cyclicRange);

            ar.xmlPushNewNode("transformation");
            ar.xmlAddNode_enum("type", _transformationType, sim_stream_transf_raw, "raw", sim_stream_transf_derivative,
                               "derivative", sim_stream_transf_integral, "integral", sim_stream_transf_cumulative,
                               "cumulative");
            ar.xmlAddNode_float("scaling", _transformationMult);
            ar.xmlAddNode_float("offset", _transformationOff);
            ar.xmlAddNode_int("movingAveragePeriod", _movingAveragePeriod);
            ar.xmlPopNode();

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("visible", _visible);
            ar.xmlAddNode_bool("showLabel", _showLabel);
            ar.xmlAddNode_bool("linkPoints", _linkPoints);
            ar.xmlAddNode_bool("static", _static);
            ar.xmlPopNode();

            std::vector<double> tmp;
            if (!_static)
            {
                for (int i = 0; i < ptCnt; i++)
                {
                    int absIndex = startPt + i;
                    if (absIndex >= bufferSize)
                        absIndex -= bufferSize;
                    tmp.push_back(_values[absIndex]);
                }
                ar.xmlAddNode_floats("data", tmp);

                tmp.clear();
                for (int i = 0; i < ptCnt; i++)
                {
                    int absIndex = startPt + i;
                    if (absIndex >= bufferSize)
                        absIndex -= bufferSize;
                    tmp.push_back(_transformedValues[absIndex]);
                }
                ar.xmlAddNode_floats("transformedData", tmp);

                std::vector<bool> tmp2;
                for (int i = 0; i < ptCnt; i++)
                {
                    int absIndex = startPt + i;
                    if (absIndex >= bufferSize)
                        absIndex -= bufferSize;
                    tmp2.push_back(((_valuesValidityFlags[absIndex / 8] & (1 << (absIndex & 7))) != 0));
                }
                ar.xmlAddNode_bools("dataValidity", tmp2);

                tmp2.clear();
                for (int i = 0; i < ptCnt; i++)
                {
                    int absIndex = startPt + i;
                    if (absIndex >= bufferSize)
                        absIndex -= bufferSize;
                    tmp2.push_back(((_transformedValuesValidityFlags[absIndex / 8] & (1 << (absIndex & 7))) != 0));
                }
                ar.xmlAddNode_bools("transformedDataValidity", tmp2);
            }
            else
                ar.xmlAddNode_floats("staticData", _staticCurveValues);
        }
        else
        {
            ar.xmlGetNode_string("name", _streamName);
            ar.xmlGetNode_string("unitStr", _unitStr);
            ar.xmlGetNode_int("id", _id);
            ar.xmlGetNode_int("scriptHandle", _scriptHandle);
            ar.xmlGetNode_floats("color", _color, 3);
            ar.xmlGetNode_float("cyclicRange", _cyclicRange);

            if (ar.xmlPushChildNode("transformation"))
            {
                ar.xmlGetNode_enum("type", _transformationType, true, "raw", sim_stream_transf_raw, "derivative",
                                   sim_stream_transf_derivative, "integral", sim_stream_transf_integral, "cumulative",
                                   sim_stream_transf_cumulative);
                ar.xmlGetNode_float("scaling", _transformationMult);
                ar.xmlGetNode_float("offset", _transformationOff);
                ar.xmlGetNode_int("movingAveragePeriod", _movingAveragePeriod);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("switches"))
            {
                ar.xmlGetNode_bool("visible", _visible);
                ar.xmlGetNode_bool("showLabel", _showLabel);
                ar.xmlGetNode_bool("linkPoints", _linkPoints);
                ar.xmlGetNode_bool("static", _static);
                ar.xmlPopNode();
            }

            if (!_static)
            {
                ar.xmlGetNode_floats("data", _values);

                ar.xmlGetNode_floats("transformedData", _transformedValues);

                std::vector<bool> tmp;
                ar.xmlGetNode_bools("dataValidity", tmp);
                _valuesValidityFlags.resize(1 + bufferSize / 8, 0);
                for (size_t i = 0; i < tmp.size(); i++)
                {
                    if (tmp[i])
                        _valuesValidityFlags[i / 8] |= (1 << (i & 7));
                }

                tmp.clear();
                ar.xmlGetNode_bools("transformedDataValidity", tmp);
                _transformedValuesValidityFlags.resize(1 + bufferSize / 8, 0);
                for (size_t i = 0; i < tmp.size(); i++)
                {
                    if (tmp[i])
                        _transformedValuesValidityFlags[i / 8] |= (1 << (i & 7));
                }
            }
            else
                ar.xmlGetNode_floats("staticData", _staticCurveValues);
        }
    }
}

CGraphDataStream* CGraphDataStream::copyYourself() const
{
    CGraphDataStream* newObj = new CGraphDataStream();
    newObj->_streamName = _streamName;
    newObj->_unitStr = _unitStr;
    newObj->_visible = _visible;
    newObj->_showLabel = _showLabel;
    newObj->_linkPoints = _linkPoints;
    newObj->_static = _static;
    newObj->_cyclicRange = _cyclicRange;
    newObj->_transformationType = _transformationType;
    newObj->_transformationMult = _transformationMult;
    newObj->_transformationOff = _transformationOff;
    newObj->_movingAveragePeriod = _movingAveragePeriod;
    newObj->_id = _id;
    newObj->_scriptHandle = _scriptHandle;
    for (int i = 0; i < 3; i++)
        newObj->_color[i] = _color[i];
    newObj->_values.assign(_values.begin(), _values.end());
    newObj->_transformedValues.assign(_transformedValues.begin(), _transformedValues.end());
    newObj->_valuesValidityFlags.assign(_valuesValidityFlags.begin(), _valuesValidityFlags.end());
    newObj->_transformedValuesValidityFlags.assign(_transformedValuesValidityFlags.begin(),
                                                   _transformedValuesValidityFlags.end());
    newObj->_staticCurveValues.assign(_staticCurveValues.begin(), _staticCurveValues.end());
    return (newObj);
}

bool CGraphDataStream::announceScriptWillBeErased(int scriptHandle, bool simulationScript,
                                                  bool sceneSwitchPersistentScript, bool copyBuffer)
{
    return ((scriptHandle == _scriptHandle) && (!sceneSwitchPersistentScript));
}

void CGraphDataStream::performScriptLoadingMapping(const std::map<int, int>* map)
{
    _scriptHandle = CWorld::getLoadingMapping(map, _scriptHandle);
}
