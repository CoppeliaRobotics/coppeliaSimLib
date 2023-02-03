#pragma once

#include <ser.h>

class CGraphDataStream
{
public:
    CGraphDataStream();
    CGraphDataStream(const char* streamName,const char* unitStr,int options,const float* color,double cyclicRange,int scriptHandle);
    virtual ~CGraphDataStream();

    void setBasics(const char* unitStr,int options,const float* color,double cyclicRange,int scriptHandle);
    bool setTransformation(int trType,double mult,double off,int movAvgPeriod);
    void serialize(CSer& ar,int startPt,int ptCnt,int bufferSize);
    CGraphDataStream* copyYourself() const;
    void reset(size_t bufferSize);
    void setNextValueToInsert(double v);
    void insertNextValue(int absIndex,bool firstValue,const std::vector<double>& times);
    bool getTransformedValue(int startPt,int pos,double& retVal) const;
    bool getCurveData(bool staticCurve,int* index,int startPt,int ptCnt,const std::vector<double>& times,std::string* label,std::vector<double>& xVals,std::vector<double>& yVals,int* curveType,float col[3],double minMax[6]) const;
    bool getExportValue(int startPt,int relPos,double* val,std::string* label) const;
    void makeStatic(int startPt,int ptCnt,const std::vector<double>& times);
    bool announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript,bool copyBuffer);
    void performScriptLoadingMapping(const std::map<int,int>* map);

    void setId(int id);
    int getId() const;
    void setStreamName(const char* nm);
    std::string getStreamName() const;
    std::string getUnitStr() const;
    int getOptions() const;
    bool getIsStatic() const;
    double getCyclicRange() const;
    const float* getColorPtr() const;
    int getScriptHandle() const;


protected:
    std::vector <double> _values;
    std::vector <double> _transformedValues;
    std::vector <unsigned char> _valuesValidityFlags;
    std::vector <unsigned char> _transformedValuesValidityFlags;

    std::vector<double> _staticCurveValues;

    std::string _streamName;
    std::string _unitStr;
    int _scriptHandle;
    bool _visible;
    bool _showLabel;
    bool _linkPoints;
    bool _static;
    double _cyclicRange;
    float _color[3];
    int _transformationType;
    double _transformationMult;
    double _transformationOff;
    int _movingAveragePeriod;
    int _id;

    double _nextValueToInsert;
    bool _nextValueToInsertIsValid;
};
