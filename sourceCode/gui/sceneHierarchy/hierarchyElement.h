// This file requires some serious refactoring!!

#pragma once

#include <sceneObject.h>

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
class CHierarchy;

class CHierarchyElement  
{
public:
    CHierarchyElement(int theObjectID);
    virtual ~CHierarchyElement();
    void addYourChildren();
    int getLinkedObjectID();
    CHierarchyElement* getElementLinkedWithObject(int objID);

    void renderElement_sceneObject(CHierarchy* hier,int labelEditObjectID,bool& bright,bool dontDisplay,
        int renderingSize[2],int textPos[2],
        int indentNb,std::vector<int>* vertLines,int minRenderedPos[2],int maxRenderedPos[2],bool forDragAndDrop=false,int transparentForTreeObjects=-1,int dropID=-2,int worldClick=-9999);
    void renderElement_editModeList(CHierarchy* hier,int labelEditObjectID,bool& bright,bool dontDisplay,
        int renderingSize[2],int textPos[2],int indentNb,int minRenderedPos[2],int maxRenderedPos[2],
        int selectedState,int editModeType);
    static bool renderDummyElement(bool& bright,int renderingSize[2],int textPos[2]);
    void setSceneName(const char* sn);
    bool isLocalWorld();

    std::vector<CHierarchyElement*> children;

private:
    int _drawIcon_sceneObject(CHierarchy* hier,int tPosX,int tPosY,CSceneObject* it,int pictureID,bool drawIt,double transparencyFactor,bool forDragAndDrop);
    int _drawIcon_editModeList(CHierarchy* hier,int tPosX,int tPosY,int pictureID,bool drawIt);
    void _drawTexturedIcon(int tPosX,int tPosY,int sizeX,int sizeY,double transparencyFactor);
    int objectID;
    std::string _sceneName;
};
