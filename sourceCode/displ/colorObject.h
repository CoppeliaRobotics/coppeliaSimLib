#pragma once

#include "_colorObject_.h"
#include "ser.h"

class CColorObject : public _CColorObject_
{
public:
    CColorObject();
    virtual ~CColorObject();

    // Overridden from CSyncObject:
    void buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting);
    void connectSynchronizationObject();
    void removeSynchronizationObject();

    void makeCurrentColor(bool useAuxiliaryComponent) const;
    void makeCurrentColor2(bool forceNonTransparent,bool useAuxiliaryComponent) const;
    void setDefaultValues();
    void setColorsAllBlack();
    void setColor(const float theColor[3],unsigned char colorMode);
    void setColor(float r,float g,float b,unsigned char colorMode);
    void copyYourselfInto(CColorObject* it) const;
    void serialize(CSer& ar,int objType); // 0=3d mesh, 1=3d lines, 2=3d points, 3=3d light, 4=2d thing
    void setConvexColors();

    void getColor(float col[3],unsigned char colorMode) const;

    void setFlash(bool flashIsOn);
    bool getFlash() const;
    void setFlashFrequency(float f);
    float getFlashFrequency() const;
    void setFlashRatio(float f);
    float getFlashRatio() const;
    void setFlashPhase(float f);
    float getFlashPhase() const;
    void setUseSimulationTime(bool sim);
    bool getUseSimulationTime() const;

private:
    std::string _getPatternStringFromPatternId_backwardCompatibility_3_2_2016(int id);

    bool _useSimulationTime;
    float _flashFrequency;
    float _flashRatio;
    float _flashPhase;
    bool _flash;
};
