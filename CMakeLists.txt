cmake_minimum_required(VERSION 2.8.11)
project(v_rep)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_MACOSX_RPATH 1)

set(WITH_QT true CACHE BOOL "Enable Qt")
if(WITH_QT)
    set(WITH_GUI true CACHE BOOL "Enable gui")
    set(WITH_OPENGL true CACHE BOOL "Enable OpenGL")
    set(WITH_SERIAL true CACHE BOOL "Enable serial")
else()
    unset(WITH_GUI CACHE)
    unset(WITH_OPENGL CACHE)
    unset(WITH_SERIAL CACHE)
    set(WITH_GUI false)
    set(WITH_OPENGL false)
    set(WITH_SERIAL false)
endif()

find_package(Qt5PrintSupport REQUIRED)

find_package(Lua REQUIRED)
find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${LUA_INCLUDE_DIR})

if(WITH_GUI)
    find_package(Qt5Widgets REQUIRED)
    add_definitions(-DSIM_WITH_GUI)
    add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)
endif()

if(WITH_OPENGL)
    find_package(OpenGL REQUIRED)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5OpenGL REQUIRED)
    add_definitions(-DSIM_WITH_OPENGL)
endif()

if(WITH_SERIAL)
    find_package(Qt5SerialPort REQUIRED)
    add_definitions(-DSIM_WITH_SERIAL)
endif()

if(WITH_QT)
    find_package(Qt5Core REQUIRED)
else()
    add_definitions(-DSIM_WITHOUT_QT_AT_ALL)
endif()

if(MSVC)
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -fp:precise")
    set(CMAKE_CXXFLAGS_RELEASE "${CMAKE_CXXFLAGS_RELEASE} -O2")

    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -W3")
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -wd4100") # "unreferenced formal parameter"
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -wd4996") # "function or variable may be unsafe..."
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -wd4101") # "unreferenced local variable"

    if(MSVC_VERSION EQUAL 1900)
        add_definitions(-DVREP_COMPILER_STR=\"MSVC2015\")
    elseif(MSVC_VERSION GREATER 1900 AND MSVC_VERSION LESS 1920)
        add_definitions(-DVREP_COMPILER_STR=\"MSVC2017\")
    endif()
else()
    set(CMAKE_CXXFLAGS_DEBUG "${CMAKE_CXXFLAGS_DEBUG} -O0 -g -ggdb")
    set(CMAKE_CXXFLAGS_RELEASE "${CMAKE_CXXFLAGS_RELEASE} -O3")
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -Wall")
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -Wno-strict-aliasing")
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -Wno-unused-parameter")
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -Wno-unused-but-set-variable")
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -Wno-unused-local-typedefs")
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -Wno-empty-body")
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -Wno-write-strings")
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -Wno-narrowing")
    if(APPLE)
        set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -mmacosx-version-min=10.7")
        add_definitions(-DVREP_COMPILER_STR=\"Clang\")
    else()
        add_definitions(-DVREP_COMPILER_STR=\"GCC\")
    endif()
    # XXX: why don't we simply use CMAKE_CXX_COMPILER_ID?
endif()

if(WIN32)
    add_definitions(-DWIN_VREP)
elseif(APPLE)
    add_definitions(-DMAC_VREP)
elseif(UNIX)
    add_definitions(-DLIN_VREP)
endif()

include_directories(sourceCode)
include_directories(sourceCode/inverseKinematics/geomConstraintSolver)
include_directories(sourceCode/inverseKinematics/ik)
include_directories(sourceCode/shared/sharedBufferFunctions)
include_directories(sourceCode/communication)
include_directories(sourceCode/communication/tubes)
include_directories(sourceCode/communication/wireless)
include_directories(sourceCode/drawingObjects)
include_directories(sourceCode/platform)
include_directories(sourceCode/collections)
include_directories(sourceCode/collisions)
include_directories(sourceCode/distances)
include_directories(sourceCode/pathPlanning_old)
include_directories(sourceCode/motionPlanning_old)
include_directories(sourceCode/3dObjects)
include_directories(sourceCode/3dObjects/related)
include_directories(sourceCode/3dObjects/graphObjectRelated)
include_directories(sourceCode/3dObjects/millObjectRelated)
include_directories(sourceCode/3dObjects/pathObjectRelated)
include_directories(sourceCode/3dObjects/proximitySensorObjectRelated)
include_directories(sourceCode/3dObjects/shapeObjectRelated)
include_directories(sourceCode/3dObjects/visionSensorObjectRelated)
include_directories(sourceCode/mainContainers)
include_directories(sourceCode/mainContainers/sceneContainers)
include_directories(sourceCode/mainContainers/applicationContainers)
include_directories(sourceCode/luaScripting)
include_directories(sourceCode/luaScripting/customLuaFuncAndVar)
include_directories(sourceCode/pagesAndViews)
include_directories(sourceCode/textures)
include_directories(sourceCode/serialization)
include_directories(sourceCode/strings)
include_directories(sourceCode/interfaces)
include_directories(sourceCode/interfaces/interfaceStack)
include_directories(sourceCode/menusAndSimilar)
include_directories(sourceCode/variousFunctions)
include_directories(sourceCode/geometricAlgorithms)
include_directories(sourceCode/various)
include_directories(sourceCode/libsAndPlugins)
include_directories(sourceCode/visual)
include_directories(sourceCode/utils)
include_directories(sourceCode/sharedBufferFunctions)
include_directories(sourceCode/backwardCompatibility/pathPlanning)
include_directories(sourceCode/backwardCompatibility/motionPlanning)
include_directories(sourceCode/customUserInterfaces)
include_directories(sourceCode/undoRedo)
include_directories(sourceCode/rendering)
include_directories(../programming/include)
include_directories(../programming/v_repMath)

include_directories(sourceCode/verSpec)

if(WITH_SERIAL)
    include_directories(sourceCode/communication/serialPort)
endif()

if(WITH_GUI)
    include_directories(sourceCode/gui/qtPropertyBrowserRelated)
    include_directories(sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles)
    include_directories(sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserRelatedFiles)
    include_directories(sourceCode/gui/dialogs)
    include_directories(sourceCode/gui/dialogs/other)
    include_directories(sourceCode/gui/platform)
    include_directories(sourceCode/gui/mainContainers)
    include_directories(sourceCode/gui/mainContainers/sceneContainers)
    include_directories(sourceCode/gui/mainContainers/applicationContainers)
    include_directories(sourceCode/gui/editModes)
    include_directories(sourceCode/gui/modelBrowser)
    include_directories(sourceCode/gui/sceneHierarchy)
    include_directories(sourceCode/gui/selectors)
    include_directories(sourceCode/gui/menusAndSimilar)
    include_directories(sourceCode/gui/various)
    include_directories(sourceCode/gui/libs)
endif()

set(SOURCES "")

if(WITH_GUI)
    set(SOURCES ${SOURCES}
        targaFiles.qrc
        toolbarFiles.qrc
        variousImageFiles.qrc
        iconFiles.qrc
        imageFiles_vrep.qrc
    )
endif()

if(WITH_GUI)
    set(SOURCES ${SOURCES}
        ui/qdlgsettings.ui
        ui/qdlglayers.ui
        ui/qdlgavirecorder.ui
        ui/qdlgselection.ui
        ui/qdlgabout.ui
        ui/qdlgsimulation.ui
        ui/qdlginteractiveik.ui
        ui/qdlgcommonproperties.ui
        ui/qdlgdetectableproperties.ui
        ui/qdlgmodelproperties.ui
        ui/qdlgassembly.ui
        ui/qdlgslider.ui
        ui/qdlgslider2.ui
        ui/qdlginertiatensor.ui
        ui/qdlgmodelthumbnailvisu.ui
        ui/qdlgmodelthumbnail.ui
        ui/qdlgenvironment.ui
        ui/qdlgcolor.ui
        ui/qdlgmaterial.ui
        ui/qdlglightmaterial.ui
        ui/qdlgcolorpulsation.ui
        ui/qdlgfog.ui
        ui/qdlgcollections.ui
        ui/qdlgcollisions.ui
        ui/qdlgdistances.ui
        ui/qdlgentityvsentityselection.ui
        ui/qdlgscripts.ui
        ui/qdlginsertscript.ui
        ui/qdlgdynamics.ui
        ui/qdlgscaling.ui
        ui/qdlgcameras.ui
        ui/qdlglights.ui
        ui/qdlgmirrors.ui
        ui/qdlgdummies.ui
        ui/qdlgoctrees.ui
        ui/qdlgpointclouds.ui
        ui/qdlgscriptparameters.ui
        ui/qdlgforcesensors.ui
        ui/qdlgprimitives.ui
        ui/qdlgconvexdecomposition.ui
        ui/qdlgconstraintsolver.ui
        ui/qdlgvisionsensors.ui
        ui/qdlgimagecolor.ui
        ui/qdlgsimpleSearch.ui
        ui/qdlgfilters.ui
        ui/qdlgsimplefilter_rotate.ui
        ui/qdlgsimplefilter_uniformImage.ui
        ui/qdlgsimplefilter_shift.ui
        ui/qdlgsimplefilter_scaleCols.ui
        ui/qdlgsimplefilter_resize.ui
        ui/qdlgsimplefilter_coordExtraction.ui
        ui/qdlgsimplefilter_velodyne.ui
        ui/qdlgsimplefilter_pixelChange.ui
        ui/qdlgsimplefilter_rectangularCut.ui
        ui/qdlgsimplefilter_keepRemoveCols.ui
        ui/qdlgsimplefilter_intensityScale.ui
        ui/qdlgsimplefilter_edge.ui
        ui/qdlgsimplefilter_correlation.ui
        ui/qdlgsimplefilter_colorSegmentation.ui
        ui/qdlgsimplefilter_circularCut.ui
        ui/qdlgsimplefilter_blob.ui
        ui/qdlgsimplefilter_binary.ui
        ui/qdlgsimplefilter_5x5.ui
        ui/qdlgsimplefilter_3x3.ui
        ui/qdlgshapes.ui
        ui/qdlgmultishapeedition.ui
        ui/qdlgtextures.ui
        ui/qdlgtextureselection.ui
        ui/qdlggeometry.ui
        ui/qdlgshapedyn.ui
        ui/qdlgdetectionvolume.ui
        ui/qdlgproximitysensors.ui
        ui/qdlgproxsensdetectionparam.ui
        ui/qdlgmills.ui
        ui/qdlgobjectdialogcontainer.ui
        ui/qdlgshapeeditioncontainer.ui
        ui/qdlgcalcdialogcontainer.ui
        ui/qdlgheightfielddimension.ui
        ui/qdlgstopscripts.ui
        ui/qdlgtextureloadoptions.ui
        ui/qdlgprogress.ui
        ui/qdlgjoints.ui
        ui/qdlgdependencyequation.ui
        ui/qdlgjointdyn.ui
        ui/qdlggraphs.ui
        ui/qdlgdatastreamselection.ui
        ui/qdlg2d3dgraphproperties.ui
        ui/qdlgaddgraphcurve.ui
        ui/qdlgik.ui
        ui/qdlgikelements.ui
        ui/qdlgikconditional.ui
        ui/qdlgikavoidance.ui
        ui/qdlgui.ui
        ui/qdlguidialogcontainer.ui
        ui/qdlguibuttons.ui
        ui/qdlgnewui.ui
        ui/qdlguirolledup.ui
        ui/qdlgpathplanning.ui
        ui/qdlgmotionplanning.ui
        ui/qdlgpathplanningparams.ui
        ui/qdlgmotionplanningjoints.ui
        ui/qdlgworkspacemetric.ui
        ui/qdlgpathplanningaddnew.ui
        ui/qdlgpaths.ui
        ui/qdlgpathshaping.ui
        ui/qdlgmessageandcheckbox.ui
        ui/qdlgopenglsettings.ui
        ui/qdlgtriangleedit.ui
        ui/qdlgvertexedit.ui
        ui/qdlgedgeedit.ui
        ui/qdlgpathedit.ui
        ui/qdlgtranslation.ui
        ui/qdlgrotation.ui
        ui/qdlgtranslationrotationcont.ui
    )
endif()

set(SOURCES ${SOURCES}
    ../programming/v_repMath/Vector.cpp
    ../programming/v_repMath/MyMath.cpp
    ../programming/v_repMath/MMatrix.cpp
    ../programming/v_repMath/7Vector.cpp
    ../programming/v_repMath/6X6Matrix.cpp
    ../programming/v_repMath/6Vector.cpp
    ../programming/v_repMath/4X4Matrix.cpp
    ../programming/v_repMath/4X4FullMatrix.cpp
    ../programming/v_repMath/4Vector.cpp
    ../programming/v_repMath/3X3Matrix.cpp
    ../programming/v_repMath/3Vector.cpp

    sourceCode/inverseKinematics/geomConstraintSolver/iKObject.cpp
    sourceCode/inverseKinematics/geomConstraintSolver/iKObjCont.cpp
    sourceCode/inverseKinematics/geomConstraintSolver/iKMesh.cpp
    sourceCode/inverseKinematics/geomConstraintSolver/iKJoint.cpp
    sourceCode/inverseKinematics/geomConstraintSolver/iKGraphObject.cpp
    sourceCode/inverseKinematics/geomConstraintSolver/iKGraphObjCont.cpp
    sourceCode/inverseKinematics/geomConstraintSolver/iKGraphNode.cpp
    sourceCode/inverseKinematics/geomConstraintSolver/iKGraphJoint.cpp
    sourceCode/inverseKinematics/geomConstraintSolver/iKDummy.cpp
    sourceCode/inverseKinematics/geomConstraintSolver/iKChainCont.cpp
    sourceCode/inverseKinematics/geomConstraintSolver/iKChain.cpp
    sourceCode/inverseKinematics/geomConstraintSolver/geometricConstraintSolverInt.cpp
    sourceCode/inverseKinematics/geomConstraintSolver/geometricConstraintSolver.cpp

    sourceCode/inverseKinematics/ik/ikRoutine.cpp
    sourceCode/inverseKinematics/ik/ikGroup.cpp
    sourceCode/inverseKinematics/ik/ikEl.cpp

    sourceCode/shared/sharedBufferFunctions/sharedFloatVector.cpp
    sourceCode/shared/sharedBufferFunctions/sharedIntVector.cpp
    sourceCode/shared/sharedBufferFunctions/sharedUCharVector.cpp

    sourceCode/drawingObjects/bannerObject.cpp
    sourceCode/drawingObjects/drawingObject2D.cpp
    sourceCode/drawingObjects/drawingObject.cpp
    sourceCode/drawingObjects/drawingContainer2D.cpp
    sourceCode/drawingObjects/ptCloud_old.cpp

    sourceCode/platform/vVarious.cpp
    sourceCode/platform/vThread.cpp
    sourceCode/platform/vMutex.cpp
    sourceCode/platform/vFileFinder.cpp
    sourceCode/platform/vFile.cpp
    sourceCode/platform/vDateTime.cpp
    sourceCode/platform/vArchive.cpp

    sourceCode/collections/regCollectionEl.cpp
    sourceCode/collections/regCollection.cpp

    sourceCode/collisions/regCollision.cpp
    sourceCode/collisions/collisionRoutine.cpp

    sourceCode/distances/regDist.cpp
    sourceCode/distances/distanceRoutine.cpp
    sourceCode/distances/statDistObj.cpp

    sourceCode/3dObjects/related/3DObject.cpp
    sourceCode/3dObjects/related/convexVolume.cpp
    sourceCode/3dObjects/related/viewableBase.cpp

    sourceCode/3dObjects/jointObject.cpp
    sourceCode/3dObjects/camera.cpp
    sourceCode/3dObjects/dummy.cpp
    sourceCode/3dObjects/octree.cpp
    sourceCode/3dObjects/pointCloud.cpp
    sourceCode/3dObjects/forceSensor.cpp
    sourceCode/3dObjects/graph.cpp
    sourceCode/3dObjects/light.cpp
    sourceCode/3dObjects/mirror.cpp
    sourceCode/3dObjects/mill.cpp
    sourceCode/3dObjects/path.cpp
    sourceCode/3dObjects/proximitySensor.cpp
    sourceCode/3dObjects/shape.cpp
    sourceCode/3dObjects/visionSensor.cpp

    sourceCode/3dObjects/graphObjectRelated/graphingRoutines.cpp
    sourceCode/3dObjects/graphObjectRelated/graphDataComb.cpp
    sourceCode/3dObjects/graphObjectRelated/graphData.cpp
    sourceCode/3dObjects/graphObjectRelated/staticGraphCurve.cpp

    sourceCode/3dObjects/millObjectRelated/cuttingRoutine.cpp

    sourceCode/3dObjects/pathObjectRelated/bezierPathPoint.cpp
    sourceCode/3dObjects/pathObjectRelated/simplePathPoint.cpp
    sourceCode/3dObjects/pathObjectRelated/pathPoint.cpp
    sourceCode/3dObjects/pathObjectRelated/pathCont.cpp

    sourceCode/3dObjects/proximitySensorObjectRelated/proxSensorRoutine.cpp

    sourceCode/3dObjects/shapeObjectRelated/geometric.cpp
    sourceCode/3dObjects/shapeObjectRelated/geomWrap.cpp
    sourceCode/3dObjects/shapeObjectRelated/geomProxy.cpp
    sourceCode/3dObjects/shapeObjectRelated/volInt.cpp

    sourceCode/backwardCompatibility/pathPlanning/pathPlanning.cpp
    sourceCode/backwardCompatibility/pathPlanning/holonomicPathPlanning.cpp
    sourceCode/backwardCompatibility/pathPlanning/holonomicPathNode.cpp
    sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathPlanning.cpp
    sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathNode.cpp
    sourceCode/backwardCompatibility/motionPlanning/mpPhase1Node.cpp
    sourceCode/backwardCompatibility/motionPlanning/mpPhase2Node.cpp
    sourceCode/backwardCompatibility/motionPlanning/mpObject.cpp

    sourceCode/communication/tubes/commTube.cpp

    sourceCode/communication/wireless/broadcastDataContainer.cpp
    sourceCode/communication/wireless/broadcastData.cpp
    sourceCode/communication/wireless/broadcastDataVisual.cpp

    sourceCode/mainContainers/mainContainer.cpp

    sourceCode/mainContainers/sceneContainers/bannerContainer.cpp
    sourceCode/mainContainers/sceneContainers/drawingContainer.cpp
    sourceCode/mainContainers/sceneContainers/textureContainer.cpp
    sourceCode/mainContainers/sceneContainers/simulation.cpp
    sourceCode/mainContainers/sceneContainers/signalContainer.cpp
    sourceCode/mainContainers/sceneContainers/registeredPathPlanningTasks.cpp
    sourceCode/mainContainers/sceneContainers/registeredMotionPlanningTasks.cpp
    sourceCode/mainContainers/sceneContainers/registerediks.cpp
    sourceCode/mainContainers/sceneContainers/registeredCollections.cpp
    sourceCode/mainContainers/sceneContainers/registeredDistances.cpp
    sourceCode/mainContainers/sceneContainers/registeredCollisions.cpp
    sourceCode/mainContainers/sceneContainers/outsideCommandQueue.cpp
    sourceCode/mainContainers/sceneContainers/objCont.cpp
    sourceCode/mainContainers/sceneContainers/memorizedConfContainer.cpp
    sourceCode/mainContainers/sceneContainers/mainSettings.cpp
    sourceCode/mainContainers/sceneContainers/mainCont.cpp
    sourceCode/mainContainers/sceneContainers/luaScriptContainer.cpp
    sourceCode/mainContainers/sceneContainers/environment.cpp
    sourceCode/mainContainers/sceneContainers/dynamicsContainer.cpp
    sourceCode/mainContainers/sceneContainers/customData.cpp
    sourceCode/mainContainers/sceneContainers/constraintSolverContainer.cpp
    sourceCode/mainContainers/sceneContainers/commTubeContainer.cpp
    sourceCode/mainContainers/sceneContainers/cacheCont.cpp
    sourceCode/mainContainers/sceneContainers/apiErrors.cpp
    sourceCode/mainContainers/sceneContainers/ghostObjectContainer.cpp
    sourceCode/mainContainers/sceneContainers/pointCloudContainer_old.cpp
    sourceCode/mainContainers/sceneContainers/buttonBlockContainer.cpp
    sourceCode/mainContainers/sceneContainers/undoBufferCont.cpp

    sourceCode/mainContainers/applicationContainers/copyBuffer.cpp
    sourceCode/mainContainers/applicationContainers/persistentDataContainer.cpp
    sourceCode/mainContainers/applicationContainers/simulatorMessageQueue.cpp
    sourceCode/mainContainers/applicationContainers/calculationInfo.cpp
    sourceCode/mainContainers/applicationContainers/interfaceStackContainer.cpp
    sourceCode/mainContainers/applicationContainers/addOnScriptContainer.cpp

    sourceCode/3dObjects/visionSensorObjectRelated/imageProcess.cpp
    sourceCode/3dObjects/visionSensorObjectRelated/simpleFilter.cpp
    sourceCode/3dObjects/visionSensorObjectRelated/composedFilter.cpp

    sourceCode/pathPlanning_old/pathPlanningTask_old.cpp

    sourceCode/motionPlanning_old/motionPlanningTask_old.cpp

    sourceCode/luaScripting/luaScriptParameters.cpp
    sourceCode/luaScripting/luaScriptObject.cpp
    sourceCode/luaScripting/outsideCommandQueueForScript.cpp
    sourceCode/luaScripting/luaWrapper.cpp

    sourceCode/luaScripting/customLuaFuncAndVar/luaCustomFuncAndVarContainer.cpp
    sourceCode/luaScripting/customLuaFuncAndVar/luaCustomFunction.cpp
    sourceCode/luaScripting/customLuaFuncAndVar/luaCustomVariable.cpp

    sourceCode/pagesAndViews/pageContainer.cpp
    sourceCode/pagesAndViews/sPage.cpp
    sourceCode/pagesAndViews/sView.cpp

    sourceCode/textures/textureObject.cpp
    sourceCode/textures/imgLoaderSaver.cpp
    sourceCode/textures/tGAFormat.cpp
    sourceCode/textures/stb_image.c
    sourceCode/textures/textureProperty.cpp

    sourceCode/serialization/ser.cpp
    sourceCode/serialization/xmlSer.cpp
    sourceCode/serialization/extIkSer.cpp
    sourceCode/serialization/huffman.c
    sourceCode/serialization/tinyxml2.cpp

    sourceCode/interfaces/v_rep.cpp
    sourceCode/interfaces/v_rep_internal.cpp
    sourceCode/interfaces/luaScriptFunctions.cpp
    sourceCode/interfaces/pathPlanningInterface.cpp

    sourceCode/interfaces/interfaceStack/interfaceStack.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackObject.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackNull.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackBool.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackNumber.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackString.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackTable.cpp

    sourceCode/menusAndSimilar/addOperations.cpp
    sourceCode/menusAndSimilar/fileOperations.cpp

    sourceCode/variousFunctions/sceneObjectOperations.cpp

    sourceCode/geometricAlgorithms/linMotionRoutines.cpp
    sourceCode/geometricAlgorithms/meshRoutines.cpp
    sourceCode/geometricAlgorithms/meshManip.cpp
    sourceCode/geometricAlgorithms/edgeElement.cpp
    sourceCode/geometricAlgorithms/algos.cpp

    sourceCode/various/gV.cpp
    sourceCode/various/memorizedConf.cpp
    sourceCode/various/userSettings.cpp
    sourceCode/various/directoryPaths.cpp
    sourceCode/various/constraintSolverObject.cpp
    sourceCode/various/uiThread.cpp
    sourceCode/various/simThread.cpp
    sourceCode/various/app.cpp
    sourceCode/various/dynMaterialObject.cpp
    sourceCode/various/easyLock.cpp
    sourceCode/various/funcDebug.cpp
    sourceCode/various/ghostObject.cpp
    sourceCode/various/debugLogFile.cpp
    sourceCode/various/sigHandler.cpp

    sourceCode/undoRedo/undoBufferArrays.cpp
    sourceCode/undoRedo/undoBuffer.cpp
    sourceCode/undoRedo/undoBufferCameras.cpp

    sourceCode/rendering/rendering.cpp
    sourceCode/rendering/cameraRendering.cpp
    sourceCode/rendering/visionSensorRendering.cpp
    sourceCode/rendering/forceSensorRendering.cpp
    sourceCode/rendering/proximitySensorRendering.cpp
    sourceCode/rendering/dummyRendering.cpp
    sourceCode/rendering/lightRendering.cpp
    sourceCode/rendering/pointCloudRendering.cpp
    sourceCode/rendering/octreeRendering.cpp
    sourceCode/rendering/jointRendering.cpp
    sourceCode/rendering/graphRendering.cpp
    sourceCode/rendering/millRendering.cpp
    sourceCode/rendering/mirrorRendering.cpp
    sourceCode/rendering/pathRendering.cpp
    sourceCode/rendering/shapeRendering.cpp
    sourceCode/rendering/ghostRendering.cpp
    sourceCode/rendering/drawingObjectRendering.cpp
    sourceCode/rendering/drawingObject2DRendering.cpp
    sourceCode/rendering/ptCloudRendering_old.cpp
    sourceCode/rendering/collisionContourRendering.cpp
    sourceCode/rendering/distanceRendering.cpp
    sourceCode/rendering/bannerRendering.cpp
    sourceCode/rendering/thumbnailRendering.cpp
    sourceCode/rendering/pathPlanningTaskRendering_old.cpp
    sourceCode/rendering/motionPlanningTaskRendering_old.cpp
    sourceCode/rendering/broadcastDataVisualRendering.cpp
    sourceCode/rendering/dynamicsRendering.cpp
    sourceCode/rendering/environmentRendering.cpp
    sourceCode/rendering/pageRendering.cpp
    sourceCode/rendering/viewRendering.cpp

    sourceCode/libsAndPlugins/pluginContainer.cpp

    sourceCode/visual/visualParam.cpp
    sourceCode/visual/thumbnail.cpp

    sourceCode/utils/threadPool.cpp
    sourceCode/utils/ttUtil.cpp
    sourceCode/utils/tt.cpp
    sourceCode/utils/confReaderAndWriter.cpp
    sourceCode/utils/base64.cpp

    sourceCode/customUserInterfaces/buttonBlock.cpp
    sourceCode/customUserInterfaces/softButton.cpp
)

if(WITH_QT)
    set(SOURCES ${SOURCES}
        sourceCode/platform/vSimUiMutex.cpp
        sourceCode/platform/wThread.cpp

        sourceCode/various/simAndUiThreadSync.cpp
        sourceCode/various/vrepQApp.cpp
    )
endif()

if(WITH_SERIAL)
    set(SOURCES ${SOURCES}
        sourceCode/mainContainers/applicationContainers/serialPortContainer.cpp

        sourceCode/communication/serialPort/serialPort.cpp
        sourceCode/communication/serialPort/serial_win_mac_linux.cpp
        sourceCode/communication/serialPort/serialPortWin.cpp
    )
endif()

if(WITH_OPENGL)
    set(SOURCES ${SOURCES}
        sourceCode/3dObjects/visionSensorObjectRelated/offscreenGlContext.cpp
        sourceCode/3dObjects/visionSensorObjectRelated/frameBufferObject.cpp
        sourceCode/3dObjects/visionSensorObjectRelated/visionSensorGlStuff.cpp

        sourceCode/visual/oGL.cpp
        sourceCode/visual/oglExt.cpp
        sourceCode/visual/glShader.cpp
        sourceCode/visual/glBufferObjects.cpp
    )
endif()

if(WITH_GUI)
    set(SOURCES ${SOURCES}
        sourceCode/gui/dialogs/qdlgsettings.cpp
        sourceCode/gui/dialogs/qdlglayers.cpp
        sourceCode/gui/dialogs/qdlgavirecorder.cpp
        sourceCode/gui/dialogs/qdlgselection.cpp
        sourceCode/gui/dialogs/qdlgabout.cpp
        sourceCode/gui/dialogs/qdlgsimulation.cpp
        sourceCode/gui/dialogs/qdlginteractiveik.cpp
        sourceCode/gui/dialogs/qdlgcommonproperties.cpp
        sourceCode/gui/dialogs/qdlgdetectableproperties.cpp
        sourceCode/gui/dialogs/qdlgmodelproperties.cpp
        sourceCode/gui/dialogs/qdlgassembly.cpp
        sourceCode/gui/dialogs/qdlgslider.cpp
        sourceCode/gui/dialogs/qdlgslider2.cpp
        sourceCode/gui/dialogs/qdlginertiatensor.cpp
        sourceCode/gui/dialogs/qdlgmodelthumbnailvisu.cpp
        sourceCode/gui/dialogs/qdlgmodelthumbnail.cpp
        sourceCode/gui/dialogs/qdlgenvironment.cpp
        sourceCode/gui/dialogs/qdlgcolor.cpp
        sourceCode/gui/dialogs/qdlgmaterial.cpp
        sourceCode/gui/dialogs/qdlglightmaterial.cpp
        sourceCode/gui/dialogs/qdlgcolorpulsation.cpp
        sourceCode/gui/dialogs/qdlgfog.cpp
        sourceCode/gui/dialogs/qdlgcollections.cpp
        sourceCode/gui/dialogs/qdlgcollisions.cpp
        sourceCode/gui/dialogs/qdlgdistances.cpp
        sourceCode/gui/dialogs/qdlgentityvsentityselection.cpp
        sourceCode/gui/dialogs/qdlgscripts.cpp
        sourceCode/gui/dialogs/qdlginsertscript.cpp
        sourceCode/gui/dialogs/qdlgdynamics.cpp
        sourceCode/gui/dialogs/qdlgscaling.cpp
        sourceCode/gui/dialogs/qdlgcameras.cpp
        sourceCode/gui/dialogs/qdlglights.cpp
        sourceCode/gui/dialogs/qdlgmirrors.cpp
        sourceCode/gui/dialogs/qdlgdummies.cpp
        sourceCode/gui/dialogs/qdlgoctrees.cpp
        sourceCode/gui/dialogs/qdlgpointclouds.cpp
        sourceCode/gui/dialogs/qdlgscriptparameters.cpp
        sourceCode/gui/dialogs/qdlgforcesensors.cpp
        sourceCode/gui/dialogs/qdlgprimitives.cpp
        sourceCode/gui/dialogs/qdlgconvexdecomposition.cpp
        sourceCode/gui/dialogs/qdlgconstraintsolver.cpp
        sourceCode/gui/dialogs/qdlgvisionsensors.cpp
        sourceCode/gui/dialogs/qdlgimagecolor.cpp
        sourceCode/gui/dialogs/qdlgsimpleSearch.cpp
        sourceCode/gui/dialogs/qdlgfilters.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_rotate.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_uniformImage.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_shift.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_coordExtraction.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_velodyne.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_pixelChange.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_scaleCols.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_resize.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_rectangularCut.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_keepRemoveCols.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_intensityScale.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_edge.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_correlation.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_colorSegmentation.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_circularCut.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_blob.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_binary.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_5x5.cpp
        sourceCode/gui/dialogs/qdlgsimplefilter_3x3.cpp
        sourceCode/gui/dialogs/qdlgshapes.cpp
        sourceCode/gui/dialogs/qdlgmultishapeedition.cpp
        sourceCode/gui/dialogs/qdlgtextures.cpp
        sourceCode/gui/dialogs/qdlgtextureselection.cpp
        sourceCode/gui/dialogs/qdlggeometry.cpp
        sourceCode/gui/dialogs/qdlgshapedyn.cpp
        sourceCode/gui/dialogs/qdlgdetectionvolume.cpp
        sourceCode/gui/dialogs/qdlgproximitysensors.cpp
        sourceCode/gui/dialogs/qdlgproxsensdetectionparam.cpp
        sourceCode/gui/dialogs/qdlgmills.cpp
        sourceCode/gui/dialogs/qdlgobjectdialogcontainer.cpp
        sourceCode/gui/dialogs/qdlgshapeeditioncontainer.cpp
        sourceCode/gui/dialogs/qdlgcalcdialogcontainer.cpp
        sourceCode/gui/dialogs/qdlgheightfielddimension.cpp
        sourceCode/gui/dialogs/qdlgstopscripts.cpp
        sourceCode/gui/dialogs/qdlgtextureloadoptions.cpp
        sourceCode/gui/dialogs/qdlgprogress.cpp
        sourceCode/gui/dialogs/qdlgjoints.cpp
        sourceCode/gui/dialogs/qdlgdependencyequation.cpp
        sourceCode/gui/dialogs/qdlgjointdyn.cpp
        sourceCode/gui/dialogs/qdlggraphs.cpp
        sourceCode/gui/dialogs/qdlgdatastreamselection.cpp
        sourceCode/gui/dialogs/qdlg2d3dgraphproperties.cpp
        sourceCode/gui/dialogs/qdlgaddgraphcurve.cpp
        sourceCode/gui/dialogs/qdlgik.cpp
        sourceCode/gui/dialogs/qdlgikelements.cpp
        sourceCode/gui/dialogs/qdlgikconditional.cpp
        sourceCode/gui/dialogs/qdlgikavoidance.cpp
        sourceCode/gui/dialogs/qdlgui.cpp
        sourceCode/gui/dialogs/qdlguidialogcontainer.cpp
        sourceCode/gui/dialogs/qdlguibuttons.cpp
        sourceCode/gui/dialogs/qdlgnewui.cpp
        sourceCode/gui/dialogs/qdlguirolledup.cpp
        sourceCode/gui/dialogs/qdlgpathplanning.cpp
        sourceCode/gui/dialogs/qdlgmotionplanning.cpp
        sourceCode/gui/dialogs/qdlgpathplanningparams.cpp
        sourceCode/gui/dialogs/qdlgmotionplanningjoints.cpp
        sourceCode/gui/dialogs/qdlgworkspacemetric.cpp
        sourceCode/gui/dialogs/qdlgpathplanningaddnew.cpp
        sourceCode/gui/dialogs/qdlgpaths.cpp
        sourceCode/gui/dialogs/qdlgpathshaping.cpp
        sourceCode/gui/dialogs/qdlgmessageandcheckbox.cpp
        sourceCode/gui/dialogs/qdlgopenglsettings.cpp
        sourceCode/gui/dialogs/qdlgtriangleedit.cpp
        sourceCode/gui/dialogs/qdlgvertexedit.cpp
        sourceCode/gui/dialogs/qdlgedgeedit.cpp
        sourceCode/gui/dialogs/qdlgpathedit.cpp
        sourceCode/gui/dialogs/qdlgtranslation.cpp
        sourceCode/gui/dialogs/qdlgrotation.cpp
        sourceCode/gui/dialogs/qdlgtranslationrotationcont.cpp

        sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtbuttonpropertybrowser.cpp
        sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qteditorfactory.cpp
        sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtgroupboxpropertybrowser.cpp
        sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtpropertybrowser.cpp
        sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtpropertybrowserutils.cpp
        sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtpropertymanager.cpp
        sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qttreepropertybrowser.cpp
        sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtvariantproperty.cpp

        sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserRelatedFiles/buttoneditfactory.cpp
        sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserRelatedFiles/buttoneditmanager.cpp
        sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserRelatedFiles/pushbuttonedit.cpp

        sourceCode/gui/qtPropertyBrowserRelated/propBrowser_engineProp_material.cpp
        sourceCode/gui/qtPropertyBrowserRelated/propBrowser_engineProp_general.cpp
        sourceCode/gui/qtPropertyBrowserRelated/propBrowser_engineProp_joint.cpp

        sourceCode/gui/dialogs/other/editboxdelegate.cpp
        sourceCode/gui/dialogs/other/toolDlgWrapper.cpp
        sourceCode/gui/dialogs/other/dlgEx.cpp
        sourceCode/gui/dialogs/other/dlgCont.cpp
        sourceCode/gui/dialogs/other/qvrep_openglwidget.cpp

        sourceCode/gui/platform/vMessageBox.cpp
        sourceCode/gui/platform/vMenubar.cpp
        sourceCode/gui/platform/vFileDialog.cpp
        sourceCode/gui/platform/vDialog.cpp

        sourceCode/gui/mainContainers/sceneContainers/oglSurface.cpp

        sourceCode/gui/mainContainers/applicationContainers/globalGuiTextureContainer.cpp
        sourceCode/gui/mainContainers/applicationContainers/codeEditorContainer.cpp

        sourceCode/gui/editModes/editModeContainer.cpp
        sourceCode/gui/editModes/shapeEditMode.cpp
        sourceCode/gui/editModes/multishapeEditMode.cpp
        sourceCode/gui/editModes/pathEditMode.cpp
        sourceCode/gui/editModes/uiEditMode.cpp
        sourceCode/gui/editModes/edgeCont.cpp
        sourceCode/gui/editModes/pathPointManipulation.cpp

        sourceCode/gui/sceneHierarchy/hierarchy.cpp
        sourceCode/gui/sceneHierarchy/hierarchyElement.cpp
        sourceCode/gui/sceneHierarchy/sceneHierarchyWidget.cpp
        sourceCode/gui/sceneHierarchy/sceneHierarchyItemDelegate.cpp

        sourceCode/gui/modelBrowser/modelFolderWidget.cpp
        sourceCode/gui/modelBrowser/modelListWidget.cpp
        sourceCode/gui/modelBrowser/modelListWidgetBase.cpp

        sourceCode/gui/selectors/pageSelector.cpp
        sourceCode/gui/selectors/viewSelector.cpp
        sourceCode/gui/selectors/sceneSelector.cpp

        sourceCode/gui/menusAndSimilar/toolBarCommand.cpp
        sourceCode/gui/menusAndSimilar/customMenuBarItemContainer.cpp
        sourceCode/gui/menusAndSimilar/customMenuBarItem.cpp
        sourceCode/gui/menusAndSimilar/mainWindow.cpp
        sourceCode/gui/menusAndSimilar/statusBar.cpp
        sourceCode/gui/menusAndSimilar/helpMenu.cpp

        sourceCode/gui/various/simRecorder.cpp

        sourceCode/gui/libs/auxLibVideo.cpp
    )
endif()

add_library(v_rep SHARED ${SOURCES})
target_link_libraries(v_rep ${BOOST_LIBRARIES})
target_link_libraries(v_rep ${LUA_LIBRARIES})
if(WITH_QT)
    target_link_libraries(v_rep Qt5::Core)
endif()
if(WITH_GUI)
    target_link_libraries(v_rep Qt5::Gui)
    target_link_libraries(v_rep Qt5::Widgets)
endif()
if(WITH_OPENGL)
    target_link_libraries(v_rep OpenGL::GL)
    target_link_libraries(v_rep Qt5::Gui)
    target_link_libraries(v_rep Qt5::OpenGL)
endif()
if(WITH_SERIAL)
    target_link_libraries(v_rep Qt5::SerialPort)
endif()
if(WIN32)
    target_link_libraries(v_rep
        -lwinmm
        -lShell32
        -lopengl32
        -luser32
        -lDbghelp
    )
endif()
