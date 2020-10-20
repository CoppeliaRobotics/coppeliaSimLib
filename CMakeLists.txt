cmake_minimum_required(VERSION 3.9.0)
project(coppeliaSim)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_MACOSX_RPATH 1)

set(CMAKE_AUTOUIC_SEARCH_PATHS ui)
set(WITH_QT true CACHE BOOL "Enable Qt")

if(WITH_QT)
    set(WITH_GUI true CACHE BOOL "Enable gui")
    set(WITH_OPENGL true CACHE BOOL "Enable OpenGL")
    set(WITH_SERIAL false)
    add_definitions(-DSIM_WITH_QT)
else()
    unset(WITH_GUI CACHE)
    unset(WITH_OPENGL CACHE)
    unset(WITH_SERIAL CACHE)
    set(WITH_GUI false)
    set(WITH_OPENGL false)
    set(WITH_SERIAL false)
endif()

find_package(Qt5PrintSupport REQUIRED)

find_package(Lua 5.1 EXACT REQUIRED)
find_package(Eigen3)
find_package(Boost REQUIRED)

include_directories(${LUA_INCLUDE_DIR})

if(NOT EIGEN_FOUND)
    set(EIGEN_INCLUDE_DIR /usr/local/include/eigen3 CACHE STRING "Eigen include path")
endif()
include_directories(${EIGEN_INCLUDE_DIR})

add_definitions(-DSIM_LIB)

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
    add_definitions(/FI"simMainHeader.h")
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -fp:precise")
    set(CMAKE_CXXFLAGS_RELEASE "${CMAKE_CXXFLAGS_RELEASE} -O2")

    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -W3")
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -wd4100") # "unreferenced formal parameter"
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -wd4996") # "function or variable may be unsafe..."
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -wd4101") # "unreferenced local variable"

    if(MSVC_VERSION EQUAL 1900)
        add_definitions(-DSIM_COMPILER_STR=\"MSVC2015\")
    elseif(MSVC_VERSION GREATER 1900 AND MSVC_VERSION LESS 1920)
        add_definitions(-DSIM_COMPILER_STR=\"MSVC2017\")
    else()
        add_definitions(-DSIM_COMPILER_STR=\"MSVC2019\")
    endif()
else()
    add_definitions(-include simMainHeader.h)
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
        add_definitions(-DSIM_COMPILER_STR=\"Clang\")
    else()
        add_definitions(-DSIM_COMPILER_STR=\"GCC\")
    endif()
    # XXX: why don't we simply use CMAKE_CXX_COMPILER_ID?
endif()

if(WIN32)
    add_definitions(-DWIN_SIM)
elseif(APPLE)
    add_definitions(-DMAC_SIM)
elseif(UNIX)
    add_definitions(-DLIN_SIM)
endif()

include_directories(sourceCode)
include_directories(sourceCode/shared)
include_directories(sourceCode/kinematics)
include_directories(sourceCode/shared/kinematics)
include_directories(sourceCode/communication)
include_directories(sourceCode/communication/tubes)
include_directories(sourceCode/communication/wireless)
include_directories(sourceCode/drawingObjects)
include_directories(sourceCode/platform)
include_directories(sourceCode/collections)
include_directories(sourceCode/shared/collections)
include_directories(sourceCode/collisions)
include_directories(sourceCode/shared/collisions)
include_directories(sourceCode/distances)
include_directories(sourceCode/shared/distances)
include_directories(sourceCode/pathPlanning_old)
include_directories(sourceCode/sceneObjects)
include_directories(sourceCode/shared/sceneObjects)
include_directories(sourceCode/sceneObjects/related)
include_directories(sourceCode/shared/sceneObjects/related)
include_directories(sourceCode/sceneObjects/graphObjectRelated)
include_directories(sourceCode/sceneObjects/pathObjectRelated)
include_directories(sourceCode/sceneObjects/proximitySensorObjectRelated)
include_directories(sourceCode/sceneObjects/shapeObjectRelated)
include_directories(sourceCode/sceneObjects/visionSensorObjectRelated)
include_directories(sourceCode/mainContainers)
include_directories(sourceCode/shared/mainContainers)
include_directories(sourceCode/mainContainers/sceneContainers)
include_directories(sourceCode/shared/mainContainers/sceneContainers)
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
include_directories(sourceCode/shared/various)
include_directories(sourceCode/libsAndPlugins)
include_directories(sourceCode/visual)
include_directories(sourceCode/displ)
include_directories(sourceCode/shared/displ)
include_directories(sourceCode/utils)
include_directories(sourceCode/backwardCompatibility/pathPlanning)
include_directories(sourceCode/customUserInterfaces)
include_directories(sourceCode/undoRedo)
include_directories(sourceCode/rendering)
include_directories(../programming/include)
include_directories(../programming/simMath)


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
    list(APPEND SOURCES
        targaFiles.qrc
        toolbarFiles.qrc
        variousImageFiles.qrc
        iconFiles.qrc
        imageFiles.qrc
        qdarkstyle/style.qrc
    )
endif()


if(WITH_GUI)
    list(APPEND SOURCES
        sourceCode/gui/dialogs/qdlgsettings.cpp
        sourceCode/gui/dialogs/qdlglayers.cpp
        sourceCode/gui/dialogs/qdlgavirecorder.cpp
        sourceCode/gui/dialogs/qdlgabout.cpp
        sourceCode/gui/dialogs/qdlgsimulation.cpp
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
        sourceCode/gui/dialogs/qdlguserparameters.cpp
        sourceCode/gui/dialogs/qdlgforcesensors.cpp
        sourceCode/gui/dialogs/qdlgprimitives.cpp
        sourceCode/gui/dialogs/qdlgconvexdecomposition.cpp
        sourceCode/gui/dialogs/qdlgvisionsensors.cpp
        sourceCode/gui/dialogs/qdlgimagecolor.cpp
        sourceCode/gui/dialogs/qdlgshapes.cpp
        sourceCode/gui/dialogs/qdlgmultishapeedition.cpp
        sourceCode/gui/dialogs/qdlgtextures.cpp
        sourceCode/gui/dialogs/qdlgtextureselection.cpp
        sourceCode/gui/dialogs/qdlggeometry.cpp
        sourceCode/gui/dialogs/qdlgshapedyn.cpp
        sourceCode/gui/dialogs/qdlgdetectionvolume.cpp
        sourceCode/gui/dialogs/qdlgproximitysensors.cpp
        sourceCode/gui/dialogs/qdlgproxsensdetectionparam.cpp
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
        sourceCode/gui/dialogs/other/qsim_openglwidget.cpp

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

list(APPEND SOURCES
    ../programming/simMath/Vector.cpp
    ../programming/simMath/MyMath.cpp
    ../programming/simMath/MMatrix.cpp
    ../programming/simMath/7Vector.cpp
    ../programming/simMath/6X6Matrix.cpp
    ../programming/simMath/6Vector.cpp
    ../programming/simMath/4X4Matrix.cpp
    ../programming/simMath/4X4FullMatrix.cpp
    ../programming/simMath/4Vector.cpp
    ../programming/simMath/3X3Matrix.cpp
    ../programming/simMath/3Vector.cpp

    sourceCode/kinematics/ikGroup.cpp
    sourceCode/shared/kinematics/_ikGroup_.cpp
    sourceCode/kinematics/ikElement.cpp
    sourceCode/shared/kinematics/_ikElement_.cpp

    sourceCode/drawingObjects/bannerObject.cpp
    sourceCode/drawingObjects/drawingObject.cpp
    sourceCode/drawingObjects/ptCloud_old.cpp

    sourceCode/platform/vVarious.cpp
    sourceCode/platform/vThread.cpp
    sourceCode/platform/vMutex.cpp
    sourceCode/platform/vFileFinder.cpp
    sourceCode/platform/vFile.cpp
    sourceCode/platform/vDateTime.cpp
    sourceCode/platform/vArchive.cpp

    sourceCode/collections/collectionElement.cpp
    sourceCode/shared/collections/_collectionElement_.cpp
    sourceCode/collections/collection.cpp
    sourceCode/shared/collections/_collection_.cpp

    sourceCode/collisions/collisionObject.cpp
    sourceCode/shared/collisions/_collisionObject_.cpp
    sourceCode/collisions/collisionRoutines.cpp

    sourceCode/distances/distanceObject.cpp
    sourceCode/shared/distances/_distanceObject_.cpp
    sourceCode/distances/distanceRoutines.cpp

    sourceCode/sceneObjects/related/sceneObject.cpp
    sourceCode/shared/sceneObjects/related/_sceneObject_.cpp
    sourceCode/sceneObjects/related/convexVolume.cpp
    sourceCode/sceneObjects/related/viewableBase.cpp

    sourceCode/sceneObjects/jointObject.cpp
    sourceCode/shared/sceneObjects/_jointObject_.cpp
    sourceCode/sceneObjects/camera.cpp
    sourceCode/sceneObjects/dummy.cpp
    sourceCode/shared/sceneObjects/_dummy_.cpp
    sourceCode/sceneObjects/octree.cpp
    sourceCode/sceneObjects/pointCloud.cpp
    sourceCode/sceneObjects/forceSensor.cpp
    sourceCode/sceneObjects/graph.cpp
    sourceCode/sceneObjects/light.cpp
    sourceCode/sceneObjects/mirror.cpp
    sourceCode/sceneObjects/mill.cpp
    sourceCode/sceneObjects/path.cpp
    sourceCode/sceneObjects/proximitySensor.cpp
    sourceCode/sceneObjects/shape.cpp
    sourceCode/sceneObjects/visionSensor.cpp

    sourceCode/sceneObjects/graphObjectRelated/graphingRoutines.cpp
    sourceCode/sceneObjects/graphObjectRelated/graphDataComb.cpp
    sourceCode/sceneObjects/graphObjectRelated/graphData.cpp
    sourceCode/sceneObjects/graphObjectRelated/staticGraphCurve.cpp

    sourceCode/sceneObjects/pathObjectRelated/bezierPathPoint.cpp
    sourceCode/sceneObjects/pathObjectRelated/simplePathPoint.cpp
    sourceCode/sceneObjects/pathObjectRelated/pathPoint.cpp
    sourceCode/sceneObjects/pathObjectRelated/pathCont.cpp

    sourceCode/sceneObjects/proximitySensorObjectRelated/proxSensorRoutine.cpp

    sourceCode/sceneObjects/shapeObjectRelated/mesh.cpp
    sourceCode/sceneObjects/shapeObjectRelated/meshWrapper.cpp
    sourceCode/sceneObjects/shapeObjectRelated/volInt.cpp


    sourceCode/backwardCompatibility/pathPlanning/pathPlanning.cpp
    sourceCode/backwardCompatibility/pathPlanning/holonomicPathPlanning.cpp
    sourceCode/backwardCompatibility/pathPlanning/holonomicPathNode.cpp
    sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathPlanning.cpp
    sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathNode.cpp

    sourceCode/communication/tubes/commTube.cpp

    sourceCode/communication/wireless/broadcastDataContainer.cpp
    sourceCode/communication/wireless/broadcastData.cpp
    sourceCode/communication/wireless/broadcastDataVisual.cpp

    sourceCode/mainContainers/worldContainer.cpp
    sourceCode/shared/mainContainers/_worldContainer_.cpp
    sourceCode/mainContainers/world.cpp
    sourceCode/shared/mainContainers/_world_.cpp

    sourceCode/mainContainers/sceneContainers/bannerContainer.cpp
    sourceCode/mainContainers/sceneContainers/drawingContainer.cpp
    sourceCode/mainContainers/sceneContainers/textureContainer.cpp
    sourceCode/mainContainers/sceneContainers/simulation.cpp
    sourceCode/mainContainers/sceneContainers/signalContainer.cpp
    sourceCode/mainContainers/sceneContainers/registeredPathPlanningTasks.cpp
    sourceCode/mainContainers/sceneContainers/ikGroupContainer.cpp
    sourceCode/shared/mainContainers/sceneContainers/_ikGroupContainer_.cpp
    sourceCode/mainContainers/sceneContainers/collectionContainer.cpp
    sourceCode/shared/mainContainers/sceneContainers/_collectionContainer_.cpp
    sourceCode/mainContainers/sceneContainers/distanceObjectContainer.cpp
    sourceCode/shared/mainContainers/sceneContainers/_distanceObjectContainer_.cpp
    sourceCode/mainContainers/sceneContainers/collisionObjectContainer.cpp
    sourceCode/shared/mainContainers/sceneContainers/_collisionObjectContainer_.cpp
    sourceCode/mainContainers/sceneContainers/outsideCommandQueue.cpp
    sourceCode/mainContainers/sceneContainers/sceneObjectContainer.cpp
    sourceCode/shared/mainContainers/sceneContainers/_sceneObjectContainer_.cpp
    sourceCode/mainContainers/sceneContainers/mainSettings.cpp
    sourceCode/mainContainers/sceneContainers/luaScriptContainer.cpp
    sourceCode/mainContainers/sceneContainers/environment.cpp
    sourceCode/mainContainers/sceneContainers/dynamicsContainer.cpp
    sourceCode/mainContainers/sceneContainers/customData.cpp
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

    sourceCode/sceneObjects/visionSensorObjectRelated/simpleFilter.cpp
    sourceCode/sceneObjects/visionSensorObjectRelated/composedFilter.cpp

    sourceCode/pathPlanning_old/pathPlanningTask_old.cpp

    sourceCode/luaScripting/userParameters.cpp
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
    sourceCode/serialization/extIkSer.cpp
    sourceCode/serialization/huffman.c
    sourceCode/serialization/tinyxml2.cpp

    sourceCode/interfaces/sim.cpp
    sourceCode/interfaces/simInternal.cpp
    sourceCode/interfaces/luaScriptFunctions.cpp
    sourceCode/interfaces/pathPlanningInterface.cpp

    sourceCode/interfaces/simFlavor.cpp

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
    sourceCode/various/folderSystem.cpp
    sourceCode/various/uiThread.cpp
    sourceCode/various/simThread.cpp
    sourceCode/various/app.cpp
    sourceCode/various/dynMaterialObject.cpp
    sourceCode/various/easyLock.cpp
    sourceCode/various/ghostObject.cpp
    sourceCode/various/sigHandler.cpp
    sourceCode/various/syncObject.cpp
    sourceCode/shared/various/_syncObject_.cpp

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
    sourceCode/rendering/ptCloudRendering_old.cpp
    sourceCode/rendering/collisionContourRendering.cpp
    sourceCode/rendering/distanceRendering.cpp
    sourceCode/rendering/bannerRendering.cpp
    sourceCode/rendering/thumbnailRendering.cpp
    sourceCode/rendering/pathPlanningTaskRendering_old.cpp
    sourceCode/rendering/broadcastDataVisualRendering.cpp
    sourceCode/rendering/dynamicsRendering.cpp
    sourceCode/rendering/environmentRendering.cpp
    sourceCode/rendering/pageRendering.cpp
    sourceCode/rendering/viewRendering.cpp

    sourceCode/libsAndPlugins/pluginContainer.cpp

    sourceCode/displ/colorObject.cpp
    sourceCode/shared/displ/_colorObject_.cpp

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
    list(APPEND SOURCES
        sourceCode/platform/vSimUiMutex.cpp
        sourceCode/platform/wThread.cpp

        sourceCode/various/simAndUiThreadSync.cpp
        sourceCode/various/simQApp.cpp
    )
endif()

if(WITH_SERIAL)
    list(APPEND SOURCES
        sourceCode/mainContainers/applicationContainers/serialPortContainer.cpp

        sourceCode/communication/serialPort/serialPort.cpp
        sourceCode/communication/serialPort/serial_win_mac_linux.cpp
        sourceCode/communication/serialPort/serialPortWin.cpp
    )
endif()

if(WITH_OPENGL)
    list(APPEND SOURCES
        sourceCode/sceneObjects/visionSensorObjectRelated/offscreenGlContext.cpp
        sourceCode/sceneObjects/visionSensorObjectRelated/frameBufferObject.cpp
        sourceCode/sceneObjects/visionSensorObjectRelated/visionSensorGlStuff.cpp

        sourceCode/visual/oGL.cpp
        sourceCode/visual/oglExt.cpp
        sourceCode/visual/glShader.cpp
        sourceCode/visual/glBufferObjects.cpp
    )
endif()


add_library(coppeliaSim SHARED ${SOURCES})
target_link_libraries(coppeliaSim Boost::boost)
target_link_libraries(coppeliaSim ${LUA_LIBRARIES})
if(WITH_QT)
    target_link_libraries(coppeliaSim Qt5::Core)
endif()
if(WITH_GUI)
    target_link_libraries(coppeliaSim Qt5::Gui)
    target_link_libraries(coppeliaSim Qt5::Widgets)
endif()
if(WITH_OPENGL)
    target_link_libraries(coppeliaSim OpenGL::GL)
    target_link_libraries(coppeliaSim Qt5::Gui)
    target_link_libraries(coppeliaSim Qt5::OpenGL)
endif()
if(WITH_SERIAL)
    target_link_libraries(coppeliaSim Qt5::SerialPort)
endif()
if(WIN32)
    target_link_libraries(coppeliaSim
        -lwinmm
        -lShell32
        -lopengl32
        -luser32
        -lDbghelp
    )
endif()
