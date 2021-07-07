#pragma once

#include "sceneObject.h"
#include "scriptObject.h"
#include "dynMaterialObject.h"

// old:
#include "collection.h"
#include "collisionObject_old.h"
#include "distanceObject_old.h"
#include "ikGroup_old.h"
#include "pathPlanningTask_old.h"
#include "buttonBlock.h"

class CCopyBuffer  
{
public:
    CCopyBuffer();
    virtual ~CCopyBuffer();

    void clearBuffer();
    int pasteBuffer(bool intoLockedScene,int selectionMode);
    bool isBufferEmpty();
    void copyCurrentSelection(std::vector<int>* sel,bool fromLockedScene,int options);
    void serializeCurrentSelection(CSer& ar,std::vector<int>* sel,C7Vector& modelTr,C3Vector& modelBBSize,float modelNonDefaultTranslationStepSize);
    bool isCopyForPasting();

    void memorizeBuffer();
    void restoreBuffer();
    void clearMemorizedBuffer();

    void _backupBuffers_temp();
    void _restoreBuffers_temp();
private:

    void _announceObjectWillBeErased(int objectID);
    void _announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript);

    // Old:
    void _announceCollectionWillBeErased(int groupID);
    void _announceIkGroupWillBeErased(int ikGroupID);
    void _announceCollisionWillBeErased(int collisionID);
    void _announceDistanceWillBeErased(int distanceID);
    void _announce2DElementWillBeErased(int elementID);

    void _eraseObjectInBuffer(int objectID);
    void _eraseScriptInBuffer(int objectID);
    void _eraseTextureObjectInBuffer(int objectID);

    // Old:
    void _erase2DElementInBuffer(int objectID);
    void _erasePathPlanningTaskInBuffer(int objectID);
    void _eraseCollisionInBuffer(int objectID);
    void _eraseDistanceInBuffer(int objectID);
    void _eraseCollectionInBuffer(int objectID);
    void _eraseIkObjectInBuffer(int objectID);

    bool _copyIsForPasting;


    bool _bufferIsFromLockedScene;
    std::vector<CSceneObject*> objectBuffer;
    std::vector<CScriptObject*> luaScriptBuffer;
    std::vector<CTextureObject*> textureObjectBuffer;

    // Old:
    std::vector<CCollection*> collectionBuffer;
    std::vector<CCollisionObject_old*> collisionBuffer;
    std::vector<CDistanceObject_old*> distanceBuffer;
    std::vector<CIkGroup_old*> ikGroupBuffer;
    std::vector<CPathPlanningTask*> pathPlanningTaskBuffer;
    std::vector<CButtonBlock*> buttonBlockBuffer;

    // Following buffers are used TEMPORARILY when saving a model. They will always be empty before and after serialization
    std::vector<CSceneObject*> objectBuffer_tempSer;
    std::vector<CScriptObject*> luaScriptBuffer_tempSer;
    std::vector<CTextureObject*> textureObjectBuffer_tempSer;

    // Old:
    std::vector<CCollection*> collectionBuffer_tempSer;
    std::vector<CCollisionObject_old*> collisionBuffer_tempSer;
    std::vector<CDistanceObject_old*> distanceBuffer_tempSer;
    std::vector<CIkGroup_old*> ikGroupBuffer_tempSer;
    std::vector<CPathPlanningTask*> pathPlanningTaskBuffer_tempSer;
    std::vector<CButtonBlock*> buttonBlockBuffer_tempSer;



    bool _bufferIsFromLockedScene_memorized;
    std::vector<CSceneObject*> objectBuffer_memorized;
    std::vector<CScriptObject*> luaScriptBuffer_memorized;
    std::vector<CTextureObject*> textureObjectBuffer_memorized;

    // Old:
    std::vector<CCollection*> collectionBuffer_memorized;
    std::vector<CCollisionObject_old*> collisionBuffer_memorized;
    std::vector<CDistanceObject_old*> distanceBuffer_memorized;
    std::vector<CIkGroup_old*> ikGroupBuffer_memorized;
    std::vector<CPathPlanningTask*> pathPlanningTaskBuffer_memorized;
    std::vector<CButtonBlock*> buttonBlockBuffer_memorized;
};
