
#pragma once

#include "vrepMainHeader.h"

// FULLY STATIC CLASS
class CCuttingRoutine  
{
public:
    // The one and only general routine:
    static int cutEntity(int millID,int entityID,int& cutObject,float& cutSurface,float& cutVolume,bool justForInitialization,bool overrideCuttableFlagIfNonCollection);
private:
    static bool _cutShape(int millID,int shapeID,float& cutSurface,bool justForInitialization,bool overrideShapeCuttableFlag);
    static int _cutGroup(int millID,int groupID,int& cutObject,float& cutSurface,float& cutVolume,bool justForInitialization);
};
