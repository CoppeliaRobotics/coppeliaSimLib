cmake_minimum_required(VERSION 3.9.0)
project(coppeliaSim)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_MACOSX_RPATH 1)

set(CMAKE_AUTOUIC_SEARCH_PATHS ui)
set(WITH_QT true CACHE BOOL "Enable Qt")
set(INSTALL_DIR "" CACHE PATH "If specified, it will be used as install destination")
if(INSTALL_DIR)
    if(NOT EXISTS "${INSTALL_DIR}")
        message(FATAL_ERROR "'${INSTALL_DIR}' does not exist")
    elseif(NOT IS_DIRECTORY "${INSTALL_DIR}")
        message(FATAL_ERROR "'${INSTALL_DIR}' is not a directory")
    endif()
    message(STATUS "INSTALL_DIR: ${INSTALL_DIR}")
endif()

if(NOT LUA_INCLUDE_DIR OR NOT LUA_LIBRARIES)
    find_package(Lua 5.3 EXACT REQUIRED)
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

if(NOT COPPELIASIM_PROGRAMMING_DIR)
    set(COPPELIASIM_PROGRAMMING_DIR ../programming)
endif()

add_library(coppeliaSim SHARED "")

if(WITH_QT)
    set(WITH_GUI true CACHE BOOL "Enable gui")
    set(WITH_OPENGL true CACHE BOOL "Enable OpenGL")
    set(WITH_SERIAL false)
    target_compile_definitions(coppeliaSim PRIVATE SIM_WITH_QT)
else()
    unset(WITH_GUI CACHE)
    unset(WITH_OPENGL CACHE)
    unset(WITH_SERIAL CACHE)
    set(WITH_GUI false)
    set(WITH_OPENGL false)
    set(WITH_SERIAL false)
endif()

target_compile_features(coppeliaSim PRIVATE cxx_std_17)
target_include_directories(coppeliaSim PRIVATE ${LUA_INCLUDE_DIR})
target_include_directories(coppeliaSim PRIVATE ${EIGEN_INCLUDE_DIR})
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
    if(NOT EXISTS "${QSCINTILLA_BUILD_DIR}/release/moc_qsciscintilla.cpp")
        message(FATAL_ERROR "The specified QSCINTILLA_BUILD_DIR (${QSCINTILLA_BUILD_DIR}) does not exist or it is not valid")
    endif()
    message(STATUS "QSCINTILLA_BUILD_DIR: ${QSCINTILLA_BUILD_DIR}")
    target_sources(coppeliaSim PRIVATE
        ${QSCINTILLA_BUILD_DIR}/release/moc_qsciscintilla.cpp
        ${QSCINTILLA_BUILD_DIR}/release/moc_qsciscintillabase.cpp
    )
endif()

if(EXISTS "${QSCINTILLA_LIBRARY}")
    message(STATUS "QSCINTILLA_LIBRARY: ${QSCINTILLA_LIBRARY}")
else()
    message(FATAL_ERROR "The specified QSCINTILLA_LIBRARY (${QSCINTILLA_LIBRARY}) does not exist")
endif()

target_compile_definitions(coppeliaSim PRIVATE SIM_LIB)

if(WITH_GUI)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    target_compile_definitions(coppeliaSim PRIVATE SIM_WITH_GUI)
    target_compile_definitions(coppeliaSim PRIVATE QT_DISABLE_DEPRECATED_BEFORE=0)
endif()

if(WITH_OPENGL)
    find_package(OpenGL REQUIRED)
    find_package(Qt5 COMPONENTS Gui OpenGL REQUIRED)
    target_compile_definitions(coppeliaSim PRIVATE SIM_WITH_OPENGL)
endif()

if(WITH_SERIAL)
    find_package(Qt5 COMPONENTS SerialPort REQUIRED)
    target_compile_definitions(coppeliaSim PRIVATE SIM_WITH_SERIAL)
endif()

if(WITH_QT)
    find_package(Qt5 COMPONENTS Core PrintSupport Network REQUIRED)
else()
    target_compile_definitions(coppeliaSim PRIVATE SIM_WITHOUT_QT_AT_ALL)
endif()

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
elseif(APPLE)
    target_compile_definitions(coppeliaSim PRIVATE MAC_SIM)
elseif(UNIX)
    target_compile_definitions(coppeliaSim PRIVATE LIN_SIM)
endif()

target_include_directories(coppeliaSim
    PRIVATE
    sourceCode
    sourceCode/shared
    sourceCode/shared/backwardCompatibility/kinematics
    sourceCode/communication
    sourceCode/communication/tubes
    sourceCode/communication/wireless
    sourceCode/drawingObjects
    sourceCode/platform
    sourceCode/collections
    sourceCode/collisions
    sourceCode/distances
    sourceCode/pathPlanning_old
    sourceCode/sceneObjects
    sourceCode/shared/sceneObjects
    sourceCode/sceneObjects/related
    sourceCode/shared/sceneObjects/related
    sourceCode/sceneObjects/graphObjectRelated
    sourceCode/sceneObjects/pathObjectRelated
    sourceCode/sceneObjects/proximitySensorObjectRelated
    sourceCode/sceneObjects/shapeObjectRelated
    sourceCode/sceneObjects/visionSensorObjectRelated
    sourceCode/mainContainers
    sourceCode/shared/mainContainers
    sourceCode/mainContainers/sceneContainers
    sourceCode/shared/mainContainers/sceneContainers
    sourceCode/mainContainers/applicationContainers
    sourceCode/scripting
    sourceCode/scripting/customScriptFuncAndVar
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
    sourceCode/shared/various
    sourceCode/libsAndPlugins
    sourceCode/visual
    sourceCode/displ
    sourceCode/utils
    sourceCode/backwardCompatibility/pathPlanning
    sourceCode/backwardCompatibility/collisions
    sourceCode/backwardCompatibility/distances
    sourceCode/backwardCompatibility/kinematics
    sourceCode/customUserInterfaces
    sourceCode/undoRedo
    sourceCode/rendering
    ${COPPELIASIM_PROGRAMMING_DIR}/include
    ${COPPELIASIM_PROGRAMMING_DIR}/simMath
)


if(WITH_SERIAL)
    target_include_directories(coppeliaSim PRIVATE sourceCode/communication/serialPort)
endif()

if(WITH_GUI)
    target_include_directories(coppeliaSim PRIVATE
        sourceCode/gui/qtPropertyBrowserRelated
        sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles
        sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserRelatedFiles
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

        sourceCode/gui/modelBrowser/modelFolderWidget.cpp
        sourceCode/gui/modelBrowser/modelListWidget.cpp
        sourceCode/gui/modelBrowser/modelListWidgetBase.cpp

        sourceCode/gui/selectors/pageSelector.cpp
        sourceCode/gui/selectors/viewSelector.cpp

        sourceCode/gui/menusAndSimilar/toolBarCommand.cpp
        sourceCode/gui/menusAndSimilar/mainWindow.cpp
        sourceCode/gui/menusAndSimilar/statusBar.cpp
        sourceCode/gui/menusAndSimilar/helpMenu.cpp

        sourceCode/gui/various/simRecorder.cpp

        sourceCode/gui/libs/auxLibVideo.cpp
    )
endif()

target_sources(coppeliaSim PRIVATE
    ${COPPELIASIM_PROGRAMMING_DIR}/simMath/Vector.cpp
    ${COPPELIASIM_PROGRAMMING_DIR}/simMath/MyMath.cpp
    ${COPPELIASIM_PROGRAMMING_DIR}/simMath/MMatrix.cpp
    ${COPPELIASIM_PROGRAMMING_DIR}/simMath/7Vector.cpp
    ${COPPELIASIM_PROGRAMMING_DIR}/simMath/6X6Matrix.cpp
    ${COPPELIASIM_PROGRAMMING_DIR}/simMath/6Vector.cpp
    ${COPPELIASIM_PROGRAMMING_DIR}/simMath/4X4Matrix.cpp
    ${COPPELIASIM_PROGRAMMING_DIR}/simMath/4X4FullMatrix.cpp
    ${COPPELIASIM_PROGRAMMING_DIR}/simMath/4Vector.cpp
    ${COPPELIASIM_PROGRAMMING_DIR}/simMath/3X3Matrix.cpp
    ${COPPELIASIM_PROGRAMMING_DIR}/simMath/3Vector.cpp

    sourceCode/backwardCompatibility/kinematics/ikGroup_old.cpp
    sourceCode/shared/backwardCompatibility/kinematics/_ikGroup_old.cpp
    sourceCode/backwardCompatibility/kinematics/ikElement_old.cpp
    sourceCode/shared/backwardCompatibility/kinematics/_ikElement_old.cpp

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
    sourceCode/collections/collection.cpp

    sourceCode/backwardCompatibility/collisions/collisionObject_old.cpp
    sourceCode/collisions/collisionRoutines.cpp

    sourceCode/backwardCompatibility/distances/distanceObject_old.cpp
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
    sourceCode/sceneObjects/path_old.cpp
    sourceCode/sceneObjects/proximitySensor.cpp
    sourceCode/sceneObjects/shape.cpp
    sourceCode/sceneObjects/visionSensor.cpp

    sourceCode/sceneObjects/graphObjectRelated/graphDataStream.cpp
    sourceCode/sceneObjects/graphObjectRelated/graphCurve.cpp
    sourceCode/sceneObjects/graphObjectRelated/graphingRoutines_old.cpp
    sourceCode/sceneObjects/graphObjectRelated/graphDataComb_old.cpp
    sourceCode/sceneObjects/graphObjectRelated/graphData_old.cpp
    sourceCode/sceneObjects/graphObjectRelated/staticGraphCurve_old.cpp

    sourceCode/sceneObjects/pathObjectRelated/bezierPathPoint_old.cpp
    sourceCode/sceneObjects/pathObjectRelated/simplePathPoint_old.cpp
    sourceCode/sceneObjects/pathObjectRelated/pathPoint_old.cpp
    sourceCode/sceneObjects/pathObjectRelated/pathCont_old.cpp

    sourceCode/sceneObjects/proximitySensorObjectRelated/proxSensorRoutine.cpp

    sourceCode/sceneObjects/shapeObjectRelated/mesh.cpp
    sourceCode/sceneObjects/shapeObjectRelated/meshWrapper.cpp
    sourceCode/sceneObjects/shapeObjectRelated/volInt.cpp


    sourceCode/backwardCompatibility/pathPlanning/pathPlanning_old.cpp
    sourceCode/backwardCompatibility/pathPlanning/holonomicPathPlanning_old.cpp
    sourceCode/backwardCompatibility/pathPlanning/holonomicPathNode_old.cpp
    sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathPlanning_old.cpp
    sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathNode_old.cpp

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
    sourceCode/mainContainers/sceneContainers/distanceObjectContainer_old.cpp
    sourceCode/mainContainers/sceneContainers/collisionObjectContainer_old.cpp
    sourceCode/mainContainers/sceneContainers/outsideCommandQueue.cpp
    sourceCode/mainContainers/sceneContainers/sceneObjectContainer.cpp
    sourceCode/shared/mainContainers/sceneContainers/_sceneObjectContainer_.cpp
    sourceCode/mainContainers/sceneContainers/mainSettings.cpp
    sourceCode/mainContainers/sceneContainers/embeddedScriptContainer.cpp
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

    sourceCode/scripting/userParameters.cpp
    sourceCode/scripting/scriptObject.cpp
    sourceCode/scripting/outsideCommandQueueForScript.cpp
    sourceCode/scripting/luaWrapper.cpp

    sourceCode/scripting/customScriptFuncAndVar/scriptCustomFuncAndVarContainer.cpp
    sourceCode/scripting/customScriptFuncAndVar/scriptCustomFunction.cpp
    sourceCode/scripting/customScriptFuncAndVar/scriptCustomVariable.cpp

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
    sourceCode/interfaces/interfaceStack/interfaceStackInteger.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackString.cpp
    sourceCode/interfaces/interfaceStack/interfaceStackTable.cpp

    sourceCode/menusAndSimilar/addOperations.cpp
    sourceCode/menusAndSimilar/fileOperations.cpp
    sourceCode/menusAndSimilar/moduleMenuItemContainer.cpp
    sourceCode/menusAndSimilar/moduleMenuItem.cpp

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

    sourceCode/visual/thumbnail.cpp

    sourceCode/utils/threadPool_old.cpp
    sourceCode/utils/ttUtil.cpp
    sourceCode/utils/tt.cpp
    sourceCode/utils/confReaderAndWriter.cpp
    sourceCode/utils/base64.cpp
    sourceCode/utils/cbor.cpp

    sourceCode/customUserInterfaces/buttonBlock.cpp
    sourceCode/customUserInterfaces/softButton.cpp
)

if(WITH_QT)
    target_sources(coppeliaSim PRIVATE
        sourceCode/platform/vSimUiMutex.cpp
        sourceCode/platform/wThread.cpp

        sourceCode/various/simAndUiThreadSync.cpp
        sourceCode/various/simQApp.cpp
    )
endif()

if(WITH_SERIAL)
    target_sources(coppeliaSim PRIVATE
        sourceCode/mainContainers/applicationContainers/serialPortContainer.cpp

        sourceCode/communication/serialPort/serialPort.cpp
        sourceCode/communication/serialPort/serial_win_mac_linux.cpp
        sourceCode/communication/serialPort/serialPortWin.cpp
    )
endif()

if(WITH_OPENGL)
    target_sources(coppeliaSim PRIVATE
        sourceCode/sceneObjects/visionSensorObjectRelated/offscreenGlContext.cpp
        sourceCode/sceneObjects/visionSensorObjectRelated/frameBufferObject.cpp
        sourceCode/sceneObjects/visionSensorObjectRelated/visionSensorGlStuff.cpp

        sourceCode/visual/oGL.cpp
        sourceCode/visual/oglExt.cpp
        sourceCode/visual/glShader.cpp
        sourceCode/visual/glBufferObjects.cpp
    )
endif()


target_link_libraries(coppeliaSim Boost::boost)
target_link_libraries(coppeliaSim ${LUA_LIBRARIES})
target_link_libraries(coppeliaSim ${QSCINTILLA_LIBRARY})

if(WITH_QT)
    target_link_libraries(coppeliaSim Qt5::Core Qt5::PrintSupport Qt5::Network)
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
        winmm
        Shell32
        opengl32
        user32
        Dbghelp
    )
endif()

if(PLUS)
    add_subdirectory(${PLUS} ${CMAKE_CURRENT_BINARY_DIR}/plus)
endif()

if(APPLE)
    get_filename_component(QSCINTILLA_LIB_NAME ${QSCINTILLA_LIBRARY} NAME)
    add_custom_command(TARGET coppeliaSim POST_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change ${QSCINTILLA_LIB_NAME} @executable_path/../Frameworks/${QSCINTILLA_LIB_NAME} $<TARGET_FILE:coppeliaSim>)
endif()

if(INSTALL_DIR)
    install(TARGETS coppeliaSim DESTINATION "${INSTALL_DIR}")
endif()
