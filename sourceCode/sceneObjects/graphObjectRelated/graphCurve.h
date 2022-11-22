#pragma once

#include "ser.h"
#include "graphDataStream.h"
#include "7Vector.h"

class CGraphCurve
{
public:
    CGraphCurve();
    CGraphCurve(int dim,const int streamIds[3],const floatDouble defaultVals[3],const char* curveName,const char* unitStr,int options,const floatDouble* color,int curveWidth,int scriptHandle);
    virtual ~CGraphCurve();

    void serialize(CSer& ar,int startPt,int ptCnt,int bufferSize);
    CGraphCurve* copyYourself() const;
    void setBasics(int dim,const int streamIds[3],const floatDouble defaultVals[3],const char* unitStr,int options,const floatDouble* color,int curveWidth,int scriptHandle);
    void updateStreamIds(const std::vector<int>& allStreamIds);
    bool getCurveData_xy(CGraphDataStream* streams[3],int* index,int bufferSize,int startPt,int ptCnt,std::string* label,std::vector<floatDouble>& xVals,std::vector<floatDouble>& yVals,int* curveType,floatDouble col[3],floatDouble minMax[6]) const;
    bool getCurveData_xyz(CGraphDataStream* streams[3],int* index,int bufferSize,int startPt,int ptCnt,std::string* label,std::vector<floatDouble>& xVals,int* curveType,floatDouble col[3],floatDouble minMax[6],int* curveWidth) const;
    void makeStatic(CGraphDataStream* streams[3],int bufferSize,int startPt,int ptCnt);
    bool announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript,bool copyBuffer);
    void performScriptLoadingMapping(const std::map<int,int>* map);

    void setId(int id);
    int getId() const;
    void setCurveName(const char* nm);
    std::string getCurveName() const;
    std::string getUnitStr() const;
    int getOptions() const;
    bool getIsStatic() const;
    const int* getStreamIdsPtr() const;
    int getCurveWidth() const;
    const floatDouble* getColorPtr() const;
    int getDim() const;
    const floatDouble* getDefaultValsPtr() const;
    int getScriptHandle() const;

protected:
    std::vector<floatDouble> _staticCurveValues;

    int _streamIds[3];
    std::string _curveName;
    std::string _unitStr;
    bool _relativeToGraph;
    bool _showLabel;
    bool _linkPoints;
    bool _isStatic;
    floatDouble _color[3];
    floatDouble _defaultVals[3];
    int _curveWidth;
    int _id;
    int _scriptHandle;
    int _dim;
};
