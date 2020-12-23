#pragma once

#include "ser.h"

class CGraphDataStream
{
public:
    CGraphDataStream();
    CGraphDataStream(const char* streamName,const char* unitStr,int options,const float* color,float cyclicRange,int scriptHandle);
    virtual ~CGraphDataStream();

    void setBasics(const char* unitStr,int options,const float* color,float cyclicRange,int scriptHandle);
    bool setTransformation(int trType,float mult,float off,int movAvgPeriod);
    void serialize(CSer& ar,int startPt,int ptCnt,int bufferSize);
    CGraphDataStream* copyYourself() const;
    void reset(size_t bufferSize);
    void setNextValueToInsert(float v);
    void insertNextValue(int absIndex,bool firstValue,const std::vector<float>& times);
    bool getTransformedValue(int startPt,int pos,float& retVal) const;
    bool getCurveData(bool staticCurve,int* index,int startPt,int ptCnt,const std::vector<float>& times,std::string* label,std::vector<float>& xVals,std::vector<float>& yVals,int* curveType,float col[3],float minMax[6]) const;
    void makeStatic(int startPt,int ptCnt,const std::vector<float>& times);
    bool announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript,bool copyBuffer);
    void performScriptLoadingMapping(const std::vector<int>* map);

    void setId(int id);
    int getId() const;
    void setStreamName(const char* nm);
    std::string getStreamName() const;
    std::string getUnitStr() const;
    int getOptions() const;
    bool getIsStatic() const;
    float getCyclicRange() const;
    const float* getColorPtr() const;
    int getScriptHandle() const;


protected:
    std::vector <float> _values;
    std::vector <float> _transformedValues;
    std::vector <unsigned char> _valuesValidityFlags;
    std::vector <unsigned char> _transformedValuesValidityFlags;

    std::vector<float> _staticCurveValues;

    std::string _streamName;
    std::string _unitStr;
    int _scriptHandle;
    bool _visible;
    bool _showLabel;
    bool _linkPoints;
    bool _static;
    float _cyclicRange;
    float _color[3];
    int _transformationType;
    float _transformationMult;
    float _transformationOff;
    int _movingAveragePeriod;
    int _id;

    float _nextValueToInsert;
    bool _nextValueToInsertIsValid;
};
