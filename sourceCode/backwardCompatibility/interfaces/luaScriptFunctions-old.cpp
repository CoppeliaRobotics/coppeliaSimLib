bool isObjectAssociatedWithThisThreadedChildScriptValid_old(luaWrap_lua_State* L)
{
    int id = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* script = App::worldContainer->getScriptObjectFromHandle(id);
    if (script == nullptr)
        return (false);
    int h = script->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_simulation);
    return (h != -1);
}

void pushCorrectTypeOntoLuaStack_old(luaWrap_lua_State* L, const std::string& buff)
{ // Pushes nil, false, true, number or string (in that order!!) onto the stack depending on the buff content!
    int t = getCorrectType_old(buff);
    if (t == 0)
        luaWrap_lua_pushnil(L);
    if (t == 1)
        luaWrap_lua_pushboolean(L, 0);
    if (t == 2)
        luaWrap_lua_pushboolean(L, 1);
    if (t == 3)
    {
        double floatVal;
        tt::getValidFloat(buff.c_str(), floatVal);
        luaWrap_lua_pushnumber(L, floatVal);
    }
    if (t == 4)
        luaWrap_lua_pushbinarystring(L, buff.c_str(), buff.length()); // push binary string for backw. comp.
}

int getCorrectType_old(const std::string& buff)
{ // returns 0=nil, 1=boolean false, 2=boolean true, 3=number or 4=string (in that order!!) depending on the buff
    // content!
    if (buff.length() != 0)
    {
        if (buff.length() != strlen(buff.c_str()))
            return (4); // We have embedded zeros, this has definitively to be a string:
    }
    if (strcmp(buff.c_str(), "nil") == 0)
        return (0);
    if (strcmp(buff.c_str(), "false") == 0)
        return (1);
    if (strcmp(buff.c_str(), "true") == 0)
        return (2);
    double floatVal;
    if (tt::getValidFloat(buff.c_str(), floatVal))
        return (3);
    return (4);
}

void getScriptTree_old(luaWrap_lua_State* L, bool selfIncluded, std::vector<int>& scriptHandles)
{ // Returns all scripts that are built under the current one
    scriptHandles.clear();
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);

    CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    if (it != nullptr)
    {
        if (it->getScriptType() == sim_scripttype_main)
        { // we have a main script here
            if (selfIncluded)
                scriptHandles.push_back(currentScriptID);
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
            {
                CSceneObject* q = App::currentWorld->sceneObjects->getObjectFromIndex(i);
                CScriptObject* lso = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                    sim_scripttype_simulation, q->getObjectHandle());
                if (lso != nullptr)
                    scriptHandles.push_back(lso->getScriptHandle());
            }
        }

        if (it->getScriptType() == sim_scripttype_simulation)
        { // we have a simulation script
            CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(
                it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_simulation));
            if (obj != nullptr)
            { // should always pass
                if (selfIncluded)
                    scriptHandles.push_back(currentScriptID);

                std::vector<CSceneObject*> objList;
                obj->getAllObjectsRecursive(&objList, false);
                for (int i = 0; i < int(objList.size()); i++)
                {
                    CScriptObject* lso = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                        sim_scripttype_simulation, objList[i]->getObjectHandle());
                    if (lso != nullptr)
                        scriptHandles.push_back(lso->getScriptHandle());
                }
            }
        }

        if (it->getScriptType() == sim_scripttype_customization)
        { // we have a customization script
            CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(
                it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_customization));
            if (obj != nullptr)
            { // should always pass
                if (selfIncluded)
                {
                    CScriptObject* aScript = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                        sim_scripttype_simulation, obj->getObjectHandle());
                    if (aScript != nullptr)
                        scriptHandles.push_back(aScript->getScriptHandle());
                }

                std::vector<CSceneObject*> objList;
                obj->getAllObjectsRecursive(&objList, false);
                for (int i = 0; i < int(objList.size()); i++)
                {
                    CScriptObject* lso = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                        sim_scripttype_simulation, objList[i]->getObjectHandle());
                    if (lso != nullptr)
                    {
                        scriptHandles.push_back(lso->getScriptHandle());
                    }
                }
            }
        }
    }
}

void getScriptChain_old(luaWrap_lua_State* L, bool selfIncluded, bool mainIncluded, std::vector<int>& scriptHandles)
{ // Returns all script IDs that are parents (or grand-parents,grand-grand-parents, etc.) of the current one
    scriptHandles.clear();
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);

    CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);

    if (it != nullptr)
    {
        if (it->getScriptType() == sim_scripttype_main)
        { // we have a main script here
            if (selfIncluded && mainIncluded)
                scriptHandles.push_back(currentScriptID);
        }

        if (it->getScriptType() == sim_scripttype_simulation)
        { // we have a simulation script here
            CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(
                it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_simulation));
            if (obj != nullptr)
            {
                if (selfIncluded)
                    scriptHandles.push_back(currentScriptID);
                while (obj->getParent() != nullptr)
                {
                    obj = obj->getParent();
                    CScriptObject* lso = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                        sim_scripttype_simulation, obj->getObjectHandle());
                    if (lso != nullptr)
                        scriptHandles.push_back(lso->getScriptHandle());
                }
                if (mainIncluded)
                {
                    CScriptObject* lso = App::currentWorld->sceneObjects->embeddedScriptContainer->getMainScript();
                    if (lso != nullptr)
                        scriptHandles.push_back(lso->getScriptHandle());
                }
            }
        }

        if (it->getScriptType() == sim_scripttype_customization)
        { // we have a customization script here
            CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(
                it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_customization));
            if (obj != nullptr)
            {
                if (selfIncluded)
                {
                    CScriptObject* aScript = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                        sim_scripttype_simulation, obj->getObjectHandle());
                    if (aScript != nullptr)
                        scriptHandles.push_back(aScript->getScriptHandle());
                }
                while (obj->getParent() != nullptr)
                {
                    obj = obj->getParent();
                    CScriptObject* lso = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                        sim_scripttype_simulation, obj->getObjectHandle());
                    if (lso != nullptr)
                        scriptHandles.push_back(lso->getScriptHandle());
                }
                if (mainIncluded)
                {
                    CScriptObject* lso = App::currentWorld->sceneObjects->embeddedScriptContainer->getMainScript();
                    if (lso != nullptr)
                        scriptHandles.push_back(lso->getScriptHandle());
                }
            }
        }
    }
}
bool readCustomFunctionDataFromStack_old(luaWrap_lua_State* L, int ind, int dataType, std::vector<char>& inBoolVector,
                                         std::vector<int>& inIntVector, std::vector<float>& inFloatVector,
                                         std::vector<double>& inDoubleVector, std::vector<std::string>& inStringVector,
                                         std::vector<std::string>& inCharVector, std::vector<int>& inInfoVector)
{ // return value false means there is no more data on the stack
    if (luaWrap_lua_gettop(L) < ind)
        return (false);                             // not enough data on the stack
    inInfoVector.push_back(sim_script_arg_invalid); // Dummy value for type
    inInfoVector.push_back(0);                      // dummy value for size
    if (luaWrap_lua_isnil(L, ind))
    { // Special case: nil should not generate a sim_script_arg_invalid type!
        inInfoVector[inInfoVector.size() - 2] = sim_script_arg_null;
        return (true);
    }
    if (dataType & sim_script_arg_table)
    { // we have to read a table:
        dataType ^= sim_script_arg_table;
        if (!luaWrap_lua_isnonbuffertable(L, ind))
            return (true); // this is not a table
        int dataSize = int(luaWrap_lua_rawlen(L, ind));
        std::vector<char> boolV;
        std::vector<int> intV;
        std::vector<float> floatV;
        std::vector<double> doubleV;
        std::vector<std::string> stringV;
        for (int i = 0; i < dataSize; i++)
        {
            luaWrap_lua_rawgeti(L, ind, i + 1);
            if (dataType == sim_script_arg_bool)
            {
                if (!luaWrap_lua_isboolean(L, -1))
                {
                    luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
                    return (true);         // we don't have the correct data type
                }
                boolV.push_back(luaToBool(L, -1));
            }
            else if (dataType == sim_script_arg_int32)
            {
                if (!luaWrap_lua_isnumber(L, -1))
                {
                    luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
                    return (true);         // we don't have the correct data type
                }
                intV.push_back(luaToInt(L, -1));
            }
            else if (dataType == sim_script_arg_float)
            {
                if (!luaWrap_lua_isnumber(L, -1))
                {
                    luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
                    return (true);         // we don't have the correct data type
                }
                floatV.push_back(luaToDouble(L, -1));
            }
            else if (dataType == sim_script_arg_double)
            {
                if (!luaWrap_lua_isnumber(L, -1))
                {
                    luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
                    return (true);         // we don't have the correct data type
                }
                doubleV.push_back(luaToDouble(L, -1));
            }
            else if (dataType == sim_script_arg_string)
            {
                if (!luaWrap_lua_isstring(L, -1))
                {
                    luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
                    return (true);         // we don't have the correct data type
                }
                stringV.push_back(std::string(luaWrap_lua_tostring(L, -1)));
            }
            else
            {
                luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
                return (true);         // data type not recognized!
            }
            luaWrap_lua_pop(L, 1); // we have to pop the value that was pushed with luaWrap_lua_rawgeti
        }
        // All values in the tables passed the check!
        for (int i = 0; i < int(boolV.size()); i++)
            inBoolVector.push_back(boolV[i]);
        for (int i = 0; i < int(intV.size()); i++)
            inIntVector.push_back(intV[i]);
        for (int i = 0; i < int(floatV.size()); i++)
            inFloatVector.push_back(floatV[i]);
        for (int i = 0; i < int(doubleV.size()); i++)
            inDoubleVector.push_back(doubleV[i]);
        for (int i = 0; i < int(stringV.size()); i++)
            inStringVector.push_back(stringV[i]);
        inInfoVector[inInfoVector.size() - 1] = dataSize;                        // Size of the table
        inInfoVector[inInfoVector.size() - 2] = dataType | sim_script_arg_table; // Type
        return (true);
    }
    else
    { // we have simple data here (not a table)
        if (dataType == sim_script_arg_bool)
        {
            if (!luaWrap_lua_isboolean(L, ind))
                return (true); // we don't have the correct data type
            inBoolVector.push_back(luaToBool(L, ind));
            inInfoVector[inInfoVector.size() - 2] = dataType;
        }
        else if (dataType == sim_script_arg_int32)
        {
            if (!luaWrap_lua_isnumber(L, ind))
                return (true); // we don't have the correct data type
            inIntVector.push_back(luaToInt(L, ind));
            inInfoVector[inInfoVector.size() - 2] = dataType;
        }
        else if (dataType == sim_script_arg_float)
        {
            if (!luaWrap_lua_isnumber(L, ind))
                return (true); // we don't have the correct data type
            inFloatVector.push_back(luaToDouble(L, ind));
            inInfoVector[inInfoVector.size() - 2] = dataType;
        }
        else if (dataType == sim_script_arg_double)
        {
            if (!luaWrap_lua_isnumber(L, ind))
                return (true); // we don't have the correct data type
            inDoubleVector.push_back(luaToDouble(L, ind));
            inInfoVector[inInfoVector.size() - 2] = dataType;
        }
        else if (dataType == sim_script_arg_string)
        {
            if (!luaWrap_lua_isstring(L, ind))
                return (true); // we don't have the correct data type
            inStringVector.push_back(std::string(luaWrap_lua_tostring(L, ind)));
            inInfoVector[inInfoVector.size() - 2] = dataType;
        }
        else if (dataType == sim_script_arg_charbuff)
        {
            if (!luaWrap_lua_isstring(L, ind))
                return (true); // we don't have the correct data type
            size_t dataLength;
            char* data = (char*)luaWrap_lua_tobuffer(L, ind, &dataLength);
            inCharVector.push_back(std::string(data, dataLength));
            inInfoVector[inInfoVector.size() - 2] = dataType;
            inInfoVector[inInfoVector.size() - 1] = int(dataLength);
        }
        return (true); // data type not recognized!
    }
    return (true);
}

void writeCustomFunctionDataOntoStack_old(luaWrap_lua_State* L, int dataType, int dataSize, bool* boolData,
                                          int& boolDataPos, int* intData, int& intDataPos, float* floatData,
                                          int& floatDataPos, double* doubleData, int& doubleDataPos, char* stringData,
                                          int& stringDataPos, char* charData, int& charDataPos)
{
    if (((dataType | sim_script_arg_table) - sim_script_arg_table) == sim_script_arg_charbuff)
    {                                                                      // special handling here
        luaWrap_lua_pushbinarystring(L, charData + charDataPos, dataSize); // push binary string for backw. comp.
        charDataPos += dataSize;
    }
    else
    {
        int newTablePos = 0;
        bool weHaveATable = false;
        if (dataType & sim_script_arg_table)
        { // we have a table
            luaWrap_lua_newtable(L);
            newTablePos = luaWrap_lua_gettop(L);
            dataType ^= sim_script_arg_table;
            weHaveATable = true;
        }
        else
            dataSize = 1;
        for (int i = 0; i < dataSize; i++)
        {
            if (dataType == sim_script_arg_bool)
                luaWrap_lua_pushboolean(L, boolData[boolDataPos++]);
            else if (dataType == sim_script_arg_int32)
                luaWrap_lua_pushinteger(L, intData[intDataPos++]);
            else if (dataType == sim_script_arg_float)
                luaWrap_lua_pushnumber(L, floatData[floatDataPos++]);
            else if (dataType == sim_script_arg_double)
                luaWrap_lua_pushnumber(L, doubleData[doubleDataPos++]);
            else if (dataType == sim_script_arg_string)
            {
                luaWrap_lua_pushtext(L, stringData + stringDataPos);
                stringDataPos += (int)strlen(stringData + stringDataPos) +
                                 1; // Thanks to Ulrich Schwesinger for noticing a bug here!
            }
            else
                luaWrap_lua_pushnil(L); // that is an error!

            if (weHaveATable) // that's when we have a table
                luaWrap_lua_rawseti(L, newTablePos, i + 1);
        }
    }
}
int _simResetMill(luaWrap_lua_State* L)
{ // DEPRECATED since V4.0.1. has no effect anymore
    LUA_START("sim.ResetMill");
    LUA_END(0);
}

int _simHandleMill(luaWrap_lua_State* L)
{ // DEPRECATED since V4.0.1. has no effect anymore
    LUA_START("sim.HandleMill");
    LUA_END(0);
}

int _simResetMilling(luaWrap_lua_State* L)
{ // DEPRECATED since V4.0.1. has no effect anymore
    LUA_START("sim.ResetMilling");
    LUA_END(0);
}

int _simOpenTextEditor(luaWrap_lua_State* L)
{ // DEPRECATED since V3.6.0
    TRACE_LUA_API;
    LUA_START("sim.openTextEditor");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        std::string initText(luaWrap_lua_tostring(L, 1));
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_string, 0, true, true, &errorString, argOffset);
        if (res >= 0)
        {
            std::string xml;
            const char* _xml = nullptr;
            if (res == 2)
            {
                xml = luaWrap_lua_tostring(L, 2);
                _xml = xml.c_str();
            }
            int res = checkOneGeneralInputArgument(L, 3, lua_arg_string, 0, true, true, &errorString, argOffset);
            if (res != 2)
            { // Modal dlg
                int various[4];
                char* outText = CALL_C_API(simOpenTextEditor, initText.c_str(), _xml, various);
                if (outText != nullptr)
                {
                    luaWrap_lua_pushtext(L, outText);
                    delete[] outText;
                    pushIntTableOntoStack(L, 2, various + 0);
                    pushIntTableOntoStack(L, 2, various + 2);
                    LUA_END(3);
                }
            }
            else
            { // non-modal dlg
                int handle = -1;
                CScriptObject* it =
                    App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
                if (it != nullptr)
                {
                    std::string callbackFunction(luaWrap_lua_tostring(L, 3));
#ifdef SIM_WITH_GUI
                    if (GuiApp::mainWindow != nullptr)
                        handle = GuiApp::mainWindow->codeEditorContainer->openTextEditor_old(
                            initText.c_str(), xml.c_str(), callbackFunction.c_str(), it);
#endif
                }
                luaWrap_lua_pushinteger(L, handle);
                LUA_END(1);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simCloseTextEditor(luaWrap_lua_State* L)
{ // DEPRECATED since V3.6.0
    TRACE_LUA_API;
    LUA_START("sim.closeTextEditor");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_bool, 0))
    {
        int h = luaToInt(L, 1);
        bool ignoreCb = luaToBool(L, 2);
        int res = 0;
        std::string txt;
        std::string cb;
        int posAndSize[4];
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
        {
            if (GuiApp::mainWindow->codeEditorContainer->close(h, posAndSize, &txt, &cb))
                res = 1;
        }
#endif
        if ((res > 0) && (!ignoreCb))
        { // We call the callback directly from here:
            CScriptObject* it =
                App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
            if (it != nullptr)
            {
                CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
                stack->pushTextOntoStack(txt.c_str());
                stack->pushInt32ArrayOntoStack(posAndSize + 0, 2);
                stack->pushInt32ArrayOntoStack(posAndSize + 2, 2);
                it->callCustomScriptFunction(cb.c_str(), stack);
                App::worldContainer->interfaceStackContainer->destroyStack(stack);
            }
        }
        luaWrap_lua_pushinteger(L, res);
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simHandleVarious(luaWrap_lua_State* L)
{ // DEPRECATED since V3.4.0
    TRACE_LUA_API;
    LUA_START("simHandleVarious");

    int retVal = CALL_C_API(simHandleVarious, );

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetMpConfigForTipPose(luaWrap_lua_State* L)
{ // DEPRECATED since V3.3.0
    TRACE_LUA_API;
    LUA_START("simGetMpConfigForTipPose");
    errorString = "not supported anymore.";
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, -1);
    LUA_END(1);
}

int _simResetPath(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simResetPath");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simResetPath, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simResetJoint(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simResetJoint");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simResetJoint, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simResetTracing(luaWrap_lua_State* L)
{ // deprecated
    TRACE_LUA_API;
    LUA_START("simResetTracing");

    warningString = SIM_ERROR_FUNCTION_DEPRECATED_AND_HAS_NO_EFFECT;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simHandleTracing(luaWrap_lua_State* L)
{ // deprecated
    TRACE_LUA_API;
    LUA_START("simHandleTracing");

    warningString = SIM_ERROR_FUNCTION_DEPRECATED_AND_HAS_NO_EFFECT;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

static int _nextMemHandle_old = 0;
static std::vector<int> _memHandles_old;
static std::vector<void*> _memBuffers_old;

typedef struct
{
    double accel;
    double vdl;
    double currentPos;
    double lastTime;
    double maxVelocity;
    double currentVel;
    C7Vector startTr;
    C7Vector targetTr;
    int objID;
    CSceneObject* object;
    int relativeToObjID;
    CSceneObject* relToObject;
    unsigned char posAndOrient;
} simMoveToPosData_old;

typedef struct
{
    double lastTime;
    bool sameTimeFinish;
    int maxVirtualDistIndex;
    double maxVelocity;
    double accel;
    std::vector<int> jointHandles;
    std::vector<double> jointCurrentVirtualPositions;
    std::vector<double> jointCurrentVirtualVelocities;
    std::vector<double> jointStartPositions;
    std::vector<double> jointTargetPositions;
    std::vector<double> jointVirtualDistances;
    std::vector<double> jointMaxVelocities;
    std::vector<double> jointAccels;
} simMoveToJointPosData_old;

int _sim_moveToPos_1(luaWrap_lua_State* L)
{ // for backward compatibility with simMoveToPosition on old threaded scripts
    TRACE_LUA_API;
    LUA_START("sim._moveToPos_1");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    { // Those are the arguments that are always required! (the rest can be ignored or set to nil!
        int objID = luaToInt(L, 1);
        int relativeToObjID = luaToInt(L, 2);
        double posTarget[3];
        double eulerTarget[3];
        double maxVelocity;
        CSceneObject* object = App::currentWorld->sceneObjects->getObjectFromHandle(objID);
        CSceneObject* relToObject = nullptr;
        double accel = 0.0;                                    // means infinite accel!! (default value)
        double angleToLinearCoeff = 0.1 / (90.0 * degToRad);   // (default value)
        int distCalcMethod = sim_distcalcmethod_dl_if_nonzero; // (default value)
        bool foundError = false;
        if ((!foundError) && (object == nullptr))
        {
            errorString = SIM_ERROR_OBJECT_INEXISTANT;
            foundError = true;
        }
        if ((!foundError) && (relativeToObjID == sim_handle_parent))
        {
            relativeToObjID = -1;
            CSceneObject* parent = object->getParent();
            if (parent != nullptr)
                relativeToObjID = parent->getObjectHandle();
        }
        if ((!foundError) && (relativeToObjID != -1))
        {
            relToObject = App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjID);
            if (relToObject == nullptr)
            { // error, object doesn't exist!
                errorString = SIM_ERROR_OBJECT_INEXISTANT;
                foundError = true;
            }
        }

        // Now check the optional arguments:
        int res;
        unsigned char posAndOrient = 0;
        if (!foundError) // position argument:
        {
            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 3, true, true, &errorString, argOffset);
            if (res == 2)
            { // get the data
                getDoublesFromTable(L, 3, 3, posTarget);
                posAndOrient |= 1;
            }
            foundError = (res == -1);
        }
        if (!foundError) // orientation argument:
        {
            res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 3, true, true, &errorString, argOffset);
            if (res == 2)
            { // get the data
                getDoublesFromTable(L, 4, 3, eulerTarget);
                posAndOrient |= 2;
            }
            foundError = (res == -1);
        }
        if ((!foundError) && (posAndOrient == 0))
        {
            foundError = true;
            errorString = "target position and/or target orientation has to be specified.";
        }
        if (!foundError) // target velocity argument:
        {
            res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, false, false, &errorString, argOffset);
            if (res == 2)
            { // get the data
                maxVelocity = luaToDouble(L, 5);
            }
            else
                foundError = true;
        }
        if (!foundError) // Accel argument:
        {
            res = checkOneGeneralInputArgument(L, 6, lua_arg_number, 0, true, true, &errorString, argOffset);
            if (res == 2)
            { // get the data
                accel = fabs(luaToDouble(L, 6));
            }
            foundError = (res == -1);
        }
        if (!foundError) // distance method:
        {
            res = checkOneGeneralInputArgument(L, 7, lua_arg_number, 2, true, true, &errorString, argOffset);
            if (res == 2)
            { // get the data
                double tmpF[2];
                getDoublesFromTable(L, 7, 2, tmpF);
                angleToLinearCoeff = tmpF[1];
                getIntsFromTable(L, 7, 1, &distCalcMethod);
            }
            foundError = (res == -1);
        }
        if (!foundError)
        { // do the job here!
            C7Vector startTr(object->getCumulativeTransformation());
            C7Vector relTr;
            relTr.setIdentity();
            if (relToObject != nullptr)
                relTr = relToObject->getFullCumulativeTransformation();
            startTr = relTr.getInverse() * startTr;

            C7Vector targetTr(startTr);
            if (posAndOrient & 1)
                targetTr.X.setData(posTarget);
            if (posAndOrient & 2)
                targetTr.Q.setEulerAngles(eulerTarget[0], eulerTarget[1], eulerTarget[2]);
            double currentVel = 0.0;

            double dl = (targetTr.X - startTr.X).getLength();
            double da = targetTr.Q.getAngleBetweenQuaternions(startTr.Q) * angleToLinearCoeff;
            double vdl = dl;
            if (distCalcMethod == sim_distcalcmethod_dl)
                vdl = dl;
            if (distCalcMethod == sim_distcalcmethod_dac)
                vdl = da;
            if (distCalcMethod == sim_distcalcmethod_max_dl_dac)
                vdl = std::max<double>(dl, da);
            if (distCalcMethod == sim_distcalcmethod_dl_and_dac)
                vdl = dl + da;
            if (distCalcMethod == sim_distcalcmethod_sqrt_dl2_and_dac2)
                vdl = sqrt(dl * dl + da * da);
            if (distCalcMethod == sim_distcalcmethod_dl_if_nonzero)
            {
                vdl = dl;
                if (dl < 0.00005) // Was dl==0.0 before (tolerance problem). Changed on 1/4/2011
                    vdl = da;
            }
            if (distCalcMethod == sim_distcalcmethod_dac_if_nonzero)
            {
                vdl = da;
                if (da < 0.01 * degToRad) // Was da==0.0 before (tolerance problem). Changed on 1/4/2011
                    vdl = dl;
            }
            // vld is the totalvirtual distance
            double currentPos = 0.0;
            bool movementFinished = false;

            if (vdl == 0.0)
            { // if the path length is 0 (the two positions might still be not-coincident, depending on the calculation
                // method!)
                if (App::currentWorld->sceneObjects->getObjectFromHandle(objID) ==
                    object) // make sure the object is still valid (running in a thread)
                {
                    if (relToObject == nullptr)
                    { // absolute
                        C7Vector parentInv(object->getFullParentCumulativeTransformation().getInverse());
                        object->setLocalTransformation(parentInv * targetTr);
                    }
                    else
                    { // relative to a specific object (2009/11/17)
                        if (App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjID) ==
                            relToObject) // make sure the object is still valid (running in a thread)
                        {                // ok
                            C7Vector relToTr(relToObject->getFullCumulativeTransformation());
                            targetTr = relToTr * targetTr;
                            C7Vector parentInv(object->getFullParentCumulativeTransformation().getInverse());
                            object->setLocalTransformation(parentInv * targetTr);
                        }
                    }
                }
                movementFinished = true;
            }

            if (movementFinished)
                luaWrap_lua_pushinteger(L, -1);
            else
            {
                _memHandles_old.push_back(_nextMemHandle_old);
                simMoveToPosData_old* mem = new simMoveToPosData_old();
                mem->accel = accel;
                mem->vdl = vdl;
                mem->currentPos = currentPos;
                mem->lastTime = App::currentWorld->simulation->getSimulationTime();
                mem->maxVelocity = maxVelocity;
                mem->currentVel = currentVel;
                mem->startTr = startTr;
                mem->targetTr = targetTr;
                mem->objID = objID;
                mem->object = object;
                mem->relativeToObjID = relativeToObjID;
                mem->relToObject = relToObject;
                mem->posAndOrient = posAndOrient;

                _memBuffers_old.push_back(mem);
                luaWrap_lua_pushinteger(L, _nextMemHandle_old);
                _nextMemHandle_old++;
            }
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED();
    LUA_END(0);
}

int _sim_moveToPos_2(luaWrap_lua_State* L)
{ // for backward compatibility with simMoveToPosition on old threaded scripts
    TRACE_LUA_API;
    LUA_START("sim._moveToPos_2");
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_integer, 0))
    {
        int h = luaToInt(L, 1);
        simMoveToPosData_old* mem = nullptr;
        for (size_t i = 0; i < _memHandles_old.size(); i++)
        {
            if (_memHandles_old[i] == h)
            {
                mem = (simMoveToPosData_old*)_memBuffers_old[i];
                break;
            }
        }
        if (mem != nullptr)
        {
            bool err = false;
            bool movementFinished = false;
            double currentTime =
                App::currentWorld->simulation->getSimulationTime() + App::currentWorld->simulation->getTimeStep();
            double dt = currentTime - mem->lastTime;
            mem->lastTime = currentTime;

            if (mem->accel == 0.0)
            { // Means infinite acceleration
                double timeNeeded = (mem->vdl - mem->currentPos) / mem->maxVelocity;
                mem->currentVel = mem->maxVelocity;
                if (timeNeeded > dt)
                {
                    mem->currentPos += dt * mem->maxVelocity;
                    dt = 0.0; // this is what is left
                }
                else
                {
                    mem->currentPos = mem->vdl;
                    if (timeNeeded >= 0.0)
                        dt -= timeNeeded;
                }
            }
            else
            {
                double p = mem->currentPos;
                double v = mem->currentVel;
                double t = dt;
                CLinMotionRoutines::getNextValues(p, v, mem->maxVelocity, mem->accel, 0.0, mem->vdl, 0.0, 0.0, t);
                mem->currentPos = double(p);
                mem->currentVel = double(v);
                dt = double(t);
            }

            // Now check if we are within tolerances:
            if (fabs(mem->currentPos - mem->vdl) <= 0.00001) // tol[0])
                movementFinished = true;

            // Set the new configuration of the object:
            double ll = mem->currentPos / mem->vdl;
            if (ll > 1.0)
                ll = 1.0;
            C7Vector newAbs;
            newAbs.buildInterpolation(mem->startTr, mem->targetTr, ll);
            if (App::currentWorld->sceneObjects->getObjectFromHandle(mem->objID) ==
                mem->object) // make sure the object is still valid (running in a thread)
            {
                if ((mem->relToObject != nullptr) &&
                    (App::currentWorld->sceneObjects->getObjectFromHandle(mem->relativeToObjID) != mem->relToObject))
                    movementFinished = true; // the object was destroyed during execution of the command!
                else
                {
                    C7Vector parentInv(mem->object->getFullParentCumulativeTransformation().getInverse());
                    C7Vector currAbs(mem->object->getCumulativeTransformation());
                    C7Vector relToTr;
                    relToTr.setIdentity();
                    if (mem->relToObject != nullptr)
                        relToTr = mem->relToObject->getFullCumulativeTransformation();
                    currAbs = relToTr.getInverse() * currAbs;
                    if ((mem->posAndOrient & 1) == 0)
                        newAbs.X = currAbs.X;
                    if ((mem->posAndOrient & 2) == 0)
                        newAbs.Q = currAbs.Q;
                    newAbs = relToTr * newAbs;
                    mem->object->setLocalTransformation(parentInv * newAbs);
                }
            }
            else
                movementFinished = true; // the object was destroyed during execution of the command!

            if (!movementFinished)
            {
                luaWrap_lua_pushinteger(L, 0); // mov. not yet finished
                LUA_END(1);
            }
            if (!err)
            {
                luaWrap_lua_pushinteger(L, 1); // mov. finished
                luaWrap_lua_pushnumber(L, dt); // success (deltaTime left)
                LUA_END(2);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED();
    LUA_END(0);
}

int _sim_del(luaWrap_lua_State* L)
{ // for backward compatibility with simMoveToPosition on old threaded scripts
    TRACE_LUA_API;
    LUA_START("sim._del");
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_integer, 0))
    {
        int h = luaToInt(L, 1);
        for (size_t i = 0; i < _memHandles_old.size(); i++)
        {
            if (_memHandles_old[i] == h)
            {
                delete _memBuffers_old[i];
                _memHandles_old.erase(_memHandles_old.begin() + i);
                _memBuffers_old.erase(_memBuffers_old.begin() + i);
                break;
            }
        }
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED();
    LUA_END(0);
}

int _sim_moveToJointPos_1(luaWrap_lua_State* L)
{ // for backward compatibility with simMoveToJointPositions on old threaded scripts
    TRACE_LUA_API;
    LUA_START("sim._moveToJointPos_1");
    if (!((!luaWrap_lua_isnonbuffertable(L, 1)) || (!luaWrap_lua_isnonbuffertable(L, 2)) ||
          (luaWrap_lua_rawlen(L, 1) > luaWrap_lua_rawlen(L, 2)) || (luaWrap_lua_rawlen(L, 1) == 0)))
    { // Ok we have 2 tables with same sizes.
        int tableLen = (int)luaWrap_lua_rawlen(L, 1);
        bool sameTimeFinish = true;
        double maxVelocity = 0.0;
        double accel = 0.0; // means infinite accel!! (default value)
        bool accelTablePresent = false;
        double angleToLinearCoeff = 1.0;
        bool foundError = false;
        // Now check the other arguments:
        int res;
        if (luaWrap_lua_isnonbuffertable(L, 3))
            sameTimeFinish = false; // we do not finish at the same time!
        if (!foundError)            // velocity or velocities argument (not optional!):
        {
            if (sameTimeFinish)
            {
                res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, false, false, &errorString, argOffset);
                if (res == 2)
                    maxVelocity = luaToDouble(L, 3);
                else
                    foundError = true;
            }
            else
            {
                res = checkOneGeneralInputArgument(L, 3, lua_arg_number, tableLen, false, false, &errorString, argOffset);
                if (res != 2)
                    foundError = true;
            }
        }
        if (!foundError) // Accel argument:
        {
            if (sameTimeFinish)
            {
                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, true, &errorString, argOffset);
                if (res == 2)
                { // get the data
                    accel = fabs(luaToDouble(L, 4));
                }
                foundError = (res == -1);
            }
            else
            {
                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, tableLen, true, true, &errorString, argOffset);
                if (res == 2)
                    accelTablePresent = true;
                foundError = (res == -1);
            }
        }
        if (!foundError) // angleToLinearCoeff argument:
        {
            if (sameTimeFinish)
            {
                res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, true, true, &errorString, argOffset);
                if (res == 2)
                { // get the data
                    angleToLinearCoeff = fabs(luaToDouble(L, 5));
                }
                foundError = (res == -1);
            }
            else
                angleToLinearCoeff = 1.0; // no conversion!
        }
        if (!foundError)
        { // do the job here!
            std::vector<int> jointHandles;
            jointHandles.resize(tableLen);
            std::vector<double> jointCurrentVirtualPositions;
            jointCurrentVirtualPositions.resize(tableLen);
            std::vector<double> jointCurrentVirtualVelocities;
            jointCurrentVirtualVelocities.resize(tableLen);
            std::vector<double> jointStartPositions;
            jointStartPositions.resize(tableLen);
            std::vector<double> jointTargetPositions;
            jointTargetPositions.resize(tableLen);
            std::vector<double> jointVirtualDistances;
            jointVirtualDistances.resize(tableLen);
            std::vector<double> jointMaxVelocities;
            jointMaxVelocities.resize(tableLen);
            std::vector<double> jointAccels;
            jointAccels.resize(tableLen);

            getIntsFromTable(L, 1, tableLen, &jointHandles[0]);
            getDoublesFromTable(L, 2, tableLen, &jointTargetPositions[0]);
            if (!sameTimeFinish)
            {
                getDoublesFromTable(L, 3, tableLen, &jointMaxVelocities[0]);
                for (int i = 0; i < tableLen; i++)
                    jointMaxVelocities[i] = fabs(jointMaxVelocities[i]);
                if (accelTablePresent)
                {
                    getDoublesFromTable(L, 4, tableLen, &jointAccels[0]);
                    for (int i = 0; i < tableLen; i++)
                        jointAccels[i] = fabs(jointAccels[i]);
                }
            }
            double maxVirtualDist = 0.0;
            int maxVirtualDistIndex = 0;
            for (int i = 0; i < tableLen; i++)
            {
                jointCurrentVirtualPositions[i] = 0.0;
                jointCurrentVirtualVelocities[i] = 0.0;
                if (sameTimeFinish)
                    jointMaxVelocities[i] = maxVelocity;
                if (!accelTablePresent)
                    jointAccels[i] = accel;

                CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(jointHandles[i]);
                if ((it != nullptr) && (it->getJointType() != sim_joint_spherical))
                { // make sure target is within allowed range, and check the maximum virtual distance:
                    jointStartPositions[i] = it->getPosition();
                    double minP, maxP;
                    it->getInterval(minP, maxP);
                    if (it->getIsCyclic())
                    {
                        double da = tt::getAngleMinusAlpha(jointTargetPositions[i], jointStartPositions[i]);
                        jointTargetPositions[i] = jointStartPositions[i] + da;
                    }
                    else
                    {
                        if (minP > jointTargetPositions[i])
                            jointTargetPositions[i] = minP;
                        if (maxP < jointTargetPositions[i])
                            jointTargetPositions[i] = maxP;
                    }
                    double d = fabs(jointTargetPositions[i] - jointStartPositions[i]);
                    if (it->getJointType() == sim_joint_revolute)
                        d *= angleToLinearCoeff;
                    jointVirtualDistances[i] = d;
                    if (d > maxVirtualDist)
                    {
                        maxVirtualDist = d;
                        maxVirtualDistIndex = i;
                    }
                }
                else
                {
                    // Following are default values in case the joint doesn't exist or is spherical:
                    jointStartPositions[i] = 0.0;
                    jointTargetPositions[i] = 0.0;
                    jointVirtualDistances[i] = 0.0;
                }
            }
            double lastTime = App::currentWorld->simulation->getSimulationTime();
            bool movementFinished = false;
            double dt = App::currentWorld->simulation->getTimeStep(); // this is the time left if we leave here

            if (maxVirtualDist == 0.0)
                luaWrap_lua_pushinteger(L, -1);
            else
            {
                _memHandles_old.push_back(_nextMemHandle_old);
                simMoveToJointPosData_old* mem = new simMoveToJointPosData_old();
                mem->lastTime = lastTime;
                mem->sameTimeFinish = sameTimeFinish;
                mem->maxVirtualDistIndex = maxVirtualDistIndex;
                mem->maxVelocity = maxVelocity;
                mem->accel = accel;
                mem->jointHandles.assign(jointHandles.begin(), jointHandles.end());
                mem->jointCurrentVirtualPositions.assign(jointCurrentVirtualPositions.begin(),
                                                         jointCurrentVirtualPositions.end());
                mem->jointCurrentVirtualVelocities.assign(jointCurrentVirtualVelocities.begin(),
                                                          jointCurrentVirtualVelocities.end());
                mem->jointStartPositions.assign(jointStartPositions.begin(), jointStartPositions.end());
                mem->jointTargetPositions.assign(jointTargetPositions.begin(), jointTargetPositions.end());
                mem->jointVirtualDistances.assign(jointVirtualDistances.begin(), jointVirtualDistances.end());
                mem->jointMaxVelocities.assign(jointMaxVelocities.begin(), jointMaxVelocities.end());
                mem->jointAccels.assign(jointAccels.begin(), jointAccels.end());

                _memBuffers_old.push_back(mem);
                luaWrap_lua_pushinteger(L, _nextMemHandle_old);
                _nextMemHandle_old++;
            }
            LUA_END(1);
        }
    }
    else
        errorString = "one of the function's argument type is not correct or table sizes are invalid or do not match";

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _sim_moveToJointPos_2(luaWrap_lua_State* L)
{ // for backward compatibility with simMoveToJointPositions on old threaded scripts
    TRACE_LUA_API;
    LUA_START("sim._moveToJointPos_2");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_integer, 0))
    {
        int h = luaToInt(L, 1);
        simMoveToJointPosData_old* mem = nullptr;
        for (size_t i = 0; i < _memHandles_old.size(); i++)
        {
            if (_memHandles_old[i] == h)
            {
                mem = (simMoveToJointPosData_old*)_memBuffers_old[i];
                break;
            }
        }
        if (mem != nullptr)
        {
            int tableLen = int(mem->jointHandles.size());
            bool err = false;
            bool movementFinished = false;
            double currentTime =
                App::currentWorld->simulation->getSimulationTime() + App::currentWorld->simulation->getTimeStep();
            double dt = currentTime - mem->lastTime;
            double minTimeLeft = dt;
            mem->lastTime = currentTime;
            if (mem->sameTimeFinish)
            {
                double timeLeftLocal = dt;
                // 1. handle the joint with longest distance first:
                // Does the main joint still exist?
                if (App::currentWorld->sceneObjects->getJointFromHandle(mem->jointHandles[mem->maxVirtualDistIndex]) !=
                    nullptr)
                {
                    if (mem->accel == 0.0)
                    { // means infinite accel
                        double timeNeeded = (mem->jointVirtualDistances[mem->maxVirtualDistIndex] -
                                             mem->jointCurrentVirtualPositions[mem->maxVirtualDistIndex]) /
                                            mem->maxVelocity;
                        mem->jointCurrentVirtualVelocities[mem->maxVirtualDistIndex] = mem->maxVelocity;
                        if (timeNeeded > timeLeftLocal)
                        {
                            mem->jointCurrentVirtualPositions[mem->maxVirtualDistIndex] +=
                                timeLeftLocal * mem->maxVelocity;
                            timeLeftLocal = 0.0; // this is what is left
                        }
                        else
                        {
                            mem->jointCurrentVirtualPositions[mem->maxVirtualDistIndex] =
                                mem->jointVirtualDistances[mem->maxVirtualDistIndex];
                            if (timeNeeded >= 0.0)
                                timeLeftLocal -= timeNeeded;
                        }
                    }
                    else
                    {
                        double p = mem->jointCurrentVirtualPositions[mem->maxVirtualDistIndex];
                        double v = mem->jointCurrentVirtualVelocities[mem->maxVirtualDistIndex];
                        double t = timeLeftLocal;
                        CLinMotionRoutines::getNextValues(p, v, mem->maxVelocity, mem->accel, 0.0,
                                                          mem->jointVirtualDistances[mem->maxVirtualDistIndex], 0.0,
                                                          0.0, t);
                        mem->jointCurrentVirtualPositions[mem->maxVirtualDistIndex] = double(p);
                        mem->jointCurrentVirtualVelocities[mem->maxVirtualDistIndex] = double(v);
                        timeLeftLocal = double(t);
                    }
                    minTimeLeft = timeLeftLocal;
                    // 2. We adjust the other joints accordingly:
                    double f = 1;
                    if (mem->jointVirtualDistances[mem->maxVirtualDistIndex] != 0.0)
                        f = mem->jointCurrentVirtualPositions[mem->maxVirtualDistIndex] /
                            mem->jointVirtualDistances[mem->maxVirtualDistIndex];
                    for (int i = 0; i < tableLen; i++)
                    {
                        if (i != mem->maxVirtualDistIndex)
                            mem->jointCurrentVirtualPositions[i] = mem->jointVirtualDistances[i] * f;
                    }
                    // 3. Check if within tolerance:
                    if (fabs(mem->jointCurrentVirtualPositions[mem->maxVirtualDistIndex] -
                             mem->jointVirtualDistances[mem->maxVirtualDistIndex]) <= 0.00001)
                        movementFinished = true;
                }
                else
                { // the main joint was removed. End here!
                    movementFinished = true;
                }
            }
            else
            {
                bool withinTolerance = true;
                for (int i = 0; i < tableLen; i++)
                {
                    if (App::currentWorld->sceneObjects->getJointFromHandle(mem->jointHandles[i]) != nullptr)
                    {
                        // Check if within tolerance (before):
                        if (fabs(mem->jointCurrentVirtualPositions[i] - mem->jointVirtualDistances[i]) > 0.00001)
                        {
                            double timeLeftLocal = dt;
                            if (mem->jointAccels[i] == 0.0)
                            { // means infinite accel
                                double timeNeeded =
                                    (mem->jointVirtualDistances[i] - mem->jointCurrentVirtualPositions[i]) /
                                    mem->jointMaxVelocities[i];
                                mem->jointCurrentVirtualVelocities[i] = mem->jointMaxVelocities[i];
                                if (timeNeeded > timeLeftLocal)
                                {
                                    mem->jointCurrentVirtualPositions[i] += timeLeftLocal * mem->jointMaxVelocities[i];
                                    timeLeftLocal = 0.0; // this is what is left
                                }
                                else
                                {
                                    mem->jointCurrentVirtualPositions[i] = mem->jointVirtualDistances[i];
                                    if (timeNeeded >= 0.0)
                                        timeLeftLocal -= timeNeeded;
                                }
                            }
                            else
                            {
                                double p = mem->jointCurrentVirtualPositions[i];
                                double v = mem->jointCurrentVirtualVelocities[i];
                                double t = timeLeftLocal;
                                CLinMotionRoutines::getNextValues(p, v, mem->jointMaxVelocities[i], mem->jointAccels[i],
                                                                  0.0, mem->jointVirtualDistances[i], 0.0, 0.0, t);
                                mem->jointCurrentVirtualPositions[i] = double(p);
                                mem->jointCurrentVirtualVelocities[i] = double(v);
                                timeLeftLocal = double(t);
                            }
                            if (timeLeftLocal < minTimeLeft)
                                minTimeLeft = timeLeftLocal;
                            // Check if within tolerance (after):
                            if (fabs(mem->jointCurrentVirtualPositions[i] - mem->jointVirtualDistances[i]) > 0.00001)
                                withinTolerance = false;
                        }
                    }
                }
                if (withinTolerance)
                    movementFinished = true;
            }
            dt = minTimeLeft;

            // We set all joint positions:
            for (int i = 0; i < tableLen; i++)
            {
                CJoint* joint = App::currentWorld->sceneObjects->getJointFromHandle(mem->jointHandles[i]);
                if ((joint != nullptr) && (joint->getJointType() != sim_joint_spherical) &&
                    (mem->jointVirtualDistances[i] != 0.0))
                {
                    joint->setTargetPosition(mem->jointStartPositions[i] +
                                             (mem->jointTargetPositions[i] - mem->jointStartPositions[i]) *
                                                 mem->jointCurrentVirtualPositions[i] / mem->jointVirtualDistances[i]);
                    if (joint->getJointMode() == sim_jointmode_kinematic)
                        joint->setPosition(mem->jointStartPositions[i] +
                                           (mem->jointTargetPositions[i] - mem->jointStartPositions[i]) *
                                               mem->jointCurrentVirtualPositions[i] / mem->jointVirtualDistances[i]);
                }
            }

            if (!movementFinished)
            {
                luaWrap_lua_pushinteger(L, 0); // mov. not yet finished
                LUA_END(1);
            }
            if (!err)
            {
                luaWrap_lua_pushinteger(L, 1); // mov. finished
                luaWrap_lua_pushnumber(L, dt); // success (deltaTime left)
                LUA_END(2);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetInstanceIndex(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("");

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, 0);
    LUA_END(1);
}

int _simGetVisibleInstanceIndex(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("");

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, 0);
    LUA_END(1);
}

int _simGetSystemTimeInMilliseconds(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simGetSystemTimeInMilliseconds");

    int res = checkOneGeneralInputArgument(L, 1, lua_arg_number, 0, true, false, &errorString, argOffset);
    if (res == 0)
    {
        luaWrap_lua_pushinteger(L, VDateTime::getTimeInMs());
        LUA_END(1);
    }
    if (res == 2)
    {
        int lastTime = luaToInt(L, 1);
        luaWrap_lua_pushinteger(L, VDateTime::getTimeDiffInMs(lastTime));
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simLockInterface(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simLockInterface");

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, -1);
    LUA_END(1);
}

int _simJointGetForce(luaWrap_lua_State* L)
{ // DEPRECATED since release 3.1.2
    TRACE_LUA_API;
    LUA_START("simJointGetForce");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        double jointF[1];
        if (CALL_C_API(simGetJointForce, luaToInt(L, 1), jointF) > 0)
        {
            luaWrap_lua_pushnumber(L, jointF[0]);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simCopyPasteSelectedObjects(luaWrap_lua_State* L)
{ // DEPRECATED since release 3.1.3
    TRACE_LUA_API;
    LUA_START("simCopyPasteSelectedObjects");

    int retVal = CALL_C_API(simCopyPasteSelectedObjects, );

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simFindIkPath(luaWrap_lua_State* L)
{ // DEPRECATED since 3.3.0
    TRACE_LUA_API;
    LUA_START("simFindIkPath");
    errorString = "not supported anymore.";
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetPathPlanningHandle(luaWrap_lua_State* L)
{ // DEPRECATED since 3.3.0
    TRACE_LUA_API;
    LUA_START("simGetPathPlanningHandle");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        std::string name(luaWrap_lua_tostring(L, 1));
        setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                                   CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                       L)); // for transmitting to the master function additional info (e.g.for autom.
                                            // name adjustment, or for autom. object deletion when script ends)
        retVal = CALL_C_API(simGetPathPlanningHandle, name.c_str());
        setCurrentScriptInfo_cSide(-1, -1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSearchPath(luaWrap_lua_State* L)
{ // DEPRECATED since 3.3.0
    TRACE_LUA_API;
    LUA_START("simSearchPath");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int pathPlanningObjectHandle = luaToInt(L, 1);
        double maximumSearchTime = tt::getLimitedFloat(0.001, 36000.0, luaToDouble(L, 2));
        double subDt = 0.05; // 50 ms at a time (default)
        bool foundError = false;
        // Now check the optional argument:
        int res;
        if (!foundError) // sub-dt argument:
        {
            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, true, &errorString, argOffset);
            if (res == 2)
            { // get the data
                subDt = tt::getLimitedFloat(0.001, std::min<double>(1.0, maximumSearchTime), luaToDouble(L, 3));
            }
            foundError = (res == -1);
        }
        if (!foundError)
        {
            CPathPlanningTask* it = App::currentWorld->pathPlanning_old->getObject(pathPlanningObjectHandle);
            if (it == nullptr)
                errorString = SIM_ERROR_PATH_PLANNING_OBJECT_INEXISTANT;
            else
            {
                retVal = 0;
                // if (VThread::isSimThread())
                { // non-threaded
                    if (it->performSearch(false, maximumSearchTime))
                        retVal = 1;
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simInitializePathSearch(luaWrap_lua_State* L)
{ // DEPRECATED since 3.3.0
    TRACE_LUA_API;
    LUA_START("simInitializePathSearch");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        int pathPlanningObjectHandle = luaToInt(L, 1);
        double maximumSearchTime = luaToDouble(L, 2);
        double searchTimeStep = luaToDouble(L, 3);
        retVal = CALL_C_API(simInitializePathSearch, pathPlanningObjectHandle, maximumSearchTime, searchTimeStep);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simPerformPathSearchStep(luaWrap_lua_State* L)
{ // DEPRECATED since 3.3.0
    TRACE_LUA_API;
    LUA_START("simPerformPathSearchStep");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_bool, 0))
    {
        int temporaryPathSearchObjectHandle = luaToInt(L, 1);
        bool abortSearch = luaToBool(L, 2);
        retVal = CALL_C_API(simPerformPathSearchStep, temporaryPathSearchObjectHandle, abortSearch);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simEnableWorkThreads(luaWrap_lua_State* L)
{ // DEPRECATED since 3/4/2016
    TRACE_LUA_API;
    LUA_START("simEnableWorkThreads");
    luaWrap_lua_pushinteger(L, 0);
    LUA_END(1);
}

int _simWaitForWorkThreads(luaWrap_lua_State* L)
{ // DEPRECATED since 3/4/2016
    TRACE_LUA_API;
    LUA_START("simWaitForWorkThreads");
    LUA_END(0);
}

int _simGetInvertedMatrix(luaWrap_lua_State* L)
{ // DEPRECATED since 10/5/2016
    TRACE_LUA_API;
    LUA_START("simGetInvertedMatrix");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 12))
    {
        double arr[12];
        getDoublesFromTable(L, 1, 12, arr);
        CALL_C_API(simInvertMatrix, arr);
        pushDoubleTableOntoStack(L, 12, arr);
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simAddSceneCustomData(luaWrap_lua_State* L)
{ // DEPRECATED since 26/12/2016
    TRACE_LUA_API;
    LUA_START("simAddSceneCustomData");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0))
    {
        int headerNumber = luaToInt(L, 1);
        size_t dataLength;
        char* data = (char*)luaWrap_lua_tobuffer(L, 2, &dataLength);
        retVal = CALL_C_API(simAddSceneCustomData, headerNumber, data, (int)dataLength);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetSceneCustomData(luaWrap_lua_State* L)
{ // DEPRECATED since 26/12/2016
    TRACE_LUA_API;
    LUA_START("simGetSceneCustomData");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int headerNumber = luaToInt(L, 1);
        int dataLength = CALL_C_API(simGetSceneCustomDataLength, headerNumber);
        if (dataLength >= 0)
        {
            char* data = new char[dataLength];
            int retVal = CALL_C_API(simGetSceneCustomData, headerNumber, data);
            if (retVal == -1)
                delete[] data;
            else
            {
                luaWrap_lua_pushbinarystring(L, data, dataLength); // push binary string for backw. comp.
                delete[] data;
                LUA_END(1);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simAddObjectCustomData(luaWrap_lua_State* L)
{ // DEPRECATED since 26/12/2016
    TRACE_LUA_API;
    LUA_START("simAddObjectCustomData");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_string, 0))
    {
        int objectHandle = luaToInt(L, 1);
        int headerNumber = luaToInt(L, 2);
        size_t dataLength;
        char* data = (char*)luaWrap_lua_tobuffer(L, 3, &dataLength);
        retVal = CALL_C_API(simAddObjectCustomData, objectHandle, headerNumber, data, (int)dataLength);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectCustomData(luaWrap_lua_State* L)
{ // DEPRECATED since 26/12/2016
    TRACE_LUA_API;
    LUA_START("simGetObjectCustomData");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int objectHandle = luaToInt(L, 1);
        int headerNumber = luaToInt(L, 2);
        int dataLength = CALL_C_API(simGetObjectCustomDataLength, objectHandle, headerNumber);
        if (dataLength >= 0)
        {
            char* data = new char[dataLength];
            int retVal = CALL_C_API(simGetObjectCustomData, objectHandle, headerNumber, data);
            if (retVal == -1)
                delete[] data;
            else
            {
                luaWrap_lua_pushbinarystring(L, data, dataLength); // push binary string for backw. comp.
                delete[] data;
                LUA_END(1);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetUIHandle(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simGetUIHandle");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        std::string name(luaWrap_lua_tostring(L, 1));
        setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                                   CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                       L)); // for transmitting to the master function additional info (e.g.for autom.
                                            // name adjustment, or for autom. object deletion when script ends)
        retVal = CALL_C_API(simGetUIHandle, name.c_str());
        setCurrentScriptInfo_cSide(-1, -1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetUIProperty(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simGetUIProperty");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simGetUIProperty, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetUIEventButton(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simGetUIEventButton");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int auxVals[2];
        retVal = CALL_C_API(simGetUIEventButton, luaToInt(L, 1), auxVals);
        luaWrap_lua_pushinteger(L, retVal);
        if (retVal == -1)
        {
            LUA_END(1);
        }
        pushIntTableOntoStack(L, 2, auxVals);
        LUA_END(2);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetUIProperty(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simSetUIProperty");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetUIProperty, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetUIButtonSize(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simGetUIButtonSize");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int size[2];
        if (CALL_C_API(simGetUIButtonSize, luaToInt(L, 1), luaToInt(L, 2), size) == 1)
        {
            pushIntTableOntoStack(L, 2, size);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetUIButtonProperty(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simGetUIButtonProperty");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simGetUIButtonProperty, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetUIButtonProperty(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simSetUIButtonProperty");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetUIButtonProperty, luaToInt(L, 1), luaToInt(L, 2), luaToInt(L, 3));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetUISlider(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simGetUISlider");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simGetUISlider, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetUISlider(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simSetUISlider");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetUISlider, luaToInt(L, 1), luaToInt(L, 2), luaToInt(L, 3));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetUIButtonLabel(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simSetUIButtonLabel");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        std::string stra;
        std::string strb;
        char* str1 = nullptr;
        char* str2 = nullptr;
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_string, 0, false, true, &errorString, argOffset);
        if (res != -1)
        {
            if (res == 2)
            {
                stra = luaWrap_lua_tostring(L, 3);
                str1 = (char*)stra.c_str();
            }
            res = checkOneGeneralInputArgument(L, 4, lua_arg_string, 0, true, true, &errorString, argOffset);
            if (res != -1)
            {
                if (res == 2)
                {
                    strb = luaWrap_lua_tostring(L, 4);
                    str2 = (char*)strb.c_str();
                }
                retVal = CALL_C_API(simSetUIButtonLabel, luaToInt(L, 1), luaToInt(L, 2), str1, str2);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetUIButtonLabel(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simGetUIButtonLabel");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        char* label = CALL_C_API(simGetUIButtonLabel, luaToInt(L, 1), luaToInt(L, 2));
        if (label != nullptr)
        {
            luaWrap_lua_pushtext(L, label);
            CALL_C_API(simReleaseBuffer, label);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simCreateUIButtonArray(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simCreateUIButtonArray");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simCreateUIButtonArray, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetUIButtonArrayColor(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simSetUIButtonArrayColor");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 2, lua_arg_number,
                            3))
    {
        int pos[2];
        float col[3];
        getIntsFromTable(L, 3, 2, pos);
        getFloatsFromTable(L, 4, 3, col);
        retVal = CALL_C_API(simSetUIButtonArrayColor, luaToInt(L, 1), luaToInt(L, 2), pos, col);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simDeleteUIButtonArray(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simDeleteUIButtonArray");
    int retVal = -1; // error

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simDeleteUIButtonArray, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreateUI(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simCreateUI");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0, lua_arg_number, 0, lua_arg_number, 2, lua_arg_number,
                            2))
    {
        int clientSize[2];
        int cellSize[2];
        getIntsFromTable(L, 3, 2, clientSize);
        getIntsFromTable(L, 4, 2, cellSize);
        int menuAttributes = luaToInt(L, 2);
        int b = 0;
        for (int i = 0; i < 8; i++)
        {
            if (menuAttributes & (1 << i))
                b++;
        }
        int* buttonHandles = new int[b];
        menuAttributes = (menuAttributes | sim_ui_menu_systemblock) - sim_ui_menu_systemblock;
        retVal = CALL_C_API(simCreateUI, luaWrap_lua_tostring(L, 1), menuAttributes, clientSize, cellSize, buttonHandles);
        if (retVal != -1)
        {
            luaWrap_lua_pushinteger(L, retVal);
            pushIntTableOntoStack(L, b, buttonHandles);
            delete[] buttonHandles;
            LUA_END(2);
        }
        delete[] buttonHandles;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreateUIButton(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simCreateUIButton");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 2, lua_arg_number, 2, lua_arg_number,
                            0))
    {
        int pos[2];
        int size[2];
        getIntsFromTable(L, 2, 2, pos);
        getIntsFromTable(L, 3, 2, size);
        retVal = CALL_C_API(simCreateUIButton, luaToInt(L, 1), pos, size, luaToInt(L, 4));
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSaveUI(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simSaveUI");

    int retVal = -1; // error
    int res = checkOneGeneralInputArgument(L, 1, lua_arg_table, 1, false, true, &errorString, argOffset);
    if (res > 0)
    {
        int res2 = checkOneGeneralInputArgument(L, 2, lua_arg_string, 0, false, false, &errorString, argOffset);
        if (res2 == 2)
        {
            if (res == 1)
                retVal = CALL_C_API(simSaveUI, 0, nullptr, luaWrap_lua_tostring(L, 2));
            else
            {
                int tl = int(luaWrap_lua_rawlen(L, 1));
                int* tble = new int[tl];
                getIntsFromTable(L, 1, tl, tble);
                retVal = CALL_C_API(simSaveUI, tl, tble, luaWrap_lua_tostring(L, 2));
                delete[] tble;
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simLoadUI(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simLoadUI");
    LUA_END(0);
}

int _simRemoveUI(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simRemoveUI");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simRemoveUI, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetUIPosition(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simGetUIPosition");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int position[2];
        if (CALL_C_API(simGetUIPosition, luaToInt(L, 1), position) != -1)
        {
            pushIntTableOntoStack(L, 2, position);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetUIPosition(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simSetUIPosition");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 2))
    {
        int position[2];
        getIntsFromTable(L, 2, 2, position);
        retVal = CALL_C_API(simSetUIPosition, luaToInt(L, 1), position);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetUIButtonColor(luaWrap_lua_State* L)
{ // DEPRECATED since 09/02/2017
    TRACE_LUA_API;
    LUA_START("simSetUIButtonColor");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        float col[9];
        float* acp[3] = {col, col + 3, col + 6};
        //      bool failed=true;
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 3, false, true, &errorString, argOffset);
        if (res != -1)
        {
            if (res != 2)
                acp[0] = nullptr;
            res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 3, true, true, &errorString, argOffset);
            if (res != -1)
            {
                if (res != 2)
                    acp[1] = nullptr;
                res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 3, true, true, &errorString, argOffset);
                if (res != -1)
                {
                    if (res != 2)
                        acp[2] = nullptr;
                    for (int i = 0; i < 3; i++)
                    {
                        if (acp[i] != nullptr)
                            getFloatsFromTable(L, 3 + i, 3, acp[i]);
                    }
                    retVal = CALL_C_API(simSetUIButtonColor, luaToInt(L, 1), luaToInt(L, 2), acp[0], acp[1], acp[2]);
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simHandleChildScript(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simHandleChildScript");
    warningString = "function is deprecated. Use simHandleSimulationScripts instead.";
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simHandleSensingChildScripts(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simHandleSensingChildScripts");

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simScaleSelectedObjects(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simScaleSelectedObjects");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_bool, 0))
        retVal = CALL_C_API(simScaleSelectedObjects, luaToDouble(L, 1), luaToBool(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simDeleteSelectedObjects(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simDeleteSelectedObjects");

    int retVal = CALL_C_API(simDeleteSelectedObjects, );

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simDelegateChildScriptExecution(luaWrap_lua_State* L)
{ // DEPRECATED. can only be called from a script running in a thread!!
    TRACE_LUA_API;
    LUA_START("simDelegateChildScriptExecution");

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetShapeVertex(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simGetShapeVertex");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        double relPos[3];
        retVal = CALL_C_API(simGetShapeVertex, luaToInt(L, 1), luaToInt(L, 2), luaToInt(L, 3), relPos);
        if (retVal == 1)
        {
            luaWrap_lua_pushinteger(L, retVal);
            pushDoubleTableOntoStack(L, 3, relPos);
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetShapeTriangle(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simGetShapeTriangle");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        int indices[3];
        double normals[9];
        retVal = CALL_C_API(simGetShapeTriangle, luaToInt(L, 1), luaToInt(L, 2), luaToInt(L, 3), indices, normals);
        if (retVal == 1)
        {
            luaWrap_lua_pushinteger(L, retVal);
            pushIntTableOntoStack(L, 3, indices);
            pushDoubleTableOntoStack(L, 9, normals);
            LUA_END(3);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetMaterialId(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simGetMaterialId");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        std::string matName(luaWrap_lua_tostring(L, 1));
        retVal = CALL_C_API(simGetMaterialId, matName.c_str());
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetShapeMaterial(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simGetShapeMaterial");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int shapeHandle = luaToInt(L, 1);
        retVal = CALL_C_API(simGetShapeMaterial, shapeHandle);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simReleaseScriptRawBuffer(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simReleaseScriptRawBuffer");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        if (handle == sim_handle_self)
            handle = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        if ((handle != sim_handle_tree) && (handle != sim_handle_chain))
            retVal = CALL_C_API(simReleaseScriptRawBuffer, handle, luaToInt(L, 2));
        else
        {
            std::vector<int> scriptHandles;
            if (handle == sim_handle_tree)
                getScriptTree_old(L, false, scriptHandles);
            else
                getScriptChain_old(L, false, false, scriptHandles);
            for (int i = 0; i < int(scriptHandles.size()); i++)
                retVal = CALL_C_API(simReleaseScriptRawBuffer, scriptHandles[i], sim_handle_all);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectSelectionSize(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simGetObjectSelectionSize");

    int retVal = CALL_C_API(simGetObjectSelectionSize, );

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectLastSelection(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("simGetObjectLastSelection");

    int retVal = CALL_C_API(simGetObjectLastSelection, );

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetVisionSensorFilter(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("sim.setVisionSensorFilter");
    int retVal = -1; // error

    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetVisionSensorFilter(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("sim.getVisionSensorFilter");

    luaWrap_lua_pushinteger(L, -1);
    LUA_END(1);
}

int _simGetScriptSimulationParameter(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("sim.getScriptSimulationParameter");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0))
    {
        bool goOn = true;
        int handle = luaToInt(L, 1);
        int selfScriptHandle = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        if (selfScriptHandle <= SIM_IDEND_SCENEOBJECT)
        {
            if (handle == sim_handle_self)
            {
                handle = selfScriptHandle;
                CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(handle);
                if (it != nullptr)
                {
                    it = it->getParent();
                    if (it != nullptr)
                        handle = it->getObjectHandle();
                    else
                        goOn = false;
                }
                else
                    goOn = false;
            }
        }
        else
        {
            if (handle == sim_handle_self)
            {
                handle = selfScriptHandle;

                // Since this routine can also be called by customization scripts, check for that here:
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(handle);
                if (it->getScriptType() == sim_scripttype_customization)
                {
                    handle = it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_customization);
                    it = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                        sim_scripttype_simulation, handle);
                    if (it != nullptr)
                        handle = it->getScriptHandle();
                    else
                        goOn = false;
                }
            }
        }
        if (goOn)
        {
            bool returnString = false;
            int ret = checkOneGeneralInputArgument(L, 3, lua_arg_bool, 0, true, false, &errorString, argOffset);
            if ((ret == 0) || (ret == 2))
            {
                if (ret == 2)
                    returnString = luaToBool(L, 3);
                std::string parameterName(luaWrap_lua_tostring(L, 2));
                if ((handle != sim_handle_tree) && (handle != sim_handle_chain) && (handle != sim_handle_all))
                {
                    int l;
                    char* p = CALL_C_API(simGetScriptSimulationParameter, handle, parameterName.c_str(), &l);
                    if (p != nullptr)
                    {
                        std::string a;
                        a.assign(p, l);
                        if (returnString)
                            luaWrap_lua_pushbinarystring(L, a.c_str(), a.length()); // push binary string for backw. comp.
                        else
                            pushCorrectTypeOntoLuaStack_old(L, a);
                        CALL_C_API(simReleaseBuffer, p);
                        LUA_END(1);
                    }
                }
            }
        }
        else
            errorString = SIM_ERROR_NO_ASSOCIATED_CHILD_SCRIPT_FOUND;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetScriptSimulationParameter(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("sim.setScriptSimulationParameter");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0, lua_arg_string, 0))
    {
        bool goOn = true;
        int handle = luaToInt(L, 1);
        int selfScriptHandle = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        if (selfScriptHandle <= SIM_IDEND_SCENEOBJECT)
        {
            if (handle == sim_handle_self)
            {
                handle = selfScriptHandle;
                CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(handle);
                if (it != nullptr)
                {
                    it = it->getParent();
                    if (it != nullptr)
                        handle = it->getObjectHandle();
                    else
                        goOn = false;
                }
                else
                    goOn = false;
            }
        }
        else
        {
            if (handle == sim_handle_self)
            {
                handle = selfScriptHandle;
                // Since this routine can also be called by customization scripts, check for that here:
                CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(handle);
                if (it->getScriptType() == sim_scripttype_customization)
                {
                    handle = it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_customization);
                    it = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                        sim_scripttype_simulation, handle);
                    if (it != nullptr)
                        handle = it->getScriptHandle();
                    else
                        goOn = false;
                }
            }
        }
        if (goOn)
        {
            std::string parameterName(luaWrap_lua_tostring(L, 2));
            size_t parameterValueLength;
            char* parameterValue = (char*)luaWrap_lua_tobuffer(L, 3, &parameterValueLength);
            if ((handle != sim_handle_tree) && (handle != sim_handle_chain))
            {
                retVal = CALL_C_API(simSetScriptSimulationParameter, handle, parameterName.c_str(), parameterValue,
                                                                  (int)parameterValueLength);
            }
            else
            {
                std::vector<int> scriptHandles;
                if (handle == sim_handle_tree)
                    getScriptTree_old(L, false, scriptHandles);
                else
                    getScriptChain_old(L, false, false, scriptHandles);
                retVal = 0;
                for (size_t i = 0; i < scriptHandles.size(); i++)
                {
                    if (CALL_C_API(simSetScriptSimulationParameter, scriptHandles[i], parameterName.c_str(),
                                                                 parameterValue, (int)parameterValueLength) == 1)
                        retVal++;
                }
            }
        }
        else
            errorString = SIM_ERROR_NO_ASSOCIATED_CHILD_SCRIPT_FOUND;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simHandleMechanism(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("sim.handleMechanism");
    int retVal = -1;                      // means error
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetPathTargetNominalVelocity(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("sim.setPathTargetNominalVelocity");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetPathTargetNominalVelocity, luaToInt(L, 1), luaToDouble(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetNameSuffix(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("sim.getNameSuffix");

    if (checkInputArguments(L, nullptr, argOffset, lua_arg_nil, 0))
    { // we want the suffix of current script
        luaWrap_lua_pushinteger(L, CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(L));
        LUA_END(1);
    }
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    { // we want the suffix of the provided name
        std::string nameWithSuffix(luaWrap_lua_tostring(L, 1));
        std::string name(tt::getNameWithoutSuffixNumber(nameWithSuffix.c_str(), true));
        int suffixNumber = tt::getNameSuffixNumber(nameWithSuffix.c_str(), true);
        luaWrap_lua_pushinteger(L, suffixNumber);
        luaWrap_lua_pushtext(L, name.c_str());
        LUA_END(2);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetNameSuffix(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("sim.setNameSuffix");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int nb = luaToInt(L, 1);
        CScriptObject::setScriptNameIndexToInterpreterState_lua_old(L, nb);
        retVal = 1;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAddStatusbarMessage(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("sim.addStatusbarMessage");

    int retVal = -1; // means error
    if (luaWrap_lua_gettop(L) == 0)
    {
#ifdef SIM_WITH_GUI
        GuiApp::clearStatusbar();
#endif
        retVal = 1;
    }
    else
    {
        if (checkInputArguments(L, nullptr, argOffset, lua_arg_nil, 0))
        {
#ifdef SIM_WITH_GUI
            GuiApp::clearStatusbar();
#endif
            retVal = 1;
        }
        else
        {
            if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
            {
                // retVal=CALL_C_API(simAddStatusbarMessage, luaWrap_lua_tostring(L,1));
                CScriptObject* it =
                    App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
                if (it != nullptr)
                {
                    App::logScriptMsg(it, sim_verbosity_msgs, luaWrap_lua_tostring(L, 1));
                    retVal = 1;
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetShapeMassAndInertia(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("sim.setShapeMassAndInertia");

    int result = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 9, lua_arg_number,
                            3))
    {
        int handle = luaToInt(L, 1);
        double mass = luaToDouble(L, 2);
        double inertiaMatrix[9];
        getDoublesFromTable(L, 3, 9, inertiaMatrix);
        double centerOfMass[3];
        getDoublesFromTable(L, 4, 3, centerOfMass);
        double* transf = nullptr;
        double transformation[12];
        int res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 12, true, true, &errorString, argOffset);
        if (res >= 0)
        {
            if (res == 2)
            {
                getDoublesFromTable(L, 5, 12, transformation);
                transf = transformation;
            }
            result = CALL_C_API(simSetShapeMassAndInertia, handle, mass, inertiaMatrix, centerOfMass, transf);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, result);
    LUA_END(1);
}

int _simGetShapeMassAndInertia(luaWrap_lua_State* L)
{ // DEPRECATED
    TRACE_LUA_API;
    LUA_START("sim.getShapeMassAndInertia");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        double* transf = nullptr;
        double transformation[12];
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 12, true, true, &errorString, argOffset);
        if (res >= 0)
        {
            if (res == 2)
            {
                getDoublesFromTable(L, 2, 12, transformation);
                transf = transformation;
            }
            double mass;
            double inertiaMatrix[9];
            double centerOfMass[3];
            int result = CALL_C_API(simGetShapeMassAndInertia, handle, &mass, inertiaMatrix, centerOfMass, transf);
            if (result == -1)
                luaWrap_lua_pushnil(L);
            luaWrap_lua_pushnumber(L, mass);
            pushDoubleTableOntoStack(L, 9, inertiaMatrix);
            pushDoubleTableOntoStack(L, 3, centerOfMass);
            LUA_END(3);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

typedef struct
{
    double lastTime;
    double accel;
    int vdl;
    double currentPos;
    double maxVelocity;
    double currentVel;
    int objID;
    CSceneObject* object;
    int targetObjID;
    CSceneObject* targetObject;
    double relativeDistanceOnPath;
    double previousLL;
    C7Vector startTr;
    int positionAndOrOrientation;
} simMoveToObjData_old;

int _sim_moveToObj_1(luaWrap_lua_State* L)
{ // for backward compatibility with simMoveToObject on old threaded scripts
    TRACE_LUA_API;
    LUA_START("sim._moveToObj_1");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    { // Those are the arguments that are always required! (the rest can be ignored or set to nil!
        int objID = luaToInt(L, 1);
        int targetObjID = luaToInt(L, 2);
        double maxVelocity = 0.1;
        double relativeDistanceOnPath = -1.0;
        int positionAndOrOrientation = 3; // position and orientation (default value)
        CSceneObject* object = App::currentWorld->sceneObjects->getObjectFromHandle(objID);
        CSceneObject* targetObject = App::currentWorld->sceneObjects->getObjectFromHandle(targetObjID);
        double accel = 0.0; // means infinite accel!! (default value)
        bool foundError = false;
        if ((!foundError) && ((object == nullptr) || (targetObject == nullptr)))
        {
            errorString = SIM_ERROR_OBJECT_OR_TARGET_OBJECT_DOES_NOT_EXIST;
            foundError = true;
        }
        if ((!foundError) && (targetObject == object))
        {
            errorString = SIM_ERROR_OBJECT_IS_SAME_AS_TARGET_OBJECT;
            foundError = true;
        }
        // Now check the optional arguments:
        int res;
        if (!foundError) // position and/or orientation argument:
        {
            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, true, &errorString, argOffset);
            if (res == 2)
            { // get the data
                positionAndOrOrientation = abs(luaToInt(L, 3));
                if ((positionAndOrOrientation & 3) == 0)
                    positionAndOrOrientation = 1; // position only
            }
            foundError = (res == -1);
        }
        if (!foundError) // positionOnPath argument:
        {
            res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, true, &errorString, argOffset);
            if (res == 2)
            { // get the data
                relativeDistanceOnPath = tt::getLimitedFloat(0.0, 1.0, luaToDouble(L, 4));
                if (targetObject->getObjectType() != sim_sceneobject_path)
                {
                    errorString = SIM_ERROR_TARGET_OBJECT_IS_NOT_A_PATH;
                    foundError = true;
                }
            }
            foundError = (res == -1);
        }
        if (!foundError) // Velocity argument:
        {
            res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, false, false, &errorString, argOffset);
            if (res == 2)
            { // get the data
                maxVelocity = luaToDouble(L, 5);
            }
            else
                foundError = true; // this argument is not optional!
        }
        if (!foundError) // Accel argument:
        {
            res = checkOneGeneralInputArgument(L, 6, lua_arg_number, 0, true, true, &errorString, argOffset);
            if (res == 2)
            { // get the data
                accel = fabs(luaToDouble(L, 6));
            }
            foundError = (res == -1);
        }
        if (!foundError)
        { // do the job here!
            C7Vector startTr(object->getCumulativeTransformation());
            double currentVel = 0.0;
            double lastTime = App::currentWorld->simulation->getSimulationTime();
            double vdl = 1.0;
            // vld is the totalvirtual distance
            double currentPos = 0.0;
            double previousLL = 0.0;

            _memHandles_old.push_back(_nextMemHandle_old);
            simMoveToObjData_old* mem = new simMoveToObjData_old();
            mem->lastTime = lastTime;
            mem->accel = accel;
            mem->vdl = vdl;
            mem->currentPos = currentPos;
            mem->maxVelocity = maxVelocity;
            mem->currentVel = currentVel;
            mem->objID = objID;
            mem->object = object;
            mem->targetObjID = targetObjID;
            mem->targetObject = targetObject;
            mem->relativeDistanceOnPath = relativeDistanceOnPath;
            mem->previousLL = previousLL;
            mem->startTr = startTr;
            mem->positionAndOrOrientation = positionAndOrOrientation;

            _memBuffers_old.push_back(mem);
            luaWrap_lua_pushinteger(L, _nextMemHandle_old);
            _nextMemHandle_old++;
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED();
    LUA_END(0);
}

int _sim_moveToObj_2(luaWrap_lua_State* L)
{ // for backward compatibility with simMoveToObject on old threaded scripts
    TRACE_LUA_API;
    LUA_START("sim._moveToObj_2");
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_integer, 0))
    {
        int h = luaToInt(L, 1);
        simMoveToObjData_old* mem = nullptr;
        for (size_t i = 0; i < _memHandles_old.size(); i++)
        {
            if (_memHandles_old[i] == h)
            {
                mem = (simMoveToObjData_old*)_memBuffers_old[i];
                break;
            }
        }
        if (mem != nullptr)
        {
            bool movementFinished = false;
            double currentTime =
                App::currentWorld->simulation->getSimulationTime() + App::currentWorld->simulation->getTimeStep();
            double dt = currentTime - mem->lastTime;
            mem->lastTime = currentTime;

            if (mem->accel == 0.0)
            { // Means infinite acceleration
                double timeNeeded = (mem->vdl - mem->currentPos) / mem->maxVelocity;
                mem->currentVel = mem->maxVelocity;
                if (timeNeeded > dt)
                {
                    mem->currentPos += dt * mem->maxVelocity;
                    dt = 0.0; // this is what is left
                }
                else
                {
                    mem->currentPos = mem->vdl;
                    if (timeNeeded >= 0.0)
                        dt -= timeNeeded;
                }
            }
            else
            {
                double p = mem->currentPos;
                double v = mem->currentVel;
                double t = dt;
                CLinMotionRoutines::getNextValues(p, v, mem->maxVelocity, mem->accel, 0.0, mem->vdl, 0.0, 0.0, t);
                mem->currentPos = double(p);
                mem->currentVel = double(v);
                dt = double(t);
            }

            // Now check if we are within tolerances:
            if (fabs(mem->currentPos - mem->vdl) <= 0.00001)
                movementFinished = true;

            // Set the new configuration of the object:
            double ll = mem->currentPos / mem->vdl;
            if (ll > 1.0)
                ll = 1.0;
            if ((App::currentWorld->sceneObjects->getObjectFromHandle(mem->objID) == mem->object) &&
                (App::currentWorld->sceneObjects->getObjectFromHandle(mem->targetObjID) ==
                 mem->targetObject)) // make sure the objects are still valid (running in a thread)
            {
                C7Vector targetTr(mem->targetObject->getCumulativeTransformation());
                bool goOn = true;
                if (mem->relativeDistanceOnPath >= 0.0)
                { // we should have a path here
                    if (mem->targetObject->getObjectType() == sim_sceneobject_path)
                    {
                        C7Vector pathLoc;
                        if (((CPath_old*)mem->targetObject)
                                ->pathContainer->getTransformationOnBezierCurveAtNormalizedVirtualDistance(
                                    mem->relativeDistanceOnPath, pathLoc))
                            targetTr *= pathLoc;
                        else
                            mem->relativeDistanceOnPath = -1.0; // the path is empty!
                    }
                    else
                        goOn = false;
                }
                if (goOn)
                {
                    C7Vector newAbs;
                    newAbs.buildInterpolation(mem->startTr, targetTr, (ll - mem->previousLL) / (1.0 - mem->previousLL));
                    mem->startTr = newAbs;
                    C7Vector parentInv(mem->object->getFullParentCumulativeTransformation().getInverse());
                    C7Vector currentTr(mem->object->getCumulativeTransformation());
                    if ((mem->positionAndOrOrientation & 1) == 0)
                        newAbs.X = currentTr.X;
                    if ((mem->positionAndOrOrientation & 2) == 0)
                        newAbs.Q = currentTr.Q;
                    mem->object->setLocalTransformation(parentInv * newAbs);
                }
                else
                    movementFinished = true; // the target object is not a path anymore!!

                mem->previousLL = ll;
                if (!movementFinished)
                {
                    luaWrap_lua_pushinteger(L, 0); // mov. not yet finished
                    LUA_END(1);
                }
                luaWrap_lua_pushinteger(L, 1); // mov. finished
                luaWrap_lua_pushnumber(L, dt); // success (deltaTime left)
                LUA_END(2);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED();
    LUA_END(0);
}

int _simCheckIkGroup(luaWrap_lua_State* L)
{ // deprecated on 29.09.2020
    TRACE_LUA_API;
    LUA_START("sim.checkIkGroup");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        if (luaWrap_lua_isnonbuffertable(L, 2))
        {
            int jointCnt = (int)luaWrap_lua_rawlen(L, 2);
            int* handles = new int[jointCnt];
            getIntsFromTable(L, 2, jointCnt, handles);
            double* values = new double[jointCnt];

            int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, jointCnt, true, true, &errorString, argOffset);
            if (res >= 0)
            {
                int* jointOptionsP = nullptr;
                std::vector<int> jointOptions;
                if (res == 2)
                {
                    jointOptions.resize(jointCnt);
                    getIntsFromTable(L, 3, jointCnt, &jointOptions[0]);
                    jointOptionsP = &jointOptions[0];
                }

                int retVal = CALL_C_API(simCheckIkGroup, luaToInt(L, 1), jointCnt, handles, values, jointOptionsP);
                luaWrap_lua_pushinteger(L, retVal);
                pushDoubleTableOntoStack(L, jointCnt, values);
                delete[] values;
                delete[] handles;
                LUA_END(2);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, -1);
    LUA_END(1);
}

int _simCreateIkGroup(luaWrap_lua_State* L)
{ // DEPRECATED ON 29.09.2020
    TRACE_LUA_API;
    LUA_START("sim.createIkGroup");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int options = luaToInt(L, 1);
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 2, true, true, &errorString, argOffset);
        if (res >= 0)
        {
            int intParams[2];
            int* intP = nullptr;
            if (res == 2)
            {
                getIntsFromTable(L, 2, 2, intParams);
                intP = intParams;
            }
            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 4, true, true, &errorString, argOffset);
            if (res >= 0)
            {
                double floatParams[4];
                double* floatP = nullptr;
                if (res == 2)
                {
                    getDoublesFromTable(L, 3, 4, floatParams);
                    floatP = floatParams;
                }
                retVal = CALL_C_API(simCreateIkGroup, options, intP, floatP, nullptr);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simRemoveIkGroup(luaWrap_lua_State* L)
{ // DEPRECATED ON 29.09.2020
    TRACE_LUA_API;
    LUA_START("sim.removeIkGroup");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        retVal = CALL_C_API(simRemoveIkGroup, handle);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreateIkElement(luaWrap_lua_State* L)
{ // DEPRECATED ON 29.09.2020
    TRACE_LUA_API;
    LUA_START("sim.createIkElement");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 4))
    {
        int ikGroup = luaToInt(L, 1);
        int options = luaToInt(L, 2);
        int intParams[4];
        getIntsFromTable(L, 3, 4, intParams);

        int res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 4, true, true, &errorString, argOffset);
        if (res >= 0)
        {
            double floatParams[4];
            double* floatP = nullptr;
            if (res == 2)
            {
                getDoublesFromTable(L, 4, 4, floatParams);
                floatP = floatParams;
            }
            retVal = CALL_C_API(simCreateIkElement, ikGroup, options, intParams, floatP, nullptr);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simExportIk(luaWrap_lua_State* L)
{ // deprecated on 29.09.2020
    TRACE_LUA_API;
    LUA_START("sim.exportIk");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        std::string pathAndFilename(luaWrap_lua_tostring(L, 1));
        retVal = CALL_C_API(simExportIk, pathAndFilename.c_str(), 0, nullptr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simComputeJacobian(luaWrap_lua_State* L)
{ // deprecated on 29.09.2020
    TRACE_LUA_API;
    LUA_START("sim.computeJacobian");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simComputeJacobian, luaToInt(L, 1), luaToInt(L, 2), nullptr);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetConfigForTipPose(luaWrap_lua_State* L)
{ // deprecated on 29.09.2020
    TRACE_LUA_API;
    LUA_START("sim.getConfigForTipPose");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 1, lua_arg_number, 0, lua_arg_number,
                            0))
    {
        int ikGroupHandle = luaToInt(L, 1);
        std::vector<int> jointHandles;
        int jointCnt = int(luaWrap_lua_rawlen(L, 2));
        jointHandles.resize(jointCnt);
        getIntsFromTable(L, 2, jointCnt, &jointHandles[0]);
        double thresholdDist = luaToDouble(L, 3);
        int maxTimeInMs = luaToInt(L, 4);
        double metric[4] = {1.0, 1.0, 1.0, 0.1};
        int res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 4, true, true, &errorString, argOffset);
        if (res >= 0)
        {
            if (res == 2)
                getDoublesFromTable(L, 5, 4, metric);
            int collisionPairCnt = 0;
            std::vector<int> _collisionPairs;
            int* collisionPairs = nullptr;
            res = checkOneGeneralInputArgument(L, 6, lua_arg_number, -1, true, true, &errorString, argOffset);
            if (res >= 0)
            {
                if (res == 2)
                {
                    collisionPairCnt = int(luaWrap_lua_rawlen(L, 6)) / 2;
                    if (collisionPairCnt > 0)
                    {
                        _collisionPairs.resize(collisionPairCnt * 2);
                        getIntsFromTable(L, 6, collisionPairCnt * 2, &_collisionPairs[0]);
                        collisionPairs = &_collisionPairs[0];
                    }
                }
                std::vector<int> _jointOptions;
                int* jointOptions = nullptr;
                res = checkOneGeneralInputArgument(L, 7, lua_arg_number, jointCnt, true, true, &errorString, argOffset);
                if (res >= 0)
                {
                    if (res == 2)
                    {
                        _jointOptions.resize(jointCnt);
                        getIntsFromTable(L, 7, jointCnt, &_jointOptions[0]);
                        jointOptions = &_jointOptions[0];
                    }

                    res = checkOneGeneralInputArgument(L, 8, lua_arg_number, jointCnt, true, true, &errorString, argOffset);
                    if (res >= 0)
                    {
                        std::vector<double> _lowLimits;
                        _lowLimits.resize(jointCnt);
                        std::vector<double> _ranges;
                        _ranges.resize(jointCnt);
                        double* lowLimits = nullptr;
                        double* ranges = nullptr;
                        if (res == 2)
                        {
                            getDoublesFromTable(L, 8, jointCnt, &_lowLimits[0]);
                            lowLimits = &_lowLimits[0];
                        }
                        res = checkOneGeneralInputArgument(L, 9, lua_arg_number, jointCnt, lowLimits == nullptr,
                                                           lowLimits == nullptr, &errorString, argOffset);
                        if (res >= 0)
                        {
                            if (res == 2)
                            {
                                getDoublesFromTable(L, 9, jointCnt, &_ranges[0]);
                                ranges = &_ranges[0];
                            }
                            std::vector<double> foundConfig;
                            foundConfig.resize(jointCnt);
                            res = CALL_C_API(simGetConfigForTipPose,
                                ikGroupHandle, jointCnt, &jointHandles[0], thresholdDist, maxTimeInMs, &foundConfig[0],
                                metric, collisionPairCnt, collisionPairs, jointOptions, lowLimits, ranges, nullptr);
                            if (res > 0)
                            {
                                pushDoubleTableOntoStack(L, jointCnt, &foundConfig[0]);
                                LUA_END(1);
                            }
                        }
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGenerateIkPath(luaWrap_lua_State* L)
{ // deprecated on 29.09.2020
    TRACE_LUA_API;
    LUA_START("sim.generateIkPath");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 1, lua_arg_number, 0))
    {
        int ikGroupHandle = luaToInt(L, 1);
        std::vector<int> jointHandles;
        int jointCnt = int(luaWrap_lua_rawlen(L, 2));
        jointHandles.resize(jointCnt);
        getIntsFromTable(L, 2, jointCnt, &jointHandles[0]);
        int ptCnt = luaToInt(L, 3);
        int res = checkOneGeneralInputArgument(L, 4, lua_arg_number, -1, true, true, &errorString, argOffset);
        if (res >= 0)
        {
            int collisionPairCnt = 0;
            std::vector<int> _collisionPairs;
            int* collisionPairs = nullptr;
            if (res == 2)
            {
                collisionPairCnt = int(luaWrap_lua_rawlen(L, 4)) / 2;
                if (collisionPairCnt > 0)
                {
                    _collisionPairs.resize(collisionPairCnt * 2);
                    getIntsFromTable(L, 4, collisionPairCnt * 2, &_collisionPairs[0]);
                    collisionPairs = &_collisionPairs[0];
                }
            }
            res = checkOneGeneralInputArgument(L, 5, lua_arg_number, jointCnt, true, true, &errorString, argOffset);
            if (res >= 0)
            {
                std::vector<int> _jointOptions;
                int* jointOptions = nullptr;
                if (res == 2)
                {
                    _jointOptions.resize(jointCnt);
                    getIntsFromTable(L, 5, jointCnt, &_jointOptions[0]);
                    jointOptions = &_jointOptions[0];
                }
                double* path = CALL_C_API(simGenerateIkPath, ikGroupHandle, jointCnt, &jointHandles[0], ptCnt,
                                                          collisionPairCnt, collisionPairs, jointOptions, nullptr);
                if (path != nullptr)
                {
                    pushDoubleTableOntoStack(L, jointCnt * ptCnt, path);
                    CALL_C_API(simReleaseBuffer, (char*)path);
                    LUA_END(1);
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetIkGroupHandle(luaWrap_lua_State* L)
{ // deprecated on 29.09.2020
    TRACE_LUA_API;
    LUA_START("sim.getIkGroupHandle");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        std::string name(luaWrap_lua_tostring(L, 1));
        setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                                   CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                       L)); // for transmitting to the master function additional info (e.g.for autom.
                                            // name adjustment, or for autom. object deletion when script ends)
        retVal = CALL_C_API(simGetIkGroupHandle, name.c_str());
        setCurrentScriptInfo_cSide(-1, -1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetIkGroupMatrix(luaWrap_lua_State* L)
{ // deprecated on 29.09.2020
    TRACE_LUA_API;
    LUA_START("sim.getIkGroupMatrix");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int ikGroupHandle = luaToInt(L, 1);
        int options = luaToInt(L, 2);
        int matrixSize[2];
        double* data = CALL_C_API(simGetIkGroupMatrix, ikGroupHandle, options, matrixSize);
        if (data != nullptr)
        {
            pushDoubleTableOntoStack(L, matrixSize[0] * matrixSize[1], data);
            pushIntTableOntoStack(L, 2, matrixSize);
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simHandleIkGroup(luaWrap_lua_State* L)
{ // deprecated on 29.09.2020
    TRACE_LUA_API;
    LUA_START("sim.handleIkGroup");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simHandleIkGroup, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetIkGroupProperties(luaWrap_lua_State* L)
{ // deprecated on 29.09.2020
    TRACE_LUA_API;
    LUA_START("sim.setIkGroupProperties");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number,
                            0))
        retVal = CALL_C_API(simSetIkGroupProperties, luaToInt(L, 1), luaToInt(L, 2), luaToInt(L, 3), luaToDouble(L, 4),
                                                  nullptr);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetIkElementProperties(luaWrap_lua_State* L)
{ // deprecated on 29.09.2020
    TRACE_LUA_API;
    LUA_START("sim.setIkElementProperties");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        int res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 2, true, true, &errorString, argOffset);
        if (res >= 0)
        {
            double* precision = nullptr;
            double prec[2];
            if (res == 2)
            {
                getDoublesFromTable(L, 4, 2, prec);
                precision = prec;
            }
            res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 2, true, true, &errorString, argOffset);
            if (res >= 0)
            {
                double* weight = nullptr;
                double w[2];
                if (res == 2)
                {
                    getDoublesFromTable(L, 5, 2, w);
                    weight = w;
                }
                retVal = CALL_C_API(simSetIkElementProperties, luaToInt(L, 1), luaToInt(L, 2), luaToInt(L, 3), precision,
                                                            weight, nullptr);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetThreadIsFree(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.setThreadIsFree");

    int retVal = 0;
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simTubeRead(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim._tubeRead");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int dataLength;
        char* data = CALL_C_API(simTubeRead, luaToInt(L, 1), &dataLength);
        if (data)
        {
            luaWrap_lua_pushbinarystring(L, (const char*)data, dataLength); // push binary string for backw. comp.
            delete[] data;
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simTubeOpen(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.tubeOpen");

    int retVal = -1; // Error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0, lua_arg_number, 0))
    {
        std::string strTmp = luaWrap_lua_tostring(L, 2);
        int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
        retVal = App::currentWorld->commTubeContainer_old->openTube(luaToInt(L, 1), strTmp.c_str(),
                                                                    (it->getScriptType() == sim_scripttype_main) ||
                                                                        (it->getScriptType() == sim_scripttype_simulation),
                                                                    luaToInt(L, 3));
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simTubeClose(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.tubeClose");

    int retVal = -1; // Error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simTubeClose, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simTubeWrite(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.tubeWrite");

    int retVal = -1; // Error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0))
    {
        size_t dataLength;
        char* data = (char*)luaWrap_lua_tobuffer(L, 2, &dataLength);
        retVal = CALL_C_API(simTubeWrite, luaToInt(L, 1), data, (int)dataLength);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simTubeStatus(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.tubeStatus");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int readSize;
        int writeSize;
        int status = CALL_C_API(simTubeStatus, luaToInt(L, 1), &readSize, &writeSize);
        if (status >= 0)
        {
            luaWrap_lua_pushinteger(L, status);
            luaWrap_lua_pushinteger(L, readSize);
            luaWrap_lua_pushinteger(L, writeSize);
            LUA_END(3);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSendData(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.sendData");

    int retVal = -1;
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    if ((it->getScriptType() == sim_scripttype_main) || (it->getScriptType() == sim_scripttype_simulation))
    {
        if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_string, 0,
                                lua_arg_string, 0))
        {
            int targetID = luaToInt(L, 1);
            if ((targetID <= 0) && (targetID != sim_handle_all) && (targetID != sim_handle_tree) &&
                (targetID != sim_handle_chain))
                errorString = SIM_ERROR_INVALID_TARGET_HANDLE;
            else
            {
                int dataHeader = luaToInt(L, 2);
                if (dataHeader < 0)
                    errorString = SIM_ERROR_INVALID_DATA_HEADER;
                else
                {
                    std::string dataName(luaWrap_lua_tostring(L, 3));
                    if ((dataName.length() == 0) || (dataName.find(char(0)) != std::string::npos))
                        errorString = SIM_ERROR_INVALID_DATA_NAME;
                    else
                    {
                        size_t dataLength;
                        char* data = (char*)luaWrap_lua_tobuffer(L, 4, &dataLength);
                        if (dataLength < 1)
                            errorString = SIM_ERROR_INVALID_DATA;
                        else
                        {
                            // Following are default values:
                            int antennaHandle = sim_handle_self;
                            double actionRadius = 100.0;
                            double emissionAngle1 = piValue;
                            double emissionAngle2 = piValT2;
                            double persistence = 0.0;
                            bool err = false;
                            int res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, true, false, &errorString, argOffset);
                            if (res == 2)
                                antennaHandle = luaToInt(L, 5);
                            err = err || ((res != 0) && (res != 2));
                            if (!err)
                            {
                                if ((antennaHandle < 0) && (antennaHandle != sim_handle_default) &&
                                    ((antennaHandle != sim_handle_self) ||
                                     (it->getScriptType() != sim_scripttype_simulation)))
                                {
                                    errorString = SIM_ERROR_INVALID_ANTENNA_HANDLE;
                                    err = true;
                                }
                                else
                                {
                                    if (antennaHandle == sim_handle_self)
                                        antennaHandle =
                                            it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_simulation);
                                    if (antennaHandle != sim_handle_default)
                                    {
                                        CSceneObject* ant =
                                            App::currentWorld->sceneObjects->getObjectFromHandle(antennaHandle);
                                        if (ant == nullptr)
                                        {
                                            errorString = SIM_ERROR_INVALID_ANTENNA_HANDLE;
                                            err = true;
                                        }
                                    }
                                }
                            }
                            if (!err)
                            {
                                int res =
                                    checkOneGeneralInputArgument(L, 6, lua_arg_number, 0, true, false, &errorString, argOffset);
                                if (res == 2)
                                    actionRadius = luaToDouble(L, 6);
                                err = err || ((res != 0) && (res != 2));
                            }
                            if (!err)
                            {
                                int res =
                                    checkOneGeneralInputArgument(L, 7, lua_arg_number, 0, true, false, &errorString, argOffset);
                                if (res == 2)
                                    emissionAngle1 = luaToDouble(L, 7);
                                err = err || ((res != 0) && (res != 2));
                            }
                            if (!err)
                            {
                                int res =
                                    checkOneGeneralInputArgument(L, 8, lua_arg_number, 0, true, false, &errorString, argOffset);
                                if (res == 2)
                                    emissionAngle2 = luaToDouble(L, 8);
                                err = err || ((res != 0) && (res != 2));
                            }
                            if (!err)
                            {
                                int res =
                                    checkOneGeneralInputArgument(L, 9, lua_arg_number, 0, true, false, &errorString, argOffset);
                                if (res == 2)
                                    persistence = luaToDouble(L, 9);
                                err = err || ((res != 0) && (res != 2));
                            }
                            if (!err)
                            {
                                actionRadius = tt::getLimitedFloat(0.0, DBL_MAX, actionRadius);
                                emissionAngle1 = tt::getLimitedFloat(0.0, piValue, emissionAngle1);
                                emissionAngle2 = tt::getLimitedFloat(0.0, piValT2, emissionAngle2);
                                persistence = tt::getLimitedFloat(0.0, 99999999999999.9, persistence);
                                if (persistence == 0.0)
                                    persistence = App::currentWorld->simulation->getTimeStep() * 1.5;

                                App::currentWorld->sceneObjects->embeddedScriptContainer->broadcastDataContainer.broadcastData(
                                    currentScriptID, targetID, dataHeader, dataName,
                                    App::currentWorld->simulation->getSimulationTime() + persistence, actionRadius,
                                    antennaHandle, emissionAngle1, emissionAngle2, data, (int)dataLength);
                                retVal = 1;
                            }
                        }
                    }
                }
            }
        }
    }
    else
        errorString = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT_OR_CHILD_SCRIPT;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simReceiveData(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.receiveData");

    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    if ((it->getScriptType() == sim_scripttype_main) || (it->getScriptType() == sim_scripttype_simulation))
    {
        int dataHeader = -1;
        std::string dataName;
        int antennaHandle = sim_handle_self;
        int index = -1;
        bool err = false;
        int res = checkOneGeneralInputArgument(L, 1, lua_arg_number, 0, true, false, &errorString, argOffset);
        if (res == 2)
        {
            dataHeader = luaToInt(L, 1);
            if (dataHeader < 0)
                dataHeader = -1;
        }
        err = err || ((res != 0) && (res != 2));
        if (!err)
        {
            int res = checkOneGeneralInputArgument(L, 2, lua_arg_string, 0, true, true, &errorString, argOffset);
            if (res == 2)
            {
                dataName = luaWrap_lua_tostring(L, 2);
                if (dataName.length() < 1)
                {
                    errorString = SIM_ERROR_INVALID_DATA_NAME;
                    err = true;
                }
            }
            err = err || (res < 0);
        }
        if (!err)
        {
            int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString, argOffset);
            if (res == 2)
            {
                antennaHandle = luaToInt(L, 3);
                if ((antennaHandle < 0) && (antennaHandle != sim_handle_default) &&
                    ((antennaHandle != sim_handle_self) || (it->getScriptType() != sim_scripttype_simulation)))
                {
                    errorString = SIM_ERROR_INVALID_ANTENNA_HANDLE;
                    err = true;
                }
            }
            err = err || ((res != 0) && (res != 2));
            if (!err)
            {
                if (antennaHandle == sim_handle_self)
                    antennaHandle = it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_simulation);
                if (antennaHandle != sim_handle_default)
                {
                    CSceneObject* ant = App::currentWorld->sceneObjects->getObjectFromHandle(antennaHandle);
                    if (ant == nullptr)
                    {
                        errorString = SIM_ERROR_INVALID_ANTENNA_HANDLE;
                        err = true;
                    }
                }
            }
        }
        if (!err)
        {
            int res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, false, &errorString, argOffset);
            if (res == 2)
            {
                index = luaToInt(L, 4);
                if (index < 0)
                    index = -1;
            }
            err = err || ((res != 0) && (res != 2));
        }
        if (!err)
        {
            int theDataHeader;
            int theDataLength;
            int theSenderID;
            std::string theDataName;
            char* data0 = App::currentWorld->sceneObjects->embeddedScriptContainer->broadcastDataContainer.receiveData(
                currentScriptID, App::currentWorld->simulation->getSimulationTime(), dataHeader, dataName,
                antennaHandle, theDataLength, index, theSenderID, theDataHeader, theDataName);
            if (data0 != nullptr)
            {
                luaWrap_lua_pushbinarystring(L, data0, theDataLength); // push binary string for backw. comp.
                luaWrap_lua_pushinteger(L, theSenderID);
                luaWrap_lua_pushinteger(L, theDataHeader);
                luaWrap_lua_pushtext(L, theDataName.c_str());
                LUA_END(4);
            }
        }
    }
    else
        errorString = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT_OR_CHILD_SCRIPT;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

typedef struct
{
    int objID;
    CSceneObject* object;
    int pathID;
    CPath_old* path;
    double lastTime;
    double accel;
    double pos;
    double vel;
    double maxVelocity;
    double bezierPathLength;
    int positionAndOrOrientation;
} simFollowPath_old;

int _sim_followPath_1(luaWrap_lua_State* L)
{ // for backward compatibility with simFollowPath on old threaded scripts
    TRACE_LUA_API;
    LUA_START("sim._followPath_1");
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0,
                            lua_arg_number, 0))
    { // Those are the arguments that are always required! (the rest can be ignored or set to nil!
        int objID = luaToInt(L, 1);
        int pathID = luaToInt(L, 2);
        double posOnPath = luaToDouble(L, 4);
        int positionAndOrOrientation = abs(luaToInt(L, 3));
        if (positionAndOrOrientation == 0)
            positionAndOrOrientation = 1;
        double maxVelocity = luaToDouble(L, 5);
        CSceneObject* object = App::currentWorld->sceneObjects->getObjectFromHandle(objID);
        CPath_old* path = App::currentWorld->sceneObjects->getPathFromHandle(pathID);
        double accel = 0.0; // means infinite accel!! (default value)
        bool foundError = false;
        if ((!foundError) && (object == nullptr))
        {
            errorString = SIM_ERROR_OBJECT_INEXISTANT;
            foundError = true;
        }
        if ((!foundError) && (path == nullptr))
        {
            errorString = SIM_ERROR_PATH_INEXISTANT;
            foundError = true;
        }
        if (!foundError)
        {
            if (path->pathContainer->getAttributes() & sim_pathproperty_closed_path)
            {
                if (posOnPath < 0.0)
                    posOnPath = 0.0;
            }
            else
                posOnPath = tt::getLimitedFloat(0.0, 1.0, posOnPath);
        }

        // Now check the optional arguments:
        int res;
        if (!foundError) // Accel argument:
        {
            res = checkOneGeneralInputArgument(L, 6, lua_arg_number, 0, true, true, &errorString, argOffset);
            if (res == 2)
            { // get the data
                accel = fabs(luaToDouble(L, 6));
            }
            foundError = (res == -1);
        }
        if (!foundError)
        { // do the job here!
            double bezierPathLength = path->pathContainer->getBezierVirtualPathLength();
            double pos = posOnPath * bezierPathLength;
            double vel = 0.0;
            double lastTime = App::currentWorld->simulation->getSimulationTime();
            bool movementFinished = (bezierPathLength == 0.0);
            if (movementFinished)
                luaWrap_lua_pushinteger(L, -1);
            else
            {
                _memHandles_old.push_back(_nextMemHandle_old);
                simFollowPath_old* mem = new simFollowPath_old();
                mem->objID = objID;
                mem->object = object;
                mem->pathID = pathID;
                mem->path = path;
                mem->lastTime = lastTime;
                mem->accel = accel;
                mem->pos = pos;
                mem->vel = vel;
                mem->maxVelocity = maxVelocity;
                mem->bezierPathLength = bezierPathLength;
                mem->positionAndOrOrientation = positionAndOrOrientation;
                _memBuffers_old.push_back(mem);
                luaWrap_lua_pushinteger(L, _nextMemHandle_old);
                _nextMemHandle_old++;
            }
            LUA_END(1);
        }
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED();
    LUA_END(0);
}

int _sim_followPath_2(luaWrap_lua_State* L)
{ // for backward compatibility with simFollowPath on old threaded scripts
    TRACE_LUA_API;
    LUA_START("sim._followPath_2");
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_integer, 0))
    {
        int h = luaToInt(L, 1);
        simFollowPath_old* mem = nullptr;
        for (size_t i = 0; i < _memHandles_old.size(); i++)
        {
            if (_memHandles_old[i] == h)
            {
                mem = (simFollowPath_old*)_memBuffers_old[i];
                break;
            }
        }
        if (mem != nullptr)
        {
            if ((App::currentWorld->sceneObjects->getObjectFromHandle(mem->objID) == mem->object) &&
                (App::currentWorld->sceneObjects->getPathFromHandle(mem->pathID) ==
                 mem->path)) // make sure the objects are still valid (running in a thread)
            {
                double dt = App::currentWorld->simulation->getTimeStep(); // this is the time left if we leave here
                bool movementFinished = false;
                double currentTime =
                    App::currentWorld->simulation->getSimulationTime() + App::currentWorld->simulation->getTimeStep();
                dt = currentTime - mem->lastTime;
                mem->lastTime = currentTime;
                if (mem->accel == 0.0)
                { // Means infinite acceleration
                    mem->path->pathContainer->handlePath_keepObjectUnchanged(dt, mem->pos, mem->vel, mem->maxVelocity,
                                                                             1.0, false, true);
                }
                else
                {
                    mem->path->pathContainer->handlePath_keepObjectUnchanged(dt, mem->pos, mem->vel, mem->maxVelocity,
                                                                             mem->accel, false, false);
                }

                // Now check if we are within tolerances:
                if (((fabs(mem->pos - mem->bezierPathLength) <= 0.00001) && (mem->maxVelocity >= 0.0)) ||
                    ((fabs(mem->pos - 0.0) <= 0.00001) && (mem->maxVelocity <= 0.0)))
                    movementFinished = true;

                // Set the new configuration of the object:
                double ll = double(mem->pos / mem->bezierPathLength);
                C7Vector newAbs;
                if (mem->path->pathContainer->getTransformationOnBezierCurveAtNormalizedVirtualDistance(ll, newAbs))
                {
                    newAbs = mem->path->getCumulativeTransformation() * newAbs;
                    C7Vector parentInv(mem->object->getFullParentCumulativeTransformation().getInverse());
                    C7Vector currAbs(mem->object->getCumulativeTransformation());
                    if ((mem->positionAndOrOrientation & 1) == 0)
                        newAbs.X = currAbs.X;
                    if ((mem->positionAndOrOrientation & 2) == 0)
                        newAbs.Q = currAbs.Q;
                    mem->object->setLocalTransformation(parentInv * newAbs);
                }
                else
                    movementFinished = true;

                if (!movementFinished)
                {
                    luaWrap_lua_pushinteger(L, 0); // mov. not yet finished
                    LUA_END(1);
                }
                luaWrap_lua_pushinteger(L, 1); // mov. finished
                luaWrap_lua_pushnumber(L, dt); // success (deltaTime left)
                LUA_END(2);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED();
    LUA_END(0);
}

int _simGetDataOnPath(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.getDataOnPath");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int auxFlags;
        double auxChannels[4];
        if (CALL_C_API(simGetDataOnPath, luaToInt(L, 1), luaToDouble(L, 2), 0, &auxFlags, auxChannels) == 1)
        {
            luaWrap_lua_pushinteger(L, auxFlags);
            pushDoubleTableOntoStack(L, 4, auxChannels);
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetPositionOnPath(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.getPositionOnPath");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        double coord[3];
        if (CALL_C_API(simGetPositionOnPath, luaToInt(L, 1), luaToDouble(L, 2), coord) == 1)
        {
            pushDoubleTableOntoStack(L, 3, coord);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetOrientationOnPath(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.getOrientationOnPath");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        double coord[3];
        if (CALL_C_API(simGetOrientationOnPath, luaToInt(L, 1), luaToDouble(L, 2), coord) == 1)
        {
            pushDoubleTableOntoStack(L, 3, coord);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetClosestPositionOnPath(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.getClosestPositionOnPath");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 3))
    {
        double coord[3];
        getDoublesFromTable(L, 2, 3, coord);
        double dist = 0.0;
        if (CALL_C_API(simGetClosestPositionOnPath, luaToInt(L, 1), coord, &dist) != -1)
        {
            luaWrap_lua_pushnumber(L, dist);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetPathPosition(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.getPathPosition");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        double pathVal[1];
        if (CALL_C_API(simGetPathPosition, luaToInt(L, 1), pathVal) != -1)
        {
            luaWrap_lua_pushnumber(L, pathVal[0]);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetPathPosition(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.setPathPosition");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetPathPosition, luaToInt(L, 1), luaToDouble(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetPathLength(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.getPathLength");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        double pathLen[1];
        if (CALL_C_API(simGetPathLength, luaToInt(L, 1), pathLen) != -1)
        {
            luaWrap_lua_pushnumber(L, pathLen[0]);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simInsertPathCtrlPoints(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.insertPathCtrlPoints");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number,
                            0))
    {
        int pathHandle = luaToInt(L, 1);
        int options = luaToInt(L, 2);
        int startIndex = luaToInt(L, 3);
        int ptCnt = luaToInt(L, 4);
        int floatOrIntCountPerPoint = 11;
        if (options & 2)
            floatOrIntCountPerPoint = 16;

        int res = checkOneGeneralInputArgument(L, 5, lua_arg_number, ptCnt * floatOrIntCountPerPoint, false, false, &errorString, argOffset);
        if (res == 2)
        {
            double* data = new double[ptCnt * floatOrIntCountPerPoint];
            getDoublesFromTable(L, 5, ptCnt * floatOrIntCountPerPoint, data);
            for (int i = 0; i < ptCnt; i++)
                ((int*)(data + floatOrIntCountPerPoint * i + 8))[0] = int(data[floatOrIntCountPerPoint * i + 8] + 0.5);
            if (options & 2)
            {
                for (int i = 0; i < ptCnt; i++)
                    ((int*)(data + floatOrIntCountPerPoint * i + 11))[0] =
                        int(data[floatOrIntCountPerPoint * i + 11] + 0.5);
            }
            retVal = CALL_C_API(simInsertPathCtrlPoints, pathHandle, options, startIndex, ptCnt, data);
            delete[] data;
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCutPathCtrlPoints(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.cutPathCtrlPoints");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        int pathHandle = luaToInt(L, 1);
        int startIndex = luaToInt(L, 2);
        int ptCnt = luaToInt(L, 3);
        retVal = CALL_C_API(simCutPathCtrlPoints, pathHandle, startIndex, ptCnt);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _sim_CreatePath(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim._createPath");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int attribs = luaToInt(L, 1);
        int* intP = nullptr;
        double* floatP = nullptr;
        int intParams[3];
        double floatParams[3];
        float* color = nullptr;
        float c[12];
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 3, true, true, &errorString, argOffset);
        if (res >= 0)
        {
            if (res == 2)
            {
                getIntsFromTable(L, 2, 3, intParams);
                intP = intParams;
            }
            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 3, true, true, &errorString, argOffset);
            if (res >= 0)
            {
                if (res == 2)
                {
                    getDoublesFromTable(L, 3, 3, floatParams);
                    floatP = floatParams;
                }
                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 12, true, true, &errorString, argOffset);
                if (res >= 0)
                {
                    if (res == 2)
                    {
                        getFloatsFromTable(L, 4, 12, c);
                        color = c;
                    }
                    retVal = CALL_C_API(simCreatePath, attribs, intP, floatP, color);
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetScriptExecutionCount(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.getScriptExecutionCount");

    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, it->getNumberOfPasses());
    LUA_END(1);
}

int _simIsScriptExecutionThreaded(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.isScriptExecutionThreaded");
    luaWrap_lua_pushinteger(L, 0);
    LUA_END(1);
}

int _simIsScriptRunningInThread(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.isScriptRunningInThread");
    luaWrap_lua_pushinteger(L, 0);
    LUA_END(1);
}

int _simSetThreadResumeLocation(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.setThreadResumeLocation");
    luaWrap_lua_pushinteger(L, 0);
    LUA_END(1);
}

int _simResumeThreads(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.resumeThreads");
    luaWrap_lua_pushinteger(L, 0);
    LUA_END(1);
}

int _simLaunchThreadedChildScripts(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.launchThreadedChildScripts");
    luaWrap_lua_pushinteger(L, 0);
    LUA_END(1);
}

int _simGetThreadId(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.getThreadId");

    int retVal = CALL_C_API(simGetThreadId, );

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetUserParameter(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.getUserParameter");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0))
    {
        int handle = luaToInt(L, 1);
        if (handle == sim_handle_self)
        {
            handle = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
            CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(handle);
            if (handle <= SIM_IDEND_SCENEOBJECT)
            { // is a CScript
                if (it->getParentIsProxy())
                {
                    CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(handle);
                    obj = obj->getParent();
                    if (obj != nullptr)
                        handle = obj->getObjectHandle();
                    else
                        handle = -1;
                }
            }
            else
                handle = it->getObjectHandleThatScriptIsAttachedTo(-1);
        }
        bool returnString = false;
        int ret = checkOneGeneralInputArgument(L, 3, lua_arg_bool, 0, true, false, &errorString, argOffset);
        if ((ret == 0) || (ret == 2))
        {
            if (ret == 2)
                returnString = luaToBool(L, 3);
            std::string parameterName(luaWrap_lua_tostring(L, 2));
            int l;
            char* p = CALL_C_API(simGetUserParameter, handle, parameterName.c_str(), &l);
            if (p != nullptr)
            {
                std::string a;
                a.assign(p, l);
                if (returnString)
                    luaWrap_lua_pushbinarystring(L, a.c_str(), a.length()); // push binary string for backw. comp.
                else
                    pushCorrectTypeOntoLuaStack_old(L, a);
                CALL_C_API(simReleaseBuffer, p);
                LUA_END(1);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetUserParameter(luaWrap_lua_State* L)
{ // deprecated on 01.10.2020
    TRACE_LUA_API;
    LUA_START("sim.setUserParameter");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0, lua_arg_string, 0))
    {
        int handle = luaToInt(L, 1);
        if (handle == sim_handle_self)
        {
            handle = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
            CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(handle);
            if (handle <= SIM_IDEND_SCENEOBJECT)
            { // is a CScript
                if (it->getParentIsProxy())
                {
                    CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(handle);
                    obj = obj->getParent();
                    if (obj != nullptr)
                        handle = obj->getObjectHandle();
                    else
                        handle = -1;
                }
            }
            else
                handle = it->getObjectHandleThatScriptIsAttachedTo(-1);
        }
        std::string parameterName(luaWrap_lua_tostring(L, 2));
        size_t parameterValueLength;
        char* parameterValue = (char*)luaWrap_lua_tobuffer(L, 3, &parameterValueLength);
        retVal = CALL_C_API(simSetUserParameter, handle, parameterName.c_str(), parameterValue, (int)parameterValueLength);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _genericFunctionHandler_old(luaWrap_lua_State* L, CScriptCustomFunction* func)
{ // deprecated around 2015
    TRACE_LUA_API;
    // We first read all arguments from the stack
    std::vector<char> inBoolVector;
    std::vector<int> inIntVector;
    std::vector<float> inFloatVector;
    std::vector<double> inDoubleVector;
    std::vector<std::string> inStringVector;
    std::vector<std::string> inCharVector;
    std::vector<int> inInfoVector;
    for (int i = 0; i < int(func->inputArgTypes.size()); i++)
    {
        if (!readCustomFunctionDataFromStack_old(L, i + 1, func->inputArgTypes[i], inBoolVector, inIntVector,
                                                 inFloatVector, inDoubleVector, inStringVector, inCharVector,
                                                 inInfoVector))
            break;
    }

    // Now we retrieve the object ID this script might be attached to:
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* itObj = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    int linkedObject = -1;
    if (itObj->getScriptType() == sim_scripttype_simulation)
    {
        CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(
            itObj->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_simulation));
        if (obj != nullptr)
            linkedObject = obj->getObjectHandle();
    }
    if (itObj->getScriptType() == sim_scripttype_customization)
    {
        CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(
            itObj->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_customization));
        if (obj != nullptr)
            linkedObject = obj->getObjectHandle();
    }
    // We prepare the callback structure:
    SLuaCallBack* p = new SLuaCallBack;
    p->objectID = linkedObject;
    p->scriptID = currentScriptID;
    p->inputBool = nullptr;
    p->inputInt = nullptr;
    p->inputFloat = nullptr;
    p->inputDouble = nullptr;
    p->inputChar = nullptr;
    p->inputCharBuff = nullptr;
    p->inputArgCount = 0;
    p->inputArgTypeAndSize = nullptr;
    p->outputBool = nullptr;
    p->outputInt = nullptr;
    p->outputFloat = nullptr;
    p->outputDouble = nullptr;
    p->outputChar = nullptr;
    p->outputCharBuff = nullptr;
    p->outputArgCount = 0;
    p->outputArgTypeAndSize = nullptr;
    p->waitUntilZero = 0;
    // Now we prepare the input buffers:
    p->inputBool = new bool[inBoolVector.size()];
    p->inputInt = new int[inIntVector.size()];
    p->inputFloat = new float[inFloatVector.size()];
    p->inputDouble = new double[inDoubleVector.size()];
    int charCnt = 0;
    for (size_t k = 0; k < inStringVector.size(); k++)
        charCnt += (int)inStringVector[k].length() + 1; // terminal 0
    p->inputChar = new char[charCnt];

    int charBuffCnt = 0;
    for (size_t k = 0; k < inCharVector.size(); k++)
        charBuffCnt += (int)inCharVector[k].length();
    p->inputCharBuff = new char[charBuffCnt];

    p->inputArgCount = (int)inInfoVector.size() / 2;
    p->inputArgTypeAndSize = new int[inInfoVector.size()];
    // We fill the input buffers:
    for (int k = 0; k < int(inBoolVector.size()); k++)
        p->inputBool[k] = inBoolVector[k];
    for (int k = 0; k < int(inIntVector.size()); k++)
        p->inputInt[k] = inIntVector[k];
    for (int k = 0; k < int(inFloatVector.size()); k++)
        p->inputFloat[k] = inFloatVector[k];
    for (int k = 0; k < int(inDoubleVector.size()); k++)
        p->inputDouble[k] = inDoubleVector[k];
    charCnt = 0;
    for (int k = 0; k < int(inStringVector.size()); k++)
    {
        for (int l = 0; l < int(inStringVector[k].length()); l++)
            p->inputChar[charCnt + l] = inStringVector[k][l];
        charCnt += (int)inStringVector[k].length();
        // terminal 0:
        p->inputChar[charCnt] = 0;
        charCnt++;
    }

    charBuffCnt = 0;
    for (int k = 0; k < int(inCharVector.size()); k++)
    {
        for (int l = 0; l < int(inCharVector[k].length()); l++)
            p->inputCharBuff[charBuffCnt + l] = inCharVector[k][l];
        charBuffCnt += (int)inCharVector[k].length();
    }

    for (int k = 0; k < int(inInfoVector.size()); k++)
        p->inputArgTypeAndSize[k] = inInfoVector[k];

    // Now we can call the callback:
    func->callBackFunction_old(p);

    // We first delete the input buffers:
    delete[] p->inputBool;
    delete[] p->inputInt;
    delete[] p->inputFloat;
    delete[] p->inputDouble;
    delete[] p->inputChar;
    delete[] p->inputCharBuff;
    delete[] p->inputArgTypeAndSize;
    // Now we have to build the returned data onto the stack:
    if (p->outputArgCount != 0)
    {
        int boolPt = 0;
        int intPt = 0;
        int floatPt = 0;
        int doublePt = 0;
        int stringPt = 0;
        int stringBuffPt = 0;
        for (int i = 0; i < p->outputArgCount; i++)
        {
            writeCustomFunctionDataOntoStack_old(L, p->outputArgTypeAndSize[2 * i + 0],
                                                 p->outputArgTypeAndSize[2 * i + 1], p->outputBool, boolPt,
                                                 p->outputInt, intPt, p->outputFloat, floatPt, p->outputDouble,
                                                 doublePt, p->outputChar, stringPt, p->outputCharBuff, stringBuffPt);
        }
    }
    // We now delete the output buffers:
    delete[] p->outputBool;
    delete[] p->outputInt;
    delete[] p->outputFloat;
    delete[] p->outputDouble;
    delete[] p->outputChar;
    delete[] p->outputCharBuff;
    delete[] p->outputArgTypeAndSize;
    // And we return the number of arguments:
    int outputArgCount = p->outputArgCount;
    delete p;
    if (outputArgCount == 0)
    {
        luaWrap_lua_pushnil(L);
        outputArgCount = 1;
    }
    return (outputArgCount);
}

int _simSetCollectionName(luaWrap_lua_State* L)
{ // deprecated on 17.11.2020
    TRACE_LUA_API;
    LUA_START("sim.setCollectionName");
    int retVal = -1; // error

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0))
        retVal = CALL_C_API(simSetCollectionName, luaToInt(L, 1), luaWrap_lua_tostring(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _sim_CreateCollection(luaWrap_lua_State* L)
{ // deprecated on 17.11.2020
    TRACE_LUA_API;
    LUA_START("sim._createCollection");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0, lua_arg_number, 0))
    {
        std::string collName(luaWrap_lua_tostring(L, 1));
        int options = luaToInt(L, 2);
        retVal = CALL_C_API(simCreateCollection, collName.c_str(), options);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAddObjectToCollection(luaWrap_lua_State* L)
{ // deprecated on 17.11.2020
    TRACE_LUA_API;
    LUA_START("sim.addObjectToCollection");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number,
                            0))
    {
        int collHandle = luaToInt(L, 1);
        int objHandle = luaToInt(L, 2);
        int what = luaToInt(L, 3);
        int options = luaToInt(L, 4);
        retVal = CALL_C_API(simAddObjectToCollection, collHandle, objHandle, what, options);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetCollectionHandle(luaWrap_lua_State* L)
{ // deprecated on 17.11.2020
    TRACE_LUA_API;
    LUA_START("sim.getCollectionHandle");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        std::string name(luaWrap_lua_tostring(L, 1));
        setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                                   CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                       L)); // for transmitting to the master function additional info (e.g.for autom.
                                            // name adjustment, or for autom. object deletion when script ends)
        retVal = CALL_C_API(simGetCollectionHandle, name.c_str());
        setCurrentScriptInfo_cSide(-1, -1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simRemoveCollection(luaWrap_lua_State* L)
{ // deprecated on 17.11.2020
    TRACE_LUA_API;
    LUA_START("sim.removeCollection");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simRemoveCollection, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simEmptyCollection(luaWrap_lua_State* L)
{ // deprecated on 17.11.2020
    TRACE_LUA_API;
    LUA_START("sim.emptyCollection");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simEmptyCollection, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetCollectionName(luaWrap_lua_State* L)
{ // deprecated on 17.11.2020
    TRACE_LUA_API;
    LUA_START("sim.getCollectionName");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        char* name = CALL_C_API(simGetCollectionName, luaToInt(L, 1));
        if (name != nullptr)
        {
            luaWrap_lua_pushtext(L, name);
            CALL_C_API(simReleaseBuffer, name);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simHandleCollision(luaWrap_lua_State* L)
{ // deprecated on 20.11.2020
    TRACE_LUA_API;
    LUA_START("sim.handleCollision");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int objHandle = luaToInt(L, 1);
        retVal = CALL_C_API(simHandleCollision, objHandle);
        if ((retVal > 0) && (objHandle >= 0))
        {
            int collObjHandles[2];
            CCollisionObject_old* it = App::currentWorld->collisions_old->getObjectFromHandle(objHandle);
            if (it != nullptr)
            {
                it->readCollision(collObjHandles);
                luaWrap_lua_pushinteger(L, retVal);
                pushIntTableOntoStack(L, 2, collObjHandles);
                LUA_END(2);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simReadCollision(luaWrap_lua_State* L)
{ // deprecated on 20.11.2020
    TRACE_LUA_API;
    LUA_START("sim.readCollision");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int objHandle = luaToInt(L, 1);
        retVal = CALL_C_API(simReadCollision, objHandle);
        if (retVal > 0)
        {
            int collObjHandles[2];
            CCollisionObject_old* it = App::currentWorld->collisions_old->getObjectFromHandle(objHandle);
            if (it != nullptr)
            {
                it->readCollision(collObjHandles);
                luaWrap_lua_pushinteger(L, retVal);
                pushIntTableOntoStack(L, 2, collObjHandles);
                LUA_END(2);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simHandleDistance(luaWrap_lua_State* L)
{ // deprecated on 20.11.2020
    TRACE_LUA_API;
    LUA_START("sim.handleDistance");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        double d;
        retVal = CALL_C_API(simHandleDistance, luaToInt(L, 1), &d);
        if (retVal == 1)
        {
            luaWrap_lua_pushinteger(L, retVal);
            luaWrap_lua_pushnumber(L, d);
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simReadDistance(luaWrap_lua_State* L)
{ // deprecated on 20.11.2020
    TRACE_LUA_API;
    LUA_START("sim.readDistance");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        double d;
        retVal = CALL_C_API(simReadDistance, luaToInt(L, 1), &d);
        if (retVal == 1)
        {
            luaWrap_lua_pushinteger(L, retVal);
            luaWrap_lua_pushnumber(L, d);
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetCollisionHandle(luaWrap_lua_State* L)
{ // deprecated on 20.11.2020
    TRACE_LUA_API;
    LUA_START("sim.getCollisionHandle");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        std::string name(luaWrap_lua_tostring(L, 1));
        setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                                   CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                       L)); // for transmitting to the master function additional info (e.g.for autom.
                                            // name adjustment, or for autom. object deletion when script ends)
        retVal = CALL_C_API(simGetCollisionHandle, name.c_str());
        setCurrentScriptInfo_cSide(-1, -1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetDistanceHandle(luaWrap_lua_State* L)
{ // deprecated on 20.11.2020
    TRACE_LUA_API;
    LUA_START("sim.getDistanceHandle");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        std::string name(luaWrap_lua_tostring(L, 1));
        setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                                   CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                       L)); // for transmitting to the master function additional info (e.g.for autom.
                                            // name adjustment, or for autom. object deletion when script ends)
        retVal = CALL_C_API(simGetDistanceHandle, name.c_str());
        setCurrentScriptInfo_cSide(-1, -1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simResetCollision(luaWrap_lua_State* L)
{ // deprecated on 20.11.2020
    TRACE_LUA_API;
    LUA_START("sim.resetCollision");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simResetCollision, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simResetDistance(luaWrap_lua_State* L)
{ // deprecated on 20.11.2020
    TRACE_LUA_API;
    LUA_START("sim.resetDistance");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simResetDistance, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAddBanner(luaWrap_lua_State* L)
{ // deprecated on 23.11.2020
    TRACE_LUA_API;
    LUA_START("sim.addBanner");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        std::string label(luaWrap_lua_tostring(L, 1));
        double size = luaToDouble(L, 2);
        int options = luaToInt(L, 3);

        double* positionAndEulerAngles = nullptr;
        int parentObjectHandle = -1;
        float* labelColors = nullptr;
        float* backgroundColors = nullptr;

        int res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 6, true, true, &errorString, argOffset);
        int okToGo = (res != -1);
        if (okToGo)
        {
            double positionAndEulerAnglesC[6];
            if (res > 0)
            {
                if (res == 2)
                {
                    getDoublesFromTable(L, 4, 6, positionAndEulerAnglesC);
                    positionAndEulerAngles = positionAndEulerAnglesC;
                }
                res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, true, true, &errorString, argOffset);
                okToGo = (res != -1);
                if (okToGo)
                {
                    if (res > 0)
                    {
                        if (res == 2)
                            parentObjectHandle = luaToInt(L, 5);
                        res = checkOneGeneralInputArgument(L, 6, lua_arg_number, 12, true, true, &errorString, argOffset);
                        okToGo = (res != -1);
                        if (okToGo)
                        {
                            float labelColorsC[12];
                            if (res > 0)
                            {
                                if (res == 2)
                                {
                                    getFloatsFromTable(L, 6, 12, labelColorsC);
                                    labelColors = labelColorsC;
                                }
                                res = checkOneGeneralInputArgument(L, 7, lua_arg_number, 12, true, true, &errorString, argOffset);
                                okToGo = (res != -1);
                                if (okToGo)
                                {
                                    float backgroundColorsC[12];
                                    if (res > 0)
                                    {
                                        if (res == 2)
                                        {
                                            getFloatsFromTable(L, 7, 12, backgroundColorsC);
                                            backgroundColors = backgroundColorsC;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (okToGo)
            {
                retVal = CALL_C_API(simAddBanner, label.c_str(), size, options, positionAndEulerAngles, parentObjectHandle,
                                               labelColors, backgroundColors);
                if (retVal != -1)
                { // following condition added on 2011/01/06 so as to not remove objects created from the c/c++
                    // interface or an add-on:
                    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
                    CScriptObject* itScrObj = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
                    CBannerObject* anObj = App::currentWorld->bannerCont_old->getObject(retVal);
                    if (anObj != nullptr)
                        anObj->setCreatedFromScript((itScrObj->getScriptType() == sim_scripttype_main) ||
                                                    (itScrObj->getScriptType() == sim_scripttype_simulation));
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simRemoveBanner(luaWrap_lua_State* L)
{ // deprecated on 23.11.2020
    TRACE_LUA_API;
    LUA_START("sim.removeBanner");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int objectHandle = luaToInt(L, 1);
        if (objectHandle == sim_handle_all)
        { // following condition added here on 2011/01/06 so as not to remove objects created from a C/c++ call
            App::currentWorld->bannerCont_old->eraseAllObjects(true);
            retVal = 1;
        }
        else
            retVal = CALL_C_API(simRemoveBanner, objectHandle);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAddGhost(luaWrap_lua_State* L)
{ // deprecated on 23.11.2020
    TRACE_LUA_API;
    LUA_START("sim.addGhost");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0,
                            lua_arg_number, 0))
    {
        int ghostGroup = luaToInt(L, 1);
        int objectHandle = luaToInt(L, 2);
        int options = luaToInt(L, 3);
        double startTime = luaToDouble(L, 4);
        double endTime = luaToDouble(L, 5);
        int res = checkOneGeneralInputArgument(L, 6, lua_arg_number, 12, true, true, &errorString, argOffset);
        if (res >= 0)
        {
            bool defaultColors = true;
            float color[12];
            if (res == 2)
            {
                defaultColors = false;
                getFloatsFromTable(L, 6, 12, color);
            }
            if (defaultColors)
                retVal = CALL_C_API(simAddGhost, ghostGroup, objectHandle, options, startTime, endTime, nullptr);
            else
                retVal = CALL_C_API(simAddGhost, ghostGroup, objectHandle, options, startTime, endTime, color);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simModifyGhost(luaWrap_lua_State* L)
{ // deprecated on 23.11.2020
    TRACE_LUA_API;
    LUA_START("sim.modifyGhost");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number,
                            0))
    {
        int ghostGroup = luaToInt(L, 1);
        int ghostId = luaToInt(L, 2);
        int operation = luaToInt(L, 3);
        double floatValue = luaToDouble(L, 4);

        int options = 0;
        int optionsMask = 0;
        int res = 0;
        if (operation == 10)
            res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, false, false, &errorString, argOffset);
        if (res >= 0)
        {
            if ((res == 2) && (operation == 10))
            {
                options = luaToInt(L, 5);
                res = checkOneGeneralInputArgument(L, 6, lua_arg_number, 0, false, false, &errorString, argOffset);
                if (res == 2)
                    optionsMask = luaToInt(L, 6);
                else
                    res = -1;
            }
            if (res >= 0)
            {
                int floatCnt = 7;
                if (operation == 13)
                    floatCnt = 12;
                res = 0;
                if ((operation >= 11) && (operation <= 13))
                    res = checkOneGeneralInputArgument(L, 7, lua_arg_number, floatCnt, false, false, &errorString, argOffset);
                if (res >= 0)
                {
                    double colorOrTransfData[12];
                    if ((res == 2) && (operation >= 11) && (operation <= 13))
                        getDoublesFromTable(L, 7, floatCnt, colorOrTransfData);
                    retVal = CALL_C_API(simModifyGhost, ghostGroup, ghostId, operation, floatValue, options, optionsMask,
                                                     colorOrTransfData);
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetGraphUserData(luaWrap_lua_State* L)
{ // deprecated on 23.11.2020
    TRACE_LUA_API;
    LUA_START("sim.setGraphUserData");

    int retVal = -1; // for error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0, lua_arg_number, 0))
    {
        int graphHandle = luaToInt(L, 1);
        std::string dataName(luaWrap_lua_tostring(L, 2));
        double data = luaToDouble(L, 3);
        retVal = CALL_C_API(simSetGraphUserData, graphHandle, dataName.c_str(), data);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAddPointCloud(luaWrap_lua_State* L)
{ // deprecated on 23.11.2020
    TRACE_LUA_API;
    LUA_START("sim.addPointCloud");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0,
                            lua_arg_number, 0, lua_arg_number, 3))
    {
        int pageMask = luaToInt(L, 1);
        int layerMask = luaToInt(L, 2);
        int objectHandle = luaToInt(L, 3);
        int options = luaToInt(L, 4);
        int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
        if ((it->getScriptType() == sim_scripttype_main) || (it->getScriptType() == sim_scripttype_simulation))
            options = (options | 1) -
                      1; // cloud is automatically removed at the end of the simulation (i.e. is not persistent)
        double pointSize = luaToDouble(L, 5);
        int pointCnt = (int)luaWrap_lua_rawlen(L, 6) / 3;
        std::vector<double> pointCoordinates(pointCnt * 3, 0.0);
        getDoublesFromTable(L, 6, pointCnt * 3, &pointCoordinates[0]);
        int res;
        res = checkOneGeneralInputArgument(L, 7, lua_arg_number, 12, true, true, &errorString, argOffset);
        if (res >= 0)
        {
            unsigned char* defaultColors = nullptr;
            std::vector<unsigned char> _defCols(12, 0);
            if (res == 2)
            {
                std::vector<int> _defCols_(12, 0);
                getIntsFromTable(L, 7, 12, &_defCols_[0]);
                for (int i = 0; i < 12; i++)
                    _defCols[i] = (unsigned char)_defCols_[i];
                defaultColors = &_defCols[0];
            }
            res = checkOneGeneralInputArgument(L, 8, lua_arg_number, pointCnt * 3, true, true, &errorString, argOffset);
            if (res >= 0)
            {
                unsigned char* pointColors = nullptr;
                std::vector<unsigned char> _pointCols;
                if (res == 2)
                {
                    _pointCols.resize(pointCnt * 3, 0);
                    std::vector<int> _pointCols_(pointCnt * 3, 0);
                    getIntsFromTable(L, 8, pointCnt * 3, &_pointCols_[0]);
                    for (int i = 0; i < pointCnt * 3; i++)
                        _pointCols[i] = (unsigned char)_pointCols_[i];
                    pointColors = &_pointCols[0];
                }
                res = checkOneGeneralInputArgument(L, 9, lua_arg_number, pointCnt * 3, true, true, &errorString, argOffset);
                if (res >= 0)
                {
                    double* pointNormals = nullptr;
                    std::vector<double> _pointNormals;
                    if (res == 2)
                    {
                        _pointNormals.resize(pointCnt * 3, 0);
                        getDoublesFromTable(L, 9, pointCnt * 3, &_pointNormals[0]);
                        pointNormals = &_pointNormals[0];
                    }
                    retVal = CALL_C_API(simAddPointCloud, pageMask, layerMask, objectHandle, options, pointSize,
                                                       (int)pointCoordinates.size() / 3, &pointCoordinates[0],
                                                       (char*)defaultColors, (char*)pointColors, pointNormals);
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simModifyPointCloud(luaWrap_lua_State* L)
{ // deprecated on 23.11.2020
    TRACE_LUA_API;
    LUA_START("sim.modifyPointCloud");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int pointCloudHandle = luaToInt(L, 1);
        int operation = luaToInt(L, 2);
        retVal = CALL_C_API(simModifyPointCloud, pointCloudHandle, operation, nullptr, nullptr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCopyMatrix(luaWrap_lua_State* L)
{ // deprecated on 23.11.2020
    TRACE_LUA_API;
    LUA_START("sim.copyMatrix");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 12))
    {
        double arr[12];
        getDoublesFromTable(L, 1, 12, arr);
        pushDoubleTableOntoStack(L, 12, arr);
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetObjectInt32Parameter(luaWrap_lua_State* L)
{ // deprecated on 22.04.2021
    TRACE_LUA_API;
    LUA_START("sim.getObjectInt32Parameter");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int param;
        retVal = CALL_C_API(simGetObjectInt32Param, luaToInt(L, 1), luaToInt(L, 2), &param);
        if (retVal > 0)
        {
            luaWrap_lua_pushinteger(L, retVal);
            luaWrap_lua_pushinteger(L, param);
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectFloatParameter(luaWrap_lua_State* L)
{ // deprecated on 22.04.2021
    TRACE_LUA_API;
    LUA_START("sim.getObjectFloatParameter");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        double param;
        retVal = CALL_C_API(simGetObjectFloatParam, luaToInt(L, 1), luaToInt(L, 2), &param);
        if (retVal > 0)
        {
            luaWrap_lua_pushinteger(L, retVal);
            luaWrap_lua_pushnumber(L, param);
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simIsHandleValid(luaWrap_lua_State* L)
{ // deprecated on 23.04.2021
    TRACE_LUA_API;
    LUA_START("sim.isHandleValid");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int objType = -1;
        int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString, argOffset);
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                objType = luaToInt(L, 2);
            retVal = CALL_C_API(simIsHandle, luaToInt(L, 1), objType);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectName(luaWrap_lua_State* L)
{ // deprecated on 08.06.2021
    TRACE_LUA_API;
    LUA_START("sim.getObjectName");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        char* name = CALL_C_API(simGetObjectName, luaToInt(L, 1));
        if (name != nullptr)
        {
            luaWrap_lua_pushtext(L, name);
            CALL_C_API(simReleaseBuffer, name);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectName(luaWrap_lua_State* L)
{ // deprecated on 08.06.2021
    TRACE_LUA_API;
    LUA_START("sim.setObjectName");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0))
        retVal = CALL_C_API(simSetObjectName, luaToInt(L, 1), luaWrap_lua_tostring(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetScriptName(luaWrap_lua_State* L)
{ // deprecated on 08.06.2021
    TRACE_LUA_API;
    LUA_START("sim.getScriptName");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int a = luaToInt(L, 1);
        if (a == sim_handle_self)
            a = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        char* name = CALL_C_API(simGetScriptName, a);
        if (name != nullptr)
        {
            luaWrap_lua_pushtext(L, name);
            CALL_C_API(simReleaseBuffer, name);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetScriptVariable(luaWrap_lua_State* L)
{ // deprecated on 16.06.2021
    TRACE_LUA_API;
    LUA_START("sim.setScriptVariable");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0, lua_arg_number, 0))
    {
        std::string varAndScriptName(luaWrap_lua_tostring(L, 1));
        int scriptHandleOrType = luaToInt(L, 2);
        int numberOfArguments = luaWrap_lua_gettop(L);
        if (numberOfArguments >= 3)
        {
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
            CScriptObject::buildFromInterpreterStack_lua(L, stack, 3, 1);
            retVal = CALL_C_API(simSetScriptVariable, scriptHandleOrType, varAndScriptName.c_str(), stack->getId());
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        else
            errorString = SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectAssociatedWithScript(luaWrap_lua_State* L)
{ // deprecated on 18.06.2021
    TRACE_LUA_API;
    LUA_START("sim.getObjectAssociatedWithScript");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int a = luaToInt(L, 1);
        if (a == sim_handle_self)
            a = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        retVal = CALL_C_API(simGetObjectAssociatedWithScript, a);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetScriptAssociatedWithObject(luaWrap_lua_State* L)
{ // deprecated on 18.06.2021
    TRACE_LUA_API;
    LUA_START("sim.getScriptAssociatedWithObject");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simGetScriptAssociatedWithObject, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetCustomizationScriptAssociatedWithObject(luaWrap_lua_State* L)
{ // deprecated on 18.06.2021
    TRACE_LUA_API;
    LUA_START("sim.getCustomizationScriptAssociatedWithObject");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simGetCustomizationScriptAssociatedWithObject, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectConfiguration(luaWrap_lua_State* L)
{ // deprecated on 18.06.2021
    TRACE_LUA_API;
    LUA_START("sim.getObjectConfiguration");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        char* data = CALL_C_API(simGetObjectConfiguration, luaToInt(L, 1));
        if (data != nullptr)
        {
            int dataSize = ((int*)data)[0] + 4;
            luaWrap_lua_pushbuffer(L, data, dataSize);
            delete[] data;
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectConfiguration(luaWrap_lua_State* L)
{ // deprecated on 18.06.2021
    TRACE_LUA_API;
    LUA_START("sim.setObjectConfiguration");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        size_t l;
        const char* data = luaWrap_lua_tobuffer(L, 1, &l);
        retVal = CALL_C_API(simSetObjectConfiguration, data);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetConfigurationTree(luaWrap_lua_State* L)
{ // deprecated on 18.06.2021
    TRACE_LUA_API;
    LUA_START("sim.getConfigurationTree");

    CScriptObject* it =
        App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int id = luaToInt(L, 1);
        if (id == sim_handle_self)
        {
            int objID = it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_simulation);
            id = objID;
            if (id != -1)
            {
                CScript* s = App::currentWorld->sceneObjects->getScriptFromHandle(id);
                if (s != nullptr)
                {
                    if (s->scriptObject->getParentIsProxy())
                    {
                        CSceneObject* o = s->getParent();
                        if (o != nullptr)
                            id = o->getObjectHandle();
                        else
                            id = -1;
                    }
                }
            }
            if (id == -1)
                errorString = SIM_ERROR_ARGUMENT_VALID_ONLY_WITH_CHILD_SCRIPTS;
        }
        if (id != -1)
        {
            char* data = CALL_C_API(simGetConfigurationTree, id);
            if (data != nullptr)
            {
                int dataSize = ((int*)data)[0] + 4;
                luaWrap_lua_pushbuffer(L, data, dataSize);
                delete[] data;
                LUA_END(1);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetConfigurationTree(luaWrap_lua_State* L)
{ // deprecated on 18.06.2021
    TRACE_LUA_API;
    LUA_START("sim.setConfigurationTree");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        size_t l;
        const char* data = luaWrap_lua_tobuffer(L, 1, &l);
        retVal = CALL_C_API(simSetConfigurationTree, data);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetObjectSizeValues(luaWrap_lua_State* L)
{ // deprecated on 28.06.2021
    TRACE_LUA_API;
    LUA_START("sim.setObjectSizeValues");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 3))
    {
        int handle = luaToInt(L, 1);
        double s[3];
        getDoublesFromTable(L, 2, 3, s);
        retVal = CALL_C_API(simSetObjectSizeValues, handle, s);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectSizeValues(luaWrap_lua_State* L)
{ // deprecated on 28.06.2021
    TRACE_LUA_API;
    LUA_START("sim.getObjectSizeValues");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        double s[3];
        if (CALL_C_API(simGetObjectSizeValues, handle, s) != -1)
        {
            pushDoubleTableOntoStack(L, 3, s);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simOpenModule(luaWrap_lua_State* L)
{ // deprecated in 2019-2020 sometimes
    TRACE_LUA_API;
    LUA_START("sim.openModule");

    moduleCommonPart_old(L, sim_message_eventcallback_moduleopen, &errorString);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(1);
}
int _simCloseModule(luaWrap_lua_State* L)
{ // deprecated in 2019-2020 sometimes
    TRACE_LUA_API;
    LUA_START("sim.closeModule");

    moduleCommonPart_old(L, sim_message_eventcallback_moduleclose, &errorString);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(1);
}
int _simHandleModule(luaWrap_lua_State* L)
{ // deprecated in 2019-2020 sometimes
    TRACE_LUA_API;
    LUA_START_NO_CSIDE_ERROR("sim.handleModule");
    bool sensingPart = false;
    int res = checkOneGeneralInputArgument(L, 2, lua_arg_bool, 0, true, false, &errorString, argOffset);

    if ((res == 0) || (res == 2))
    {
        if (res == 2)
            sensingPart = luaToBool(L, 2);
        if (sensingPart)
            moduleCommonPart_old(L, sim_message_eventcallback_modulehandleinsensingpart, &errorString);
        else
            moduleCommonPart_old(L, sim_message_eventcallback_modulehandle, &errorString);
    }
    else
        luaWrap_lua_pushinteger(L, -1);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(1);
}
void moduleCommonPart_old(luaWrap_lua_State* L, int action, std::string* errorString)
{ // deprecated in 2019-2020 sometimes
    TRACE_LUA_API;
    std::string functionName;
    if (action == sim_message_eventcallback_moduleopen)
        functionName = "sim.openModule";
    if (action == sim_message_eventcallback_moduleclose)
        functionName = "sim.closeModule";
    if ((action == sim_message_eventcallback_modulehandle) ||
        (action == sim_message_eventcallback_modulehandleinsensingpart))
        functionName = "sim.handleModule";
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    if (it->getScriptType() != sim_scripttype_main)
    {
        if (errorString != nullptr)
            errorString->assign(SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT);
        luaWrap_lua_pushinteger(L, -1);
    }
    else
    {
        bool handleAll = false;
        if (luaWrap_lua_isnumber(L, 1))
        { // We try to check whether we have sim_handle_all as a number:
            if (luaToInt(L, 1) == sim_handle_all)
            {
                handleAll = true;
                App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(action);
                luaWrap_lua_pushinteger(L, 1);
            }
        }
        if (!handleAll)
            luaWrap_lua_pushinteger(L, 1);
    }
}
int _simGetLastError(luaWrap_lua_State* L)
{ // deprecated on 01.07.2021
    TRACE_LUA_API;
    LUA_START("sim.getLastError");

    int scriptHandle = -1;
    if (luaWrap_lua_gettop(L) != 0)
    {
        if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
            scriptHandle = luaToInt(L, 1);
    }
    else
        scriptHandle = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(scriptHandle);
    if (it != nullptr)
    {
        luaWrap_lua_pushtext(L, "");
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}
int _simSwitchThread(luaWrap_lua_State* L)
{ // now implemented in Lua, except for old threads. Deprecated since V4.2.0
    TRACE_LUA_API;
    LUA_START("sim._switchThread");
    luaWrap_lua_pushinteger(L, 0);
    LUA_END(1);
}
int _simFileDialog(luaWrap_lua_State* L)
{ // deprecated
    TRACE_LUA_API;
    LUA_START("sim.fileDialog");
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0, lua_arg_string, 0, lua_arg_string, 0,
                            lua_arg_string, 0, lua_arg_string, 0))
    {
        int mode = luaToInt(L, 1);
        std::string title(luaWrap_lua_tostring(L, 2));
        std::string startPath(luaWrap_lua_tostring(L, 3));
        std::string initName(luaWrap_lua_tostring(L, 4));
        std::string extName(luaWrap_lua_tostring(L, 5));
        std::string ext(luaWrap_lua_tostring(L, 6));
        char* pathAndName = CALL_C_API(simFileDialog, mode, title.c_str(), startPath.c_str(), initName.c_str(),
                                                   extName.c_str(), ext.c_str());
        if (pathAndName != nullptr)
        {
            luaWrap_lua_pushtext(L, pathAndName);
            CALL_C_API(simReleaseBuffer, pathAndName);
            LUA_END(1);
        }
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}
int _simMsgBox(luaWrap_lua_State* L)
{ // deprecated
    TRACE_LUA_API;
    LUA_START("sim.msgBox");
    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_string, 0, lua_arg_string,
                            0))
    {
        int dlgType = luaToInt(L, 1);
        int dlgButtons = luaToInt(L, 2);
        std::string title(luaWrap_lua_tostring(L, 3));
        std::string message(luaWrap_lua_tostring(L, 4));
        retVal = CALL_C_API(simMsgBox, dlgType, dlgButtons, title.c_str(), message.c_str());
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simIsObjectInSelection(luaWrap_lua_State* L)
{ // deprecated since 24.09.2021
    TRACE_LUA_API;
    LUA_START("sim.isObjectInSelection");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simIsObjectInSelection, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAddObjectToSelection(luaWrap_lua_State* L)
{ // deprecated since 24.09.2021
    TRACE_LUA_API;
    LUA_START("sim.addObjectToSelection");

    int retVal = -1; // error
    // We check if argument 1 is nil (special case):
    if (checkOneGeneralInputArgument(L, 1, lua_arg_nil, 0, false, true, nullptr, argOffset) ==
        1) // we do not generate an error message!
    {
        retVal = 1; // nothing happens
    }
    else
    {
        // We check if we have a table at position 1:
        if (!luaWrap_lua_isnonbuffertable(L, 1))
        {                                                                              // It is not a table!
            if (checkInputArguments(L, nullptr, argOffset, lua_arg_number, 0, lua_arg_number, 0)) // we don't generate an error
                retVal = CALL_C_API(simAddObjectToSelection, luaToInt(L, 1), luaToInt(L, 2));
            else
            { // Maybe we have a special case with one argument only?
                // nil is a valid argument!
                if (checkInputArguments(L, nullptr, argOffset, lua_arg_nil, 0)) // we don't generate an error
                    retVal = 1;
                else
                {
                    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
                    {
                        if (luaToInt(L, 1) == sim_handle_all)
                            retVal = CALL_C_API(simAddObjectToSelection, luaToInt(L, 1), -1);
                        else
                            checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number,
                                                0); // we just generate an error
                    }
                }
            }
        }
        else
        { // Ok we have a table. Now what size is it?
            int tableLen = int(luaWrap_lua_rawlen(L, 1));
            int* buffer = new int[tableLen];
            if (getIntsFromTable(L, 1, tableLen, buffer))
            {
                for (int i = 0; i < tableLen; i++)
                {
                    if (App::currentWorld->sceneObjects->getObjectFromHandle(buffer[i]) != nullptr)
                        App::currentWorld->sceneObjects->addObjectToSelection(buffer[i]);
                }
                retVal = 1;
            }
            else
                errorString = SIM_ERROR_TABLE_CONTAINS_INVALID_TYPES;
            delete[] buffer;
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simRemoveObjectFromSelection(luaWrap_lua_State* L)
{ // deprecated since 24.09.2021
    TRACE_LUA_API;
    LUA_START("sim.removeObjectFromSelection");

    int retVal = -1; // error
    // We check if argument 1 is nil (special case):
    if (checkOneGeneralInputArgument(L, 1, lua_arg_nil, 0, false, true, nullptr, argOffset) ==
        1)          // we do not generate an error message!
        retVal = 1; // nothing happens
    else
    {
        // We check if we have a table at position 1:
        if (!luaWrap_lua_isnonbuffertable(L, 1))
        {                                                                              // It is not a table!
            if (checkInputArguments(L, nullptr, argOffset, lua_arg_number, 0, lua_arg_number, 0)) // we don't generate an error
                retVal = CALL_C_API(simRemoveObjectFromSelection, luaToInt(L, 1), luaToInt(L, 2));
            else
            {
                if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
                {
                    if (luaToInt(L, 1) == sim_handle_all)
                        retVal = CALL_C_API(simRemoveObjectFromSelection, luaToInt(L, 1), -1);
                    else
                        checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number,
                                            0); // we just generate an error
                }
            }
        }
        else
        { // Ok we have a table. Now what size is it?
            int tableLen = int(luaWrap_lua_rawlen(L, 1));
            int* buffer = new int[tableLen];
            if (getIntsFromTable(L, 1, tableLen, buffer))
            {
                for (int i = 0; i < tableLen; i++)
                    retVal = CALL_C_API(simRemoveObjectFromSelection, sim_handle_single, buffer[i]);
            }
            else
                errorString = SIM_ERROR_TABLE_CONTAINS_INVALID_TYPES;
            delete[] buffer;
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectUniqueIdentifier(luaWrap_lua_State* L)
{ // deprecated since 08.10.2021
    TRACE_LUA_API;
    LUA_START("sim.getObjectUniqueIdentifier");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        if (handle != sim_handle_all)
        {
            int retVal;
            if (CALL_C_API(simGetObjectUniqueIdentifier, handle, &retVal) != -1)
            {
                luaWrap_lua_pushinteger(L, retVal);
                LUA_END(1);
            }
        }
        else
        { // for backward compatibility
            int cnt = int(App::currentWorld->sceneObjects->getObjectCount());
            int* buffer = new int[cnt];
            if (CALL_C_API(simGetObjectUniqueIdentifier, handle, buffer) != -1)
            {
                pushIntTableOntoStack(L, cnt, buffer);
                delete[] buffer;
                LUA_END(1);
            }
            delete[] buffer;
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simBreakForceSensor(luaWrap_lua_State* L)
{ // deprecated since 08.11.2021
    TRACE_LUA_API;
    LUA_START("sim.breakForceSensor");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simBreakForceSensor, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetSphericalJointMatrix(luaWrap_lua_State* L)
{ // deprecated since 09.11.2021
    TRACE_LUA_API;
    LUA_START("sim.setSphericalJointMatrix");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 12))
    {
        double arr[12];
        getDoublesFromTable(L, 2, 12, arr);
        retVal = CALL_C_API(simSetSphericalJointMatrix, luaToInt(L, 1), arr);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetJointMatrix(luaWrap_lua_State* L)
{ // deprecated since 09.11.2021
    TRACE_LUA_API;
    LUA_START("sim.getJointMatrix");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        double arr[12];
        if (CALL_C_API(simGetJointMatrix, luaToInt(L, 1), arr) == 1)
        {
            pushDoubleTableOntoStack(L, 12, arr); // Success
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _sim_getObjectHandle(luaWrap_lua_State* L)
{ // deprecated on 03.12.2021
    TRACE_LUA_API;
    LUA_START("sim._getObjectHandle");

    int retVal = -1; // means error

    bool checkWithString = true;
    if (checkInputArguments(L, nullptr, argOffset, lua_arg_integer, 0)) // do not output error if not string
    {                                                        // argument sim.handle_self
        if (luaToInt(L, 1) == sim_handle_self)
        {
            checkWithString = false;
            int a = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
            if (a <= SIM_IDEND_SCENEOBJECT)
            { // is a CScript
                CScriptObject* it = App::currentWorld->sceneObjects->getScriptObjectFromHandle(a);
                if (it->getParentIsProxy())
                {
                    CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(a);
                    obj = obj->getParent();
                    if (obj != nullptr)
                        retVal = obj->getObjectHandle();
                    else
                        retVal = -1;
                }
                else
                    retVal = a;
            }
            else
                retVal = CALL_C_API(simGetObjectAssociatedWithScript, a);
        }
    }
    if (checkWithString)
    {
        if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
        {
            int index = -1;
            int res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString, argOffset);
            if (res >= 0)
            {
                if (res == 2)
                    index = luaToInt(L, 2);
                int proxyForSearch = -1;
                res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString, argOffset);
                if (res >= 0)
                {
                    if (res == 2)
                        proxyForSearch = luaToInt(L, 3);

                    int options = 0;
                    res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, false, &errorString, argOffset);
                    if (res >= 0)
                    {
                        if (res == 2)
                            options = luaToInt(L, 4);
                        std::string name(luaWrap_lua_tostring(L, 1));
                        setCurrentScriptInfo_cSide(
                            CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                            CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                L)); // for transmitting to the master function additional info (e.g.for autom. name
                                     // adjustment, or for autom. object deletion when script ends)
                        retVal = CALL_C_API(simGetObjectHandleEx, name.c_str(), index, proxyForSearch, options);
                        setCurrentScriptInfo_cSide(-1, -1);
                    }
                }
            }
        }
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetScriptAttribute(luaWrap_lua_State* L)
{ // deprecated on 05.01.2022
    TRACE_LUA_API;
    LUA_START("sim.setScriptAttribute");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int scriptID = luaToInt(L, 1);
        if (scriptID == sim_handle_self)
            scriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        int attribID = luaToInt(L, 2);
        int thirdArgType = lua_arg_number;
        if ((attribID == sim_customizationscriptattribute_activeduringsimulation) ||
            (attribID == sim_childscriptattribute_automaticcascadingcalls) ||
            (attribID == sim_scriptattribute_enabled) ||
            (attribID == sim_customizationscriptattribute_cleanupbeforesave))
            thirdArgType = lua_arg_bool;

        if ((attribID == sim_scriptattribute_executionorder) || (attribID == sim_scriptattribute_executioncount) ||
            (attribID == sim_scriptattribute_debuglevel))
            thirdArgType = lua_arg_number;
        int res = checkOneGeneralInputArgument(L, 3, thirdArgType, 0, false, false, &errorString, argOffset);
        if (res == 2)
        {
            if ((attribID == sim_customizationscriptattribute_activeduringsimulation) ||
                (attribID == sim_childscriptattribute_automaticcascadingcalls) ||
                (attribID == sim_scriptattribute_enabled) ||
                (attribID == sim_customizationscriptattribute_cleanupbeforesave))
                retVal = CALL_C_API(simSetScriptAttribute, scriptID, attribID, 0.0, luaToBool(L, 3));
            if ((attribID == sim_scriptattribute_executionorder) || (attribID == sim_scriptattribute_executioncount) ||
                (attribID == sim_scriptattribute_debuglevel))
                retVal = CALL_C_API(simSetScriptAttribute, scriptID, attribID, 0.0, luaToInt(L, 3));
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetScriptAttribute(luaWrap_lua_State* L)
{ // deprecated on 05.01.2022
    TRACE_LUA_API;
    LUA_START("sim.getScriptAttribute");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int scriptID = luaToInt(L, 1);
        if (scriptID == sim_handle_self)
            scriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        int attribID = luaToInt(L, 2);
        int intVal;
        double floatVal;
        int result = CALL_C_API(simGetScriptAttribute, scriptID, attribID, &floatVal, &intVal);
        if (result != -1)
        {
            if ((attribID == sim_customizationscriptattribute_activeduringsimulation) ||
                (attribID == sim_childscriptattribute_automaticcascadingcalls) ||
                (attribID == sim_scriptattribute_enabled) ||
                (attribID == sim_customizationscriptattribute_cleanupbeforesave))
                luaWrap_lua_pushboolean(L, intVal);
            if ((attribID == sim_scriptattribute_executionorder) || (attribID == sim_scriptattribute_executioncount) ||
                (attribID == sim_scriptattribute_scripttype) || (attribID == sim_scriptattribute_scripthandle))
                luaWrap_lua_pushinteger(L, intVal);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetScriptText(luaWrap_lua_State* L)
{ // deprecated on 04.02.2022
    TRACE_LUA_API;
    LUA_START("sim.setScriptText");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0))
    {
        int scriptHandle = luaToInt(L, 1);
        std::string scriptText(luaWrap_lua_tostring(L, 2));
        retVal = CALL_C_API(simSetScriptText, scriptHandle, scriptText.c_str());
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetJointMaxForce(luaWrap_lua_State* L)
{ // deprecated on 24.02.2022
    TRACE_LUA_API;
    LUA_START("sim.getJointMaxForce");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        double jointF[1];
        if (CALL_C_API(simGetJointMaxForce, luaToInt(L, 1), jointF) > 0)
        {
            luaWrap_lua_pushnumber(L, jointF[0]);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetJointMaxForce(luaWrap_lua_State* L)
{ // deprecated on 24.02.2022
    TRACE_LUA_API;
    LUA_START("sim.setJointMaxForce");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetJointMaxForce, luaToInt(L, 1), luaToDouble(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simRemoveObject(luaWrap_lua_State* L)
{ // deprecated on 07.03.2022
    TRACE_LUA_API;
    LUA_START("sim.removeObject");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int objId = luaToInt(L, 1);
        int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
        retVal = CALL_C_API(simRemoveObject, objId);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetSystemTimeInMs(luaWrap_lua_State* L)
{ // deprecated on 01.04.2022
    TRACE_LUA_API;
    LUA_START("sim.getSystemTimeInMs");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int lastTime = luaToInt(L, 1);
        luaWrap_lua_pushinteger(L, CALL_C_API(simGetSystemTimeInMs, lastTime));
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetVisionSensorResolution(luaWrap_lua_State* L)
{ // deprecated on 11.04.2022
    TRACE_LUA_API;
    LUA_START("sim.getVisionSensorResolution");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int resolution[2];
        if (CALL_C_API(simGetVisionSensorRes, luaToInt(L, 1), resolution) == 1)
        {
            pushIntTableOntoStack(L, 2, resolution);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetVisionSensorImage(luaWrap_lua_State* L)
{ // deprecated on 11.04.2022
    TRACE_LUA_API;
    LUA_START("sim.getVisionSensorImage");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int arg1 = luaToInt(L, 1);
        int handleFlags = arg1 & 0xff00000;
        int objectHandle = arg1 & 0xfffff;
        int valPerPix = 3;
        int rgbOrGreyOrDepth = 0;
        if ((handleFlags & sim_handleflag_greyscale) != 0)
        {
            valPerPix = 1;
            rgbOrGreyOrDepth = 1;
        }
        int posX = 0;
        int posY = 0;
        int sizeX = 0;
        int sizeY = 0;
        int retType = 0;
        // Now check the optional arguments:
        int res;
        res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString, argOffset);
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                posX = luaToInt(L, 2);
            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString, argOffset);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    posY = luaToInt(L, 3);
                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, false, &errorString, argOffset);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        sizeX = luaToInt(L, 4);
                    res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, true, false, &errorString, argOffset);
                    if ((res == 0) || (res == 2))
                    {
                        if (res == 2)
                            sizeY = luaToInt(L, 5);
                        res = checkOneGeneralInputArgument(L, 6, lua_arg_number, 0, true, false, &errorString, argOffset);
                        if ((res == 0) || (res == 2))
                        {
                            if (res == 2)
                                retType = luaToInt(L, 6);
                            CVisionSensor* rs =
                                App::currentWorld->sceneObjects->getVisionSensorFromHandle(objectHandle);
                            if (rs != nullptr)
                            {
                                if ((sizeX == 0) && (sizeY == 0))
                                { // we have default values here (the whole picture)
                                    int reso[2];
                                    rs->getResolution(reso);
                                    sizeX = reso[0];
                                    sizeY = reso[1];
                                }
                                float* buffer = rs->readPortionOfImage(posX, posY, sizeX, sizeY, rgbOrGreyOrDepth);
                                if (buffer != nullptr)
                                {
                                    if (retType == 0)
                                        pushFloatTableOntoStack(L, sizeX * sizeY * valPerPix, buffer);
                                    else
                                    { // here we return RGB data in a string
                                        char* str = new char[sizeX * sizeY * valPerPix];
                                        int vvv = sizeX * sizeY * valPerPix;
                                        for (int i = 0; i < vvv; i++)
                                            str[i] = char(buffer[i] * 255.0001);
                                        luaWrap_lua_pushbuffer(L, (const char*)str, vvv);
                                        delete[]((char*)str);
                                    }
                                    delete[]((char*)buffer);
                                    LUA_END(1);
                                }
                                else
                                    errorString = SIM_ERROR_INVALID_ARGUMENTS;
                            }
                            else
                                errorString = SIM_ERROR_VISION_SENSOR_INEXISTANT;
                        }
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetVisionSensorImage(luaWrap_lua_State* L)
{ // deprecated on 11.04.2022
    TRACE_LUA_API;
    LUA_START("sim.setVisionSensorImage");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int arg1 = luaToInt(L, 1);
        int handleFlags = arg1 & 0xff00000;
        int sensHandle = arg1 & 0xfffff;
        int valPerPix = 3;
        bool noProcessing = false;
        bool setDepthBufferInstead = false;
        if ((handleFlags & sim_handleflag_greyscale) != 0)
            valPerPix = 1;
        if ((handleFlags & sim_handleflag_rawvalue) != 0)
            noProcessing = true;
        if ((handleFlags & sim_handleflag_depthbuffer) != 0)
            setDepthBufferInstead = true;
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(sensHandle);
        if (it != nullptr)
        { // Ok we have a valid object
            if (it->getObjectType() == sim_sceneobject_visionsensor)
            { // ok we have a valid vision sensor
                int res[2];
                CVisionSensor* rendSens = (CVisionSensor*)it;
                rendSens->getResolution(res);
                // We check if we have a table or string at position 2:
                bool notTableNorString = true;
                if (luaWrap_lua_isnonbuffertable(L, 2))
                { // Ok we have a table. Now what size is it?
                    notTableNorString = false;
                    if (setDepthBufferInstead)
                    {
                        // Now we check if the provided table has correct size:
                        if (int(luaWrap_lua_rawlen(L, 2)) >= res[0] * res[1])
                        {
                            float* img = new float[res[0] * res[1]];
                            getFloatsFromTable(L, 2, res[0] * res[1], img);
                            rendSens->setDepthBuffer(img);
                            retVal = 1;
                            delete[] img;
                        }
                        else
                            errorString = SIM_ERROR_ONE_TABLE_SIZE_IS_WRONG;
                    }
                    else
                    {
                        // Now we check if the provided table has correct size:
                        if (int(luaWrap_lua_rawlen(L, 2)) >= res[0] * res[1] * valPerPix)
                        {
                            float* img = new float[res[0] * res[1] * valPerPix];
                            getFloatsFromTable(L, 2, res[0] * res[1] * valPerPix,
                                               img); // we do the operation directly without going through the c-api
                            if (rendSens->setExternalImage_old(img, valPerPix == 1, noProcessing))
                                retVal = 1;
                            delete[] img;
                        }
                        else
                            errorString = SIM_ERROR_ONE_TABLE_SIZE_IS_WRONG;
                    }
                }
                if (luaWrap_lua_isstring(L, 2))
                { // Ok we have a string. Now what size is it?
                    notTableNorString = false;
                    // Now we check if the provided string has correct size:
                    size_t dataLength;
                    char* data = (char*)luaWrap_lua_tobuffer(L, 2, &dataLength);
                    if (setDepthBufferInstead)
                    {
                        if (int(dataLength) >= res[0] * res[1] * sizeof(float))
                        {
                            rendSens->setDepthBuffer((float*)data);
                            retVal = 1;
                        }
                        else
                            errorString = SIM_ERROR_INCORRECT_BUFFER_SIZE;
                    }
                    else
                    {
                        if (int(dataLength) >= res[0] * res[1] * valPerPix)
                        {
                            float* img = new float[res[0] * res[1] * valPerPix];
                            for (int i = 0; i < res[0] * res[1] * valPerPix; i++)
                                img[i] = float(data[i]) / 255.0;
                            if (rendSens->setExternalImage_old(img, valPerPix == 1, noProcessing))
                                retVal = 1;
                            delete[] img;
                        }
                        else
                            errorString = SIM_ERROR_INCORRECT_BUFFER_SIZE;
                    }
                }
                if (notTableNorString)
                    errorString = SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG;
            }
            else
                errorString = SIM_ERROR_OBJECT_NOT_VISION_SENSOR;
        }
        else
            errorString = SIM_ERROR_OBJECT_INEXISTANT;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetVisionSensorCharImage(luaWrap_lua_State* L)
{ // deprecated on 11.04.2022
    TRACE_LUA_API;
    LUA_START("sim.getVisionSensorCharImage");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int arg1 = luaToInt(L, 1);
        int handleFlags = arg1 & 0xff00000;
        int objectHandle = arg1 & 0xfffff;
        int valPerPix = 3;
        if ((handleFlags & sim_handleflag_greyscale) != 0)
            valPerPix = 1;
        int posX = 0;
        int posY = 0;
        int sizeX = 0;
        int sizeY = 0;
        double rgbaCutOff = 0.0;
        // Now check the optional arguments:
        int res;
        res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString, argOffset);
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                posX = luaToInt(L, 2);
            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString, argOffset);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    posY = luaToInt(L, 3);
                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, false, &errorString, argOffset);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        sizeX = luaToInt(L, 4);
                    res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, true, false, &errorString, argOffset);
                    if ((res == 0) || (res == 2))
                    {
                        if (res == 2)
                            sizeY = luaToInt(L, 5);
                        res = checkOneGeneralInputArgument(L, 6, lua_arg_number, 0, true, false, &errorString, argOffset);
                        if ((res == 0) || (res == 2))
                        {
                            if (res == 2)
                                rgbaCutOff = luaToDouble(L, 6);
                            CVisionSensor* rs =
                                App::currentWorld->sceneObjects->getVisionSensorFromHandle(objectHandle);
                            if (rs != nullptr)
                            {
                                int reso[2];
                                rs->getResolution(reso);
                                if ((sizeX == 0) && (sizeY == 0))
                                { // we have default values here (the whole picture)
                                    sizeX = reso[0];
                                    sizeY = reso[1];
                                }
                                int options = 0;
                                if (valPerPix == 1)
                                    options = options | 1;
                                if (rgbaCutOff > 0.0)
                                    options = options | 2;
                                unsigned char* buffer =
                                    rs->readPortionOfCharImage(posX, posY, sizeX, sizeY, rgbaCutOff, valPerPix == 1);
                                if (buffer != nullptr)
                                {
                                    int vvv = sizeX * sizeY * valPerPix;
                                    if (rgbaCutOff > 0.0)
                                    {
                                        if (valPerPix == 1)
                                            vvv = sizeX * sizeY * 2;
                                        else
                                            vvv = sizeX * sizeY * 4;
                                    }
                                    luaWrap_lua_pushbuffer(L, (const char*)buffer, vvv);
                                    delete[]((char*)buffer);
                                    luaWrap_lua_pushinteger(L, reso[0]);
                                    luaWrap_lua_pushinteger(L, reso[1]);
                                    LUA_END(3);
                                }
                                else
                                    errorString = SIM_ERROR_INVALID_ARGUMENTS;
                            }
                            else
                                errorString = SIM_ERROR_VISION_SENSOR_INEXISTANT;
                        }
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetVisionSensorCharImage(luaWrap_lua_State* L)
{ // deprecated on 11.04.2022
    TRACE_LUA_API;
    LUA_START("sim.setVisionSensorCharImage");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int arg1 = luaToInt(L, 1);
        int handleFlags = arg1 & 0xff00000;
        int sensHandle = arg1 & 0xfffff;
        int valPerPix = 3;
        if ((handleFlags & sim_handleflag_greyscale) != 0)
            valPerPix = 1;
        bool noProcessing = false;
        if ((handleFlags & sim_handleflag_rawvalue) != 0)
            noProcessing = true;
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(sensHandle);
        if (it != nullptr)
        { // Ok we have a valid object
            if (it->getObjectType() == sim_sceneobject_visionsensor)
            { // ok we have a valid vision sensor
                int res[2];
                CVisionSensor* rendSens = (CVisionSensor*)it;
                rendSens->getResolution(res);
                // We check if we have a string at position 2:
                if (luaWrap_lua_isstring(L, 2))
                { // Ok we have a string. Now what size is it?
                    // Now we check if the provided string has correct size:
                    size_t dataLength;
                    char* data = (char*)luaWrap_lua_tobuffer(L, 2, &dataLength);
                    if (int(dataLength) >= res[0] * res[1] * valPerPix)
                    {
                        if (rendSens->setExternalCharImage_old((unsigned char*)data, valPerPix == 1, noProcessing))
                            retVal = 1;
                    }
                    else
                        errorString = SIM_ERROR_INCORRECT_BUFFER_SIZE;
                }
                else
                    errorString = SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG;
            }
            else
                errorString = SIM_ERROR_OBJECT_NOT_VISION_SENSOR;
        }
        else
            errorString = SIM_ERROR_OBJECT_INEXISTANT;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetVisionSensorDepthBuffer(luaWrap_lua_State* L)
{ // deprecated on 11.04.2022
    TRACE_LUA_API;
    LUA_START("sim.getVisionSensorDepthBuffer");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int posX = 0;
        int posY = 0;
        int sizeX = 0;
        int sizeY = 0;
        // Now check the optional arguments:
        int res;
        res = checkOneGeneralInputArgument(L, 2, lua_arg_number, 0, true, false, &errorString, argOffset);
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                posX = luaToInt(L, 2);
            res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString, argOffset);
            if ((res == 0) || (res == 2))
            {
                if (res == 2)
                    posY = luaToInt(L, 3);
                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, true, false, &errorString, argOffset);
                if ((res == 0) || (res == 2))
                {
                    if (res == 2)
                        sizeX = luaToInt(L, 4);
                    res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 0, true, false, &errorString, argOffset);
                    if ((res == 0) || (res == 2))
                    {
                        if (res == 2)
                            sizeY = luaToInt(L, 5);
                        int sensHandle = luaToInt(L, 1);
                        bool returnString = (sensHandle & sim_handleflag_codedstring) != 0;
                        bool toMeters = (sensHandle & sim_handleflag_depthbuffermeters) != 0;
                        sensHandle = sensHandle & 0xfffff;
                        CVisionSensor* rs = App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensHandle);
                        if (rs != nullptr)
                        {
                            if ((sizeX == 0) && (sizeY == 0))
                            { // we have default values here (the whole picture)
                                int reso[2];
                                rs->getResolution(reso);
                                sizeX = reso[0];
                                sizeY = reso[1];
                            }
                            float* buffer = rs->readPortionOfImage(posX, posY, sizeX, sizeY, 2);
                            if (buffer != nullptr)
                            {
                                if (toMeters)
                                { // Here we need to convert values to distances in meters:
                                    double np, fp;
                                    rs->getClippingPlanes(np, fp);
                                    float n = (float)np;
                                    float f = (float)fp;
                                    float fmn = f - n;
                                    for (int i = 0; i < sizeX * sizeY; i++)
                                        buffer[i] = n + fmn * buffer[i];
                                }
                                if (returnString)
                                    luaWrap_lua_pushbuffer(L, (char*)buffer, sizeX * sizeY * sizeof(float));
                                else
                                    pushFloatTableOntoStack(L, sizeX * sizeY, buffer);
                                delete[]((char*)buffer);
                                LUA_END(1);
                            }
                            else
                                errorString = SIM_ERROR_INVALID_ARGUMENTS;
                        }
                        else
                            errorString = SIM_ERROR_VISION_SENSOR_INEXISTANT;
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simCreatePureShape(luaWrap_lua_State* L)
{ // deprecated on 27.04.2022
    TRACE_LUA_API;
    LUA_START("sim.createPureShape");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 3, lua_arg_number,
                            0))
    {
        int primitiveType = luaToInt(L, 1);
        int options = luaToInt(L, 2);
        double sizes[3];
        getDoublesFromTable(L, 3, 3, sizes);
        double mass = luaToDouble(L, 4);
        int* precision = nullptr;
        int prec[2];
        int res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 2, true, true, &errorString, argOffset);
        if (res >= 0)
        {
            if (res == 2)
            {
                getIntsFromTable(L, 5, 2, prec);
                precision = prec;
            }
            retVal = CALL_C_API(simCreatePureShape, primitiveType, options, sizes, mass, precision);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetScriptHandle(luaWrap_lua_State* L)
{ // deprecated on 06.05.2022
    TRACE_LUA_API;
    LUA_START("sim.getScriptHandle");

    int retVal = -1; // means error
    if (luaWrap_lua_gettop(L) == 0)
        retVal = CScriptObject::getScriptHandleFromInterpreterState_lua(L); // no arguments, return itself
    else
    {
        if (checkInputArguments(L, nullptr, argOffset, lua_arg_nil, 0))
            retVal = CScriptObject::getScriptHandleFromInterpreterState_lua(
                L); // nil arg, return itself (back. compatibility)
        else
        {
            if (checkInputArguments(L, nullptr, argOffset, lua_arg_integer, 0))
            { // script type arg.
                int scriptType = luaToInt(L, 1);
                if (scriptType != sim_handle_self)
                {
                    int objectHandle = -1;
                    std::string scriptName;
                    if (scriptType == sim_scripttype_addon)
                    {
                        if (checkInputArguments(L, &errorString, argOffset, lua_arg_integer, 0, lua_arg_string, 0))
                            scriptName = luaWrap_lua_tostring(L, 2);
                    }
                    if ((scriptType == sim_scripttype_simulation) ||
                        (scriptType == sim_scripttype_customization))
                    {
                        if (checkInputArguments(L, nullptr, argOffset, lua_arg_integer, 0, lua_arg_integer, 0))
                            objectHandle = luaToInt(L, 2); // back compatibility actually
                        else
                        {
                            if (checkInputArguments(L, &errorString, argOffset, lua_arg_integer, 0, lua_arg_string, 0))
                            {
                                scriptName = luaWrap_lua_tostring(L, 2);
                                if (scriptName.size() == 0)
                                {
                                    if (checkInputArguments(L, &errorString, argOffset, lua_arg_integer, 0, lua_arg_string, 0,
                                                            lua_arg_integer, 0))
                                        objectHandle = luaToInt(L, 3);
                                }
                            }
                        }
                    }
                    if (((scriptName.size() > 0) || (objectHandle >= 0)) ||
                        ((scriptType == sim_scripttype_main) || (scriptType == sim_scripttype_sandbox)))
                    {
                        setCurrentScriptInfo_cSide(
                            CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                            CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                L)); // for transmitting to the master function additional info (e.g.for autom. name
                                     // adjustment, or for autom. object deletion when script ends)
                        retVal = CALL_C_API(simGetScriptHandleEx, scriptType, objectHandle, scriptName.c_str());
                        setCurrentScriptInfo_cSide(-1, -1);
                    }
                }
                else
                    retVal = CScriptObject::getScriptHandleFromInterpreterState_lua(L); // for backward compatibility
            }
            else
            { // string argument, for backward compatibility:
                if (checkInputArguments(L, nullptr, argOffset, lua_arg_string, 0))
                {
                    std::string name(luaWrap_lua_tostring(L, 1));
                    setCurrentScriptInfo_cSide(
                        CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                        CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                            L));                                        // for transmitting to the master function additional info (e.g.for autom. name
                                                                        // adjustment, or for autom. object deletion when script ends)
                    retVal = CALL_C_API(simGetScriptHandle, name.c_str()); // deprecated func.
                    setCurrentScriptInfo_cSide(-1, -1);
                }
                else
                    checkInputArguments(L, &errorString, argOffset, lua_arg_integer, 0); // just generate an error
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simHandleCustomizationScripts(luaWrap_lua_State* L)
{ // deprecated on 19.09.2022
    TRACE_LUA_API;
    LUA_START("sim.handleCustomizationScripts");

    int retVal = -1;
    int currentScriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
    CScriptObject* itScrObj = App::worldContainer->getScriptObjectFromHandle(currentScriptID);
    if (itScrObj->getScriptType() == sim_scripttype_main)
    {
        if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        {
            int callType = luaToInt(L, 1);
            retVal = 0;
            int editMode = NO_EDIT_MODE;
#ifdef SIM_WITH_GUI
            editMode = GuiApp::getEditModeType();
#endif
            if (editMode == NO_EDIT_MODE)
            {
                retVal = App::currentWorld->sceneObjects->callScripts_noMainScript(sim_scripttype_customization, callType, nullptr, nullptr);
                App::currentWorld->sceneObjects->embeddedScriptContainer->removeDestroyedScripts(sim_scripttype_customization);
            }
        }
    }
    else
        errorString = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetDoubleSignal(luaWrap_lua_State* L)
{ // deprecated on 13.10.2022
    TRACE_LUA_API;
    LUA_START("sim.setDoubleSignal");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0, lua_arg_number, 0))
    {
        setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                                   CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                       L)); // for transmitting to the master function additional info (e.g.for autom.
                                            // name adjustment, or for autom. object deletion when script ends)
        retVal = CALL_C_API(simSetDoubleSignalOld, luaWrap_lua_tostring(L, 1), luaToDouble(L, 2));
        setCurrentScriptInfo_cSide(-1, -1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetDoubleSignal(luaWrap_lua_State* L)
{ // deprecated on 13.10.2022
    TRACE_LUA_API;
    LUA_START("sim.getDoubleSignal");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        double doubleVal;
        if (CALL_C_API(simGetDoubleSignalOld, std::string(luaWrap_lua_tostring(L, 1)).c_str(), &doubleVal) == 1)
        {
            luaWrap_lua_pushnumber(L, doubleVal);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simClearDoubleSignal(luaWrap_lua_State* L)
{ // deprecated on 13.10.2022
    TRACE_LUA_API;
    LUA_START("sim.clearDoubleSignal");

    int retVal = -1; // error
    int res = checkOneGeneralInputArgument(L, 1, lua_arg_string, 0, true, true, &errorString, argOffset);
    if (res >= 0)
    {
        if (res != 2)
            retVal = CALL_C_API(simClearDoubleSignalOld, nullptr); // actually deprecated. No need for that
        else
            retVal = CALL_C_API(simClearDoubleSignalOld, luaWrap_lua_tostring(L, 1));
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simReorientShapeBoundingBox(luaWrap_lua_State* L)
{ // deprecated on 15.03.2023
    TRACE_LUA_API;
    LUA_START("sim.reorientShapeBoundingBox");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int shapeHandle = luaToInt(L, 1);
        int relativeToHandle = luaToInt(L, 2);
        retVal = CALL_C_API(simReorientShapeBoundingBox, shapeHandle, relativeToHandle, 0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simCreateMeshShape(luaWrap_lua_State* L)
{ // deprecated on 15.03.2023
    TRACE_LUA_API;
    LUA_START("sim.createMeshShape");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int options = luaToInt(L, 1);
        double shadingAngle = luaToDouble(L, 2);

        int vl = 2;
        int il = 2;
        if ((luaWrap_lua_gettop(L) >= 4) && luaWrap_lua_isnonbuffertable(L, 3) && luaWrap_lua_isnonbuffertable(L, 4))
        {
            vl = (int)luaWrap_lua_rawlen(L, 3);
            il = (int)luaWrap_lua_rawlen(L, 4);
        }
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, vl, false, false, &errorString, argOffset);
        if (res == 2)
        {
            res = checkOneGeneralInputArgument(L, 4, lua_arg_number, il, false, false, &errorString, argOffset);
            if (res == 2)
            {
                double* vertices = new double[vl];
                int* indices = new int[il];
                getDoublesFromTable(L, 3, vl, vertices);
                getIntsFromTable(L, 4, il, indices);
                retVal = CALL_C_API(simCreateMeshShape, options, shadingAngle, vertices, vl, indices, il, nullptr);
                delete[] indices;
                delete[] vertices;
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simInvertMatrix(luaWrap_lua_State* L)
{ // deprecated on 29.03.2023
    TRACE_LUA_API;
    LUA_START("sim.invertMatrix");
    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 12))
    {
        double arr[12];
        getDoublesFromTable(L, 1, 12, arr);
        retVal = CALL_C_API(simInvertMatrix, arr);
        for (int i = 0; i < 12; i++)
        {
            luaWrap_lua_pushnumber(L, arr[i]);
            luaWrap_lua_rawseti(L, 1, i + 1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simInvertPose(luaWrap_lua_State* L)
{ // deprecated on 29.03.2023
    TRACE_LUA_API;
    LUA_START("sim.invertPose");
    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 7))
    {
        double arr[7];
        getDoublesFromTable(L, 1, 7, arr);
        retVal = CALL_C_API(simInvertPose, arr);
        for (int i = 0; i < 7; i++)
        {
            luaWrap_lua_pushnumber(L, arr[i]);
            luaWrap_lua_rawseti(L, 1, i + 1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simRegisterScriptFunction(luaWrap_lua_State* L)
{ // deprecated on 19.05.2023
    TRACE_LUA_API;
    LUA_START("sim.registerScriptFunction");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0, lua_arg_string, 0))
    {
        std::string funcNameAtPluginName(luaWrap_lua_tostring(L, 1));
        std::string ct(luaWrap_lua_tostring(L, 2));

        std::string funcName;
        std::string pluginName;

        size_t p = funcNameAtPluginName.find('@');
        if (p != std::string::npos)
        {
            pluginName.assign(funcNameAtPluginName.begin() + p + 1, funcNameAtPluginName.end());
            funcName.assign(funcNameAtPluginName.begin(), funcNameAtPluginName.begin() + p);
        }
        if (pluginName.size() > 0)
        {
            retVal = 1;
            if (App::worldContainer->scriptCustomFuncAndVarContainer->removeCustomFunction(
                    funcNameAtPluginName.c_str()))
                retVal = 0; // that function already existed. We remove it and replace it!
            CScriptCustomFunction* newFunction =
                new CScriptCustomFunction(funcNameAtPluginName.c_str(), ct.c_str(), nullptr, false);
            if (!App::worldContainer->scriptCustomFuncAndVarContainer->insertCustomFunction(newFunction))
            {
                delete newFunction;
                CApiErrors::setLastError(__func__, SIM_ERROR_CUSTOM_LUA_FUNC_COULD_NOT_BE_REGISTERED);
                retVal = -1;
            }
        }
        else
            errorString = SIM_ERROR_MISSING_PLUGIN_NAME;
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simRegisterScriptVariable(luaWrap_lua_State* L)
{ // deprecated on 19.05.2023
    TRACE_LUA_API;
    LUA_START("sim.registerScriptVariable");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        std::string varNameAtPluginName(luaWrap_lua_tostring(L, 1));
        std::string varName;
        std::string pluginName;

        size_t p = varNameAtPluginName.find('@');
        if (p != std::string::npos)
        {
            pluginName.assign(varNameAtPluginName.begin() + p + 1, varNameAtPluginName.end());
            varName.assign(varNameAtPluginName.begin(), varNameAtPluginName.begin() + p);
        }
        retVal = 1;
        App::worldContainer->scriptCustomFuncAndVarContainer->insertCustomVariable(varNameAtPluginName.c_str(), nullptr,
                                                                                   0);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simIsDeprecated(luaWrap_lua_State* L)
{ // deprecated on 31.05.2023
    TRACE_LUA_API;
    LUA_START("sim.isDeprecated");

    int retVal = 0;

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simLoadModule(luaWrap_lua_State* L)
{ // deprecated on 07.06.2023
    TRACE_LUA_API;
    LUA_START("sim.loadModule");

    int retVal = -3; // means plugin could not be loaded
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0, lua_arg_string, 0))
    {
        std::string fileAndPath(luaWrap_lua_tostring(L, 1));
        std::string pluginName(luaWrap_lua_tostring(L, 2));
        retVal = CALL_C_API(simLoadModule, fileAndPath.c_str(), pluginName.c_str());
        if (retVal >= 0)
        {
            CScriptObject* it =
                App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
            it->registerNewFunctions_lua(); // otherwise we can only use the custom Lua functions that the plugin
                                            // registers after this script has re-initialized!
            it->registerPluginFunctions();
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simUnloadModule(luaWrap_lua_State* L)
{ // deprecated on 07.06.2023
    TRACE_LUA_API;
    LUA_START("sim.unloadModule");

    int retVal = 0; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simUnloadModule, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetThreadSwitchTimingOLD(luaWrap_lua_State* L)
{ // deprecated on 21.09.2023
    TRACE_LUA_API;
    LUA_START("sim.setThreadSwitchTiming");
    LUA_END(0);
}

int _simConvexDecompose(luaWrap_lua_State* L)
{ // deprecated in June 2024
    TRACE_LUA_API;
    LUA_START("sim.convexDecompose");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int shapeHandle = luaToInt(L, 1);
        int options = luaToInt(L, 2);
        int intParams[10];
        double floatParams[10];
        bool goOn = true;
        if ((options & 4) == 0)
        {
            goOn = false;
            int ipc = 4;
            int fpc = 3;
            if (options & 128)
            {
                ipc = 10;
                fpc = 10;
            }
            int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, ipc, false, false, &errorString, argOffset);
            if (res == 2)
            {
                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, fpc, false, false, &errorString, argOffset);
                if (res == 2)
                {
                    getIntsFromTable(L, 3, ipc, intParams);
                    getDoublesFromTable(L, 4, fpc, floatParams);
                    goOn = true;
                }
            }
        }
        intParams[4] = 0;
        floatParams[3] = 0.0;
        floatParams[4] = 0.0;
        if (goOn)
            retVal = CALL_C_API(simConvexDecompose, shapeHandle, options, intParams, floatParams);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetDecimatedMesh(luaWrap_lua_State* L)
{ // deprecated in June 2024
    TRACE_LUA_API;
    LUA_START("sim.getDecimatedMesh");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 9, lua_arg_number, 6, lua_arg_number, 0))
    {
        int vl = (int)luaWrap_lua_rawlen(L, 1);
        int il = (int)luaWrap_lua_rawlen(L, 2);
        double percentage = luaToDouble(L, 3);
        if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, vl, lua_arg_number, il, lua_arg_number, 0))
        {
            double* vertices = new double[vl];
            getDoublesFromTable(L, 1, vl, vertices);
            int* indices = new int[il];
            getIntsFromTable(L, 2, il, indices);
            double* vertOut;
            int vertOutL;
            int* indOut;
            int indOutL;
            if (CALL_C_API(simGetDecimatedMesh, vertices, vl, indices, il, &vertOut, &vertOutL, &indOut, &indOutL,
                                             percentage, 0, nullptr))
            {
                pushDoubleTableOntoStack(L, vertOutL, vertOut);
                pushIntTableOntoStack(L, indOutL, indOut);
                delete[] vertOut;
                delete[] indOut;
                LUA_END(2);
            }
            delete[] vertices;
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simAddScript(luaWrap_lua_State* L)
{ // deprecated in June 2024
    TRACE_LUA_API;
    LUA_START("sim.addScript");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int scriptType = luaToInt(L, 1);
        retVal = CALL_C_API(simAddScript, scriptType);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simAssociateScriptWithObject(luaWrap_lua_State* L)
{ // deprecated in June 2024
    TRACE_LUA_API;
    LUA_START("sim.associateScriptWithObject");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int scriptHandle = luaToInt(L, 1);
        int objectHandle = luaToInt(L, 2);
        retVal = CALL_C_API(simAssociateScriptWithObject, scriptHandle, objectHandle);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simRemoveScript(luaWrap_lua_State* L)
{ // deprecated in June 2024
    TRACE_LUA_API;
    LUA_START("sim.removeScript");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        if (handle == sim_handle_self)
            handle = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        if (sim_handle_all != handle)
            retVal = CALL_C_API(simRemoveScript, handle);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetScriptInt32Param(luaWrap_lua_State* L)
{ // deprecated in June 2024
    TRACE_LUA_API;
    LUA_START("sim.getScriptInt32Param");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int param;
        int scriptID = luaToInt(L, 1);
        if (scriptID == sim_handle_self)
            scriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        int retVal = CALL_C_API(simGetScriptInt32Param, scriptID, luaToInt(L, 2), &param);
        if (retVal > 0)
        {
            luaWrap_lua_pushinteger(L, param);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetScriptInt32Param(luaWrap_lua_State* L)
{ // deprecated in June 2024
    TRACE_LUA_API;
    LUA_START("sim.setScriptInt32Param");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        int scriptID = luaToInt(L, 1);
        if (scriptID == sim_handle_self)
            scriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        retVal = CALL_C_API(simSetScriptInt32Param, scriptID, luaToInt(L, 2), luaToInt(L, 3));
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetScriptStringParam(luaWrap_lua_State* L)
{ // deprecated in June 2024
    TRACE_LUA_API;
    LUA_START("sim.getScriptStringParam");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int paramLength;
        int scriptID = luaToInt(L, 1);
        if (scriptID == sim_handle_self)
            scriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        char* strBuff = CALL_C_API(simGetScriptStringParam, scriptID, luaToInt(L, 2), &paramLength);
        if (strBuff != nullptr)
        {
            luaWrap_lua_pushbinarystring(L, strBuff, paramLength);
            delete[] strBuff;
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetScriptStringParam(luaWrap_lua_State* L)
{ // deprecated in June 2024
    TRACE_LUA_API;
    LUA_START("sim.setScriptStringParam");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_string, 0))
    {
        int scriptID = luaToInt(L, 1);
        if (scriptID == sim_handle_self)
            scriptID = CScriptObject::getScriptHandleFromInterpreterState_lua(L);
        size_t dataLength;
        char* data = (char*)luaWrap_lua_tobuffer(L, 3, &dataLength);
        retVal = CALL_C_API(simSetScriptStringParam, scriptID, luaToInt(L, 2), data, (int)dataLength);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simPersistentDataWrite(luaWrap_lua_State* L)
{ // deprecated on June 19 2024
    TRACE_LUA_API;
    LUA_START("sim.persistentDataWrite");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0, lua_arg_string, 0))
    {
        int options = 0;
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, true, false, &errorString, argOffset);
        if ((res == 0) || (res == 2))
        {
            if (res == 2)
                options = luaToInt(L, 3);
            size_t dataLength;
            char* data = (char*)luaWrap_lua_tobuffer(L, 2, &dataLength);
            retVal = CALL_C_API(simPersistentDataWrite, std::string(luaWrap_lua_tostring(L, 1)).c_str(), data, (int)dataLength, options);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simPersistentDataRead(luaWrap_lua_State* L)
{ // deprecated on June 19 2024
    TRACE_LUA_API;
    LUA_START("sim.persistentDataRead");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        int stringLength;
        char* str = CALL_C_API(simPersistentDataRead, std::string(luaWrap_lua_tostring(L, 1)).c_str(), &stringLength);

        if (str != nullptr)
        {
            luaWrap_lua_pushbuffer(L, str, stringLength);
            CALL_C_API(simReleaseBuffer, str);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetPersistentDataTags(luaWrap_lua_State* L)
{ // deprecated on June 19 2024
    TRACE_LUA_API;
    LUA_START("sim.getPersistentDataTags");

    int tagCount;
    char* data = CALL_C_API(simGetPersistentDataTags, &tagCount);
    if (data != nullptr)
    {
        std::vector<std::string> stringTable;
        size_t off = 0;
        for (int i = 0; i < tagCount; i++)
        {
            stringTable.push_back(data + off);
            off += strlen(data + off) + 1;
        }
        pushStringTableOntoStack(L, stringTable);
        CALL_C_API(simReleaseBuffer, data);
        LUA_END(1);
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetBoolParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setBoolParam");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_bool, 0))
        retVal = CALL_C_API(simSetBoolParam, luaToInt(L, 1), luaToBool(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetBoolParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getBoolParam");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int retVal = CALL_C_API(simGetBoolParam, luaToInt(L, 1));
        if (retVal != -1)
        {
            luaWrap_lua_pushboolean(L, retVal != 0);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetInt32Param(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setInt32Param");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int paramIndex = luaToInt(L, 1);
        int v = luaToInt(L, 2);
        if (paramIndex == sim_intparam_error_report_mode)
        { // for backward compatibility (2020)
            CScriptObject* it =
                App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
            if (it != nullptr)
            {
                bool r = true; // default
                if ((v & sim_api_error_report) == 0)
                    r = false;
                retVal = 1;
            }
        }
        else
            retVal = CALL_C_API(simSetInt32Param, paramIndex, v);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetInt32Param(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getInt32Param");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int paramIndex = luaToInt(L, 1);
        if (paramIndex == sim_intparam_error_report_mode)
        { // for backward compatibility (2020)
            CScriptObject* it =
                App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
            if (it != nullptr)
            {
                int v = 1; // default
                luaWrap_lua_pushinteger(L, v);
                LUA_END(1);
            }
        }
        else
        {
            int v;
            int retVal = CALL_C_API(simGetInt32Param, paramIndex, &v);
            if (retVal != -1)
            {
                luaWrap_lua_pushinteger(L, v);
                LUA_END(1);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetFloatParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setFloatParam");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetFloatParam, luaToInt(L, 1), luaToDouble(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetFloatParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getFloatParam");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        double v;
        int retVal = CALL_C_API(simGetFloatParam, luaToInt(L, 1), &v);
        if (retVal != -1)
        {
            luaWrap_lua_pushnumber(L, v);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(1);
}

int _simSetStringParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setStringParam");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0))
        retVal = CALL_C_API(simSetStringParam, luaToInt(L, 1), luaWrap_lua_tostring(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetStringParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getStringParam");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int param = luaToInt(L, 1);
        if (sim_stringparam_addonpath == param)
        {
            std::string s;
            CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(CScriptObject::getScriptHandleFromInterpreterState_lua(L));
            if (it != nullptr)
                s = it->getAddOnPath();
            luaWrap_lua_pushtext(L, s.c_str());
            LUA_END(1);
        }
        else
        {
            char* s = CALL_C_API(simGetStringParam, param);
            if (s != nullptr)
            {
                luaWrap_lua_pushtext(L, s);
                delete[] s;
                LUA_END(1);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetArrayParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setArrayParam");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_table, 0))
    {
        int parameter = luaToInt(L, 1);
        if (true)
        { // for now all array parameters are tables of 3 floats
            double theArray[3];
            getDoublesFromTable(L, 2, 3, theArray);
            retVal = CALL_C_API(simSetArrayParam, parameter, theArray);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetArrayParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getArrayParam");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int parameter = luaToInt(L, 1);
        if (true)
        { // for now all parameters are tables of 3 floats
            double theArray[3];
            int retVal = CALL_C_API(simGetArrayParam, parameter, theArray);
            if (retVal != -1)
            {
                pushDoubleTableOntoStack(L, 3, theArray);
                LUA_END(1);
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetNamedStringParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setNamedStringParam");
    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0, lua_arg_string, 0))
    {
        std::string paramName(luaWrap_lua_tostring(L, 1));
        size_t l;
        const char* data = ((char*)luaWrap_lua_tobuffer(L, 2, &l));
        retVal = CALL_C_API(simSetNamedStringParam, paramName.c_str(), data, int(l));
        if (retVal >= 0)
        {
            luaWrap_lua_pushinteger(L, retVal);
            LUA_END(1);
        }
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetNamedStringParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getNamedStringParam");
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        std::string paramName(luaWrap_lua_tostring(L, 1));
        int l;
        char* stringParam = CALL_C_API(simGetNamedStringParam, paramName.c_str(), &l);
        if (stringParam != nullptr)
        {
            luaWrap_lua_pushbinarystring(L, stringParam, l);
            delete[] stringParam;
            LUA_END(1);
        }
        LUA_END(0);
    }
    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetInt32Signal(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setInt32Signal");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0, lua_arg_number, 0))
    {
        setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                                   CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                       L)); // for transmitting to the master function additional info (e.g.for autom.
                                            // name adjustment, or for autom. object deletion when script ends)
        retVal = CALL_C_API(simSetInt32Signal, luaWrap_lua_tostring(L, 1), luaToInt(L, 2));
        setCurrentScriptInfo_cSide(-1, -1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetInt32Signal(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getInt32Signal");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        int intVal;
        if (CALL_C_API(simGetInt32Signal, std::string(luaWrap_lua_tostring(L, 1)).c_str(), &intVal) == 1)
        {
            luaWrap_lua_pushinteger(L, intVal);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simClearInt32Signal(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.clearInt32Signal");

    int retVal = -1; // error
    int res = checkOneGeneralInputArgument(L, 1, lua_arg_string, 0, true, true, &errorString, argOffset);
    if (res >= 0)
    {
        if (res != 2)
            retVal = CALL_C_API(simClearInt32Signal, nullptr); // actually deprecated. No need for that
        else
            retVal = CALL_C_API(simClearInt32Signal, luaWrap_lua_tostring(L, 1));
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetFloatSignal(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setFloatSignal");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0, lua_arg_number, 0))
    {
        setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                                   CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                       L)); // for transmitting to the master function additional info (e.g.for autom.
                                            // name adjustment, or for autom. object deletion when script ends)
        retVal = CALL_C_API(simSetFloatSignal, luaWrap_lua_tostring(L, 1), luaToDouble(L, 2));
        setCurrentScriptInfo_cSide(-1, -1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetFloatSignal(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getFloatSignal");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        double floatVal;
        if (CALL_C_API(simGetFloatSignal, std::string(luaWrap_lua_tostring(L, 1)).c_str(), &floatVal) == 1)
        {
            luaWrap_lua_pushnumber(L, floatVal);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simClearFloatSignal(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.clearFloatSignal");

    int retVal = -1; // error
    int res = checkOneGeneralInputArgument(L, 1, lua_arg_string, 0, true, true, &errorString, argOffset);
    if (res >= 0)
    {
        if (res != 2)
            retVal = CALL_C_API(simClearFloatSignal, nullptr); // actually deprecated. No need for that
        else
            retVal = CALL_C_API(simClearFloatSignal, luaWrap_lua_tostring(L, 1));
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetStringSignal(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setStringSignal");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0, lua_arg_string, 0))
    {
        size_t dataLength;
        const char* data = luaWrap_lua_tobuffer(L, 2, &dataLength);
        setCurrentScriptInfo_cSide(CScriptObject::getScriptHandleFromInterpreterState_lua(L),
                                   CScriptObject::getScriptNameIndexFromInterpreterState_lua_old(
                                       L)); // for transmitting to the master function additional info (e.g.for autom.
                                            // name adjustment, or for autom. object deletion when script ends)
        retVal = CALL_C_API(simSetStringSignal, luaWrap_lua_tostring(L, 1), data, int(dataLength));
        setCurrentScriptInfo_cSide(-1, -1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetStringSignal(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getStringSignal");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_string, 0))
    {
        int stringLength;
        char* str = CALL_C_API(simGetStringSignal, std::string(luaWrap_lua_tostring(L, 1)).c_str(), &stringLength);
        if (str != nullptr)
        {
            luaWrap_lua_pushbinarystring(L, str, stringLength);
            CALL_C_API(simReleaseBuffer, str);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simClearStringSignal(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.clearStringSignal");

    int retVal = -1; // error
    int res = checkOneGeneralInputArgument(L, 1, lua_arg_string, 0, true, true, &errorString, argOffset);
    if (res >= 0)
    {
        if (res != 2)
            retVal = CALL_C_API(simClearStringSignal, nullptr);
        else
            retVal = CALL_C_API(simClearStringSignal, luaWrap_lua_tostring(L, 1));
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetSignalName(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getSignalName");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        char* str = CALL_C_API(simGetSignalName, luaToInt(L, 1), luaToInt(L, 2));
        if (str != nullptr)
        {
            luaWrap_lua_pushtext(L, str);
            CALL_C_API(simReleaseBuffer, str);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetLightParameters(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getLightParameters");

    int retVal = -1;
    double ambientOld[3] = {0.0, 0.0, 0.0};
    double diffuse[3] = {0.0, 0.0, 0.0};
    double specular[3] = {0.0, 0.0, 0.0};
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simGetLightParameters, luaToInt(L, 1), nullptr, diffuse, specular);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    pushDoubleTableOntoStack(L, 3, ambientOld);
    pushDoubleTableOntoStack(L, 3, diffuse);
    pushDoubleTableOntoStack(L, 3, specular);
    LUA_END(4);
}

int _simSetLightParameters(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setLightParameters");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int objHandle = luaToInt(L, 1);
        int state = luaToInt(L, 2);
        float* diffuseP = nullptr;
        float* specularP = nullptr;
        float diffuse_[3] = {0.0, 0.0, 0.0};
        float specular_[3] = {0.0, 0.0, 0.0};
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_number, 3, true, true, &errorString, argOffset);
        if (res != -1)
        {
            int res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 3, true, true, &errorString, argOffset);
            if (res != -1)
            {
                if (res == 2)
                { // get the data
                    getFloatsFromTable(L, 4, 3, diffuse_);
                    diffuseP = diffuse_;
                }
                int res = checkOneGeneralInputArgument(L, 5, lua_arg_number, 3, true, true, &errorString, argOffset);
                if (res != -1)
                {
                    if (res == 2)
                    { // get the data
                        getFloatsFromTable(L, 5, 3, specular_);
                        specularP = specular_;
                    }
                    retVal = CALL_C_API(simSetLightParameters, objHandle, state, nullptr, diffuseP, specularP);
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectInt32Param(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectInt32Param");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int param;
        int retVal = CALL_C_API(simGetObjectInt32Param, luaToInt(L, 1), luaToInt(L, 2), &param);
        if (retVal > 0)
        {
            luaWrap_lua_pushinteger(L, param);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectInt32Param(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectInt32Param");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetObjectInt32Param, luaToInt(L, 1), luaToInt(L, 2), luaToInt(L, 3));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectFloatParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectFloatParam");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        double param;
        int retVal = CALL_C_API(simGetObjectFloatParam, luaToInt(L, 1), luaToInt(L, 2), &param);
        if (retVal > 0)
        {
            luaWrap_lua_pushnumber(L, param);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectFloatParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectFloatParam");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetObjectFloatParam, luaToInt(L, 1), luaToInt(L, 2), luaToDouble(L, 3));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectFloatArrayParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectFloatArrayParam");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int s;
        double* params = CALL_C_API(simGetObjectFloatArrayParam, luaToInt(L, 1), luaToInt(L, 2), &s);
        if (params != nullptr)
        {
            pushDoubleTableOntoStack(L, s, params);
            delete[] params;
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectFloatArrayParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectFloatArrayParam");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 1))
    {
        size_t cnt = luaWrap_lua_rawlen(L, 3);
        std::vector<double> arr;
        arr.resize(cnt);
        getDoublesFromTable(L, 3, cnt, &arr[0]);
        retVal = CALL_C_API(simSetObjectFloatArrayParam, luaToInt(L, 1), luaToInt(L, 2), &arr[0], int(cnt));
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectStringParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectStringParam");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int paramLength;
        char* strBuff = CALL_C_API(simGetObjectStringParam, luaToInt(L, 1), luaToInt(L, 2), &paramLength);
        if (strBuff != nullptr)
        {
            luaWrap_lua_pushbuffer(L, strBuff, paramLength);
            delete[] strBuff;
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectStringParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectStringParam");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_string, 0))
    {
        size_t dataLength;
        char* data = (char*)luaWrap_lua_tobuffer(L, 3, &dataLength);
        retVal = CALL_C_API(simSetObjectStringParam, luaToInt(L, 1), luaToInt(L, 2), data, (int)dataLength);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simWriteCustomStringData(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.writeCustomStringData");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0))
    {
        int objectHandle = luaToInt(L, 1);
        std::string dataName(luaWrap_lua_tostring(L, 2));
        int res;
        res = checkOneGeneralInputArgument(L, 3, lua_arg_string, 0, false, true, &errorString, argOffset);
        if (res >= 1)
        {
            size_t dataLength = 0;
            char* data = nullptr;
            if (res == 2)
                data = (char*)luaWrap_lua_tobuffer(L, 3, &dataLength);
            retVal = CALL_C_API(simWriteCustomDataBlock, objectHandle, dataName.c_str(), data, (int)dataLength);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simReadCustomStringData(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.readCustomStringData");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_string, 0))
    {
        int objectHandle = luaToInt(L, 1);
        std::string dataName(luaWrap_lua_tostring(L, 2));
        int dataLength;
        char* data = CALL_C_API(simReadCustomDataBlock, objectHandle, dataName.c_str(), &dataLength);
        if (data != nullptr)
        {
            luaWrap_lua_pushbinarystring(L, (const char*)data, dataLength);
            CALL_C_API(simReleaseBuffer, data);
        }
        else
            luaWrap_lua_pushnil(L);
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simReadCustomDataTags(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.readCustomDataTags");

    std::vector<std::string> stringTable;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int objectHandle = luaToInt(L, 1);
        int tagCount;
        char* data = CALL_C_API(simReadCustomDataBlockTags, objectHandle, &tagCount);
        if (data != nullptr)
        {
            size_t off = 0;
            for (int i = 0; i < tagCount; i++)
            {
                stringTable.push_back(data + off);
                off += strlen(data + off) + 1;
            }
            CALL_C_API(simReleaseBuffer, data);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    pushStringTableOntoStack(L, stringTable);
    LUA_END(1);
}

int _simGetEngineFloatParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getEngineFloatParam_old");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int paramId = luaToInt(L, 1);
        int objectHandle = luaToInt(L, 2);
        bool ok;
        double paramVal = CALL_C_API(simGetEngineFloatParam, paramId, objectHandle, nullptr, &ok);
        if (ok)
        {
            luaWrap_lua_pushnumber(L, paramVal);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetEngineInt32Param(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getEngineInt32Param");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int paramId = luaToInt(L, 1);
        int objectHandle = luaToInt(L, 2);
        bool ok;
        int paramVal = CALL_C_API(simGetEngineInt32Param, paramId, objectHandle, nullptr, &ok);
        if (ok)
        {
            luaWrap_lua_pushinteger(L, paramVal);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetEngineBoolParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getEngineBoolParam_old");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        int paramId = luaToInt(L, 1);
        int objectHandle = luaToInt(L, 2);
        bool ok;
        bool paramVal = CALL_C_API(simGetEngineBoolParam, paramId, objectHandle, nullptr, &ok);
        if (ok)
        {
            luaWrap_lua_pushboolean(L, paramVal);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetEngineFloatParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setEngineFloatParam_old");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        int paramId = luaToInt(L, 1);
        int objectHandle = luaToInt(L, 2);
        double paramVal = luaToDouble(L, 3);
        retVal = CALL_C_API(simSetEngineFloatParam, paramId, objectHandle, nullptr, paramVal);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetEngineInt32Param(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setEngineInt32Param");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0))
    {
        int paramId = luaToInt(L, 1);
        int objectHandle = luaToInt(L, 2);
        int paramVal = luaToInt(L, 3);
        retVal = CALL_C_API(simSetEngineInt32Param, paramId, objectHandle, nullptr, paramVal);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetEngineBoolParam(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setEngineBoolParam_old");

    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_bool, 0))
    {
        int paramId = luaToInt(L, 1);
        int objectHandle = luaToInt(L, 2);
        bool paramVal = luaToBool(L, 3);
        retVal = CALL_C_API(simSetEngineBoolParam, paramId, objectHandle, nullptr, paramVal);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectProperty");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simGetObjectProperty, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetObjectProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectProperty");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetObjectProperty, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectSpecialProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectSpecialProperty");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simGetObjectSpecialProperty, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetObjectSpecialProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectSpecialProperty");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetObjectSpecialProperty, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetModelProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getModelProperty");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simGetModelProperty, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simSetModelProperty(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setModelProperty");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simSetModelProperty, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetRealTimeSimulation(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getRealTimeSimulation");

    int retVal = CALL_C_API(simGetRealTimeSimulation, );

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushboolean(L, retVal);
    LUA_END(1);
}

int _simBuildMatrixQ(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.buildMatrixQ");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 3, lua_arg_number, 4))
    {
        double arr[12];
        double pos[3];
        double quaternion[4];
        getDoublesFromTable(L, 1, 3, pos);
        getDoublesFromTable(L, 2, 4, quaternion);
        if (CALL_C_API(simBuildMatrixQ, pos, quaternion, arr) == 1)
        {
            pushDoubleTableOntoStack(L, 12, arr);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simCheckCollisionEx(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.checkCollisionEx");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
    {
        double* intersections[1];
        retVal = CALL_C_API(simCheckCollisionEx, luaToInt(L, 1), luaToInt(L, 2), intersections);
        if (retVal > 0)
        {
            luaWrap_lua_pushinteger(L, retVal);
            pushDoubleTableOntoStack(L, retVal * 6, (*intersections));
            CALL_C_API(simReleaseBuffer, (char*)(*intersections));
            LUA_END(2);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    pushIntTableOntoStack(L, 0, nullptr); // empty table
    LUA_END(2);
}

int _simCheckProximitySensorEx(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.checkProximitySensorEx");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0,
                            lua_arg_number, 0))
    {
        double detPt[4];
        int detObj;
        double normVect[3];
        retVal = CALL_C_API(simCheckProximitySensorEx, luaToInt(L, 1), luaToInt(L, 2), luaToInt(L, 3), luaToDouble(L, 4),
                                                    luaToDouble(L, 5), detPt, &detObj, normVect);
        if (retVal == 1)
        {
            luaWrap_lua_pushinteger(L, retVal);
            luaWrap_lua_pushnumber(L, detPt[3]);
            pushDoubleTableOntoStack(L, 3, detPt);
            luaWrap_lua_pushinteger(L, detObj);
            pushDoubleTableOntoStack(L, 3, normVect);
            LUA_END(5);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    double dummy[3] = {0.0, 0.0, 0.0};
    luaWrap_lua_pushnumber(L, 0.0);
    pushDoubleTableOntoStack(L, 3, dummy);
    luaWrap_lua_pushinteger(L, -1);
    pushDoubleTableOntoStack(L, 3, dummy);
    LUA_END(5);
}

int _simCheckProximitySensorEx2(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.checkProximitySensorEx2");

    int retVal = -1; // means error
    if (checkOneGeneralInputArgument(L, 1, lua_arg_number, 0, false, false, &errorString, argOffset) == 2)
    { // first argument (sensor handle)
        int sensorID = luaToInt(L, 1);
        if (checkOneGeneralInputArgument(L, 3, lua_arg_number, 0, false, false, &errorString, argOffset) == 2)
        { // third argument (item type)
            int itemType = luaToInt(L, 3);
            if (checkOneGeneralInputArgument(L, 4, lua_arg_number, 0, false, false, &errorString, argOffset) == 2)
            { // forth argument (item count)
                int itemCount = luaToInt(L, 4);
                int requiredValues = itemCount * 3 * (itemType + 1);
                if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, requiredValues,
                                        lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 0,
                                        lua_arg_number, 0))
                {
                    int mode = luaToInt(L, 5);
                    double threshold = luaToDouble(L, 6);
                    double maxAngle = luaToDouble(L, 7);
                    double* vertices = new double[requiredValues];
                    getDoublesFromTable(L, 2, requiredValues, vertices);

                    double detPt[4];
                    double normVect[3];
                    retVal = CALL_C_API(simCheckProximitySensorEx2, sensorID, vertices, itemType, itemCount, mode,
                                                                 threshold, maxAngle, detPt, normVect);
                    delete[] vertices;
                    if (retVal == 1)
                    {
                        luaWrap_lua_pushinteger(L, retVal);
                        luaWrap_lua_pushnumber(L, detPt[3]);
                        pushDoubleTableOntoStack(L, 3, detPt);
                        pushDoubleTableOntoStack(L, 3, normVect);
                        LUA_END(4);
                    }
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    double dummy[3] = {0.0, 0.0, 0.0};
    luaWrap_lua_pushnumber(L, 0.0);
    pushDoubleTableOntoStack(L, 3, dummy);
    pushDoubleTableOntoStack(L, 3, dummy);
    LUA_END(4);
}

int _simGetObjectMatrix(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectMatrix");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_integer | lua_arg_optional, 0))
    {
        double arr[12];
        int rel = sim_handle_world;
        if (luaWrap_lua_isinteger(L, 2))
            rel = luaToInt(L, 2);
        if (CALL_C_API(simGetObjectMatrix, luaToInt(L, 1), rel, arr) == 1)
        {
            pushDoubleTableOntoStack(L, 12, arr); // Success
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectMatrix(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectMatrix");

    if (luaWrap_lua_isnonbuffertable(L, 2))
    {
        if (checkInputArguments(L, &errorString, argOffset, lua_arg_integer, 0, lua_arg_number, 12,
                                lua_arg_integer | lua_arg_optional, 0))
        {
            double arr[12];
            getDoublesFromTable(L, 2, 12, arr);
            int rel = sim_handle_world;
            if (luaWrap_lua_isinteger(L, 3))
                rel = luaToInt(L, 3);
            CALL_C_API(simSetObjectMatrix, luaToInt(L, 1), rel, arr);
        }
    }
    else
    { // old
        if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 12))
        {
            double arr[12];
            getDoublesFromTable(L, 3, 12, arr);
            CALL_C_API(simSetObjectMatrix, luaToInt(L, 1), luaToInt(L, 2), arr);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetObjectOrientation(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectOrientation");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_integer | lua_arg_optional, 0))
    {
        int rel = sim_handle_world;
        if (luaWrap_lua_isinteger(L, 2))
            rel = luaToInt(L, 2);
        double coord[3];
        if (CALL_C_API(simGetObjectOrientation, luaToInt(L, 1), rel, coord) == 1)
        {
            pushDoubleTableOntoStack(L, 3, coord);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simSetObjectOrientation(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.setObjectOrientation");

    if (luaWrap_lua_isnonbuffertable(L, 2))
    {
        if (checkInputArguments(L, &errorString, argOffset, lua_arg_integer, 0, lua_arg_number, 3,
                                lua_arg_integer | lua_arg_optional, 0))
        {
            double coord[3];
            getDoublesFromTable(L, 2, 3, coord);
            int rel = sim_handle_world;
            if (luaWrap_lua_isinteger(L, 3))
                rel = luaToInt(L, 3);
            CALL_C_API(simSetObjectOrientation, luaToInt(L, 1), rel, coord);
        }
    }
    else
    { // old
        if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_number, 3))
        {
            double coord[3];
            getDoublesFromTable(L, 3, 3, coord);
            CALL_C_API(simSetObjectOrientation, luaToInt(L, 1), luaToInt(L, 2), coord);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetRotationAxis(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getRotationAxis");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 7, lua_arg_number, 7))
    {
        double inM0[12];
        double inM1[12];

        C4X4Matrix mStart;
        C4X4Matrix mGoal;
        if (luaWrap_lua_rawlen(L, 1) >= 12)
        { // we have a matrix
            getDoublesFromTable(L, 1, 12, inM0);
            getDoublesFromTable(L, 2, 12, inM1);
            mStart.setData(inM0);
            mGoal.setData(inM1);
        }
        else
        { // we have a pose
            getDoublesFromTable(L, 1, 7, inM0);
            getDoublesFromTable(L, 2, 7, inM1);
            C7Vector p;
            p.setData(inM0, true);
            mStart = p.getMatrix();
            p.setData(inM1, true);
            mGoal = p.getMatrix();
        }

        // Following few lines taken from the quaternion interpolation part:
        C4Vector AA(mStart.M.getQuaternion());
        C4Vector BB(mGoal.M.getQuaternion());
        if (AA(0) * BB(0) + AA(1) * BB(1) + AA(2) * BB(2) + AA(3) * BB(3) < 0.0)
            AA = AA * -1.0;
        C4Vector r((AA.getInverse() * BB).getAngleAndAxis());

        C3Vector v(r(1), r(2), r(3));
        v = AA * v;

        double axis[3];
        axis[0] = v(0);
        axis[1] = v(1);
        axis[2] = v(2);
        double l = sqrt(v(0) * v(0) + v(1) * v(1) + v(2) * v(2));
        if (l != 0.0)
        {
            axis[0] /= l;
            axis[1] /= l;
            axis[2] /= l;
        }

        pushDoubleTableOntoStack(L, 3, axis);
        luaWrap_lua_pushnumber(L, r(0));
        LUA_END(2);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simRotateAroundAxis(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.rotateAroundAxis");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 7, lua_arg_number, 3, lua_arg_number, 3, lua_arg_number,
                            0))
    {
        double inM[12];
        double axis[3];
        double ppos[3];
        double outM[12];
        getDoublesFromTable(L, 2, 3, axis);
        getDoublesFromTable(L, 3, 3, ppos);

        C7Vector tr;
        if (luaWrap_lua_rawlen(L, 1) >= 12)
        { // we have a matrix
            getDoublesFromTable(L, 1, 12, inM);
            C4X4Matrix m;
            m.setData(inM);
            tr = m.getTransformation();
        }
        else
        { // we have a pose
            getDoublesFromTable(L, 1, 7, inM);
            tr.setData(inM, true);
        }
        C3Vector ax(axis);
        C3Vector pos(ppos);

        double alpha = -atan2(ax(1), ax(0));
        double beta = atan2(-sqrt(ax(0) * ax(0) + ax(1) * ax(1)), ax(2));
        tr.X -= pos;
        C7Vector r;
        r.X.clear();
        r.Q.setEulerAngles(0.0, 0.0, alpha);
        tr = r * tr;
        r.Q.setEulerAngles(0.0, beta, 0.0);
        tr = r * tr;
        r.Q.setEulerAngles(0.0, 0.0, luaToDouble(L, 4));
        tr = r * tr;
        r.Q.setEulerAngles(0.0, -beta, 0.0);
        tr = r * tr;
        r.Q.setEulerAngles(0.0, 0.0, -alpha);
        tr = r * tr;
        tr.X += pos;
        if (luaWrap_lua_rawlen(L, 1) >= 12)
        { // we have a matrix
            tr.getMatrix().getData(outM);
            pushDoubleTableOntoStack(L, 12, outM);
        }
        else
        { // we have a pose
            tr.getData(outM, true);
            pushDoubleTableOntoStack(L, 7, outM);
        }
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simBuildIdentityMatrix(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.buildIdentityMatrix");

    double arr[12];
    CALL_C_API(simBuildIdentityMatrix, arr);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    pushDoubleTableOntoStack(L, 12, arr);
    LUA_END(1);
}

int _simBuildMatrix(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.buildMatrix");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 3, lua_arg_number, 3))
    {
        double arr[12];
        double pos[3];
        double euler[3];
        getDoublesFromTable(L, 1, 3, pos);
        getDoublesFromTable(L, 2, 3, euler);
        if (CALL_C_API(simBuildMatrix, pos, euler, arr) == 1)
        {
            pushDoubleTableOntoStack(L, 12, arr);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simBuildPose(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.buildPose");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 3, lua_arg_number, 3))
    {
        double tr[7];
        double pos[3];
        double axis1[3];
        double axis2[3];
        int mode = 0;
        getDoublesFromTable(L, 1, 3, pos);
        getDoublesFromTable(L, 2, 3, axis1);
        int res = checkOneGeneralInputArgument(L, 3, lua_arg_integer, 0, true, false, &errorString, argOffset);
        if (res >= 0)
        {
            if (res == 2)
                mode = luaToInt(L, 3);
            if (mode == 0)
            {
                if (CALL_C_API(simBuildPose, pos, axis1, tr) == 1)
                {
                    pushDoubleTableOntoStack(L, 7, tr);
                    LUA_END(1);
                }
            }
            else
            {
                res = checkOneGeneralInputArgument(L, 4, lua_arg_number, 3, mode < 4, false, &errorString, argOffset);
                if (res >= 0)
                {
                    if (res == 2)
                        getDoublesFromTable(L, 4, 3, axis2);
                    C3X3Matrix m;
                    C3Vector a1(axis1);
                    a1.normalize();
                    if (mode < 4)
                    {
                        int i1 = mode - 1;
                        int i2 = i1 + 1;
                        if (i2 > 2)
                            i2 = 0;
                        int i3 = i2 + 1;
                        if (i3 > 2)
                            i3 = 0;
                        C3Vector a2;
                        if (a1(2) < 0.8)
                            a2.setData(0.0, 0.0, 1.0);
                        else
                            a2.setData(1.0, 0.0, 0.0);
                        m.axis[i1] = a1;
                        m.axis[i3] = (a1 ^ a2).getNormalized();
                        m.axis[i2] = m.axis[i3] ^ a1;
                    }
                    else
                    {
                        int i1 = mode - 4;
                        if (mode >= 7)
                            i1 = mode - 7;
                        int i2 = i1 + 1;
                        if (i2 > 2)
                            i2 = 0;
                        int i3 = i2 + 1;
                        if (i3 > 2)
                            i3 = 0;
                        C3Vector a2(axis2);
                        a2.normalize();
                        m.axis[i1] = a1;
                        if (mode < 7)
                        {
                            m.axis[i3] = (a1 ^ a2).getNormalized();
                            m.axis[i2] = m.axis[i3] ^ a1;
                        }
                        else
                        {
                            m.axis[i2] = (a2 ^ a1).getNormalized();
                            m.axis[i3] = a1 ^ m.axis[i2];
                        }
                    }
                    tr[0] = pos[0];
                    tr[1] = pos[1];
                    tr[2] = pos[2];
                    m.getQuaternion().getData(tr + 3, true);
                    pushDoubleTableOntoStack(L, 7, tr);
                    LUA_END(1);
                }
            }
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetEulerAnglesFromMatrix(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getEulerAnglesFromMatrix");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 12))
    {
        double arr[12];
        double euler[3];
        getDoublesFromTable(L, 1, 12, arr);
        if (CALL_C_API(simGetEulerAnglesFromMatrix, arr, euler) == 1)
        {
            pushDoubleTableOntoStack(L, 3, euler);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetMatrixInverse(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getMatrixInverse");
    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 12))
    {
        double arr[12];
        getDoublesFromTable(L, 1, 12, arr);
        retVal = CALL_C_API(simInvertMatrix, arr);
        if (retVal >= 0)
        {
            pushDoubleTableOntoStack(L, 12, arr); // Success
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetPoseInverse(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getPoseInverse");
    int retVal = -1;
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 7))
    {
        double arr[7];
        getDoublesFromTable(L, 1, 7, arr);
        retVal = CALL_C_API(simInvertPose, arr);
        if (retVal >= 0)
        {
            pushDoubleTableOntoStack(L, 7, arr); // Success
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simMultiplyMatrices(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.multiplyMatrices");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 12, lua_arg_number, 12))
    {
        double inM0[12];
        double inM1[12];
        double outM[12];
        getDoublesFromTable(L, 1, 12, inM0);
        getDoublesFromTable(L, 2, 12, inM1);
        if (CALL_C_API(simMultiplyMatrices, inM0, inM1, outM) != -1)
        {
            pushDoubleTableOntoStack(L, 12, outM);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simMultiplyPoses(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.multiplyPoses");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 7, lua_arg_number, 7))
    {
        double inP0[7];
        double inP1[7];
        double outP[7];
        getDoublesFromTable(L, 1, 7, inP0);
        getDoublesFromTable(L, 2, 7, inP1);
        if (CALL_C_API(simMultiplyPoses, inP0, inP1, outP) != -1)
        {
            pushDoubleTableOntoStack(L, 7, outP);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simInterpolateMatrices(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.interpolateMatrices");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 12, lua_arg_number, 12, lua_arg_number, 0))
    {
        double inM0[12];
        double inM1[12];
        double outM[12];
        getDoublesFromTable(L, 1, 12, inM0);
        getDoublesFromTable(L, 2, 12, inM1);
        if (CALL_C_API(simInterpolateMatrices, inM0, inM1, luaToDouble(L, 3), outM) != -1)
        {
            pushDoubleTableOntoStack(L, 12, outM);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simInterpolatePoses(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.interpolatePoses");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 7, lua_arg_number, 7, lua_arg_number, 0))
    {
        double inP0[7];
        double inP1[7];
        double outP[7];
        getDoublesFromTable(L, 1, 7, inP0);
        getDoublesFromTable(L, 2, 7, inP1);
        if (CALL_C_API(simInterpolatePoses, inP0, inP1, luaToDouble(L, 3), outP) != -1)
        {
            pushDoubleTableOntoStack(L, 7, outP);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simPoseToMatrix(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.poseToMatrix");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 7))
    {
        double inP[7];
        double outM[12];
        getDoublesFromTable(L, 1, 7, inP);
        if (CALL_C_API(simPoseToMatrix, inP, outM) != -1)
        {
            pushDoubleTableOntoStack(L, 12, outM);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simMatrixToPose(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.matrixToPose");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 12))
    {
        double inM[12];
        double outP[7];
        getDoublesFromTable(L, 1, 12, inM);
        if (CALL_C_API(simMatrixToPose, inM, outP) != -1)
        {
            pushDoubleTableOntoStack(L, 7, outP);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simMultiplyVector(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.multiplyVector");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 4, lua_arg_number, 3))
    {
        double matr[12];
        std::vector<double> vect;
        size_t cnt = luaWrap_lua_rawlen(L, 2) / 3;
        vect.resize(cnt * 3);
        getDoublesFromTable(L, 2, cnt * 3, &vect[0]);

        if (luaWrap_lua_rawlen(L, 1) >= 12)
        { // we have a matrix
            getDoublesFromTable(L, 1, 12, matr);
            C4X4Matrix m;
            m.setData(matr);
            for (size_t i = 0; i < cnt; i++)
            {
                C3Vector v(&vect[3 * i]);
                (m * v).getData(&vect[3 * i]);
            }
        }
        else if (luaWrap_lua_rawlen(L, 1) == 7)
        { // we have a pose
            getDoublesFromTable(L, 1, 7, matr);
            C7Vector tr;
            tr.X.setData(matr);
            tr.Q.setData(matr + 3, true);
            for (size_t i = 0; i < cnt; i++)
            {
                C3Vector v(&vect[3 * i]);
                (tr * v).getData(&vect[3 * i]);
            }
        }
        else if (luaWrap_lua_rawlen(L, 1) == 4)
        { // we have a quaternion
            getDoublesFromTable(L, 1, 4, matr);
            C4Vector q;
            q.setData(matr, true);
            for (size_t i = 0; i < cnt; i++)
            {
                C3Vector v(&vect[3 * i]);
                (q * v).getData(&vect[3 * i]);
            }
        }

        pushDoubleTableOntoStack(L, 3 * cnt, &vect[0]);
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simGetObjectChild(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectChild");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0))
        retVal = CALL_C_API(simGetObjectChild, luaToInt(L, 1), luaToInt(L, 2));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetObjectParent(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getObjectParent");

    int retVal = -1; // error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simGetObjectParent, luaToInt(L, 1));

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

int _simGetCollectionObjects(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.getCollectionObjects");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        int handle = luaToInt(L, 1);
        int cnt;
        int* objHandles = CALL_C_API(simGetCollectionObjects, handle, &cnt);
        pushIntTableOntoStack(L, cnt, objHandles);
        delete[] objHandles;
        LUA_END(1);
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simReadForceSensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.readForceSensor");

    int retVal = -1;
    double force[3] = {0.0, 0.0, 0.0};
    double torque[3] = {0.0, 0.0, 0.0};
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
        retVal = CALL_C_API(simReadForceSensor, luaToInt(L, 1), force, torque);

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    pushDoubleTableOntoStack(L, 3, force);
    pushDoubleTableOntoStack(L, 3, torque);
    LUA_END(3);
}

int _simCheckVisionSensorEx(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.checkVisionSensorEx");

    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0, lua_arg_number, 0, lua_arg_bool, 0))
    {
        bool returnImage = luaToBool(L, 3);
        int arg1 = luaToInt(L, 1);
        int handleFlags = arg1 & sim_handleflag_flagmask;
        int sensHandle = arg1 & sim_handleflag_handlemask;
        int res[2];
        CALL_C_API(simGetVisionSensorRes, sensHandle, res);
        float* buffer = CALL_C_API(simCheckVisionSensorEx, luaToInt(L, 1), luaToInt(L, 2), returnImage);
        if (buffer != nullptr)
        {
            if ((handleFlags & sim_handleflag_codedstring) != 0)
            {
                if (returnImage)
                {
                    unsigned char* buff2 = new unsigned char[res[0] * res[1] * 3];
                    for (size_t i = 0; i < res[0] * res[1] * 3; i++)
                        buff2[i] = (unsigned char)(buffer[i] * 255.1);
                    luaWrap_lua_pushbuffer(L, (const char*)buff2, res[0] * res[1] * 3);
                    delete[] buff2;
                }
                else
                    luaWrap_lua_pushbuffer(L, (const char*)buffer, res[0] * res[1] * sizeof(float));
            }
            else
            {
                if (returnImage)
                    pushFloatTableOntoStack(L, res[0] * res[1] * 3, buffer);
                else
                    pushFloatTableOntoStack(L, res[0] * res[1], buffer);
            }
            CALL_C_API(simReleaseBuffer, buffer);
            LUA_END(1);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    LUA_END(0);
}

int _simReadProximitySensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.readProximitySensor");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        double detPt[4];
        int detectedObjectID;
        double surfaceNormal[3];
        retVal = CALL_C_API(simReadProximitySensor, luaToInt(L, 1), detPt, &detectedObjectID, surfaceNormal);
        if (retVal == 1)
        {
            luaWrap_lua_pushinteger(L, retVal);
            luaWrap_lua_pushnumber(L, detPt[3]);
            pushDoubleTableOntoStack(L, 3, detPt);
            luaWrap_lua_pushinteger(L, detectedObjectID);
            pushDoubleTableOntoStack(L, 3, surfaceNormal);
            LUA_END(5);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    luaWrap_lua_pushnumber(L, 0.0);
    double ft[3] = {0.0, 0.0, 0.0};
    pushDoubleTableOntoStack(L, 3, ft);
    luaWrap_lua_pushinteger(L, -1);
    pushDoubleTableOntoStack(L, 3, ft);
    LUA_END(5);
}

int _simReadVisionSensor(luaWrap_lua_State* L)
{
    TRACE_LUA_API;
    LUA_START("sim.readVisionSensor");

    int retVal = -1; // means error
    if (checkInputArguments(L, &errorString, argOffset, lua_arg_number, 0))
    {
        double* auxVals = nullptr;
        int* auxValsCount = nullptr;
        retVal = CALL_C_API(simReadVisionSensor, luaToInt(L, 1), &auxVals, &auxValsCount);
        if (retVal != -1)
        {
            luaWrap_lua_pushinteger(L, retVal);
            int tableCount = 0;
            if (auxValsCount != nullptr)
            {
                tableCount = auxValsCount[0];
                int off = 0;
                for (int i = 0; i < tableCount; i++)
                {
                    pushDoubleTableOntoStack(L, auxValsCount[i + 1], auxVals + off);
                    off += auxValsCount[i + 1];
                }
                delete[] auxValsCount;
                delete[] auxVals;
            }
            for (int i = tableCount; i < 2; i++)
            {
                pushDoubleTableOntoStack(L, 0, nullptr); // return at least 2 aux packets, even empty
                tableCount++;
            }
            LUA_END(1 + tableCount);
        }
    }

    LUA_RAISE_ERROR_OR_YIELD_IF_NEEDED(); // we might never return from this!
    luaWrap_lua_pushinteger(L, retVal);
    LUA_END(1);
}

