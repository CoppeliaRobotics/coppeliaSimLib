#pragma once

#include "ser.h"
#include "graphDataStream.h"

class CGraphCurve
{
public:
    CGraphCurve();
    CGraphCurve(int dim,const int streamIds[3],const float defaultVals[3],const char* curveName,const char* unitStr,int options,const float* color,int curveWidth);
    virtual ~CGraphCurve();

    void serialize(CSer& ar,int startPt,int ptCnt,int bufferSize);
    CGraphCurve* copyYourself() const;
    void setBasics(int dim,const int streamIds[3],const float defaultVals[3],const char* unitStr,int options,const float* color,int curveWidth);
    void updateStreamIds(const std::vector<int>& allStreamIds);
    bool getCurveData_xy(CGraphDataStream* streams[3],int* index,int bufferSize,int startPt,int ptCnt,std::string* label,std::vector<float>& xVals,std::vector<float>& yVals,std::vector<float>& zVals,int* curveType,float col[3],float minMax[6]) const;
    bool getCurveData_xyz(CGraphDataStream* streams[3],int* index,int bufferSize,int startPt,int ptCnt,std::string* label,std::vector<float>& xVals,std::vector<float>& yVals,std::vector<float>& zVals,int* curveType,float col[3],float minMax[6]) const;
    void makeStatic(CGraphDataStream* streams[3],int bufferSize,int startPt,int ptCnt);

    void setId(int id);
    int getId() const;
    void setCurveName(const char* nm);
    std::string getCurveName() const;
    std::string getUnitStr() const;
    int getOptions() const;
    bool getIsStatic() const;
    const int* getStreamIdsPtr() const;
    int getCurveWidth() const;
    const float* getColorPtr() const;
    int getDim() const;
    const float* getDefaultValsPtr() const;

protected:
    std::vector<float> _staticCurveValues;

    int _streamIds[3];
    std::string _curveName;
    std::string _unitStr;
    bool _relativeToWorld;
    bool _showLabel;
    bool _linkPoints;
    bool _isStatic;
    float _color[3];
    float _defaultVals[3];
    int _curveWidth;
    int _id;
    int _dim;
};
