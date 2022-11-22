#pragma once

#include "ser.h"

class CGraphDataStream
{
public:
    CGraphDataStream();
    CGraphDataStream(const char* streamName,const char* unitStr,int options,const floatDouble* color,floatDouble cyclicRange,int scriptHandle);
    virtual ~CGraphDataStream();

    void setBasics(const char* unitStr,int options,const floatDouble* color,floatDouble cyclicRange,int scriptHandle);
    bool setTransformation(int trType,floatDouble mult,floatDouble off,int movAvgPeriod);
    void serialize(CSer& ar,int startPt,int ptCnt,int bufferSize);
    CGraphDataStream* copyYourself() const;
    void reset(size_t bufferSize);
    void setNextValueToInsert(floatDouble v);
    void insertNextValue(int absIndex,bool firstValue,const std::vector<floatDouble>& times);
    bool getTransformedValue(int startPt,int pos,floatDouble& retVal) const;
    bool getCurveData(bool staticCurve,int* index,int startPt,int ptCnt,const std::vector<floatDouble>& times,std::string* label,std::vector<floatDouble>& xVals,std::vector<floatDouble>& yVals,int* curveType,floatDouble col[3],floatDouble minMax[6]) const;
    bool getExportValue(int startPt,int relPos,floatDouble* val,std::string* label) const;
    void makeStatic(int startPt,int ptCnt,const std::vector<floatDouble>& times);
    bool announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript,bool copyBuffer);
    void performScriptLoadingMapping(const std::map<int,int>* map);

    void setId(int id);
    int getId() const;
    void setStreamName(const char* nm);
    std::string getStreamName() const;
    std::string getUnitStr() const;
    int getOptions() const;
    bool getIsStatic() const;
    floatDouble getCyclicRange() const;
    const floatDouble* getColorPtr() const;
    int getScriptHandle() const;


protected:
    std::vector <floatDouble> _values;
    std::vector <floatDouble> _transformedValues;
    std::vector <unsigned char> _valuesValidityFlags;
    std::vector <unsigned char> _transformedValuesValidityFlags;

    std::vector<floatDouble> _staticCurveValues;

    std::string _streamName;
    std::string _unitStr;
    int _scriptHandle;
    bool _visible;
    bool _showLabel;
    bool _linkPoints;
    bool _static;
    floatDouble _cyclicRange;
    floatDouble _color[3];
    int _transformationType;
    floatDouble _transformationMult;
    floatDouble _transformationOff;
    int _movingAveragePeriod;
    int _id;

    floatDouble _nextValueToInsert;
    bool _nextValueToInsertIsValid;
};
