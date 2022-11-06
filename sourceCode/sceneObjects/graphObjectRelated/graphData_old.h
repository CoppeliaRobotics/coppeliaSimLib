#pragma once

#include "7Vector.h"
#include "ser.h"

class CGraphData_old
{
public:
    CGraphData_old();
    CGraphData_old(int theDataType,int theDataObjectID,int theDataObjectAuxID);
    virtual ~CGraphData_old();

    // Various
    int getDataType() const;
    int getDataObjectID() const;
    int getDataObjectAuxID() const;
    void setDataObjectID(int newID);
    void setDataObjectAuxID(int newID);

    void setZoomFactor(float newZoomFactor);
    float getZoomFactor() const;
    void setAddCoeff(float newCoeff);
    float getAddCoeff() const;
    void setVisible(bool v);
    bool getVisible() const;
    void setIdentifier(int newIdentifier);
    int getIdentifier() const;
    void setLinkPoints(bool l);
    bool getLinkPoints() const;
    std::string getName() const;
    void setName(std::string theName);
    void setLabel(bool l);
    bool getLabel() const;
    void resetData(int bufferSize);
    void setDerivativeIntegralAndCumulative(int val);
    int getDerivativeIntegralAndCumulative() const;

    void setValue(const C7Vector* graphCTM,int absIndex,bool firstValue,bool cyclic,float range,const std::vector<float>& times);
    void setValueDirect(int absIndex,float theValue,bool firstValue,bool cyclic,float range,const std::vector<float>& times);
    bool getValue(int absIndex,float& v) const;
    bool getValueRaw(int absIndex,float& v) const;

    int getDataLength();
    void setUserData(float data);
    void clearUserData();

    void setMovingAverageCount(int c);
    int getMovingAverageCount() const;

    void serialize(CSer& ar,void* it);
    CGraphData_old* copyYourself();
    void performObjectLoadingMapping(const std::map<int,int>* map);
    void performCollisionLoadingMapping(const std::map<int,int>* map);
    void performDistanceLoadingMapping(const std::map<int,int>* map);
    void performIkLoadingMapping(const std::map<int,int>* map);
    bool announceObjectWillBeErased(int objID,bool copyBuffer);
    bool announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    bool announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    bool announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    
    // Variables which need to be serialized & copied 
    float ambientColor[3];

protected:
    // Variables which need to be serialized & copied 
    std::vector <float> _floatData;
    std::vector <float> _transformedFloatData;
    std::vector <unsigned char> _floatDataValidFlags;
    std::vector <unsigned char> _transformedFloatDataValidFlags;
    int dataType;
    int dataObjectID;
    int dataObjectAuxID;
    float zoomFactor;
    float addCoeff;
    int identifier;
    bool visible;
    bool linkPoints;
    bool label;
    int _derivativeIntegralAndCumulative;
    std::string name;


private:
    int _lifeID;
    float _userData;
    bool _userDataValid;
    int _movingAverageCount;
};
