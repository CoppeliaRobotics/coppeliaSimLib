#pragma once

#include <string>

// Old:
// ************************
#include <vector>
#ifndef WIN_SIM
    #include <pthread.h>
#endif
struct SThreadAndMsg_old
{
    VTHREAD_ID_TYPE threadId;
    std::string message;
};
// ************************

#define SIM_ERROR_SIMULATOR_NOT_INITIALIZED             "simulator not initialized."
#define SIM_ERRROR_EDIT_MODE_ACTIVE             "edit mode is active."
#define SIM_ERROR_CANNOT_SET_GET_PARAM_LAUNCH           "cannot set/get parameter: simulator launch state is wrong."
#define SIM_ERROR_CANNOT_SET_GET_PARAM_WINDOW           "cannot set/get parameter: simulator window initialization state is wrong."
#define SIM_ERROR_CANNOT_SET_GET_PARAM_SIM          "cannot set/get parameter: simulation run state is wrong."
#define SIM_ERROR_OBJECT_INEXISTANT             "object does not exist."
#define SIM_ERROR_OBJECT_INEXISTANT_OR_ILL_FORMATTED_PATH  "object does not exist, or alias/path is ill formatted."
#define SIM_ERROR_VISION_SENSOR_INEXISTANT          "vision sensor does not exist."
#define SIM_ERROR_PATH_INEXISTANT           "path does not exist."
#define SIM_ERROR_ENTITY_INEXISTANT             "entity does not exist."
#define SIM_ERROR_COLLECTION_INEXISTANT             "collection does not exist."
#define SIM_ERROR_COLLISION_INEXISTANT          "collision object does not exist."
#define SIM_ERROR_DISTANCE_INEXISTANT           "distance object does not exist."
#define SIM_ERROR_OBJECT_NOT_JOINT              "object is not a joint."
#define SIM_ERROR_OBJECT_NOT_SHAPE          "object is not a shape."
#define SIM_ERROR_OBJECT_NOT_PROX_SENSOR            "object is not a proximity sensor."
#define SIM_ERROR_OBJECT_NOT_MILL           "object is not a mill."
#define SIM_ERROR_OBJECT_NOT_FORCE_SENSOR           "object is not a force sensor."
#define SIM_ERROR_OBJECT_NOT_VISION_SENSOR          "object is not a vision sensor."
#define SIM_ERROR_OBJECT_NOT_CAMERA                 "object is not a camera."
#define SIM_ERROR_OBJECT_NOT_GRAPH          "object is not a graph."
#define SIM_ERROR_OBJECT_NOT_PATH           "object is not a path."
#define SIM_ERROR_OBJECT_NOT_LIGHT          "object is not a light."
#define SIM_ERROR_OBJECT_NOT_DUMMY          "object is not a dummy."
#define SIM_ERROR_OBJECT_NOT_OCTREE             "object is not an OC tree."
#define SIM_ERROR_OBJECT_NOT_POINTCLOUD         "object is not a point cloud."
#define SIM_ERROR_UI_INEXISTANT             "UI does not exist."
#define SIM_ERROR_UI_BUTTON_INEXISTANT          "UI button does not exist."
#define SIM_ERROR_IK_GROUP_INEXISTANT           "IK group does not exist."
#define SIM_ERROR_IK_ELEMENT_INEXISTANT             "IK element does not exist."
#define SIM_ERROR_INVALID_COLLISION_PAIRS       "invalid collision pairs."
#define SIM_ERROR_IK_PLUGIN_NOT_FOUND       "IK plugin was not found."
#define SIM_ERROR_MECHANISM_INEXISTANT          "mechanism does not exist."
#define SIM_ERROR_BUFFER_INEXISTANT             "buffer does not exist."
#define SIM_ERROR_PATH_PLANNING_OBJECT_INEXISTANT           "path planning object does not exist."
#define SIM_ERROR_MOTION_PLANNING_OBJECT_INEXISTANT         "motion planning object does not exist."
#define SIM_ERROR_SCRIPT_INEXISTANT             "script does not exist."
#define SIM_ERROR_ILLEGAL_SCRIPT_TYPE             "illegal script type."
#define SIM_ERROR_SCRIPT_NOT_INITIALIZED        "script is not initialized (or has already ended)"
#define SIM_ERROR_SPECIFIED_SCRIPT_IS_NOT_MAIN_OR_CHILD "specified script is not a main nor a child script."
#define SIM_ERROR_NO_ASSOCIATED_CHILD_SCRIPT_FOUND "no associated child script found."
#define SIM_ERROR_PAGE_INEXISTANT               "page does not exist."
#define SIM_ERROR_ILLEGAL_OBJECT_NAME           "illegal object name."
#define SIM_ERROR_ILLEGAL_OBJECT_ALIAS           "illegal object alias."
#define SIM_ERROR_ILLEGAL_COLLECTION_NAME           "illegal collection name."
#define SIM_ERROR_JOINT_SPHERICAL           "joint is spherical."
#define SIM_ERROR_JOINT_NOT_SPHERICAL           "joint is not spherical."
#define SIM_ERROR_JOINT_NOT_IN_FORCE_TORQUE_MODE            "joint is not in force/torque mode."
#define SIM_ERROR_SIMULATION_NOT_STOPPED            "simulation is not stopped."
#define SIM_ERROR_SIMULATION_NOT_RUNNING            "simulation is not running."
#define SIM_ERROR_SIMULATION_NOT_REAL_TIME          "simulation is not real-time."
#define SIM_ERROR_FILE_NOT_FOUND            "file not found."
#define SIM_ERROR_SCENE_COULD_NOT_BE_READ           "scene could not be read."
#define SIM_ERROR_MODEL_COULD_NOT_BE_READ           "model could not be read."
#define SIM_ERROR_UI_COULD_NOT_BE_READ          "UI could not be read."
#define SIM_ERROR_WRONG_POS_SIZE_PARAMS             "wrong position/size parameters."
#define SIM_ERROR_BUTTON_NOT_SLIDER             "button is not a slider."
#define SIM_ERROR_SCENE_LOCKED          "scene is locked."
#define SIM_ERROR_SCENE_COULD_NOT_BE_SAVED          "scene could not be saved."
#define SIM_ERROR_OBJECT_NOT_MODEL_BASE             "object is not tagged as model base."
#define SIM_ERROR_THREADED_SCRIPT_DESTROYING_OBJECTS_WITH_ACTIVE_SCRIPTS "threaded scripts cannot destroy objects that are linked to initialized scripts other that the calling script."
#define SIM_ERROR_MODEL_COULD_NOT_BE_SAVED          "model could not be saved."
#define SIM_ERROR_UI_COULD_NOT_BE_SAVED             "UI could not be saved."
#define SIM_ERROR_TOO_MANY_TEMP_OBJECTS             "too many temporary path search objects."
#define SIM_ERROR_PATH_PLANNING_OBJECT_NOT_CONSISTENT           "path planning object is not consistent."
#define SIM_ERROR_PATH_PLANNING_OBJECT_NOT_CONSISTENT_ANYMORE "path planning object is not consistent anymore."
#define SIM_ERROR_TEMP_PATH_SEARCH_OBJECT_INEXISTANT            "temporary path search object does not exist."
#define SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING           "object not tagged for explicit handling."
#define SIM_ERROR_SCRIPT_NOT_TAGGED_FOR_EXPLICIT_HANDLING           "script not tagged for explicit handling."
#define SIM_ERROR_SCRIPT_WAS_DESTROYED          "script was destroyed."
#define SIM_ERROR_SCRIPT_NOT_CHILD_OR_CUSTOMIZATION_SCRIPT           "script is not a child or customization script."
#define SIM_ERROR_OBJECT_ALREADY_ASSOCIATED_WITH_SCRIPT_TYPE         "object is already associated with such a script type."
#define SIM_ERROR_CUSTOM_LUA_FUNC_COULD_NOT_BE_REGISTERED           "custom function could not be registered."
#define SIM_ERROR_CUSTOM_LUA_VAR_COULD_NOT_BE_REGISTERED            "custom variable could not be registered."
#define SIM_ERROR_MAIN_WINDOW_NOT_INITIALIZED           "main window not initialized."
#define SIM_ERROR_OPERATION_FAILED          "operation failed."
#define SIM_ERROR_INVALID_PARAMETER             "invalid parameter."
#define SIM_ERROR_INVALID_FORMAT             "invalid format."
#define SIM_ERROR_INVALID_ARGUMENT          "invalid argument."
#define SIM_ERROR_EMPTY_STRING_NOT_ALLOWED          "empty string is not allowed."
#define SIM_ERROR_INVALID_CURVE_ID              "invalid curve id."
#define SIM_ERROR_INVALID_ARGUMENTS             "invalid arguments."
#define SIM_ERROR_INVALID_FIRST_ARGUMENT        "invalid first argument."
#define SIM_ERROR_INVALID_DIMENSIONS            "invalid dimensions."
#define SIM_ERROR_INVALID_ITEM_HANDLE           "invalid item handle."
#define SIM_ERROR_INVALID_HANDLE            "invalid handle."
#define SIM_ERROR_INVALID_HANDLES           "invalid handles."
#define SIM_ERROR_INVALID_TARGET_HANDLE             "invalid target handle."
#define SIM_ERROR_INVALID_DATA_HEADER           "invalid data header."
#define SIM_ERROR_INVALID_DATA_NAME             "invalid data name."
#define SIM_ERROR_INVALID_DATA          "invalid data."
#define SIM_ERROR_INVALID_BUFFER_SIZE   "invalid buffer size."
#define SIM_ERROR_INVALID_TYPE          "invalid type."
#define SIM_ERROR_INVALID_INDEX             "invalid index."
#define SIM_ERROR_INVALID_NUMBER_INPUT      "invalid number input."
#define SIM_ERROR_INVALID_ANTENNA_HANDLE            "invalid antenna handle."
#define SIM_ERROR_INVALID_DATA_STREAM           "invalid data stream."
#define SIM_ERROR_INVALID_PORT_HANDLE           "invalid port handle."
#define SIM_ERROR_INVALID_FILE_FORMAT           "invalid file format."
#define SIM_ERROR_INVALID_PLUGIN_NAME           "invalid plugin name."
#define SIM_ERROR_FOUND_INVALID_HANDLES     "found invalid handles."
#define SIM_ERROR_DATA_STREAM_NOT_USER_DEFINED          "data stream is not user-defined."
#define SIM_ERROR_PATH_EMPTY            "path is empty."
#define SIM_ERROR_INVALID_PATH            "invalid path data."
#define SIM_ERROR_INVALID_CTRL_PT       "invalid control point."
#define SIM_ERROR_PORT_NOT_OPEN             "port is not open."
#define SIM_ERROR_CANNOT_BE_CALLED_FROM_MAIN_THREAD             "cannot be called from the main thread."
#define SIM_ERROR_OBJECT_IS_NOT_VIEWABLE            "associated object is not viewable."
#define SIM_ERROR_NOT_ENOUGH_SHAPES             "not enough shapes specified."
#define SIM_ERROR_CANNOT_DIVIDE_COMPOUND_SHAPE "cannot divide compound shape."
#define SIM_ERROR_MATERIAL_INEXISTANT           "material does not exist."
#define SIM_ERROR_TEXTURE_INEXISTANT            "texture does not exist."
#define SIM_ERROR_BAD_INDICES            "bad indices."
#define SIM_ERROR_BAD_VERTICES            "bad vertices."

#define SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS "the function requires more arguments."
#define SIM_ERROR_ONE_ARGUMENT_TYPE_IS_WRONG "one of the function's argument type is not correct."
#define SIM_ERROR_ONE_TABLE_SIZE_IS_WRONG "incorrect table size."
#define SIM_ERROR_ONE_STRING_SIZE_IS_WRONG "incorrect string size."
#define SIM_ERROR_NOT_MAIN_NOR_CHILD_SCRIPT "script is not a main script nor a child script."
#define SIM_ERROR_NON_EXPLICIT_CHILD_SCRIPT_EXECUTIONS_WERE_DELEGATED "non-explicit child script executions were delegated."
#define SIM_ERROR_THREAD_LAUNCHING_SCRIPTS_WILL_NOT_BE_CALLED_FROM_SENSING_SECTION "scripts launching a thread will not be called when in a sensing section."
#define SIM_ERROR_INVALID_CHILD_SCRIPT_HANDLE_OR_CHILD_SCRIPT_NOT_IN_CURRENT_HIERARCHY "invalid child script handle, or child script not in current hierarchy tree."
#define SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT "can only be called from the main script."
#define SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT_OR_CHILD_SCRIPT "can only be called from the main script or a child script."
#define SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_A_THREAD "can only be called from a thread."
#define SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_THE_MAIN_THREAD "can only be called from the main thread."
#define SIM_ERROR_CANNOT_BE_CALLED_IF_DIDN_T_LAUNCH_THREAD "cannot be called from a script that didn't launch a thread."
#define SIM_ERROR_ARGUMENT_VALID_ONLY_WITH_CHILD_SCRIPTS "argument only valid with child scripts."
#define SIM_ERROR_SCRIPT_MUST_RUN_IN_THREAD_FOR_MODAL_OPERATION "script must run in a thread for modal operation."
#define SIM_ERROR_CANNOT_USE_THAT_STYLE_IN_MODAL_OPERATION "can't use that dialog style in modal operation."
#define SIM_ERROR_TABLE_CONTAINS_INVALID_TYPES "table contains invalid types."
#define SIM_ERROR_FUNCTION_DEPRECATED_AND_HAS_NO_EFFECT "function is deprecated and has no effect."
#define SIM_ERROR_OBJECT_OR_TARGET_OBJECT_DOES_NOT_EXIST "object or target object does not exist."
#define SIM_ERROR_OBJECT_IS_SAME_AS_TARGET_OBJECT "object is same as target object."
#define SIM_ERROR_TARGET_OBJECT_IS_NOT_A_PATH "target object is not a path."
#define SIM_ERROR_CANNOT_OVERWRITE_STATIC_CURVE "cannot overwrite static curve."
#define SIM_ERROR_OBJECT_IS_ANCESTOR_OF_DESIRED_PARENT "object is ancestor of desired parent."
#define SIM_ERROR_BLOCKING_OPERATION_ONLY_FROM_THREAD "blocking operation only available when called from a thread."
#define SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ "could not lock resources for data read operation."
#define SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE "could not lock resources for data write operation."
#define SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_NON_THREADED_CHILD_SCRIPTS "can only be called from non-threaded child scripts."
#define SIM_ERROR_AUTOMATIC_CASCADING_CALLS_NOT_DISABLED "cannot be called when the automatic cascading calls are not disabled."
#define SIM_ERROR_INVALID_RESOLUTION "invalid resolution."
#define SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION "failed calling script function."
#define SIM_ERROR_SCRIPT_FUNCTION_INEXISTANT "script function does not exist."
#define SIM_ERROR_ERROR_IN_SCRIPT_FUNCTION "error in script function."
#define SIM_ERROR_INVALID_STACK_CONTENT "invalid stack content."
#define SIM_ERROR_MISSING_PLUGIN_NAME "missing plugin name."
#define SIM_ERROR_FUNCTIONALITY_NOT_APPLICABLE_WITH_CURRENT_SCRIPT "functionality not applicable with current script."
#define SIM_ERROR_FUNCTIONALITY_NOT_APPLICABLE_WITH_CURRENT_SCRIPT_STATE "functionality not applicable with current script state."
#define SIM_ERROR_CB_NOT_APPLICABLE_WITH_CURRENT_SCRIPT "callback not applicable with current script."
#define SIM_ERROR_MUST_BE_CALLED_FROM_ADDON_OR_SANDBOX_SCRIPT "must be called from an add-on, or from the sandbox script."
#define SIM_ERROR_BAD_TEXTURE_COORD_SIZE "bad texture coordinates size."
#define SIM_ERROR_CANNOT_BE_COMPOUND_SHAPE "shape cannot be a compound shape."
#define SIM_ERROR_ASSIMP_PLUGIN_NOT_FOUND "assimp plugin was not found."
#define SIM_ERROR_INVALID_MODULE_INFO_TYPE "invalid module info type."
#define SIM_ERROR_COULD_NOT_SET_PARAMETER "could not set parameter."
#define SIM_ERROR_COULD_NOT_FIND_RUCKIG "could not find the Ruckig plugin."
#define SIM_ERROR_RUCKIG_OBJECT_INEXISTANT "ruckig object does not exist."
#define SIM_ERROR_STRING_NOT_RECOGNIZED_AS_FUNC_OR_CONST "string not recognized as function or constant."


// Class is fully static
class CApiErrors
{
public:
    CApiErrors();
    virtual ~CApiErrors();

    static void setLastWarningOrError(const char* functionName,const char* errMsg);
    static std::string getAndClearLastWarningOrError();

    // Old:
    static void clearThreadBasedFirstCapiErrorAndWarning_old();
    static void setThreadBasedFirstCapiWarning_old(const char* msg);
    static std::string getAndClearThreadBasedFirstCapiWarning_old();
    static void setThreadBasedFirstCapiError_old(const char* msg);
    static std::string getAndClearThreadBasedFirstCapiError_old();

private:
    static std::string _lastWarningOrError; // warnings start with "warning@"

    // Old:
    static void _clearThreadBasedFirstCapiMsg_old(std::vector<SThreadAndMsg_old>& vect);
    static void _setThreadBasedFirstCapiMsg_old(std::vector<SThreadAndMsg_old>& vect,const char* msg);
    static std::string _getAndClearThreadBasedFirstCapiMsg_old(std::vector<SThreadAndMsg_old>& vect);
    static std::vector<SThreadAndMsg_old> _threadBasedFirstCapiWarning_old;
    static std::vector<SThreadAndMsg_old> _threadBasedFirstCapiError_old;
};
