cmake_minimum_required(VERSION 3.9.0)
project(coppeliaSim)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_MACOSX_RPATH 1)

set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

if(NOT LUA_INCLUDE_DIR OR NOT LUA_LIBRARIES)
    find_package(Lua 5.3 EXACT REQUIRED)
endif()

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(tinyxml2 REQUIRED IMPORTED_TARGET tinyxml2)
    add_library(tinyxml2::tinyxml2 ALIAS PkgConfig::tinyxml2)
else()
    find_package(TinyXML2 REQUIRED COMPONENTS shared)
endif()

find_package(Eigen3)
if(NOT EIGEN_FOUND)
    include(FetchContent)
    FetchContent_Declare(eigen3src
        URL https://gitlab.com/libeigen/eigen/-/archive/3.3.8/eigen-3.3.8.tar.gz
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/eigen3
    )
    if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/eigen3/Eigen/Sparse")
        message(STATUS "Downloading eigen-3.3.8 source...")
        FetchContent_Populate(eigen3src)
    endif()
    set(EIGEN_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/eigen3)
endif()

find_package(Boost REQUIRED)

set(WITH_QSCINTILLA true CACHE BOOL "Link QScintilla library (you'll need to set QSCINTILLA_DIR and QSCINTILLA_LIBRARY variables)")
if(WITH_QSCINTILLA)
    set(QSCINTILLA_DIR NOTFOUND CACHE PATH "Path to the QScintilla directory")
    if(EXISTS "${QSCINTILLA_DIR}")
        message(STATUS "QSCINTILLA_DIR: ${QSCINTILLA_DIR}")
    else()
        message(FATAL_ERROR "The specified QSCINTILLA_DIR (${QSCINTILLA_DIR}) does not exist")
    endif()

    # try to figure out the QSCINTILLA_BUILD_DIR (only if it is valid)
    if(EXISTS "${QSCINTILLA_DIR}/Qt4Qt5/release/moc_qsciscintilla.cpp")
        set(QSCINTILLA_BUILD_DIR ${QSCINTILLA_DIR}/Qt4Qt5/)
        message(STATUS "Found QSCINTILLA_BUILD_DIR at ${QSCINTILLA_BUILD_DIR}")
    endif()
endif()

if(NOT COPPELIASIM_INCLUDE_DIR)
    if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
        set(COPPELIASIM_INCLUDE_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/include)
    else()
        set(COPPELIASIM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../programming/include)
    endif()
endif()

add_library(coppeliaSim SHARED "")

set(WITH_GUI true CACHE BOOL "Enable gui")

target_compile_features(coppeliaSim PRIVATE cxx_std_17)
target_include_directories(coppeliaSim PRIVATE ${LUA_INCLUDE_DIR})
target_include_directories(coppeliaSim PRIVATE ${EIGEN_INCLUDE_DIR})

if(WITH_QSCINTILLA)
    target_include_directories(coppeliaSim PRIVATE ${QSCINTILLA_DIR}/Qt4Qt5)
    target_include_directories(coppeliaSim PRIVATE ${QSCINTILLA_DIR}/include)
    if(NOT QSCINTILLA_LIBRARY)
        if(CMAKE_BUILD_TYPE MATCHES [Dd][Ee][Bb][Uu][Gg])
            set(QSCINTILLA_LIBRARY_SUFFIX _debug)
        endif()
        set(QSCINTILLA_LIBRARY_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}qscintilla2_qt5${QSCINTILLA_LIBRARY_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX})
        if(EXISTS ${QSCINTILLA_BUILD_DIR}/release/${QSCINTILLA_LIBRARY_NAME})
            set(QSCINTILLA_LIBRARY ${QSCINTILLA_BUILD_DIR}/release/${QSCINTILLA_LIBRARY_NAME})
        elseif(EXISTS ${QSCINTILLA_BUILD_DIR}/${QSCINTILLA_LIBRARY_NAME})
            set(QSCINTILLA_LIBRARY ${QSCINTILLA_BUILD_DIR}/${QSCINTILLA_LIBRARY_NAME})
        else()
            set(QSCINTILLA_LIBRARY NOTFOUND)
        endif()
    endif()

    if(WIN32)
        # on Windows it fails to link
        # workaround is to link some moc files from the QScintilla project
        if(NOT QSCINTILLA_BUILD_DIR)
            message(FATAL_ERROR "QSCINTILLA_BUILD_DIR is not set")
        endif()
        file(TO_CMAKE_PATH ${QSCINTILLA_BUILD_DIR} QSCINTILLA_BUILD_DIR)
        message(STATUS "QSCINTILLA_BUILD_DIR: ${QSCINTILLA_BUILD_DIR}")
        if(EXISTS "${QSCINTILLA_BUILD_DIR}/debug/moc_qsciscintilla.cpp")
            target_sources(coppeliaSim PRIVATE
                ${QSCINTILLA_BUILD_DIR}/debug/moc_qsciscintilla.cpp
                ${QSCINTILLA_BUILD_DIR}/debug/moc_qsciscintillabase.cpp
            )
        elseif(EXISTS "${QSCINTILLA_BUILD_DIR}/release/moc_qsciscintilla.cpp")
            target_sources(coppeliaSim PRIVATE
                ${QSCINTILLA_BUILD_DIR}/release/moc_qsciscintilla.cpp
                ${QSCINTILLA_BUILD_DIR}/release/moc_qsciscintillabase.cpp
            )
        else()
            message(FATAL_ERROR "The specified QSCINTILLA_BUILD_DIR (${QSCINTILLA_BUILD_DIR}) does not exist or it is not valid")
        endif()
    endif()

    if(EXISTS "${QSCINTILLA_LIBRARY}")
        message(STATUS "QSCINTILLA_LIBRARY: ${QSCINTILLA_LIBRARY}")
    else()
        message(FATAL_ERROR "The specified QSCINTILLA_LIBRARY (${QSCINTILLA_LIBRARY}) does not exist")
    endif()
endif()

target_compile_definitions(coppeliaSim PRIVATE SIM_MATH_DOUBLE)
#target_compile_definitions(coppeliaSim PRIVATE TMPOPERATION) # <-- remove once we release V4.6 (i.e. V4.5 needs to support both serialization formats)

set(Qt Qt5 CACHE STRING "Qt version to use (e.g. Qt5)")
set_property(CACHE Qt PROPERTY STRINGS Qt5 Qt6)  #

if(Qt STREQUAL Qt5)
    set(USES_QGLWIDGET true CACHE BOOL "Use the old QGLWidget instead of QOpenGLWidget")
else()
    unset(USES_QGLWIDGET CACHE)
endif()

if(WITH_GUI)
    find_package(${Qt} COMPONENTS Widgets REQUIRED)
    target_compile_definitions(coppeliaSim PRIVATE SIM_WITH_GUI)
    target_compile_definitions(coppeliaSim PRIVATE QT_DISABLE_DEPRECATED_BEFORE=0)
    find_package(OpenGL REQUIRED)
    find_package(${Qt} COMPONENTS Gui OpenGL REQUIRED)
    if(Qt STREQUAL Qt6)
        find_package(${Qt} COMPONENTS OpenGLWidgets REQUIRED)
    endif()
endif()

if(USES_QGLWIDGET)
    target_compile_definitions(coppeliaSim PRIVATE USES_QGLWIDGET)
endif()

find_package(${Qt} COMPONENTS Core PrintSupport Network REQUIRED)

if(MSVC)
    target_compile_options(coppeliaSim PRIVATE /FI"simMainHeader.h")
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -fp:precise")
    set(CMAKE_CXXFLAGS_RELEASE "${CMAKE_CXXFLAGS_RELEASE} -O2")

    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -W3")
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -wd4100") # "unreferenced formal parameter"
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -wd4996") # "function or variable may be unsafe..."
    set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -wd4101") # "unreferenced local variable"

    if(MSVC_VERSION EQUAL 1900)
        target_compile_definitions(coppeliaSim PRIVATE SIM_COMPILER_STR=\"MSVC2015\")
    elseif(MSVC_VERSION GREATER 1900 AND MSVC_VERSION LESS 1920)
        target_compile_definitions(coppeliaSim PRIVATE SIM_COMPILER_STR=\"MSVC2017\")
    else()
        target_compile_definitions(coppeliaSim PRIVATE SIM_COMPILER_STR=\"MSVC2019\")
    endif()
else()
    target_compile_options(coppeliaSim PRIVATE -include simMainHeader.h)
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
        target_compile_definitions(coppeliaSim PRIVATE SIM_COMPILER_STR=\"Clang\")
        target_compile_definitions(coppeliaSim PRIVATE GL_SILENCE_DEPRECATION)
    else()
        target_compile_definitions(coppeliaSim PRIVATE SIM_COMPILER_STR=\"GCC\")
    endif()
    # XXX: why don't we simply use CMAKE_CXX_COMPILER_ID?
endif()

if(WIN32)
    target_compile_definitions(coppeliaSim PRIVATE WIN_SIM)
    target_compile_definitions(coppeliaSim PRIVATE SIM_PLATFORM=\"windows\")
elseif(APPLE)
    target_compile_definitions(coppeliaSim PRIVATE MAC_SIM)
    target_compile_definitions(coppeliaSim PRIVATE SIM_PLATFORM=\"macOS\")
elseif(UNIX)
    target_compile_definitions(coppeliaSim PRIVATE LIN_SIM)
    target_compile_definitions(coppeliaSim PRIVATE SIM_PLATFORM=\"linux\")
endif()

target_include_directories(coppeliaSim
    PRIVATE
    sourceCode
    sourceCode/communication
    sourceCode/communication/serialPort
    sourceCode/drawingObjects
    sourceCode/platform
    sourceCode/collections
    sourceCode/collisions
    sourceCode/distances
    sourceCode/pathPlanning_old
    sourceCode/sceneObjects
    sourceCode/sceneObjects/related
    sourceCode/sceneObjects/graphObjectRelated
    sourceCode/sceneObjects/proximitySensorObjectRelated
    sourceCode/sceneObjects/shapeObjectRelated
    sourceCode/mainContainers
    sourceCode/mainContainers/sceneContainers
    sourceCode/mainContainers/applicationContainers
    sourceCode/scripting
    sourceCode/pagesAndViews
    sourceCode/textures
    sourceCode/serialization
    sourceCode/strings
    sourceCode/interfaces
    sourceCode/interfaces/interfaceStack
    sourceCode/menusAndSimilar
    sourceCode/variousFunctions
    sourceCode/geometricAlgorithms
    sourceCode/various
    sourceCode/visual
    sourceCode/displ
    sourceCode/utils
    sourceCode/undoRedo
    sourceCode/backwardCompatibility/pathPlanning
    sourceCode/backwardCompatibility/collisions
    sourceCode/backwardCompatibility/distances
    sourceCode/backwardCompatibility/kinematics
    sourceCode/backwardCompatibility/tubes
    sourceCode/backwardCompatibility/wireless
    sourceCode/backwardCompatibility/customUserInterfaces
    sourceCode/backwardCompatibility/draw
    sourceCode/backwardCompatibility/geom
    sourceCode/backwardCompatibility/interfaces
    sourceCode/backwardCompatibility/sceneContainers
    sourceCode/backwardCompatibility/sceneObjects
    sourceCode/backwardCompatibility/scripting
    sourceCode/backwardCompatibility/scripting/customScriptFuncAndVar
    sourceCode/backwardCompatibility/utils
    sourceCode/backwardCompatibility/various
    ${COPPELIASIM_INCLUDE_DIR}
)



if(WITH_GUI)
    target_include_directories(coppeliaSim PRIVATE
        sourceCode/gui/dialogs
        sourceCode/gui/dialogs/other
        sourceCode/gui/platform
        sourceCode/gui/mainContainers
        sourceCode/gui/mainContainers/sceneContainers
        sourceCode/gui/mainContainers/applicationContainers
        sourceCode/gui/editModes
        sourceCode/gui/modelBrowser
        sourceCode/gui/sceneHierarchy
        sourceCode/gui/selectors
        sourceCode/gui/menusAndSimilar
        sourceCode/gui/various
        sourceCode/gui/libs
        sourceCode/gui/visual
        sourceCode/gui/sceneObjects
        sourceCode/gui/sceneObjects/visionSensorObjectRelated
        sourceCode/gui/rendering
        sourceCode/gui/communication/wireless
    )
endif()

if(WITH_GUI)
    target_sources(coppeliaSim PRIVATE
        targaFiles.qrc
        toolbarFiles.qrc
        variousImageFiles.qrc
        iconFiles.qrc
        imageFiles.qrc
        qdarkstyle/style.qrc
    )
endif()


if(WITH_GUI)
    target_sources(coppeliaSim PRIVATE
        sourceCode/gui/rendering/rendering.cpp
        sourceCode/gui/rendering/cameraRendering.cpp
        sourceCode/gui/rendering/visionSensorRendering.cpp
        sourceCode/gui/rendering/forceSensorRendering.cpp
        sourceCode/gui/rendering/proximitySensorRendering.cpp
        sourceCode/gui/rendering/dummyRendering.cpp
        sourceCode/gui/rendering/scriptRendering.cpp
        sourceCode/gui/rendering/lightRendering.cpp
        sourceCode/gui/rendering/pointCloudRendering.cpp
        sourceCode/gui/rendering/octreeRendering.cpp
        sourceCode/gui/rendering/jointRendering.cpp
        sourceCode/gui/rendering/graphRendering.cpp
        sourceCode/gui/rendering/millRendering.cpp
        sourceCode/gui/rendering/mirrorRendering.cpp
        sourceCode/gui/rendering/pathRendering.cpp
        sourceCode/gui/rendering/shapeRendering.cpp
        sourceCode/gui/rendering/ghostRendering.cpp
        sourceCode/gui/rendering/drawingObjectRendering.cpp
        sourceCode/gui/rendering/ptCloudRendering_old.cpp
        sourceCode/gui/rendering/collisionContourRendering.cpp
        sourceCode/gui/rendering/distanceRendering.cpp
        sourceCode/gui/rendering/bannerRendering.cpp
        sourceCode/gui/rendering/thumbnailRendering.cpp
        sourceCode/gui/rendering/pathPlanningTaskRendering_old.cpp
        sourceCode/gui/rendering/broadcastDataVisualRendering.cpp
        sourceCode/gui/rendering/dynamicsRendering.cpp
        sourceCode/gui/rendering/environmentRendering.cpp
        sourceCode/gui/rendering/pageRendering.cpp
        sourceCode/gui/rendering/viewRendering.cpp

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
        sourceCode/gui/dialogs/qdlgoldscripts.cpp
        sourceCode/gui/dialogs/qdlgscaling.cpp
        sourceCode/gui/dialogs/qdlgcameras.cpp
        sourceCode/gui/dialogs/qdlglights.cpp
        sourceCode/gui/dialogs/qdlgmirrors.cpp
        sourceCode/gui/dialogs/qdlgdummies.cpp
        sourceCode/gui/dialogs/qdlgscripts.cpp
        sourceCode/gui/dialogs/qdlgoctrees.cpp
        sourceCode/gui/dialogs/qdlgpointclouds.cpp
        sourceCode/gui/dialogs/qdlguserparameters.cpp
        sourceCode/gui/dialogs/qdlgforcesensors.cpp
        sourceCode/gui/dialogs/qdlgprimitives.cpp
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
        sourceCode/gui/dialogs/qdlgcalcdialogcontainer_OLD.cpp
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

        sourceCode/gui/communication/wireless/broadcastDataVisual.cpp

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

        sourceCode/gui/modelBrowser/modelFolderWidget.cpp
        sourceCode/gui/modelBrowser/modelListWidget.cpp
        sourceCode/gui/modelBrowser/modelListWidgetBase.cpp

        sourceCode/gui/selectors/pageSelector.cpp
        sourceCode/gui/selectors/viewSelector.cpp

        sourceCode/gui/menusAndSimilar/toolBarCommand.cpp
        sourceCode/gui/menusAndSimilar/mainWindow.cpp
        sourceCode/gui/menusAndSimilar/statusBar.cpp
        sourceCode/gui/menusAndSimilar/helpMenu.cpp

        sourceCode/gui/various/guiApp.cpp
        sourceCode/gui/various/simRecorder.cpp
        sourceCode/gui/various/engineProperties.cpp
        sourceCode/gui/various/uiThread.cpp
        sourceCode/gui/various/simAndUiThreadSync.cpp

        sourceCode/gui/libs/auxLibVideo.cpp
    )
endif()

target_sources(coppeliaSim PRIVATE
    ${COPPELIASIM_INCLUDE_DIR}/simMath/mathFuncs.cpp
    ${COPPELIASIM_INCLUDE_DIR}/simMath/mXnMatrix.cpp
    ${COPPELIASIM_INCLUDE_DIR}/simMath/7Vector.cpp
    ${COPPELIASIM_INCLUDE_DIR}/simMath/4X4Matrix.cpp
    ${COPPELIASIM_INCLUDE_DIR}/simMath/4Vector.cpp
    ${COPPELIASIM_INCLUDE_DIR}/simMath/3X3Matrix.cpp
    ${COPPELIASIM_INCLUDE_DIR}/simMath/3Vector.cpp

    sourceCode/backwardCompatibility/kinematics/ikGroup_old.cpp
    sourceCode/backwardCompatibility/kinematics/_ikGroup_old.cpp
    sourceCode/backwardCompatibility/kinematics/ikElement_old.cpp
    sourceCode/backwardCompatibility/kinematics/ikGroupContainer.cpp
    sourceCode/backwardCompatibility/kinematics/_ikGroupContainer_.cpp
    sourceCode/backwardCompatibility/pathPlanning/pathPlanning_old.cpp
    sourceCode/backwardCompatibility/pathPlanning/holonomicPathPlanning_old.cpp
    sourceCode/backwardCompatibility/pathPlanning/holonomicPathNode_old.cpp
    sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathPlanning_old.cpp
    sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathNode_old.cpp
    sourceCode/backwardCompatibility/pathPlanning/pathPlanningTask_old.cpp
    sourceCode/backwardCompatibility/tubes/commTube.cpp
    sourceCode/backwardCompatibility/wireless/broadcastDataContainer.cpp
    sourceCode/backwardCompatibility/wireless/broadcastData.cpp
    sourceCode/backwardCompatibility/customUserInterfaces/buttonBlock.cpp
    sourceCode/backwardCompatibility/customUserInterfaces/softButton.cpp
    sourceCode/backwardCompatibility/draw/bannerObject.cpp
    sourceCode/backwardCompatibility/draw/ptCloud_old.cpp
    sourceCode/backwardCompatibility/geom/linMotionRoutines.cpp
    sourceCode/backwardCompatibility/interfaces/pathPlanningInterface.cpp
    sourceCode/backwardCompatibility/sceneContainers/bannerContainer.cpp
    sourceCode/backwardCompatibility/sceneContainers/registeredPathPlanningTasks.cpp
    sourceCode/backwardCompatibility/sceneContainers/distanceObjectContainer_old.cpp
    sourceCode/backwardCompatibility/sceneContainers/collisionObjectContainer_old.cpp
    sourceCode/backwardCompatibility/sceneContainers/customData_old.cpp
    sourceCode/backwardCompatibility/sceneContainers/commTubeContainer.cpp
    sourceCode/backwardCompatibility/sceneContainers/ghostObjectContainer.cpp
    sourceCode/backwardCompatibility/sceneContainers/pointCloudContainer_old.cpp
    sourceCode/backwardCompatibility/sceneContainers/buttonBlockContainer.cpp
    sourceCode/backwardCompatibility/sceneObjects/mill.cpp
    sourceCode/backwardCompatibility/sceneObjects/path_old.cpp
    sourceCode/backwardCompatibility/collisions/collisionObject_old.cpp
    sourceCode/backwardCompatibility/distances/distanceObject_old.cpp
    sourceCode/backwardCompatibility/sceneObjects/bezierPathPoint_old.cpp
    sourceCode/backwardCompatibility/sceneObjects/simplePathPoint_old.cpp
    sourceCode/backwardCompatibility/sceneObjects/pathPoint_old.cpp
    sourceCode/backwardCompatibility/sceneObjects/pathCont_old.cpp
    sourceCode/backwardCompatibility/sceneObjects/simpleFilter.cpp
    sourceCode/backwardCompatibility/sceneObjects/composedFilter.cpp
    sourceCode/backwardCompatibility/sceneObjects/graphingRoutines_old.cpp
    sourceCode/backwardCompatibility/sceneObjects/graphDataComb_old.cpp
    sourceCode/backwardCompatibility/sceneObjects/graphData_old.cpp
    sourceCode/backwardCompatibility/sceneObjects/staticGraphCurve_old.cpp
    sourceCode/backwardCompatibility/scripting/userParameters.cpp
    sourceCode/backwardCompatibility/scripting/customScriptFuncAndVar/scriptCustomFuncAndVarContainer.cpp
    sourceCode/backwardCompatibility/scripting/customScriptFuncAndVar/scriptCustomFunction.cpp
    sourceCode/backwardCompatibility/scripting/customScriptFuncAndVar/scriptCustomVariable.cpp
    sourceCode/backwardCompatibility/utils/threadPool_old.cpp
    sourceCode/backwardCompatibility/various/memorizedConf_old.cpp
    sourceCode/backwardCompatibility/various/ghostObject.cpp

    sourceCode/drawingObjects/drawingObject.cpp

    sourceCode/platform/vVarious.cpp
    sourceCode/platform/vThread.cpp
    sourceCode/platform/vMutex.cpp
    sourceCode/platform/vFileFinder.cpp
    sourceCode/platform/vFile.cpp
    sourceCode/platform/vDateTime.cpp
    sourceCode/platform/vArchive.cpp

    sourceCode/collections/collectionElement.cpp
    sourceCode/collections/collection.cpp

    sourceCode/collisions/collisionRoutines.cpp
    sourceCode/distances/distanceRoutines.cpp

    sourceCode/sceneObjects/related/sceneObject.cpp
    sourceCode/sceneObjects/related/convexVolume.cpp
    sourceCode/sceneObjects/related/viewableBase.cpp

    sourceCode/sceneObjects/jointObject.cpp
    sourceCode/sceneObjects/camera.cpp
    sourceCode/sceneObjects/dummy.cpp
    sourceCode/sceneObjects/script.cpp
    sourceCode/sceneObjects/octree.cpp
    sourceCode/sceneObjects/pointCloud.cpp
    sourceCode/sceneObjects/forceSensor.cpp
    sourceCode/sceneObjects/graph.cpp
    sourceCode/sceneObjects/light.cpp
    sourceCode/sceneObjects/mirror.cpp
    sourceCode/sceneObjects/proximitySensor.cpp
    sourceCode/sceneObjects/shape.cpp
    sourceCode/sceneObjects/visionSensor.cpp

    sourceCode/sceneObjects/graphObjectRelated/graphDataStream.cpp
    sourceCode/sceneObjects/graphObjectRelated/graphCurve.cpp

    sourceCode/sceneObjects/proximitySensorObjectRelated/proxSensorRoutine.cpp

    sourceCode/sceneObjects/shapeObjectRelated/mesh.cpp
    sourceCode/sceneObjects/shapeObjectRelated/meshWrapper.cpp
    sourceCode/sceneObjects/shapeObjectRelated/volInt.cpp

    sourceCode/communication/serialPort/serialPort.cpp
    sourceCode/communication/serialPort/serial_win_mac_linux.cpp
    sourceCode/communication/serialPort/serialPortWin.cpp

    sourceCode/mainContainers/worldContainer.cpp
    sourceCode/mainContainers/world.cpp
    
    sourceCode/mainContainers/sceneContainers/drawingContainer.cpp
    sourceCode/mainContainers/sceneContainers/textureContainer.cpp
    sourceCode/mainContainers/sceneContainers/simulation.cpp
    sourceCode/mainContainers/sceneContainers/signalContainer.cpp
    sourceCode/mainContainers/sceneContainers/collectionContainer.cpp
    sourceCode/mainContainers/sceneContainers/outsideCommandQueue.cpp
    sourceCode/mainContainers/sceneContainers/sceneObjectContainer.cpp
    sourceCode/mainContainers/sceneContainers/mainSettings.cpp
    sourceCode/mainContainers/sceneContainers/embeddedScriptContainer.cpp
    sourceCode/mainContainers/sceneContainers/environment.cpp
    sourceCode/mainContainers/sceneContainers/dynamicsContainer.cpp
    sourceCode/mainContainers/sceneContainers/customData.cpp
    sourceCode/mainContainers/sceneContainers/cacheCont.cpp
    sourceCode/mainContainers/sceneContainers/apiErrors.cpp
    sourceCode/mainContainers/sceneContainers/undoBufferCont.cpp

    sourceCode/mainContainers/applicationContainers/copyBuffer.cpp
    sourceCode/mainContainers/applicationContainers/persistentDataContainer.cpp
    sourceCode/mainContainers/applicationContainers/simulatorMessageQueue.cpp
    sourceCode/mainContainers/applicationContainers/calculationInfo.cpp
    sourceCode/mainContainers/applicationContainers/interfaceStackContainer.cpp
    sourceCode/mainContainers/applicationContainers/addOnScriptContainer.cpp
    sourceCode/mainContainers/applicationContainers/pluginContainer.cpp
    sourceCode/mainContainers/applicationContainers/plugin.cpp
    sourceCode/mainContainers/applicationContainers/pluginCallbackContainer.cpp
    sourceCode/mainContainers/applicationContainers/pluginVariableContainer.cpp
    sourceCode/mainContainers/applicationContainers/codeEditorInfos.cpp
    sourceCode/mainContainers/applicationContainers/codeEditorFunctions.cpp
    sourceCode/mainContainers/applicationContainers/codeEditorVariables.cpp
    sourceCode/mainContainers/applicationContainers/serialPortContainer.cpp

    sourceCode/scripting/scriptObject.cpp
    sourceCode/scripting/outsideCommandQueueForScript.cpp
    sourceCode/scripting/luaWrapper.cpp

    sourceCode/pagesAndViews/pageContainer.cpp
    sourceCode/pagesAndViews/sPage.cpp
    sourceCode/pagesAndViews/sView.cpp

    sourceCode/textures/textureObject.cpp
    sourceCode/textures/imgLoaderSaver.cpp
    sourceCode/textures/tGAFormat.cpp
    sourceCode/textures/stb_image.c
    sourceCode/textures/textureProperty.cpp

    sourceCode/serialization/ser.cpp
    sourceCode/serialization/huffman.c

    sourceCode/interfaces/sim.cpp
    sourceCode/interfaces/simInternal.cpp
    sourceCode/interfaces/luaScriptFunctions.cpp

    sourceCode/interfaces/interfaceStack/interfaceStack.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackObject.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackNull.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackBool.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackNumber.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackInteger.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackString.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackTable.cpp

    sourceCode/menusAndSimilar/addOperations.cpp
    sourceCode/menusAndSimilar/fileOperations.cpp
    sourceCode/menusAndSimilar/moduleMenuItemContainer.cpp
    sourceCode/menusAndSimilar/moduleMenuItem.cpp

    sourceCode/variousFunctions/sceneObjectOperations.cpp

    sourceCode/geometricAlgorithms/meshRoutines.cpp
    sourceCode/geometricAlgorithms/meshManip.cpp
    sourceCode/geometricAlgorithms/edgeElement.cpp
    sourceCode/geometricAlgorithms/algos.cpp

    sourceCode/various/userSettings.cpp
    sourceCode/various/folderSystem.cpp
    sourceCode/various/app.cpp
    sourceCode/various/simThread.cpp
    sourceCode/various/dynMaterialObject.cpp
    sourceCode/various/sigHandler.cpp

    sourceCode/undoRedo/undoBufferArrays.cpp
    sourceCode/undoRedo/undoBuffer.cpp
    sourceCode/undoRedo/undoBufferCameras.cpp

    sourceCode/displ/colorObject.cpp

    sourceCode/visual/thumbnail.cpp

    sourceCode/utils/utils.cpp
    sourceCode/utils/tt.cpp
    sourceCode/utils/confReaderAndWriter.cpp
    sourceCode/utils/base64.cpp
    sourceCode/utils/cbor.cpp
    sourceCode/utils/annJson.cpp
    sourceCode/utils/sha256.cpp
)

if(NOT PLUS)
    target_sources(coppeliaSim PRIVATE
        sourceCode/interfaces/simFlavor.cpp
        sourceCode/interfaces/gm.cpp
    )
endif()

target_sources(coppeliaSim PRIVATE
    sourceCode/platform/vSimUiMutex.cpp
    sourceCode/platform/wThread.cpp

    sourceCode/various/simQApp.cpp
)

if(WITH_GUI)
    target_sources(coppeliaSim PRIVATE
        sourceCode/gui/sceneObjects/visionSensorObjectRelated/offscreenGlContext.cpp
        sourceCode/gui/sceneObjects/visionSensorObjectRelated/frameBufferObject.cpp
        sourceCode/gui/sceneObjects/visionSensorObjectRelated/visionSensorGlStuff.cpp

        sourceCode/gui/visual/oGL.cpp
        sourceCode/gui/visual/oglExt.cpp
        sourceCode/gui/visual/glBufferObjects.cpp
    )
endif()


target_link_libraries(coppeliaSim Boost::boost)
target_link_libraries(coppeliaSim tinyxml2::tinyxml2)
target_link_libraries(coppeliaSim ${LUA_LIBRARIES})
if(WITH_QSCINTILLA)
    target_link_libraries(coppeliaSim ${QSCINTILLA_LIBRARY})
endif()

target_link_libraries(coppeliaSim Qt::Core Qt::PrintSupport Qt::Network)
if(WITH_GUI)
    target_link_libraries(coppeliaSim Qt::Gui)
    target_link_libraries(coppeliaSim Qt::Widgets)
    target_link_libraries(coppeliaSim OpenGL::GL)
    target_link_libraries(coppeliaSim Qt::OpenGL)
    if(Qt STREQUAL Qt6)
        target_link_libraries(coppeliaSim Qt::OpenGLWidgets)
    endif()
endif()
if(WIN32)
    target_link_libraries(coppeliaSim
        winmm
        Shell32
        opengl32
        user32
        Dbghelp
    )
endif()

if(PLUS)
    add_subdirectory(${PLUS} ${CMAKE_CURRENT_BINARY_DIR}/plus)
    target_include_directories(coppeliaSim PRIVATE ${PLUS}/sourceCode)
endif()

if(WITH_QSCINTILLA AND APPLE)
    get_filename_component(QSCINTILLA_LIB_NAME ${QSCINTILLA_LIBRARY} NAME)
    add_custom_command(TARGET coppeliaSim POST_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change ${QSCINTILLA_LIB_NAME} @executable_path/../Frameworks/${QSCINTILLA_LIB_NAME} $<TARGET_FILE:coppeliaSim>)
endif()

if(NOT WITH_GUI)
    set_target_properties(coppeliaSim PROPERTIES OUTPUT_NAME "coppeliaSimHeadless")
endif()

if(INSTALL_DIR)
    install(TARGETS coppeliaSim DESTINATION "${INSTALL_DIR}")
endif()
