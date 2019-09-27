
//search for following strings: //modifiedOn
//                              //addedOn

#pragma once

#include "v_repStringTableBase.h"

#define IDSN_DEBUG_MENU_ITEM "Debug"
#define IDSN_SHOW_INTERNAL_FUNCTION_ACCESS_DEBUG_MENU_ITEM "Debug internal function access (will heavily slow down V-REP)"
#define IDSN_SHOW_C_API_ACCESS_DEBUG_MENU_ITEM "Debug C API access (will drastically slow down V-REP)"
#define IDSN_SHOW_LUA_API_ACCESS_DEBUG_MENU_ITEM "Debug Lua API access (will slow down V-REP)"
#define IDSN_SEND_DEBUG_INFO_TO_FILE_MENU_ITEM "Send debug info to file (debugLog.txt)"
#define IDSN_DISABLE_FRUSTUM_CULLING_DEBUG_MENU_ITEM "Disable frustum culling"
#define IDSN_DISABLE_DISTANCE_CACHING_DEBUG_MENU_ITEM "Disable distance caching"
#define IDSN_VISUALIZE_OBB_STRUCTURE_DEBUG_MENU_ITEM "Visualize prepared OBB calculation structures"
#define IDSN_SCENE_SAVE_FAILED_WARNING "Failed saving the scene"

#define IDSN_INERTIA_FRAME "Inertia frame"
#define IDSN_INERTIA_FRAME_RELATIVE_TO_WORLD "The inertia frame is now expressed relative to the absolute reference frame. Clicking 'Apply to selection' will from now on apply the same absolute position/orientation of the inertia frames."
#define IDSN_MOTION_PLANNING "Motion planning"
#define IDSN_MOTION_PLANNING_SELECTION_INVALID "Make sure that you have selected only the joints (revolute or prismatic) involved in the motion planning task."
#define IDSN_JOINT_NAME "Joint name"
#define IDSN_RANGE_SUBDIVISIONS "Range subdivisions"
#define IDSN_METRIC_WEIGHT "Metric weight"
#define IDSN_DONE "Done."
#define IDSN_MOTION_PLANNING_PARAMETERS_ILL_DEFINED "Nodes could not be computed: the motion planning parameters are ill-defined."
#define IDSN_DONE_TOTAL_NODES "Done. Total nodes: "

#define IDSN_PURE_COMPOUND_SHAPE "Pure compound shape"
#define IDSN_HEIGHTFIELD_SHAPE "Heightfield shape"
#define IDSN_PURE_SIMPLE_SHAPE_PLANE_ "Pure simple shape (plane)"
#define IDSN_PURE_SIMPLE_SHAPE_DISC_ "Pure simple shape (disc)"
#define IDSN_PURE_SIMPLE_SHAPE_CUBOID_ "Pure simple shape (cuboid)"
#define IDSN_PURE_SIMPLE_SHAPE_SPHEROID_ "Pure simple shape (spheroid)"
#define IDSN_PURE_SIMPLE_SHAPE_CYLINDER_ "Pure simple shape (cylinder)"
#define IDSN_PURE_SIMPLE_SHAPE_CONE_ "Pure simple shape (cone)"
#define IDSN_COMPOUND_CONVEX_SHAPE "Compound convex shape"
#define IDSN_COMPOUND_RANDOM_SHAPE "Compound random shape"
#define IDSN_SIMPLE_CONVEX_SHAPE "Simple convex shape"
#define IDSN_SIMPLE_RANDOM_SHAPE "Simple random shape"
#define IDSN_PURE_HOLLOW_SHAPES_ONLY_WITH_VORTEX "Remember that pure hollow shapes are only supported by the Vortex physics engine."
#define IDSN_PURE_HOLLOW_SHAPES "Pure hollow shapes"
#define IDSN_PHYSICS_ENGINE "Physics engine"


#define IDSNS_STARTING_ADDON_FUNCTION "Starting add-on function"
#define IDSNS_ENDED_ADDON_FUNCTION "Ended add-on function"
#define IDSNS_STARTED_ADDON_SCRIPT "Started add-on script"
#define IDSNS_PAUSED_ADDON_SCRIPT "Suspended add-on script"
#define IDSNS_RESUMED_ADDON_SCRIPT "Resumed add-on script"

#define IDSN_MAP_COORD_IMPORTED "imported"
#define IDSN_MAP_COORD_PROJECTION "projection"
#define IDSN_MAP_COORD_CYLINDER "cylinder"
#define IDSN_MAP_COORD_SPHERE "sphere"
#define IDSN_MAP_COORD_CUBE "cube"

#define IDSN_TEXTURE_APPLY_MODE_MODULATE "modulate"
#define IDSN_TEXTURE_APPLY_MODE_DECAL "decal"
#define IDSN_TEXTURE_APPLY_MODE_ADD "add"
#define IDSN_TEXTURE_APPLY_MODE_BLEND "blend"

#define IDSN_EXTERNAL_EDITOR_INFO "Make sure to restart V-REP, for changes to take effect!"
#define IDSN_EXTERNAL_SCRIPT_EDITOR "External script editor"

#define IDSN_TOGGLE_VISUALIZATION "Toggle visualization"
#define IDSN_THREADED_RENDERING "Threaded rendering"
#define IDSN_THREADED_RENDERING_WARNING "You have enabled the threaded rendering during simulation. It is recommended to keep this disabled for most situations, but you can always temporarily enable it via its dedicated toolbar button."

#define IDSN_HIERARCHY_COLORING_MENU_ITEM "Hierarchy coloring"
#define IDSN_HIERARCHY_COLORING_NONE_MENU_ITEM "None"
#define IDSN_HIERARCHY_COLORING_RED_MENU_ITEM "Red"
#define IDSN_HIERARCHY_COLORING_GREEN_MENU_ITEM "Green"
#define IDSN_HIERARCHY_COLORING_BLUE_MENU_ITEM "Blue"

#define IDSN_ASSEMBLING_METHOD "Setting local matrix transformation (for assembly)"
#define IDSN_ASSEMBLING_METHOD_QUESTION "Do you want to use current local transformation matrix as local transformation matrix after assembly?"

#define IDSNS_HIERARCHY_EXPANDED "Hierarchy expanded."
#define IDSNS_HIERARCHY_COLLAPSED "Hierarchy collapsed."
#define IDSNS_HIERARCHY_TREES_EXPANDED "Hierarchy tree(s) expanded."
#define IDSNS_HIERARCHY_TREES_COLLAPSED "Hierarchy tree(s) collapsed."

#define IDSNS_DONE "done."
#define IDSNS_ABORTED "Aborted."
#define IDSNS_OPERATION_ABORTED "Operation aborted."
#define IDSNS_CANNOT_PROCEED_SELECTION_IS_EMPTY "Cannot proceed, selection is empty."
#define IDSNS_AN_ERROR_OCCURRED_DURING_THE_IMPORT_OPERATION "An error occurred during the import operation."
#define IDSNS_AN_ERROR_OCCURRED_DURING_THE_EXPORT_OPERATION "An error occurred during the export operation."
#define IDSNS_ABORTED_FILE_DOES_NOT_EXIST "Aborted (file does not exist)."
#define IDSNS_CANNOT_EXPORT_AN_EMPTY_PATH "Cannot export an empty path."
#define IDSNS_LAST_SELECTION_IS_NOT_A_PATH "Last selection is not a path."
#define IDSNS_CANNOT_PROCEED_NO_DYNAMIC_CONTENT_AVAILABLE "Cannot proceed, no dynamic content available!"
#define IDSNS_FAILED_IS_THE_QHULL_PLUGIN_LOADED "Operation failed: is the Qhull plugin loaded?"
#define IDSNS_OPERATION_FAILED "Operation failed."


#define IDSNS_STARTING_TRIANGLE_EDIT_MODE "Starting triangle edit mode..."
#define IDSNS_STARTING_COMPOUND_SHAPE_EDIT_MODE "Starting edit mode of compound shape..."
#define IDSNS_ENDING_EDIT_MODE_AND_APPLYING_CHANGES "Ending edit mode and applying changes..."
#define IDSNS_ENDING_EDIT_MODE_AND_CANCELLING_CHANGES "Ending edit mode and canceling changes..."
#define IDSNS_STARTING_PATH_EDIT_MODE "Starting path edit mode..."
#define IDSNS_COPYING_SELECTION "Copying selection..."
#define IDSNS_PASTING_BUFFER "Pasting buffer..."
#define IDSNS_DELETING_SELECTION "Deleting selection..."
#define IDSNS_CUTTING_SELECTION "Cutting selection..."
#define IDSNS_PREPARING_INITIAL_CALCULATION_STRUCTURES "Preparing initial calculation structures..."
#define IDSNS_INSERTING_BUTTONS "Inserting buttons..."
#define IDSNS_INSERTING_MERGED_BUTTON "Inserting merged button..."

#define IDSNS_OPERATION_DISABLED_WITH_INDIVIDUAL_PATH_POINTS_SELECTED "Operation disabled while individual path points are selected."

#define IDSNS_TOGGLED_TO_REAL_TIME_MODE "Toggled to real-time simulation mode."
#define IDSNS_TOGGLED_TO_NON_REAL_TIME_MODE "Toggled to non real-time simulation mode."
#define IDSNS_TOGGLED_TO_ONLINE_MODE "Toggled to online mode."
#define IDSNS_TOGGLED_TO_OFFLINE_MODE "Toggled to offline mode."
#define IDSNS_CREATED_VIEWS "Created View(s)."
#define IDSNS_REMOVED_VIEW "Removed view."

#define IDSNS_NOW_IN_WIREFRAME_RENDERING_MODE "View in wireframe rendering mode."
#define IDSNS_NOW_IN_SOLID_RENDERING_MODE "View in solid rendering mode."
#define IDSNS_NOW_IN_PERSPECTIVE_PROJECTION_MODE "View in perspective projection mode."
#define IDSNS_NOW_IN_ORTHOGRAPHIC_PROJECTION_MODE "View in orthographic projection mode."
#define IDSNS_NOW_SHOWING_EDGES "View showing edges."
#define IDSNS_NOW_HIDING_EDGES "View hiding edges."
#define IDSNS_EDGES_ARE_NOW_THICK "View showing thick edges."
#define IDSNS_EDGES_ARE_NOW_THIN "View showing thin edges."
#define IDSNS_SHAPE_TEXTURES_ENABLED "Shape textures enabled."
#define IDSNS_SHAPE_TEXTURES_DISABLED "Shape textures disabled."
#define IDSNS_SHOWING_INERTIAS "Visualizing inertias."
#define IDSNS_NOT_SHOWING_INERTIAS "Hiding inertias."
#define IDSNS_NOW_IN_AUTO_MODE "View in auto mode."
#define IDSNS_AUTO_MODE_DISABLED "View auto mode disabled."
#define IDSNS_KEEPING_PROPORTIONS_AT_1_1 "View has proportions constrained to 1:1."
#define IDSNS_PROPORTIONS_NOT_CONSTRAINED_ANYMORE "View has proportions not constrained."
#define IDSNS_NOW_IN_TIME_GRAPH_MODE "View in time graph mode."
#define IDSNS_NOW_IN_XY_GRAPH_MODE "View in x/y graph mode."

#define IDSNS_NOW_LOOKING_THROUGH_SELECTED_CAMERA "Now looking through selected camera."
#define IDSNS_NOW_LOOKING_AT_SELECTED_GRAPH "Now looking at selected graph."
#define IDSNS_NOW_LOOKING_AT_SELECTED_VISION_SENSOR "Now looking at selected vision sensor."
#define IDSNS_CAMERA_NOW_TRACKING_SELECTED_OBJECT "Camera now tracking selected object."
#define IDSNS_CAMERA_NOW_NOT_TRACKING_ANY_OBJECT "Camera now not tracking any object."

#define IDSNS_MAC_FILE_ATTRIBUTE_PROBLEM "The model folder was not found. If the model folder is present in the V-REP folder, then it might be that your system has automatically added attributes to all V-REP files, causing problems. Try running from a terminal\n\n    sudo xattr -r -d com.apple.quarantine *\n\nin the V-REP folder to strip away those attributes, otherwise you might experiences strange behaviours and crashes."

#define IDSNS_CLEARING_BUFFER "Clearing buffer..."
#define IDSNS_SWAPPING_TO_EDGE_EDIT_MODE "Swapping to edge edit mode..."
#define IDSNS_SWAPPING_TO_TRIANGLE_EDIT_MODE "Swapping to triangle edit mode..."
#define IDSNS_SWAPPING_TO_VERTEX_EDIT_MODE "Swapping to vertex edit mode..."
#define IDSNS_EDGE_VISIBILITY_TOGGLED "Edge visibility toggled."
#define IDSNS_GENERATING_DUMMIES "Generating dummies..."
#define IDSNS_INVERTING_CONTROL_POINT_ORDER "Inverting control point order..."
#define IDSNS_FAILED_SELECTED_CONTROL_POINTS_ARE_NOT_CONSECUTIVE "Failed: selected control points are not consecutive."
#define IDSNS_INSERTING_TRIANGLES "Inserting triangles..."
#define IDSNS_REDUCING_TRIANGLE_SIZES "Reducing triangle sizes..."
#define IDSNS_INSERTING_TRIANGLE_FAN "Inserting triangle fan..."
#define IDSNS_INSERTING_NEW_PATH_POINT "Inserting new path point..."
#define IDSNS_APPENDING_NEW_PATH_POINT_FROM_CAMERA_CONFIGURATION "Appending new path point from camera configuration..."
#define IDSNS_KEEPING_CONTROL_POINT_X_AXIS_AND_ALIGNING_CONTROL_POINT_Z_AXIS_WITH_LAST_SELECTION "Keeping control point's x axis and aligning control point's z axis with last selection..."
#define IDSNS_GENERATING_NEW_PATH "Generating new path..."
#define IDSNS_ADDING_A_PRIMITIVE_SHAPE "Adding a primitive shape..."
#define IDSNS_ADDING_STATIC_DUPLICATE_OF_CURVE "Adding static duplicate of curve..."
#define IDSNS_CURVE_DATA_COPIED_TO_CLIPBOARD "Copying curve data to clipboard..."
#define IDSNS_ADDING_A_FLOATING_VIEW "Adding a floating view..."

#define IDSNS_ADDING_A_JOINT "Adding a joint..."
#define IDSNS_ADDING_A_CAMERA "Adding a camera..."
#define IDSNS_ADDING_A_LIGHT "Adding a light..."
#define IDSNS_ADDING_A_DUMMY "Adding a dummy..."
#define IDSNS_ADDING_AN_OCTREE "Adding an octree..."
#define IDSNS_ADDING_A_POINTCLOUD "Adding a point cloud..."
#define IDSNS_ADDING_A_MIRROR "Adding a mirror..."
#define IDSNS_ADDING_A_PATH "Adding a path..."
#define IDSNS_ADDING_A_GRAPH "Adding a graph..."
#define IDSNS_ADDING_A_VISION_SENSOR "Adding a vision sensor..."
#define IDSNS_ADDING_A_FORCE_SENSOR "Adding a force sensor..."
#define IDSNS_ADDING_A_PROXIMITY_SENSOR "Adding a proximity sensor..."
#define IDSNS_ADDING_A_MILL "Adding a mill..."
#define IDSNS_ADDING_CONVEX_HULL "Adding the convex hull of selected shapes and dummies..."
#define IDSNS_ADDING_CONVEX_DECOMPOSITION "Adding the convex decomposition of selected shapes..."

#define IDSNS_IMPORTING_PATH_FROM_CSV_FILE "Importing path from *.csv file..."
#define IDSNS_IMPORTING_HEIGHTFIELD_SHAPE "Importing heightfield shape..."

#define IDSNS_EXPORTING_SHAPES "Exporting shapes..."
#define IDSNS_EXPORTING_GRAPH_DATA "Exporting graph data..."
#define IDSNS_EXPORTING_PATH "Exporting path..."
#define IDSNS_EXPORTING_PATHS_BEZIER_CURVE "Exporting path's Bezier curve..."
#define IDSNS_EXPORTING_DYNAMIC_CONTENT "Exporting dynamic content..."
#define IDSNS_EXPORTING_IK_CONTENT "Exporting IK content..."


#define IDSNS_SCENE_COULD_NOT_BE_OPENED "Scene could not be opened."
#define IDSNS_SCENE_OPENED "Scene opened."
//#define IDSNS_WINDING_CORRECTION_MAYBE_NOT_ACCURATE "Winding correction may not be accurate: found edges shared between more than two triangles."


#define IDSNS_GENERATING_PATH "Generating path..."
#define IDSNS_GENERATING_PURE_SHAPE "Generating pure shape..."
#define IDSNS_GENERATING_SHAPE "Generating shape..."
#define IDSNS_FLIPPING_TRIANGLE_SIDES "Flipping triangle sides..."
#define IDSNS_TOGGLED_AUTOMATIC_EDGE_FOLLOWING "Toggled automatic edge following."
#define IDSNS_EDGES_CANNOT_BE_COPIED_CUR_OR_PASTED "Edges can't be copied, cut or pasted."
#define IDSNS_TOGGLED_HIDDEN_VERTICES_EDGES_SHOW_STATE "Toggled hidden vertices/edges show state."

#define IDSNS_REPLACING_OBJECTS "Morphing objects..."
#define IDSNS_MORPHING_INTO_CONVEX_SHAPES "Morphing selected shapes into convex shapes..."
#define IDSNS_MORPHING_INTO_CONVEX_DECOMPOSITION "Morphing selected shapes into convex decomposition..."
#define IDSNS_DECIMATING_MESH "Decimating mesh..."
#define IDSNS_EXTRACTING_INSIDE_PART "Extracting inside part..."
#define IDSNS_DELETING_SELECTED_PATH_POINTS "Deleting selected path points..."

#define IDSNS_MEMORIZING_OBJECT_CONFIGURATION "Memorizing object configuration (object position/orientation, joint parameter)..."
#define IDSNS_RESTORING_OBJECT_CONFIGURATION "Restoring object configuration (object position/orientation, joint parameter)..."
#define IDSNS_INITIALIZING_USER_INTERFACE_EDIT_MODE "Initializing OpenGl-based custom UI edit mode..."
#define IDSNS_GROUPING_SELECTED_SHAPES "Grouping selected shapes..."
#define IDSNS_UNGROUPING_SELECTED_SHAPES "Ungrouping selected shapes..."
#define IDSNS_MERGING_SELECTED_SHAPES "Merging selected shapes..."
#define IDSNS_DIVIDING_SELECTED_SHAPES "Dividing selected shapes..."
#define IDSNS_MERGING_SELECTED_PATHS "Merging selected paths..."

#define IDSNS_LOADING_MODEL "Loading model"
#define IDSNS_LOADING_UI "Loading OpenGl-based custom UI"
#define IDSNS_FILE_WAS_PREVIOUSLY_WRITTEN_WITH_VREP_VERSION "File was previously written with V-REP version"

#define IDSNS_DEFAULT_SCENE_WAS_SET_UP "Default scene was set-up."
#define IDSNS_SERIALIZATION_VERSION_IS "Serialization version is"
#define IDSNS_MODEL_LOADED "Model loaded."
#define IDSNS_MODEL_COULD_NOT_BE_LOADED "Model could not be loaded."


#define IDSNS_UI_LOADED "OpenGl-based custom UIs loaded."
#define IDSNS_UI_COULD_NOT_BE_LOADED "OpenGl-based custom UIs could not be loaded."

#define IDSNS_SCENE_WAS_SAVED "Scene was saved."
#define IDSNS_SCENE_WAS_EXPORTED "Scene was exported."
#define IDSNS_SAVING_MODEL "Saving model"
#define IDSNS_MODEL_WAS_SAVED "Model was saved."
#define IDSNS_PATH_WAS_EMPTIED "Path was emptied."
#define IDSNS_FORWARD_ROLL_PERFORMED "Forward roll performed."
#define IDSNS_BACKWARD_ROLL_PERFORMED "Backward roll performed."
#define IDSNS_EXECUTING_UNDO "Executing undo... "
#define IDSNS_EXECUTING_REDO "Executing redo... "
#define IDSNS_ATTACHING_OBJECTS_TO "Attaching object(s) to '"
#define IDSNS_MAKING_ORPHANS "Making orphan(s)..."
#define IDSN_ASSEMBLING_2_OBJECTS "Assembling 2 objects..."
#define IDSN_DISASSEMBLING_OBJECT "Disassembling object..."
#define IDSN_TRANSFERING_DNA_TO_SIBLINGS "Transferring DNA to siblings..."
#define IDSN_X_SIBLINGS_WERE_UPDATED " siblings were updated."


#define IDSNS_SCENE_WAS_RESTORED_FROM_AUTO_SAVED_SCENE "Scene was restored from auto-saved scene."
#define IDSNS_AUTO_SAVED_SCENE "Auto-saved scene"
#define IDSNS_SWAPPED_TO_PAGE "Swapped to page"

#define IDSNS_LOADING_SCENE "Loading scene"

#define IDSNS_VIDEO_COMPRESSOR_INITIALIZED "Video compressor initialized."
#define IDSNS_VIDEO_COMPRESSOR_FAILED_TO_INITIALIZE "Video compressor failed initializing."
#define IDSN_VIDEO_COMPRESSOR_FAILED_INITIALIZING_WARNING "The video compressor failed initializing. Check the console for additional messages."
#define IDSNS_VIDEO_USING_PADDING "Video resolution is non multiple of 16. Resolution will be adjusted and images padded."


#define IDSN_PAGE_SELECTOR "Page selector"      //addedOn 8/3/2012
#define IDSN_SCENE_SELECTOR "Scene selector"    //addedOn 8/3/2012
#define IDSN_SELECT_VIEWABLE_OBJECT "View selector..." //addedOn 9/3/2012
#define IDSN_MODEL_SAVE_POSITION_OFFSET_INFO "Remember that the current X/Y position of your model will be the horizontal offset to the drop position during a drag-and-drop operation later."//addedOn 7/3/2012
#define IDSN_DYNAMIC_CONTENT "Dynamics content" //addedOn 24/02/2012
#define IDSN_RENDERING "Rendering" //addedOn 06/02/2013
#define IDSN_RENDERING_TOO_SLOW "It appears that your graphic card takes too much time to render a simple scene. This could be because your graphic card is very old, because your graphic card uses old/faulty drivers (or drivers not from the manufacturer), or because your graphic card overrides some application settings: 'vertical synchronization' (vsync) is disabled by V-REP, but some graphic card settings might override this (often the case with Nvidia cards)."
#define IDSN_USING_NON_PURE_NON_CONVEX_SHAPES_FOR_DYNAMICS_WARNING "It appears that the dynamic content includes non-pure non-convex shapes. This is never recommended since this will drastically slow down the simulation, and in addition to that, non-pure non-convex shapes are not as stable as pure shapes or convex shapes. To find the incriminating shape(s), make sure to inspect the dynamic scene content by using the dedicated toolbar button." //addedOn 24/02/2012
#define IDSN_USING_STATIC_SHAPE_ON_TOP_OF_DYNAMIC_CONSTRUCTION_WARNING "It appears that there is at least one static respondable shape built on top of a dynamic tree structure. This is never recommended since it might lead to some strange and unrealistic behaviour. To find the incriminating shape(s), make sure to inspect the dynamic scene content by using the dedicated toolbar button." //addedOn 24/02/2012
#define IDS_CLOSE_SCENE_MENU_ITEM "Close scene" //addedOn 4/3/2012
#define IDS_SHAPE_EDITION_TOOLBAR_TIP "Toggle shape edit mode (make sure to have a single shape object selected)" //addedOn 11/3/2012
#define IDS_PATH_EDITION_TOOLBAR_TIP "Toggle path edit mode (make sure to have a single path object selected)" //addedOn 11/3/2012
#define IDS_2D_ELEMENTS_TIP         "Toggle OpenGl-based custom UI edit mode" //addedOn 11/3/2012
#define IDSN_SHAPE_EDITION_MENU_ITEM "Shape edition" //addedOn 11/3/2012
#define IDSN_PATH_EDITION_MENU_ITEM "Path edition" //addedOn 11/3/2012
#define IDS_SHAPE_IS_NOT_SIMPLE_FOR_EDITION_INFO_MESSAGE "The shape you wish to edit is not a simple shape. Make sure you ungroup it before proceeding." //addedOn 11/3/2012
#define IDSN_SHAPE_EDIT_MODE "Shape edit mode" //addedOn 11/3/2012
#define IDSN_PATH_EDIT_MODE "Path edit mode" //addedOn 11/3/2012
#define IDSN_DO_YOU_WANT_TO_APPLY_THE_CHANGES "Do you wish to apply the changes? (clicking no will however still preserve extracted objects)" //addedOn 11/3/2012
#define IDSN_CUSTOM_USER_INTERFACES         "OpenGl-based custom UIs" //addedOn 14/3/2012
#define IDSN_SHAPE_EDIT_MODE_START_MENU_ITEM "Enter shape edit mode"
#define IDSN_RECORDER_INITIALIZATION_FAILED "Video compressor initialization failed."

#define IDS_INSTANCES_MENU_ITEM "Scenes" //modifiedOn 5/3/2012    "Instances"
#define IDSN_INSTANCE  "Scene" //modifiedOn 5/3/2012    "Instance"
#define IDS_RECTANGLE_MENU_ITEM "Cuboid" //modifiedOn 25/2/2012
#define IDSN_PRIMITIVE_RECTANGLE "Primitive cuboid" //modifiedOn 25/2/2012
#define IDS_MAKE_RECTANGLE_WITH_SELECTED_TRIANGLES_MENU_ITEM "Make cuboid with selected triangles" //modifiedOn 25/2/2012
#define IDS_INSTANCE_STILL_IN_EDIT_MODE_MESSAGE "The scene is still in edit mode. Do you want to end the edit mode and discard the changes?" //modifiedOn 5/3/2012
#define IDS_ANOTHER_INSTANCE_STILL_IN_EDIT_MODE_MESSAGE "One or more scenes are still in edit mode. Please end all edit modes before leaving." //modifiedOn 5/3/2012
#define IDS_MAINSCRIPT_EDITION_WARNING "You are about to edit the main script. This is not recommended for compatibility reasons since loaded models, or models copied from a different scene might not operate as expected anymore in this scene. We rather recomend that you adjust a child script for the behaviour you wish to achieve. Do you want to proceed anyway?" //modifiedOn 5/3/2012
#define IDS_TOOLBAR_TOOLTIP_VISUALIZE_DYNAMIC_CONTENT "Visualize and verify dynamic content (during simulation only)" //modifiedOn 23/02/2012
#define IDS_ANOTHER_INSTANCE_STILL_NOT_SAVED_WANNA_LEAVE_ANYWAY_MESSAGE "One or more scenes have not yet been saved. Do you wish to leave anyway?" //modifiedOn 5/3/2012
#define IDS_LAYOUT_MENU_ITEM "Layout"
#define IDS_JOBS_MENU_ITEM "Jobs"

//#define IDS_LOOK_AT_SELECTED_GRAPH_MENU_ITEM "Associate view with selected graph" //modifiedOn 10/3/2012
#define IDS_LOOK_THROUGH_SELECTED_CAMERA_MENU_ITEM "Associate view with selected camera" //modifiedOn 10/3/2012
#define IDS_LOOK_AT_SELECTED_VISION_SENSOR_MENU_ITEM "Associate view with selected vision sensor" //modifiedOn 10/3/2012

#define IDS_2D_ELEMENTS         "OpenGl-based custom UI edition" //modifiedOn 11/3/2012
#define IDSN_AVI_RECORDER "Video recorder" //modifiedOn 14/3/2012
#define IDS_AVI_FILE_WAS_SAVED  "The video file was saved! (" //modifiedOn 15/2/2013


#define IDS_FILE_COULD_NOT_BE_FOUND_ "File could not be found!"
#define IDSN_CREDITS "Credits"
#define IDSN_CALCULATION_STRUCTURE "Calculation structure"
#define IDSN_SAVE "Save" // for the title of message boxes related to saving
#define IDSN_EXIT "Exit" // for the title of message boxes related to quitting
#define IDSN_EXPORT "Export" // for the title of message boxes related to exporting
#define IDSN_FILE_ACCESS "File access"
#define IDSN_ACCESS_TO_FILE_WAS_DENIED "Access to file was denied."
#define IDSN_TEST_SCENE "Test scene"

#define IDSN_TEST_SCENE_INFO "If your mouse doesn't operate as expected during object selection operations, make sure to load 'mouseTestScene' in your 'scenes'-folder."
#define IDSN_DO_NOT_SHOW_THIS_MESSAGE_AGAIN "Do not show this message again"
#define IDSN_DO_NOT_SHOW_THIS_MESSAGE_AGAIN_3X "Do not show this message again (you will however have to check this checkbox a total of 3 times)"
#define IDSN_SIMULATION_PARAMETERS "Simulation parameters"
#define IDSN_NON_STANDARD_SIM_PARAMS_WARNING "This simulation will run with custom simulation parameters. Be aware that you might experience slowdowns or accuracy problems, and some scenes or models will not perform as expected. Whenever possible, it is recommended to keep default simulation parameters (e.g. simulation time step or dynamics settings)."


// Script execution order:
#define IDSN_FIRST "first"
#define IDSN_NORMAL "normal"
#define IDSN_LAST "last"

// Script tree traversal direction:
#define IDSN_REVERSE_TRAVERSAL "reverse (default)"
#define IDSN_FORWARD_TRAVERSAL "forward"
#define IDSN_PARENT_TRAVERSAL "same as parent"

// Script debug modes:
#define IDSN_SCRIPTDEBUG_NONE "none"
#define IDSN_SCRIPTDEBUG_SYSCALLS "watch system calls"
#define IDSN_SCRIPTDEBUG_VARS_1SEC  "watch variables, ~1 sec. interv."
#define IDSN_SCRIPTDEBUG_ALLCALLS "watch all calls (slow)"
#define IDSN_SCRIPTDEBUG_VARS "watch variables (slow)"
#define IDSN_SCRIPTDEBUG_FULL "watch calls and variables (slow)"

// Help menu item
#define IDS_ABOUT_V_REP         "About V-REP"

//
#define IDSN_SERIALIZATION "Serialization"
#define IDSN_SCENE "Scene"
#define IDSN_MODEL "Model"
#define IDSN_IMPORT "Import"
#define IDSN_VERTICES "Vertices"
#define IDSN_PAGE_X "Page"


// File related
#define IDSN_FILE_EXCEPTION_ERROR "File exception error"
#define IDSN_LOADING_TARGA_FILE "Loading Targa File..."
#define IDSN_LOADING_SCENE "Loading scene..."
#define IDSN_SAVING_SCENE "Saving scene"
#define IDSN_LOADING_MODEL "Loading model..."
#define IDSN_TARGA_FILES "Targa files"
#define IDSN_AVI_FILE_LOCATION "Recording file location..."


#define IDSN_SCENE_LOCKING "Scene locking"
#define IDSN_DOUBLE_CLICK_TO_EDIT "double-click to edit"


// Detection volume types:
#define IDSN_RAY "Ray"
#define IDSN_RANDOMIZED_RAY "Randomized ray"
#define IDSN_PYRAMID "Pyramid"
#define IDSN_CYLINDER "Cylinder"
#define IDSN_DISC "Disc"
#define IDSN_CONE "Cone"

#define IDSN_OBJECT_PROPERTIES_MENU_ITEM "Scene object properties"
#define IDSN_CALCULATION_MODULE_PROPERTIES_MENU_ITEM "Calculation module properties"
#define IDSN_ALL_CUTTABLE_OBJECTS_IN_SCENE "all cuttable objects in the scene"
#define IDSN_ALL_DETECTABLE_OBJECTS_IN_SCENE "all detectable objects in the scene"
#define IDSN_NONE "none"
#define IDSN_ALL_CAMERAS_AND_VISION_SENSORS "all cameras and vision sensors"
#define IDSN_NEVER_VISIBLE "never visible"

// Graph functionality:
#define IDSN_XY_CURVES "XY curves"
#define IDSN_3D_CURVES "3D curves"
#define IDSN_XY_CURVE_PROPERTIES "XY curve properties"
#define IDSN_3D_CURVE_PROPERTIES "3D curve properties"
#define IDSN_GRAPH_CURVE "Graph curve"
#define IDSN_CURVE_WAS_DUPLICATED_TO_STATIC "Curve was duplicated to a static curve."

// Joint functionality:
#define IDSN_JOINT_IS_IN_PASSIVE_MODE "Passive mode"
#define IDSN_JOINT_IS_IN_IK_MODE "Inverse kinematics mode"
#define IDSN_JOINT_IS_IN_DEPENDENT_MODE "Dependent mode"
#define IDSN_JOINT_IS_IN_MOTION_MODE "Motion mode (DEPRECATED)"
#define IDSN_JOINT_IS_IN_TORQUE_FORCE_MODE "Torque/force mode"

// Entities:
#define IDSN_COLLECTION "Collection"
#define IDS_OBJECT "Object"
#define IDSN_SHAPE "Shape"
#define IDSN_JOINT "Joint"
#define IDSN_CAMERA "Camera"
#define IDSN_DUMMY "Dummy"
#define IDSN_PROXIMITY_SENSOR "Proximity sensor"
#define IDSN_PATH "Path"
#define IDSN_VISION_SENSOR "Vision sensor"
#define IDSN_MILL "Mill"
#define IDSN_FORCE_SENSOR "Force sensor"
#define IDSN_LIGHT "Light"
#define IDSN_MIRROR "Mirror"
#define IDSN_OCTREE "Octree"
#define IDSN_POINTCLOUD "Point cloud"
#define IDSN_GRAPH "Graph"

// Other objects:
#define IDSN_COLLISION_OBJECT "Collision object"
#define IDSN_DISTANCE_OBJECT "Distance object"
#define IDSN_AVOIDANCE_OBJECT "Avoidance object"
#define IDSN_COLLISION_DISTANCE_OBJECT "Collision / Distance object"

// Selection dialog:
#define IDS_SIMPLE_SHAPES       "Simple shapes"
#define IDS_COMPOUND_SHAPES      "Compound shapes"
#define IDS_JOINTS              "Joints"
#define IDS_CAMERAS             "Cameras"
#define IDS_LIGHTS              "Lights"
#define IDS_PROXSENSORS             "Proximity sensors"
#define IDS_FORCE_SENSORS       "Force sensors"
#define IDS_DUMMIES             "Dummies"
#define IDS_GRAPHS              "Graphs"
#define IDS_PATHS               "Paths"
#define IDS_MILLS               "Mills"
#define IDS_MIRRORS             "Mirrors"
#define IDS_OCTREES             "Octrees"
#define IDS_POINTCLOUDS         "Point clouds"
#define IDS_TOTAL_SELECTED_OBJECTS       "Total selected objects"
#define IDS_TOTAL_VERTICES      "Total vertices"
#define IDS_TOTAL_TRIANGLES     "Total triangles"
#define IDS_TOTAL_EDGES         "Total edges"

// IK functionality:
#define IDS_PSEUDO_INVERSE      "Pseudo inverse"
#define IDS_DLS                 "DLS"
#define IDSN_WORLD "World"
#define IDSN_SAME_AS_BASE "Same as base"
#define IDSN_PERFORM_ALWAYS "Perform always"
#define IDSN_WAS_PERFORMED_AND_FAILED "was performed and failed"
#define IDSN_WAS_PERFORMED_AND_SUCCEEDED "was performed and succeeded"
#define IDSN_WAS_NOT_PERFORMED "was not performed"

// OpenGl-based Custom user interfaces:
#define IDSN_BUTTON "Button"
#define IDSN_LABEL "Label"
#define IDSN_EDIT_BOX "Edit box"
#define IDSN_SLIDER "Slider"

// Path planning functionality:
#define IDSN_HOLONOMIC "Holonomic"
#define IDSN_NON_HOLONOMIC "Non-holonomic"
#define IDSN_CHECK_ROBOT_OBSTACLE_COLLISION "Check robot - obstacle collision"
#define IDSN_CHECK_ROBOT_OBSTACLE_MINIMUM_DISTANCE "Check robot - obstacle minimum distance"
#define IDSN_CHECK_ROBOT_OBSTACLE_MINIMUM_AND_MAXIMUM_DISTANCE "Check robot - obstacle minimum & maximum distance"

// Physics engine preconfigured settings:
#define IDSN_ENGINE_VERY_PRECISE "Very accurate"
#define IDSN_ENGINE_PRECISE "Accurate (default)"
#define IDSN_ENGINE_FAST "Fast"
#define IDSN_ENGINE_VERY_FAST "Very fast"
#define IDSN_ENGINE_CUSTOM "Customized"

// Preconfigured simulation step sizes:
#define IDSN_TIME_STEP_CONFIG_200 "dt=200 ms"
#define IDSN_TIME_STEP_CONFIG_100 "dt=100 ms"
#define IDSN_TIME_STEP_CONFIG_50 "dt=50 ms (default)"
#define IDSN_TIME_STEP_CONFIG_25 "dt=25 ms"
#define IDSN_TIME_STEP_CONFIG_10 "dt=10 ms"
#define IDSN_TIME_STEP_CONFIG_CUSTOM " ms (custom)"

#define IDS_ALL_PAGES           "All pages"

// Collections:
#define IDSN_COLLECTIONS "Collections"
#define IDS_LOOSE_OBJECT        "Loose object"
#define IDS_FROM_OBJECT__INCL___UP "From object (incl.) up"
#define IDS_FROM_OBJECT__EXCL___UP "From object (excl.) up"
#define IDS_FROM_OBJECT__INCL___DOWN "From object (incl.) down"
#define IDS_FROM_OBJECT__EXCL___DOWN "From object (excl.) down"
#define IDS_ALL_OBJECTS         "All objects"

#define IDS_COMMON_OBJECT_PROPERTIES "Object common properties"
#define IDSN_CONSTRAINT_SOLVER "Constraint solver"

// Variable title of the volume properties dialog:
#define IDSN_DETECTION_VOLUME_PROPERTIES "Detection Volume Properties"
#define IDSN_CUTTING_VOLUME_PROPERTIES "Cutting Volume Properties"
#define IDSN_VOLUME_PROPERTIES "Volume Properties"

// Variable title of the entity vs entity selection dialog:
#define IDSN_COLLISION_OBJECT_SELECTION_OF_ENTITIES "Collision Object, Selection of Entities"
#define IDSN_DISTANCE_OBJECT_SELECTION_OF_ENTITIES "Distance Object, Selection of Entities"
#define IDSN_OBSTACLE_AVOIDANCE_SELECTION_OF_ENTITIES "Obstacle avoidance, Selection of Entities"


#define IDSN_SCRIPTS   "Scripts"
#define IDSN_SELECTION "Selection"

#define IDSN_ENVIRONMENT "Environment"
#define IDSN_SHAPE_TEXTURES_ENABLED "Shape textures enabled"
#define IDSN_SHOW_INERTIAS "Visualize inertias"

#define IDSN_USER_SETTINGS "User settings"

#define IDSN_SCENE_HIERARCHY "Scene hierarchy"
#define IDS_LAYERS "Layers"
#define IDS_SETTINGS "Settings"

#define IDS_ERROR               "Error"


#define IDS_EDIT_MENU_ITEM      "Edit"
#define IDS_ADD_MENU_ITEM       "Add"
#define IDS_GROUPING_MERGING_MENU_ITEM "Grouping / Merging"
#define IDSN_GROUPING "Grouping"
#define IDSN_MERGING "Merging"
#define IDS_EXIT_MENU_ITEM      "Quit"
#define IDS_FILE_MENU_ITEM      "File"
#define IDS_SIMULATION_MENU_ITEM "Simulation"
#define IDS_TOOLS_MENU_ITEM     "Tools"
#define IDS_ADDON_MENU_ITEM     "Add-ons"
#define IDS_HELP_MENU_ITEM      "Help"

#define IDS_UNLINK_SELECTED_DUMMIES_MENU_ITEM "Unlink selected dummies"
#define IDS_MERGING_OR_DIVIDING_REMOVES_TEXTURES_PROCEED_INFO_MESSAGE "Merging or dividing shapes will remove their associated textures. Do you want to proceed?"

#define IDS_MAKE_LAST_SELECTED_OBJECTS_PARENT_MENU_ITEM "Make last selected object parent"
#define IDS_MAKE_SELECTED_SIMPLE_SHAPES_PURE_MENU_ITEM "Make selected simple shapes pure"
#define IDS_MAKE_SELECTED_OBJECT_S__ORPHAN_MENU_ITEM "Make selected object(s) orphan"
#define IDS_VISION_SENSOR_RESOLUTION_NOT_POWER_OF_TWO_WARNING "The resolution you selected is not a power of two. This can lead to some display problems on older graphic cards. For a resolution of X*Y, best is to assign X and Y one of the following values: 2, 4, 8, 16, 32, 64, 128, 256 or 512."

#define IDS_GCSOBJECT_ALREADY_THERE_MESSAGE "The specified object is part of a mechanism that is already handled by"
#define IDS_CAD_DATA_IMPORT_PROGRESS "CAD data import progress"
#define IDS_EXPAND_SELECTED_TREE_MENU_ITEM "Expand selected trees"
#define IDS_COLLAPSE_SELECTED_TREE_MENU_ITEM "Collapse selected trees"
#define IDS_SERIALIZATION_VERSION_NOT_SUPPORTED_ANYMORE "This serialization version is not supported anymore. Please convert this file."
#define IDS_SERIALIZATION_VERSION_TOO_RECENT "The serialization version is too recent and cannot be loaded by this V-REP version."


// Main Window title bar messages:
#define IDS____RENDERING__      " - rendering: "
#define IDS____SIMULATION_TIME__ " - simulation time: "
#define IDS____TRIANGLE_EDIT_MODE " - TRIANGLE EDIT MODE"
#define IDS____VERTEX_EDIT_MODE " - VERTEX EDIT MODE"
#define IDS____EDGE_EDIT_MODE   " - EDGE EDIT MODE"
#define IDS____2D_ELEMENT_EDIT_MODE " - OPENGL-BASED CUSTOM UI EDIT MODE"
#define IDS____PATH_EDIT_MODE   " - PATH EDIT MODE"
#define IDS____SIMULATION_RUNNING " - SIMULATION RUNNING"
#define IDS____SIMULATION_PAUSED " - SIMULATION SUSPENDED"
#define IDS____SIMULATION_STOPPED " - SIMULATION STOPPED"
#define IDS_NEW_FILE "New file"

// General simulation functionality:
#define IDS_RESUME_SIMULATION_MENU_ITEM "Resume simulation"
#define IDS_START_SIMULATION_MENU_ITEM "Start simulation"
#define IDS_PAUSE_SIMULATION_MENU_ITEM "Suspend simulation"
#define IDS_STOP_SIMULATION_MENU_ITEM "Stop simulation"
#define IDSN_SIMULATION_SETTINGS "Simulation settings"

#define IDSN_INTERACTIVE_IK_DLG "Interactive IK dialog"

// "ADD" menu item:
// ****************************************************************************

#define IDS_FLOATING_VIEW_MENU_ITEM "Floating view"

#define IDS_PRIMITIVE_SHAPE_MENU_ITEM "Primitive shape"
#define IDS_PLANE_MENU_ITEM     "Plane"
#define IDS_DISC_MENU_ITEM      "Disc"
#define IDS_SPHERE_MENU_ITEM    "Sphere"
#define IDS_CYLINDER_MENU_ITEM  "Cylinder"

#define IDSN_PRIMITIVE_PLANE "Primitive plane"
#define IDSN_PRIMITIVE_SPHERE "Primitive sphere"
#define IDSN_PRIMITIVE_CYLINDER "Primitive cylinder"
#define IDSN_PRIMITIVE_DISC "Primitive disc"

#define IDS_JOINT_MENU_ITEM     "Joint"
#define IDS_REVOLUTE_MENU_ITEM  "Revolute"
#define IDS_PRISMATIC_MENU_ITEM "Prismatic"
#define IDS_SPHERICAL_MENU_ITEM "Spherical"

#define IDS_CAMERA_MENU_ITEM    "Camera"
#define IDS_MIRROR_MENU_ITEM    "Mirror"

#define IDS_LIGHT_MENU_ITEM     "Light"
#define IDS_OMNIDIRECTIONAL_MENU_ITEM "Omnidirectional"
#define IDS_SPOTLIGHT_MENU_ITEM "Spotlight"
#define IDS_DIRECTIONAL_MENU_ITEM "Directional"

#define IDS_DUMMY_MENU_ITEM     "Dummy"
#define IDS_OCTREE_MENU_ITEM     "Octree"
#define IDS_POINTCLOUD_MENU_ITEM     "Point cloud"

#define IDS_GRAPH_MENU_ITEM     "Graph"

#define IDS_PROXSENSOR_MENU_ITEM    "Proximity sensor"
#define IDS_PYRAMID_TYPE_MENU_ITEM "Pyramid type"
#define IDS_CYLINDER_TYPE_MENU_ITEM "Cylinder type"
#define IDS_DISC_TYPE_MENU_ITEM "Disc type"
#define IDS_CONE_TYPE_MENU_ITEM "Cone type"
#define IDS_RAY_TYPE_MENU_ITEM  "Ray type"
#define IDS_RANDOMIZED_RAY_TYPE_MENU_ITEM  "Randomized ray type"

#define IDS_SEGMENT_TYPE_MENU_ITEM "Segment type"

#define IDS_CONVEX_HULL_OF_SELECTION_MENU_ITEM "Convex hull of selection"
#define IDS_GROWN_CONVEX_HULL_OF_SELECTED_SHAPE_MENU_ITEM "Inflated convex hull of selection..."
#define IDS_CONVEX_DECOMPOSITION_OF_SELECTION_MENU_ITEM "Convex decomposition of selection..."
#define IDS_CONVEX_MORPH_MENU_ITEM "Morph selection into convex shapes"
#define IDS_CONVEX_DECOMPOSITION_MORPH_MENU_ITEM "Morph selection into its convex decomposition..."
#define IDS_MESH_DECIMATION_MENU_ITEM "Decimate selected shape..."
#define IDS_REMOVE_SHAPE_INSIDE_MENU_ITEM "Extract inside of selected shape..."

// ****************************************************************************


#define IDS_LINK_SELECTED_DUMMIES_LINK_TYPE_MENU_ITEM "Link selected dummies"
#define IDS_COLOR_SEGMENT_WORK_IMAGE "Color segmentation on work image"
#define IDS_COMPRESSION_SCHEME_NOT_SUPPORTED "This V-REP version doesn't support the file's compression scheme."
#define IDS_TEXTURE_RESOLUTION_NOT_POWER_OF_TWO_WARNING "The resolution of the texture you just loaded is not a power of two. This can lead to some display problems on older graphic cards. Best is to adjust the texture's resolution to X*Y, where X and Y can be one of the following values: 2, 4, 8, 16, 32, 64, 128, 256 or 512."

// File menu item:
#define IDS_NEW_SCENE_MENU_ITEM "New scene"

#define IDS_OPEN_SCENE___MENU_ITEM "Open scene..."
#define IDS_OPEN_RECENT_SCENE_MENU_ITEM "Open recent scene"
#define IDS_LOAD_MODEL___MENU_ITEM "Load model..."
#define IDS_SAVE_SCENE_MENU_ITEM "Save scene"
#define IDS_SAVE_SCENE_AS_MENU_ITEM "Save scene as"
#define IDS_SCENE_AS_VREP___MENU_ITEM "V-REP scene..."
#define IDS_SCENE_AS_BR___MENU_ITEM "Blue REALITY scene..."
#define IDS_SAVE_SCENE_AS___MENU_ITEM "Save scene as..."

#define IDS_SAVE_MODEL_AS_MENU_ITEM "Save model as"
#define IDS_MODEL_AS_VREP___MENU_ITEM "V-REP model..."
#define IDS_MODEL_AS_BR___MENU_ITEM "Blue REALITY model..."

#define IDS_SAVE_MODEL_AS___MENU_ITEM "Save model as..."

#define IDS_SAVING_MODEL___ "Saving model..."
#define IDS_IMPORT_MESH___MENU_ITEM "Mesh..."
#define IDS_IMPORTING_MESH___ "Importing mesh..."
#define IDS_EXPORT_SHAPE_MENU_ITEM "Selected shapes..."
#define IDS_EXPORT_SELECTED_GRAPHS_MENU_ITEM "Selected graphs as CSV..."
#define IDS_SAVING_GRAPHS___ "Saving graphs..."
#define IDS_IMPORTING_PATH_FROM_CSV_FILE "Importing path from CSV file..."
#define IDS_EXPORTING_PATH___ "Exporting path..."
#define IDS_EXPORTING_PATH_BEZIER_CURVE___ "Exporting path's Bezier curve..."
#define IDS_IMPORTING_HEIGHTFIELD___ "Importing heightfield..."
#define IDS_EXPORTING_DYNAMIC_CONTENT___ "Exporting dynamic content..."
#define IDS_EXPORT_IK_CONTENT_MENU_ITEM "IK content..."
#define IDS_EXPORTING_IK_CONTENT___ "Exporting IK content..."

#define IDSN_IMPORT_MENU_ITEM "Import"
#define IDSN_EXPORT_MENU_ITEM "Export"


#define IDS_SAVED_WITH_MORE_RECENT_VERSION_WARNING "The file was saved with a more recent V-REP version. Parts of the scene might not be operating as expected. Please update V-REP to the latest version."
#define IDS_MODEL_SAVED_WITH_MORE_RECENT_VERSION_WARNING "The file was saved with a more recent V-REP version. Parts of the model might not be operating as expected. Please update V-REP to the latest version."

// Edit menu item:
#define IDS_GROUP_SELECTED_SHAPES_MENU_ITEM "Group selected shapes"
#define IDS_UNGROUP_SELECTED_SHAPES_MENU_ITEM "Ungroup selected shapes"
#define IDS_MERGE_SELECTED_SHAPES_MENU_ITEM "Merge selected shapes"
#define IDS_DIVIDE_SELECTED_SHAPES_MENU_ITEM "Divide selected shapes"

// Shape edit mode edit menu item:
#define IDS_MAKE_SPHERE_WITH_SELECTED_TRIANGLES_MENU_ITEM "Make sphere with selected triangles"


#define IDSN_STATIC_TEXTURE    "static texture"
#define IDS_HELP_TOPICS_MENU_ITEM "Help topics"
#define IDS_BUSY____PLEASE_WAIT "Busy... please wait"

#define IDS_VISIBLE_COMPONENTS  "OpenGL"
#define IDS_AUXILIARY_CHANNELS  "OpenGL, auxiliary channels"
#define IDS_COLOR_CODED_IDS     "OpenGL, color coded handles"
#define IDS_RAY_TRACING         "POV-Ray"
#define IDS_EXTERNAL_RENDERER   "External renderer"
#define IDS_EXTERNAL_RENDERER_WINDOWED  "External renderer, windowed"
#define IDS_OPENGL3  "OpenGL 3"
#define IDS_OPENGL3_WINDOWED "OpenGL 3, windowed"
#define IDS_RAY_TRACING_DURING_SIMULATION_AND_RECORDING "POV-Ray (simulation & video recording)"
#define IDS_RAY_TRACING_DURING_SIMULATION   "POV-Ray (simulation)"
#define IDS_EXT_RENDERER_DURING_SIMULATION_AND_RECORDING "External renderer (simulation & video recording)"
#define IDS_EXT_RENDERER_DURING_SIMULATION   "External renderer (simulation)"
#define IDS_OPENGL3_DURING_SIMULATION_AND_RECORDING "OpenGL 3 (simulation & video recording)"
#define IDS_OPENGL3_DURING_SIMULATION   "OpenGL 3 (simulation)"


#define IDS_ALL_OTHER_COLLIDABLE_OBJECTS_IN_SCENE "all other collidable objects in the scene"
#define IDS_ALL_OTHER_MEASURABLE_OBJECTS_IN_SCENE "all other measurable objects in the scene"
#define IDS_ALL_RENDERABLE_OBJECTS_IN_SCENE "all renderable objects in the scene"
#define IDS_ALL_VISIBLE_OBJECTS_IN_SCENE "all visible objects in the scene"
#define IDS_CANNOT_CHECK_OBJECT_AGAINST_ITSELF "Cannot check an object against itself"
#define IDS_SIMILAR_OBJECT_ALREADY_EXISTS "A similar object already exists"
#define IDS_INVALID_COLLISION_ITEM_COMBINATION "The item combination is invalid (i.e. cannot collide)"
#define IDS_INVALID_DISTANCE_ITEM_COMBINATION "The item combination is invalid (i.e. cannot be measured)"

#define IDS_SCENE_CONTENT_ACKNOWLEDGMENTS "Scene content acknowledgments / information"
#define IDS_MODEL_CONTENT_ACKNOWLEDGMENTS "Model content acknowledgments / information"

#define IDS_COPY_SELECTED_OBJECTS_MENU_ITEM "Copy selected objects"
#define IDS_PASTE_BUFFER_MENU_ITEM "Paste buffer"
//#define IDS_CLEAR_COPY_BUFFER_MENU_ITEM "Clear copy buffer"
#define IDS_DELETE_SELECTED_OBJECTS_MENU_ITEM "Delete selected objects"


#define IDS_MAKE_CYLINDER_WITH_SELECTED_TRIANGLES_MENU_ITEM "Make cylinder with selected triangles"
#define IDS_CUT_SELECTED_OBJECTS_MENU_ITEM "Cut selected objects"
#define IDSN_SELECT_ALL_MENU_ITEM "Select all"

#define IDSN_REMOVE_MENU_ITEM "Remove"
#define IDSN_ASSOCIATED_CHILD_SCRIPT_MENU_ITEM "Associated child script"
#define IDSN_ASSOCIATED_CUSTOMIZATION_SCRIPT_MENU_ITEM "Associated customization script"

#define IDSNS_SELECTING_ALL_OBJECTS "Selecting all objects..."
#define IDSNS_SELECTING_ALL_TRIANGLES "Selecting all triangles..."
#define IDSNS_SELECTING_ALL_VERTICES "Selecting all vertices..."
#define IDSNS_SELECTING_ALL_EDGES "Selecting all edges..."
#define IDSNS_SELECTING_ALL_PATH_POINTS "Selecting all path points..."

// Bounding box alignment
#define IDS_BOUNDING_BOX_ALIGNMENT_MENU_ITEM "Reorient bounding box"
#define IDS_ALIGN_SELECTED_SHAPE_MAIN_AXIS_MENU_ITEM "with main axes of random shape"
#define IDS_ALIGN_SELECTED_SHAPE_WORLD_MENU_ITEM "with reference frame of world"
#define IDSN_ALIGN_BB_WITH_TUBE "with main axis of cylinder shape"
#define IDSN_ALIGN_BB_WITH_CUBOID "with main axis of cuboid shape"
#define IDSNS_ALIGNING_BOUNDING_BOXES_WITH_MAIN_AXIS "Reorienting bounding box with main axes of random shape..."
#define IDSNS_ALIGNING_BOUNDING_BOXES_WITH_WORLD "Reorienting bounding box with reference frame of world..."
#define IDSNS_ALIGNING_BOUNDING_BOXES_WITH_TUBES "Reorienting bounding box with main axis of cylinder shape..."
#define IDSNS_ALIGNING_BOUNDING_BOXES_WITH_CUBOIDS "Reorienting bounding box with main axes of cuboid shape..."
#define IDS_INFORM_PURE_PRIMITIVES_COULD_NOT_BE_REORIENTED "One or more bounding boxes could not be reoriented: bounding boxes of non-compound pure primitive shapes cannot be reoriented."
#define IDSN_INFORM_SHAPE_COULD_NOT_BE_REORIENTED_ALONG_TUBE "One or more bounding boxes could not be reoriented around a cylinder: make sure the shape is clean, cylindrical, and precisely defined."
#define IDSN_INFORM_SHAPE_COULD_NOT_BE_REORIENTED_ALONG_CUBOID "One or more bounding boxes could not be reoriented around a cuboid: make sure the shape is clean, cuboidal, and precisely defined."


#define IDS_LEAVE_2D_ELEMENT_EDIT_MODE_MENU_ITEM "Leave OpenGl-based custom UI edit mode"

#define IDS_REMOVE_PAGE_MENU_ITEM "Remove page"
#define IDS_VIEW_MENU_ITEM      "View"
#define IDS_SOLID_RENDERING_MENU_ITEM "Solid rendering"
#define IDS_PERSPECTIVE_PROJECTION_MENU_ITEM "Perspective projection"

#define IDS_SHOW_EDGES_IN_VIEW_MENU_ITEM "Visible edges"
#define IDS_THICK_EDGES_IN_VIEW_MENU_ITEM "Thick edges"

#define IDS_DISPLAY_TIME_GRAPH_MENU_ITEM "Display time-graph"
#define IDS_DISPLAY_X_Y_GRAPH_MENU_ITEM "Display x/y graph"
#define IDS_WARNING_WHEN_PURE_SHAPES_HAVE_LOW_POLYCOUNT "Your shape has a low polygon count. Visual appearance and dynamic behaviour might not match anymore."

#define IDS_FAILED_TO_CONNECT_TO_FLOATING_LICENSE_SERVER "Failed to connect to floating license server."
#define IDS_TRACK_SELECTED_OBJECT_MENU_ITEM "Track selected object"
#define IDS_FAILED_TO_SEND_DATA_TO_FLOATING_LICENSE_SERVER "Failed to send data to floating license server."
#define IDSN_DYNAMIC_TEXTURE   "dynamic texture"

#define IDS_FAILED_TO_RECEIVE_REPLY_FROM_FLOATING_LICENSE_SERVER "Failed to receive reply from floating license server."
#define IDS_TRIANGLE_EDIT_MODE_MENU_ITEM "Triangle edit mode"
#define IDS_VERTEX_EDIT_MODE_MENU_ITEM "Vertex edit mode"
#define IDS_EDGE_EDIT_MODE_MENU_ITEM "Edge edit mode"
#define IDS_SHOW_HIDDEN_VERTICES_MENU_ITEM "Show hidden vertices"
#define IDS_COPY_SELECTED_VERTICES_MENU_ITEM "Copy selected vertices"
#define IDS_PASTE_VERTICES_MENU_ITEM "Paste vertices"
#define IDS_DELETE_SELECTED_VERTICES_MENU_ITEM "Delete selected vertices"
#define IDS_CUT_SELECTED_VERTICES_MENU_ITEM "Cut selected vertices"
#define IDS_INSERT_TRIANGLES_MENU_ITEM "Insert triangles"
#define IDS_INSERT_TRIANGLE_FAN_MENU_ITEM "Insert triangle fan"
#define IDS_MAKE_DUMMIES_WITH_SELECTED_VERTICES_MENU_ITEM "Make dummies with selected vertices"
#define IDS_COPY_SELECTED_TRIANGLES_MENU_ITEM "Copy selected triangles"
#define IDS_PASTE_TRIANGLES_MENU_ITEM "Paste triangles"
#define IDS_DELETE_SELECTED_TRIANGLES_MENU_ITEM "Delete selected triangles"
#define IDS_CUT_SELECTED_TRIANGLES_MENU_ITEM "Cut selected triangles"
#define IDS_FLIP_SELECTED_TRIANGLES_MENU_ITEM "Flip selected triangles"
#define IDS_MAKE_SHAPE_WITH_SELECTED_TRIANGLES_MENU_ITEM "Make shape with selected triangles"
#define IDS_FAILED_TO_FIND_APPROPRIATE_FLOATING_LICENSE "Failed to find an appropriate or available floating license."
#define IDS_REDUCE_TRIANGLE_SIZE_MENU_ITEM "Reduce triangle size"
#define IDS_SHOW_HIDDEN_EDGES_MENU_ITEM "Show hidden edges"
#define IDS_AUTOMATIC_EDGE_FOLLOWING_MENU_ITEM "Automatic edge following"
#define IDS_DELETE_SELECTED_EDGES_MENU_ITEM "Delete selected edges"
#define IDS_OVERALL_CALCULATION_TIME "Overall calculation time"
#define IDS_LEAVE_EDIT_MODE_AND_CANCEL_CHANGES_MENU_ITEM "Leave edit mode and cancel changes"
#define IDS_LEAVE_EDIT_MODE_AND_APPLY_CHANGES_MENU_ITEM "Leave edit mode and apply changes"
#define IDS_LARGE_QUANTITY_OF_OBJECT_WARNING "The operation you are about to perform will generate a large quantity of objects. This is not recommended and might take a while. Do you still want to proceed?"
#define IDS_VIEW_TYPE_SINGLE_MENU_ITEM "Set up page with single view"
#define IDS_VIEW_TYPE_TWO_HORIZ_MENU_ITEM "Set up page with 2 views (horizontal separation)"
#define IDS_VIEW_TYPE_TWO_VERT_MENU_ITEM "Set up page with 2 views (vertical separation)"
#define IDS_VIEW_TYPE_THREE_HORIZ_MENU_ITEM "Set up page with 1+2 views (horizontal separation)"
#define IDS_VIEW_TYPE_THREE_VERT_MENU_ITEM "Set up page with 1+2 views (vertical separation)"
#define IDS_VIEW_TYPE_HORIZONTAL_ONE_PLUS_THREE_MENU_ITEM "Set up page with 1+3 views (horizontal separation)"
#define IDS_VIEW_TYPE_VERTICAL_ONE_PLUS_THREE_MENU_ITEM "Set up page with 1+3 views (vertical separation)"
#define IDS_VIEW_TYPE_HORIZONTAL_ONE_PLUS_FOUR_MENU_ITEM "Set up page with 1+4 views (horizontal separation)"
#define IDS_VIEW_TYPE_VERTICAL_ONE_PLUS_FOUR_MENU_ITEM "Set up page with 1+4 views (vertical separation)"
#define IDS_VIEW_TYPE_FOUR_MENU_ITEM "Set up page with 4 views"
#define IDS_VIEW_TYPE_SIX_MENU_ITEM "Set up page with 1+5 views"
#define IDS_VIEW_TYPE_EIGHT_MENU_ITEM "Set up page with 1+7 views"

#define IDS_OVERALL_CUT_SURFACE "Overall cut surface"
#define IDS_MODULES_MENU_ITEM   "Plugins"
#define IDS_WARNING_UPDATES_COULD_NOT_BE_CHECKED "Could not check for updates. Your internet connection might not be operational, or you might be behind a proxy server. Please manually visit www.coppeliarobotics.com to see whether an updated V-REP version is available."
#define IDS_DETECTABLE_ULTRASONIC "Ultrasonic"
#define IDS_DETECTABLE_INFRARED "Infrared"
#define IDS_DETECTABLE_LASER    "Laser"
#define IDS_DETECTABLE_INDUCTIVE "Inductive"
#define IDS_DETECTABLE_CAPACITIVE "Capacitive"
#define IDS_SCRIPT_PARAMETERS   "Script parameters"
#define IDS_OVERALL_CUT_VOLUME  "Overall cut volume"

#define IDS_PATH_EDIT_MODE_START_MENU_ITEM "Enter path edit mode"
#define IDS_COPY_SELECTED_PATH_POINTS_MENU_ITEM "Copy selected path points"
#define IDS_PASTE_PATH_POINTS_MENU_ITEM "Paste path points after selection"
#define IDS_PASTE_PATH_POINTS_AT_BEGINNING_MENU_ITEM "Paste path points at beginning of path"
#define IDS_DELETE_SELECTED_PATH_POINTS_MENU_ITEM "Delete selected path points"
#define IDS_CUT_SELECTED_PATH_POINTS_MENU_ITEM "Cut selected path points"
#define IDS_INSERT_NEW_PATH_POINT_MENU_ITEM "Insert new control point after selection"
#define IDS_INSERT_NEW_PATH_POINT_AT_BEGINNING_MENU_ITEM "Insert new control point at beginning of path"
#define IDS_MAKE_PATH_FROM_BEZIER_PATH_MENU_ITEM "Make new path from bezier curve"
#define IDS_MAKE_DUMMIES_FROM_PATH_CTRL_POINTS_MENU_ITEM "Make dummies from selected control points"
#define IDS_INVERSE_ORDER_OF_SELECTED_PATH_CTRL_POINTS_MENU_ITEM "Invert order of selected control points"
#define IDS_LARGE_QUANTITY_OF_PATH_POINTS_WARNING "The operation you are about to perform will generate a path with a large quantity of path points. This is not recommended and might take a while. Do you still want to proceed?"
#define IDS_TOTAL_PATH_POINTS   "Total path points"
#define IDS_IMPORT_PATH___MENU_ITEM "Path from CSV..."
#define IDS_EXPORT_SELECTED_PATH_MENU_ITEM "Selected path as CSV..."
#define IDS_EXPORT_SELECTED_PATH_BEZIER_CURVE_MENU_ITEM "Selected path's bezier curve as CSV..."
#define IDS_MAKE_PATH_WITH_SELECTED_EDGES_MENU_ITEM "Make path with selected edges"
#define IDS_CIRCLE_TYPE_MENU_ITEM "Circle type"


#define IDS_ENTER_2D_ELEMENT_EDIT_MODE_MENU_ITEM "Enter OpenGl-based custom UI edit mode"
#define IDS_COPY_SELECTED_2D_ELEMENT_MENU_ITEM "Copy selected OpenGl-based custom UI"
#define IDS_DELETE_SELECTED_2D_ELEMENT_MENU_ITEM "Delete selected OpenGl-based custom UI"
#define IDS_CUT_SELECTED_2D_ELEMENT_MENU_ITEM "Cut selected OpenGl-based custom UI"

#define IDS_EXECUTION_TIME      "Execution time"
#define IDS_EDIT_MODES_MENU_ITEM "Edit modes"
#define IDS_OVERALL_COLLISION_COUNT "Overall collision count"
#define IDS_SIMULATION_TIME     "Simulation time"
#define IDS_USER_DATA           "User data"
#define IDS_MAIN_SCRIPT         "Main script"
#define IDS_CHILD_SCRIPT_UNTHREADED "Child script (non-threaded)"
#define IDS_CHILD_SCRIPT_THREADED "Child script (threaded)"
#define IDS_CUSTOMIZATION_SCRIPT "Customization script"

#define IDS_SIMULATION_SCRIPTS         "Simulation scripts"
#define IDS_CUSTOMIZATION_SCRIPTS "Customization scripts"


#define IDS_PROVIDE_MORE_INFO_AND_VALID_EMAIL "Please provide more information and a valid email address."
#define IDS_IF_BEHIND_PROXY_REQUEST_KEY_AT "If you are behind a proxy server, you can request the license key at "
#define IDS_INFO_NO_MORE_THAN_ONE_MAIN_SCRIPT "There can't be more than one main script. Replace current main script?"
#define IDS_INFO_NO_MORE_THAN_ONE_CONTACT_CALLBACK_SCRIPT "There can't be more than one contact callback script. Replace current contact callback script?"
#define IDS_INFO_NO_MORE_THAN_ONE_GENERAL_CALLBACK_SCRIPT "There can't be more than one general callback script. Replace current general callback script?"
#define IDS_DONT_FORGET_CORRESPONDING_INFO "(don't forget to provide the corresponding information like hardware ID, name, institution, and other information)"
#define IDS_KEY_WAS_SENT_PER_EMAIL "The license key was sent to the indicated email address. Should you not receive the license key via email within 2 hours, then try to request the key via email instead."
#define IDS_VISION_SENSOR_ORTHOGONAL_TYPE_MENU_ITEM "Orthographic type"
#define IDS_VISION_SENSORS   "Vision sensors"



#define IDS_DONT_TRACK_OBJECT__MENU_ITEM "Don't track object '"

#define IDS_PATH_PLANNING_CALCULATION_UNDERWAY___ "Path planning calculation underway..."
#define IDS_PATH_WAS_FOUND_     "Path was found!"
#define IDS_PARTIAL_PATH_WAS_FOUND_ "Partial path was found!"

#define IDSN_REAL_TIME_SIMULATION "Real-time simulation"
#define IDSN_ONLINE_MODE "Online mode"





#define IDS_SHAPING_USER_DEFINED "user defined"
#define IDS_SHAPING_CIRCLE      "circle"
#define IDS_SHAPING_SQUARE      "square"
#define IDS_SHAPING_HORIZONTAL_SEGMENT "horizontal segment"
#define IDS_SHAPING_VERTICAL_SEGMENT "vertical segment"

#define IDS_NOW_PERFORMING_POST_PROCESSING___ "Now performing post-processing..."
#define IDS_ALL_OTHER_ENTITIES  "All other entities"
#define IDS_EXPAND_ALL_MENU_ITEM "Expand all"
#define IDS_COLLAPSE_ALL_MENU_ITEM "Collapse all"


#define IDS_PATH_LENGTH_CALC_DL "sum(dl)"
#define IDS_PATH_LENGTH_CALC_DAC "sum(da*c)"
#define IDS_PATH_LENGTH_CALC_MAX_DL_DAC "sum(max(dl,da*c))"
#define IDS_PATH_LENGTH_CALC_DL_AND_DAC "sum(dl+da*c)"
#define IDS_PATH_LENGTH_CALC_SQRT_DL2_AND_DAC2 "sum(sqrt(dl^2+(da*c)^2))"
#define IDS_PATH_LENGTH_CALC_DL_IF_NONZERO "sum(if (dl!=0) then dl else da*c)"
#define IDS_PATH_LENGTH_CALC_DAC_IF_NONZERO "sum(if (da!=0) then da*c else dl)"

#define IDS_TIME_SPENT_SEARCHING_ "Time spent searching:"
#define IDS_NB_OF_COLL_FREE_NODES_FOUND_ "Number of collision-free nodes found:"

#define IDS_REMOVE_TEXTURE      "Remove texture"
#define IDS_TEXTURE_NAME_NONE   "none"
#define IDS_SELECT_TEXTURE_FROM_EXISTING "Select texture from existing textures"

#define IDS_REPLACE_OBJECTS_MENU_ITEM "Morph selected objects into last selected object"
#define IDS_DEFAULT             "Default"
#define IDS_MANIP_NONE             "None"

#define IDS_RECORD_NOW          "Record now!"
#define IDS_STOP_RECORDING      "Stop recording!"

#define IDS_SIMULATION_STILL_RUNNING_MESSAGE "The simulation is still running/suspended. Do you want to stop the simulation?"
#define IDS_ANOTHER_SIMULATION_STILL_RUNNING_MESSAGE "One or more simulations are still running/suspended. Please stop them before leaving."

#define IDSN_MODEL_BROWSER   "Model browser"

#define IDS_END_EDIT_MODE_BEFORE_PROCEEDING "Please end the edit mode before proceeding"
#define IDS_STOP_SIMULATION_BEFORE_PROCEEDING "Please stop the simulation before proceeding"
#define IDS_WANNA_DISCARD_CURRENT_SCENE "Do you want to discard current scene?"
#define IDS_TEXTURE_FILE_COULD_NOT_BE_LOADED "The texture file could not be loaded."
#define IDS_A_TEXTURE_FILE_COULD_NOT_BE_LOADED "A texture file could not be loaded"
#define IDS_THUMBNAILS_REQUIRE_128X128_RESOLUTION "Thumbnail images should have a resolution of 128X128 pixels"


#define IDS_ASSOCIATED_CHILD_SCRIPT_MENU_ITEM "Associated child script"
#define IDS_NON_THREADED_MENU_ITEM "Non threaded"
#define IDS_THREADED_MENU_ITEM  "Threaded"
#define IDS_ASSOCIATED_CUSTOMIZATION_SCRIPT_MENU_ITEM "Associated customization script"

#define IDS_APPEND_NEW_PATH_POINT_FROM_CAMERA_MENU_ITEM "Append new control point from camera configuration"
#define IDS_MAKE_CURVE_STATIC_MENU_ITEM "Static duplicate of curve"
#define IDS_COPY_CURVE_TO_CLIPBOARD "Curve to clipboard"
#define IDS_EMPTY_LAST_SELECTED_PATH_MENU_ITEM "Empty path"


#define IDS_XY_GRAPH_AUTO_MODE_DURING_SIMULATION_MENU_ITEM "Auto-size during simulation"
#define IDS_TIME_GRAPH_X_AUTO_MODE_DURING_SIMULATION_MENU_ITEM "X-axis auto-size during simulation"
#define IDS_TIME_GRAPH_Y_AUTO_MODE_DURING_SIMULATION_MENU_ITEM "Y-axis auto-size during simulation"
#define IDS_XY_GRAPH_ONE_ONE_PROPORTION_MENU_ITEM "Keep proportions at 1:1"


#define IDS_MERGE_SELECTED_PATHS_MENU_ITEM "Merge selected paths"
#define IDS_ROLL_PATH_POINTS_FORWARD_MENU_ITEM "Roll path points forward"
#define IDS_ROLL_PATH_POINTS_BACKWARD_MENU_ITEM "Roll path points backward"

#define IDSN_UNDO_REDO "Undo / Redo"
#define IDSN_UNDO "Undo"
#define IDSN_REDO "Redo"
#define IDSN_ASSEMBLE "Assemble / Disassemble"
#define IDSN_TRANSFER_DNA "Transfer DNA to siblings"


#define IDSN_SLOW_DOWN_SIMULATION "Slow down simulation"
#define IDSN_SPEED_UP_SIMULATION "Speed up simulation"


#define IDS_RAW                 "Raw"
#define IDS_DERIVATIVE          "Derivative"
#define IDS_INTEGRAL            "Integral"
#define IDS_CUMULATIVE          "Cumulative"

#define IDS_INFO_UNDO_REDO_TAKES_TOO_LONG "It seems that the undo/redo functionality takes too long to execute and slows down V-REP. You might have a slow computer or a large scene loaded. Do you wish to disable the undo/redo functionality for current session? (in future you can disable the undo/redo functionality manually in the user settings dialog, or you can change the threshold level for this warning in the 'system/usrset.txt' file)"
#define IDS_SHAPE_IS_PURE_PRIMITIVE_INFO_MESSAGE "The shape you are about to edit is a pure primitive shape. Modifying this shape will transform it into a regular shape that won't perform as fast/stable anymore during dynamics calculations. Do you want to proceed?"
#define IDS_MERGING_SOME_PURE_SHAPES_PROCEED_INFO_MESSAGE "You are about to merge pure (and non-pure) shapes. Doing so will result in a non-pure shape. Do you want to proceed anyway? "

#define IDS_BULLET_2_78              "Bullet 2.78"
#define IDS_BULLET_2_83              "Bullet 2.83"
#define IDS_ODE                 "ODE"
#define IDS_VORTEX              "Vortex"
#define IDS_NEWTON              "Newton"
#define IDS_WARNING_WHEN_SELECTING_PURE_CONE "Pure cones are currently only supported by the Bullet and Newton engines."
#define IDS_WARNING_WHEN_SELECTING_NEGATIVE_VOLUME "Negative volumes are only supported by the Vortex engine."

#define IDS_TOOLBAR_TOOLTIP_CAMERA_SHIFT "Camera pan"
#define IDS_TOOLBAR_TOOLTIP_CAMERA_FIT_TO_SCREEN "Fit to view"
#define IDS_TOOLBAR_TOOLTIP_CAMERA_ROTATE "Camera rotate"
#define IDS_TOOLBAR_TOOLTIP_CAMERA_TILT "Camera tilt"
#define IDS_TOOLBAR_TOOLTIP_CAMERA_OPENING_ANGLE "Camera opening angle/view size"
#define IDS_TOOLBAR_TOOLTIP_CAMERA_ZOOM "Camera shift"
#define IDS_TOOLBAR_TOOLTIP_OBJECT_SHIFT "Object/item shift"
#define IDS_TOOLBAR_TOOLTIP_OBJECT_ROTATE "Object/item rotate"
#define IDS_TOOLBAR_TOOLTIP_CAMERA_FLY "Camera fly"

#define IDS_TOOLBAR_TOOLTIP_DYNAMICS_ENGINE "Dynamics engine"
#define IDS_TOOLBAR_TOOLTIP_DYNAMICS_SETTINGS "Dynamics settings"
#define IDS_TOOLBAR_TOOLTIP_SIMULATION_TIME_STEP "Simulation time step"
#define IDS_TOOLBAR_TOOLTIP_DT_SIMULATION_TIME_STEP_AND_PPF "dt=Simulation time step, ppf=simulation Passes Per Frame"

#define IDS_TOOLBAR_TOOLTIP_CLICK_SELECTION "Click selection"
#define IDS_TOOLBAR_TOOLTIP_PATH_POINT_CREATION "Path point creation"
#define IDS_TOOLBAR_TOOLTIP_CLEAR_SELECTION "Clear selection"
#define IDS_TOOLBAR_TOOLTIP_SIMULATION_START "Start/resume simulation"
#define IDS_TOOLBAR_TOOLTIP_REALTIMESIMULATION "Toggle real-time mode"
#define IDS_TOOLBAR_TOOLTIP_ONLINE "Toggle online mode"
#define IDS_TOOLBAR_TOOLTIP_VERIFY "Verify layout"
#define IDS_TOOLBAR_TOOLTIP_SWITCH_TO_PAGE "Switch to a different page"
#define IDSN_SELECTION_DIALOG "Selection"

#define IDSN_PURE_SPHEROID "Pure spheroid"
#define IDSN_PURE_CONE "Pure cone"
#define IDSN_NEWTON_NON_CONVEX_MESH "Newton non-convex mesh"


#define IDS_WARNING_WITH_NEWTON_NON_CONVEX_DYNAMIC_MESH "The scene contains one or several non-convex dynamic meshes, which are not supported with the selected dynamics engine. Those meshes will be simulated with their convex hull instead."
#define IDS_WARNING_WHEN_PURE_SPHEROID_NOT_SUPPORTED "The scene contains one or several pure spheroids that cannot be simulated with currently selected dynamics engine. The spheroid(s) will be approximated by spheres for this simulation."
#define IDS_GROUPING_PURE_AND_NON_PURE_SHAPES_PROCEED_INFO_MESSAGE "You are about to group pure and non-pure shapes. Doing so will result in a non-pure shape. Do you want to proceed anyway? "


#define IDS_FILTER_ORIGINAL_IMAGE_TO_WORK_IMAGE "Original image to work image"
#define IDS_FILTER_ORIGINAL_DEPTH_IMAGE_TO_WORK_IMAGE "Original depth image to work image"
#define IDS_FILTER_UNIFORM_IMAGE_TO_WORK_IMAGE "Uniform image to work image"
#define IDS_FILTER_WORK_IMAGE_TO_OUTPUT_IMAGE "Work image to output image"
#define IDS_FILTER_WORK_IMAGE_TO_OUTPUT_DEPTH_IMAGE "Work image to output depth image"
#define IDS_FILTER_WORK_IMAGE_TO_BUFFER1 "Work image to buffer 1"
#define IDS_SWAP_BUFFER1_WITH_BUFFER2 "Swap buffer 1 with buffer 2"
#define IDS_FILTER_BUFFER1_TO_WORK_IMAGE "Buffer 1 to work image"
#define IDS_FILTER_ADD_BUFFER1_TO_WORK_IMAGE "Add buffer 1 to work image"
#define IDS_FILTER_SUBTRACT_BUFFER1_FROM_WORK_IMAGE "Subtract buffer 1 from work image"
#define IDS_FILTER_MULTIPLY_WORK_IMAGE_WITH_BUFFER1 "Multiply work image with buffer 1"
#define IDS_FILTER_FLIP_WORK_IMAGE_HORIZONTALLY "Flip work image horizontally"
#define IDS_FILTER_FLIP_WORK_IMAGE_VERTICALLY "Flip work image vertically"
#define IDS_FILTER_NORMALIZE_WORK_IMAGE "Normalize work image"
#define IDS_FILTER_INTENSITY_SCALE_WORK_IMAGE "Intensity scale work image"
#define IDS_FILTER_SELECTIVE_COLORS_WORK_IMAGE "Selective color on work image"
#define IDS_FILTER_OFFSET_AND_SCALE_COLORS_WORK_IMAGE "Offset and scale colors on work image"
#define IDS_FILTER_BINARY_WORK_IMAGE "Binary work image and trigger"
#define IDS_FILTER_EDGE_DETECTION_ON_WORK_IMAGE "Edge detection on work image"
#define IDS_FILTER_REPOSITION_WORK_IMAGE "Shift work image"
#define IDS_FILTER_CIRCULAR_CUT_WORK_IMAGE "Circular cut work image"
#define IDS_FILTER_RESIZE_WORK_IMAGE "Resize work image"
#define IDS_FILTER_ROTATE_WORK_IMAGE "Rotate work image"
#define IDS_FILTER_WORK_IMAGE_TO_BUFFER2 "Work image to buffer 2"
#define IDS_FILTER_BUFFER2_TO_WORK_IMAGE "Buffer 2 to work image"
#define IDS_FILTER_RECTANGULAR_CUT_WORK_IMAGE "Rectangular cut work image"
#define IDS_FILTER_3X3FILTER_WORK_IMAGE "3x3 filter on work image"
#define IDS_FILTER_5X5FILTER_WORK_IMAGE "5x5 filter on work image"
#define IDS_FILTER_SHARPEN_WORK_IMAGE "Sharpen work image"
#define IDS_FILTER_SWAP_WORK_WITH_BUFFER1 "Swap work image with buffer 1"
#define IDS_FILTER_ADD_WORK_TO_BUFFER1 "Add work image to buffer 1"
#define IDS_FILTER_SUBTRACT_WORK_FROM_BUFFER1 "Subtract work image from buffer 1"
#define IDS_FILTER_BUFFER1_AND_WORK_IMAGE_CORRELATION "Work image correlation with buffer 1"

#define IDS_FILTER_DISABLED     "(DISABLED)"

#define IDS_IS_NOT_DYNAMICALLY_ENABLED_WARNING "is not dynamically enabled and may not perform as expected.\n(This might however be the intended purpose in certain situations or model configurations)"
#define IDS_IS_DYNAMICALLY_SIMULATED "is dynamically simulated."
#define IDS_SHAPE_IS_STATIC_AND_RESPONDABLE "Shape is static and respondable."
#define IDS_SHAPE_IS_STATIC_AND_NON_RESPONDABLE "Shape is static and non-respondable."
#define IDS_SHAPE_IS_NON_STATIC_AND_RESPONDABLE "Shape is non-static and respondable."
#define IDS_SHAPE_IS_NON_STATIC_AND_NON_RESPONDABLE "Shape is non-static and non-respondable."
#define IDS_JOINT_OPERATES_IN_FORCE_TORQUE_MODE "Joint operates in force/torque mode"
#define IDS_JOINT_OPERATES_IN_HYBRID_FASHION "Joint operates in hybrid fashion"
#define IDS_FORCE_SENSOR_IS_BREAKABLE "Force sensor is breakable."
#define IDS_FORCE_SENSOR_IS_NOT_BREAKABLE "Force sensor is not breakable."

#define IDS__CONTROL_LOOP_ENABLED_  "(motor enabled, control loop enabled)"
#define IDS__MOTOR_DISABLED_ "(motor disabled)"
#define IDS__CONTROL_LOOP_DISABLED_ "(motor enabled, control loop disabled)"

#define IDS_VISION_SENSOR_PERSPECTIVE_TYPE_MENU_ITEM "Perspective type"
#define IDS_READING_NEWER_SERIALIZATION_FILE_WARNING "The file you are about to read was created with a newer version of V-REP. Some functionality might not be available or not working properly."
#define IDS_MAKING_NON_PURE_CONCAVE_SHAPE_RESPONDABLE_WARNING "The shape you made respondable is not a pure shape, nor a convex shape. It is a random non-convex shape that will perform much worse than pure shapes or convex shapes. Additionally, calculations with non-pure non-convex respondable shapes are much more time-consuming. For collision response calculations, it is recomended to use pure shapes whenever possible, or if that is not possible, convex shapes."
#define IDS_SCENE_LOCKING_INFO "Next scene save will be final: after that, this scene cannot anymore be saved, exported, or script content viewed."
#define IDS_SCENE_IS_LOCKED_WARNING "Save/export operation unavailable (scene is locked)"
#define IDS_SCENE_IS_LOCKED_CANNOT_PASTE_WARNING "Paste operation unavailable (buffer is locked)"
#define IDS_FINAL_SCENE_SAVE_WARNING "The save operation you are about to perform will be final: the scene will be locked and you will not be able to save changes anymore. Are you sure you want to proceed?"
#define IDS_INCLUDE_SCRIPT_FILES_QUESTION "Some scripts include code from external files, via the 'include' function. Do you wish to replace those scripts with their actual include code? In case of doubt, click 'No'."
#define IDS_UNDO_REDO_WITH_OPEN_SCRIPT_EDITOR_MESSAGE "Make sure to close all opened script editors before hitting undo/redo. To undo/redo a change in a script editor, use the popup menu undo/redo (right-click over the script editor)"
#define IDSN_SURE_TO_REMOVE_CUSTOM_JOINT_CTRL_WARNING "The operation you are about to perform will remove the custom control script. Are you sure you want to proceed?"
#define IDSN_SURE_TO_REMOVE_CUSTOM_CONTACT_WARNING "The operation you are about to perform will remove the custom collision/contact response script. Are you sure you want to proceed?"
#define IDSN_SURE_TO_REMOVE_GENERAL_CALLBACK_WARNING "The operation you are about to perform will remove the general callback script. Are you sure you want to proceed?"
#define IDSN_SURE_TO_REMOVE_CUSTOMIZATION_SCRIPT_WARNING "The operation you are about to perform will remove the customization script. Are you sure you want to proceed?"
#define IDSN_CUSTOM_CONTACT_IS_SLOW_AND_NOT_RECOMENDED_WARNING "Performing custom collision/contact response handling is not recomended for compatibility reasons (e.g. models created by others will perform differently in this scene). Remember also that this mechanism will slow down the simulation."
#define IDSN_SURE_TO_DISABLE_UPDATABLE_WARNING "The operation you are about to perform will definitely disassociate this object/model from its siblings (i.e. this object/model will be independent, and you won't be able to re-associate it with its former siblings). Are you sure you want to proceed?"

#define IDS_DUMMY_LINK_TYPE_DYNAMICS_LOOP_CLOSURE "Dynamics, overlap constraint"
#define IDS_DUMMY_LINK_TYPE_DYNAMICS_FORCE_CONSTRAINT "Dynamics, force constraint"
#define IDS_DUMMY_LINK_TYPE_GCS_LOOP_CLOSURE "GCS, overlap constraint (deprecated)"
#define IDS_DUMMY_LINK_TYPE_GCS_TIP "GCS, tip (deprecated)"
#define IDS_DUMMY_LINK_TYPE_GCS_TARGET "GCS, target (deprecated)"
#define IDS_DUMMY_LINK_TYPE_IK_TIP_TARGET "IK, tip-target"
#define IDS_WARNING_TIP_DUMMY_NOT_LINKED "tip dummy is not linked!"
#define IDS_WARNING_WRONG_DUMMY_LINK_TYPE "wrong dummy link type!"

#define IDSN_EXPORT_DYNAMIC_CONTENT "Dynamic content..."
#define IDS_SCENE_IS_LOCKED_WARNING_NO_SCRIPT_VIEWING "Script viewing/edition unavailable (scene is locked)"

#define IDS_OBJECTS_NOT_COLLIDABLE_WARNING "One or more selected objects have their 'collidable' property disabled. Make sure you enable that property (in the object common properties dialog) in order for this collision object to be operational."
#define IDS_OBJECTS_NOT_MEASURABLE_WARNING "One or more selected objects have their 'measurable' property disabled. Make sure you enable that property (in the object common properties dialog) in order for this distance object to be operational."
#define IDS_OBJECT_NOT_DETECTABLE_WARNING "The selected object has its 'detectable' properties disabled. Make sure you enable those properties (in the object common properties dialog) in order for this proximity sensor to be operational."
#define IDS_OBJECT_NOT_RENDERABLE_WARNING "The selected object has its 'renderable' property disabled. Make sure you enable this property (in the object common properties dialog) in order for this vision sensor to be operational."
#define IDS_OBJECT_NOT_CUTTABLE_WARNING "The selected object has its 'cuttable' property disabled. Make sure you enable this property (in the object common properties dialog) in order for this mill to be operational."

#define IDS_CUTTABLE_PROP       "Cuttable"
#define IDS_CUTTABLE_BUT_NOT_SIMPLE_NON_PURE_SHAPE "Cuttable (shape must be simple && non-pure)"

#define IDS_WARNING_WHEN_PURE_CONE_NOT_SUPPORTED "The scene contains one or several pure cones that cannot be simulated with currently selected dynamics engine. The cone(s) will be approximated by cylinders for this simulation."
#define IDS_WARNING_ODE_NOT_USING_QUICKSTEP "You selected not to use the 'quickstep' method. This is only recommended for scenes with very little dynamic content, otherwise you might experience sudden crashes."
#define IDS_WARNING_WHEN_PURE_HOLLOW_SHAPE_NOT_SUPPORTED "The scene contains one or several pure hollow shapes that cannot be simulated with currently selected dynamics engine. The pure shape(s) will be filled (i.e. non-hollow) for this simulation."
#define IDS_WARNING_WHEN_PHYSICS_ENGINE_NOT_SUPPORTED "The physics engine currently selected is not supported (you might be missing a required plugin). Simulation will not run correctly."
#define IDS_WARNING_WHEN_VORTEX_NOT_YET_SUPPORTED "The Vortex engine is not yet supported on this platform.\n\nThe Vortex engine is currently supported on Windows and Linux."
#define IDS_WARNING_WITH_VORTEX_DEMO_PLUGIN_WINDOWS "You are running an unregistered version of the Vortex engine: dynamic is disabled.\nYou may obtain a free license for the Vortex engine by downloading Vortex Studio (2019a) here:\n\nwww.cm-labs.com\n\nOnce Vortex Studio is installed and registered, you will be able to run\nV-REP simulations with the Vortex engine."
#define IDS_WARNING_WITH_VORTEX_DEMO_PLUGIN_LINUX "You are running an unregistered version of the Vortex engine: dynamic is disabled.\nYou may obtain a free license for the Vortex engine by downloading Vortex Studio (2019a) here:\n\nwww.cm-labs.com\n\nOnce Vortex Studio is installed and registered, you will be able to run\nV-REP simulations with the Vortex engine."

#define IDS_SWITCH_TO_BULLET_2_78_ENGINE_MENU_ITEM "Using the Bullet physics engine (V2.78)"
#define IDS_SWITCH_TO_BULLET_2_83_ENGINE_MENU_ITEM "Using the Bullet physics engine (V2.83)"
#define IDS_SWITCH_TO_ODE_ENGINE_MENU_ITEM "Using the ODE physics engine"
#define IDS_SWITCH_TO_VORTEX_ENGINE_MENU_ITEM "Using the Vortex physics engine"
#define IDS_SWITCH_TO_NEWTON_ENGINE_MENU_ITEM "Using the Newton physics engine"

#define IDS_WANNA_SAVE_THE_SCENE_WARNING "Do you wish to save the changes?"
#define IDS_USING_EXISTING_TEXTURE_COORDINATES "Texture coordinates from a previous import operation were found and will be used"
#define IDS_GROUPING_HEIGHTFIELDS_ERROR_MESSAGE "The selection contains one or several heightfields. Heightfields cannot be grouped with other pure shapes."
#define IDSN_IMPORT_HEIGHTFIELD "Heightfield"

#define IDS_FROM_SHAPE_IMPORT   "from shape import" // i.e. "texture coordinates obtained from shape import"
#define IDS_CALCULATED          "calculated" // i.e. "texture coordinates obtained by calculation"

#define IDS_TOO_MANY_MESH_ELEMENTS_DURING_IMPORT "The file you try to import is composed by too many elements. Instead of creating a shape for each one of them, a single compound shape will be created."
#define IDS_BLOB_DETECTION_WORK_IMAGE "Blob detection on work image"
#define IDS_DEPTH_TO_COORD_WORK_IMAGE "Extract coordinates from work image"
#define IDS_DEPTH_TO_VELODYNE_WORK_IMAGE "Extract Velodyne coord. from work image"
#define IDS_PIXEL_CHANGE_WORK_IMAGE "Pixel change between work image and buffer1"
#define IDS_SAVING_CALCULATION_STRUCTURE "Next time you save this scene or a model, all existing calculation structures will be saved too. Beware that calculation structures will drastically increase file sizes."


// Graph data streams:
// ***********************************************************************************************
#define IDS_OBJECT__ABSOLUTE_X_POSITION "Object: absolute x-position"
#define IDS_OBJECT__ABSOLUTE_Y_POSITION "Object: absolute y-position"
#define IDS_OBJECT__ABSOLUTE_Z_POSITION "Object: absolute z-position"
#define IDS_OBJECT__X_POSITION_RELATIVE_TO_GRAPH "Object: x-position relative to graph"
#define IDS_OBJECT__Y_POSITION_RELATIVE_TO_GRAPH "Object: y-position relative to graph"
#define IDS_OBJECT__Z_POSITION_RELATIVE_TO_GRAPH "Object: z-position relative to graph"
#define IDS_OBJECT__ABSOLUTE_ALPHA_ORIENTATION "Object: absolute alpha-orientation"
#define IDS_OBJECT__ABSOLUTE_BETA_ORIENTATION "Object: absolute beta-orientation"
#define IDS_OBJECT__ABSOLUTE_GAMMA_ORIENTATION "Object: absolute gamma-orientation"
#define IDS_OBJECT__ALPHA_ORIENTATION_RELATIVE_TO_GRAPH "Object: alpha-orientation relative to graph"
#define IDS_OBJECT__BETA_ORIENTATION_RELATIVE_TO_GRAPH "Object: beta-orientation relative to graph"
#define IDS_OBJECT__GAMMA_ORIENTATION_RELATIVE_TO_GRAPH "Object: gamma-orientation relative to graph"


#define IDS_OBJECT__ABSOLUTE_VELOCITY "Object: absolute velocity"
#define IDS_OBJECT__ABSOLUTE_X_VELOCITY "Object: absolute x-velocity"
#define IDS_OBJECT__ABSOLUTE_Y_VELOCITY "Object: absolute y-velocity"
#define IDS_OBJECT__ABSOLUTE_Z_VELOCITY "Object: absolute z-velocity"
#define IDS_OBJECT__ANGULAR_VELOCITY "Object: angular velocity"

#define IDS_COLLISION__COLLIDING_STATE "Collision: colliding state"
#define IDS_COLLISION__NUMBER_OF_COLLISIONS "Collision: number of collisions"

#define IDS_DISTANCE__SEGMENT_LENGTH "Distance: segment length"
#define IDS_DISTANCE__SEGMENT_SIDE_1_ABSOLUTE_X_POSITION "Distance: segment side 1 absolute x-position"
#define IDS_DISTANCE__SEGMENT_SIDE_1_ABSOLUTE_Y_POSITION "Distance: segment side 1 absolute y-position"
#define IDS_DISTANCE__SEGMENT_SIDE_1_ABSOLUTE_Z_POSITION "Distance: segment side 1 absolute z-position"
#define IDS_DISTANCE__SEGMENT_SIDE_2_ABSOLUTE_X_POSITION "Distance: segment side 2 absolute x-position"
#define IDS_DISTANCE__SEGMENT_SIDE_2_ABSOLUTE_Y_POSITION "Distance: segment side 2 absolute y-position"
#define IDS_DISTANCE__SEGMENT_SIDE_2_ABSOLUTE_Z_POSITION "Distance: segment side 2 absolute z-position"
#define IDS_DISTANCE__SEGMENT_SIDE_1_X_POSITION_RELATIVE_TO_GRAPH "Distance: segment side 1 x-position relative to graph"
#define IDS_DISTANCE__SEGMENT_SIDE_1_Y_POSITION_RELATIVE_TO_GRAPH "Distance: segment side 1 y-position relative to graph"
#define IDS_DISTANCE__SEGMENT_SIDE_1_Z_POSITION_RELATIVE_TO_GRAPH "Distance: segment side 1 z-position relative to graph"
#define IDS_DISTANCE__SEGMENT_SIDE_2_X_POSITION_RELATIVE_TO_GRAPH "Distance: segment side 2 x-position relative to graph"
#define IDS_DISTANCE__SEGMENT_SIDE_2_Y_POSITION_RELATIVE_TO_GRAPH "Distance: segment side 2 y-position relative to graph"
#define IDS_DISTANCE__SEGMENT_SIDE_2_Z_POSITION_RELATIVE_TO_GRAPH "Distance: segment side 2 z-position relative to graph"

#define IDS_PROXSENSOR__ABSOLUTE_DETECTION_POSITION_X "Proximity sensor: absolute detection position x"
#define IDS_PROXSENSOR__ABSOLUTE_DETECTION_POSITION_Y "Proximity sensor: absolute detection position y"
#define IDS_PROXSENSOR__ABSOLUTE_DETECTION_POSITION_Z "Proximity sensor: absolute detection position z"
#define IDS_PROXSENSOR__DETECTION_POSITION_X_RELATIVE_TO_SENSOR "Proximity sensor: detection position x relative to sensor"
#define IDS_PROXSENSOR__DETECTION_POSITION_Y_RELATIVE_TO_SENSOR "Proximity sensor: detection position y relative to sensor"
#define IDS_PROXSENSOR__DETECTION_POSITION_Z_RELATIVE_TO_SENSOR "Proximity sensor: detection position z relative to sensor"
#define IDS_PROXSENSOR__DETECTION_POSITION_X_RELATIVE_TO_GRAPH "Proximity sensor: detection position x relative to graph"
#define IDS_PROXSENSOR__DETECTION_POSITION_Y_RELATIVE_TO_GRAPH "Proximity sensor: detection position y relative to graph"
#define IDS_PROXSENSOR__DETECTION_POSITION_Z_RELATIVE_TO_GRAPH "Proximity sensor: detection position z relative to graph"
#define IDS_PROXSENSOR__DETECTION_STATE "Proximity sensor: detection state"
#define IDS_PROXSENSOR__DETECTION_DISTANCE "Proximity sensor: detection distance"
#define IDS_PROXSENSOR__DETECTION_AZIMUTH "Proximity sensor: detection azimuth"
#define IDS_PROXSENSOR__DETECTION_ELEVATION "Proximity sensor: detection elevation"

#define IDS_JOINT__POSITION     "Joint: position" // position in the sense: intrinsic position
#define IDS_JOINT__VELOCITY     "Joint: velocity" 
#define IDS_JOINT__FORCE_OR_TORQUE "Joint: force or torque"
#define IDS_JOINT__DYN_POSITION_ERROR "Joint: internal position error"
#define IDS_JOINT__DYN_ORIENTATION_ERROR "Joint: internal orientation error"

#define IDS_FORCE_SENSOR__FORCE_X "Force sensor: force along x-axis"
#define IDS_FORCE_SENSOR__FORCE_Y "Force sensor: force along y-axis"
#define IDS_FORCE_SENSOR__FORCE_Z "Force sensor: force along z-axis"
#define IDS_FORCE_SENSOR__TORQUE_X "Force sensor: torque about x-axis"
#define IDS_FORCE_SENSOR__TORQUE_Y "Force sensor: torque about y-axis"
#define IDS_FORCE_SENSOR__TORQUE_Z "Force sensor: torque about z-axis"
#define IDS_FORCE_SENSOR__FORCE "Force sensor: force"
#define IDS_FORCE_SENSOR__TORQUE "Force sensor: torque"
#define IDS_FORCE_SENSOR__POSITION_ERROR "Force sensor: internal position error"
#define IDS_FORCE_SENSOR__ORIENTATION_ERROR "Force sensor: internal orientation error"
#define IDS_FORCE_SENSOR__BROKEN_STATE "Force sensor: broken state"

#define IDS_VISIONSENS_GRAPH_MIN_RED "Vision sensor: minimum red value"
#define IDS_VISIONSENS_GRAPH_MIN_GREEN "Vision sensor: minimum green value"
#define IDS_VISIONSENS_GRAPH_MIN_BLUE "Vision sensor: minimum blue value"
#define IDS_VISIONSENS_GRAPH_MIN_INTENSITY "Vision sensor: minimum intensity value"
#define IDS_VISIONSENS_GRAPH_MIN_DEPTH "Vision sensor: minimum depth value"
#define IDS_VISIONSENS_GRAPH_MAX_RED "Vision sensor: maximum red value"
#define IDS_VISIONSENS_GRAPH_MAX_GREEN "Vision sensor: maximum green value"
#define IDS_VISIONSENS_GRAPH_MAX_BLUE "Vision sensor: maximum blue value"
#define IDS_VISIONSENS_GRAPH_MAX_INTENSITY "Vision sensor: maximum intensity value"
#define IDS_VISIONSENS_GRAPH_MAX_DEPTH "Vision sensor: maximum depth value"
#define IDS_VISIONSENS_GRAPH_AVG_RED "Vision sensor: average red value"
#define IDS_VISIONSENS_GRAPH_AVG_GREEN "Vision sensor: average green value"
#define IDS_VISIONSENS_GRAPH_AVG_BLUE "Vision sensor: average blue value"
#define IDS_VISIONSENS_GRAPH_AVG_INTENSITY "Vision sensor: average intensity value"
#define IDS_VISIONSENS_GRAPH_AVG_DEPTH "Vision sensor: average depth value"
#define IDS_VISIONSENS_GRAPH_DETECTION_STATE "Vision sensor: detection state"

#define IDS_PATH_GRAPH_POSITION "Path: position"
#define IDS_PATH_GRAPH_POSITION_LINEAR_COMPONENT "Path: position, linear component only"
#define IDS_PATH_GRAPH_POSITION_ANGULAR_COMPONENT "Path: position, angular component only"


#define IDS_COLLISION_CALCULATION_TIME "Collision: calculation time"
#define IDS_COLLISION_OVERALL_CALCULATION_TIME "Collision: overall calculation time"

#define IDS_DISTANCE_CALCULATION_TIME "Distance: calculation time"
#define IDS_DISTANCE_OVERALL_CALCULATION_TIME "Distance: overall calculation time"

#define IDS_PROXSENSOR_CALCULATION_TIME "Proximity sensor: calculation time"
#define IDS_PROXSENSOR_OVERALL_CALCULATION_TIME "Proximity sensor: overall calculation time"

#define IDS_VISIONSENS_CALCULATION_TIME "Vision sensor: calculation time"
#define IDS_VISIONSENS_OVERALL_CALCULATION_TIME "Vision sensor: overall calculation time"

#define IDS_IK_CALCULATION_TIME "Inverse kinematics: calculation time"
#define IDS_IK_OVERALL_CALCULATION_TIME "Inverse kinematics: overall calculation time"

#define IDS_GCS_CALCULATION_TIME "Constraint solver (deprecated): calculation time"
#define IDS_GCS_OVERALL_CALCULATION_TIME "Constraint solver (deprecated): overall calculation time"

#define IDS_MILL_CUT_SURFACE    "Mill (deprecated): cut surface"
#define IDS_MILL_CUT_VOLUME     "Mill (deprecated): cut volume"
#define IDS_MILL_OVERALL_CUT_SURFACE "Mill (deprecated): overall cut surface"
#define IDS_MILL_OVERALL_CUT_VOLUME "Mill (deprecated): overall cut volume"
#define IDS_MILL_CALCULATION_TIME "Mill (deprecated): calculation time"
#define IDS_MILL_OVERALL_CALCULATION_TIME "Mill (deprecated): overall calculation time"

#define IDS_DYNAMICS_OVERALL_CALCULATION_TIME "Dynamics: overall calculation time"

#define IDS_WORK_THREADS_EXECUTION_TIME "Work threads (deprecated): execution time"

#define IDS_RENDERING_TIME "Rendering: rendering time"

#define IDS_SIMULATION__TIME    "Simulation: time"
#define IDS_SIMULATION_PASS_EXECUTION_TIME "Simulation pass: execution time"
#define IDS_CHILD_SCRIPT_EXECUTION_TIME "Child scripts: execution time"
#define IDS_VARIOUS__USER_DEFINED "Various: user-defined"
// ***********************************************************************************************
