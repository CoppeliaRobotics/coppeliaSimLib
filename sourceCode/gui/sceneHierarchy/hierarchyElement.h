
// This file requires some serious refactoring!!

#pragma once

#include "3DObject.h"

#define HIERARCHY_INTER_LINE_SPACE 18
#define HIERARCHY_HALF_INTER_LINE_SPACE 9
#define HIERARCHY_TEXT_CENTER_OFFSET (HIERARCHY_HALF_INTER_LINE_SPACE-4)
#define HIERARCHY_ICON_WIDTH 20
#define HIERARCHY_ICON_HEIGHT 16
#define HIERARCHY_ICON_HALF_WIDTH 10
#define HIERARCHY_ICON_HALF_HEIGHT 8
#define HIERARCHY_ICON_QUARTER_WIDTH 5
#define HIERARCHY_ICON_QUARTER_HEIGHT 4

#define HIERARCHY_INTER_ICON_SPACING 4
/*
#define HIERARCHY_INTER_LINE_SPACE 36
#define HIERARCHY_HALF_INTER_LINE_SPACE 18
#define HIERARCHY_TEXT_CENTER_OFFSET (HIERARCHY_HALF_INTER_LINE_SPACE-8)
#define HIERARCHY_ICON_WIDTH 40
#define HIERARCHY_ICON_HEIGHT 32
#define HIERARCHY_ICON_HALF_WIDTH 20
#define HIERARCHY_ICON_HALF_HEIGHT 16
#define HIERARCHY_ICON_QUARTER_WIDTH 10
#define HIERARCHY_ICON_QUARTER_HEIGHT 8

#define HIERARCHY_INTER_ICON_SPACING 8
*/
class CHierarchy;

class CHierarchyElement  
{
public:
    CHierarchyElement(int theObjectID);
    virtual ~CHierarchyElement();
    int addYourChildren();
    int getLinkedObjectID();
    int getNumberOfElements();
    int computeNumberOfElements();
    CHierarchyElement* getElementLinkedWithObject(int objID);

    void renderElement_3DObject(CHierarchy* hier,int labelEditObjectID,bool& bright,bool dontDisplay,
        int renderingSize[2],int textPos[2],
        int indentNb,std::vector<int>* vertLines,int minRenderedPos[2],int maxRenderedPos[2],bool forDragAndDrop=false,int transparentForTreeObjects=-1,int dropID=-2,int worldClick=-9999);
    void renderElement_editModeList(CHierarchy* hier,int labelEditObjectID,bool& bright,bool dontDisplay,
        int renderingSize[2],int textPos[2],int indentNb,int minRenderedPos[2],int maxRenderedPos[2],
        int selectedState,int editModeType);
    static bool renderDummyElement(bool& bright,int renderingSize[2],int textPos[2]);
    void setSceneName(const std::string& sn);
    bool isLocalWorld();

    std::vector<CHierarchyElement*> children;

private:
#ifdef KEYWORD__NOT_DEFINED_FORMELY_BR
    int _drawIcon_brm(CHierarchy* hier,int tPosX,int tPosY,C3DObject* it,int pictureID,bool drawIt,float transparencyFactor,bool forDragAndDrop);
#else
    int _drawIcon_3DObject(CHierarchy* hier,int tPosX,int tPosY,C3DObject* it,int pictureID,bool drawIt,float transparencyFactor,bool forDragAndDrop);
#endif
    int _drawIcon_editModeList(CHierarchy* hier,int tPosX,int tPosY,int pictureID,bool drawIt);
    void _drawTexturedIcon(int tPosX,int tPosY,int sizeX,int sizeY,float transparencyFactor);
    int objectID;
    std::string _sceneName;
    int numberOfElements;
};
