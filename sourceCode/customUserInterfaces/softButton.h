
// This file requires some serious refactoring!!

#pragma once

#include "textureProperty.h"
#include "VPoint.h"

class CSoftButton  
{
public:
    CSoftButton(std::string theLabel,int w,int h,int theLength,int theHeight);
    virtual ~CSoftButton();

    void announceSceneObjectWillBeErased(int objID);
    void performTextureObjectLoadingMapping(const std::vector<int>* map);
    void performSceneObjectLoadingMapping(const std::vector<int>* map);
    void setTextureDependencies(int buttonBlockID);

    float backgroundColor[3];
    float downBackgroundColor[3];
    float textColor[3];
    std::string label;
    std::string downLabel;
    int buttonID;
    int xPos;
    int yPos;

    void serialize(CSer& ar);

    void setSliderPos(float pos);
    float getSliderPos();
    void setVertical(bool isVertical);
    bool getVertical();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationEnded();

    int getButtonType();
    void enableArray(bool enable);
    bool isArrayEnabled();
    bool setArrayColor(int x,int y,const float col[3]);
    int getLength();
    void adjustLength(int newLength);
    int getHeight();
    void adjustHeight(int newHeight);
    void setAttributes(int attr);
    int getAttributes();
    int getUniqueID();
    void setTextureProperty(CTextureProperty* tp);
    void removeVisionSensorTexture();
    CTextureProperty* getTextureProperty();
    float* arrayColors; // not serialized, but copied!

    CSoftButton* copyYourself();

private:
    CTextureProperty* _textureProperty;
    int _buttonAttributes;
    int length;
    int height;

    float sliderPos;

    bool vertical; // don't serialize, it is calculated

    bool _initialValuesInitialized;
    std::string _initialSimulation_label;
    int _initialSimulation_buttonAttributes;
    float _initialSimulation_sliderPos;

    int _buttonUniqueID;

    static int _nextButtonUniqueID;
#ifdef SIM_WITH_GUI
public:
    void drawArray(VPoint p,VPoint s);

#endif
};
