
#pragma once

#include "vrepMainHeader.h"
#include "textureObject.h"
#include "ser.h"
#include "7Vector.h"

class CTextureProperty
{
public:
    CTextureProperty();
    CTextureProperty(int textureOrVisionSensorObjectID);
    virtual ~CTextureProperty();

    CTextureProperty* copyYourself();
    void serialize(CSer& ar);
    bool announceObjectWillBeErased(int objID);
    void performObjectLoadingMapping(std::vector<int>* map);
    void performTextureObjectLoadingMapping(std::vector<int>* map);
    void addTextureDependencies(int objID,int objSubID);
    void scaleObject(float scalingFactor);
    void setInterpolateColors(bool ic);
    bool getInterpolateColors();
    void setApplyMode(int dt);
    int getApplyMode();
    void adjustForFrameChange(const C7Vector& mCorrection);

    int getTextureObjectID();
    CTextureObject* getTextureObject();
    std::vector<float>* getTextureCoordinates(int objectStateId,const C7Vector& transf,const std::vector<float>& vertices,const std::vector<int>& triangles);
    std::vector<float>* getFixedTextureCoordinates();
    C7Vector getTextureRelativeConfig();
    void setTextureRelativeConfig(const C7Vector& c);
    void getTextureScaling(float& x,float& y);
    void setTextureScaling(float x,float y);
    void setRepeatU(bool r);
    bool getRepeatU();
    void setRepeatV(bool r);
    bool getRepeatV();

    void setFixedCoordinates(std::vector<float>* coords); // nullptr to remove them and have calculated coords
    bool getFixedCoordinates();

    void setTextureMapMode(int mode);
    int getTextureMapMode();

    int* getTexCoordBufferIdPointer();

    void setStartedTextureObject(CTextureObject* it);
    CTextureObject* getStartedTextureObject();


private:
    void _commonInit();
    CTextureObject* _startedTexture;

    // to copy and serialize:
    bool _interpolateColor;
    int _applyMode;
    bool _repeatU;
    bool _repeatV;
    int _textureOrVisionSensorObjectID;
    int _textureCoordinateMode;
    C7Vector _textureRelativeConfig;
    float _textureScalingX;
    float _textureScalingY;
    int _texCoordBufferId; // used for VBOs
    std::vector<float> _fixedTextureCoordinates;

    // do not copy nor serialize:
    int _objectStateId;
    std::vector<float> _calculatedTextureCoordinates;
};
