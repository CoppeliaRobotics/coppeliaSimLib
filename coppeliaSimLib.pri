TARGET = coppeliaSim
TEMPLATE = lib
DEFINES += SIM_MATH_DOUBLE # math and vector classes
#DEFINES += TMPOPERATION # <-- remove once we release V4.6 (i.e. V4.5 needs to support both serialization formats). Same for CMakeLists.txt
#DEFINES += HAS_PHYSX

CONFIG += USES_QGLWIDGET
USES_QGLWIDGET {
    DEFINES += USES_QGLWIDGET
} else {
}

CONFIG += shared plugin debug_and_release

!HEADLESS {
   CONFIG += WITH_GUI
}

CONFIG(debug,debug|release) {
    CONFIG += force_debug_info
}

QT += printsupport
QT += network

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
#commented on 16.06.2023 DEFINES += LUA_STACK_COMPATIBILITY_MODE # 06.11.2020, will avoid using Lua INTEGER values at interfaces (using DOUBLE type instead)

WITH_GUI {
    QT      += widgets
    QT      += opengl
    #QT      += 3dcore 3drender 3dinput 3dextras
    DEFINES += SIM_WITH_GUI
    RESOURCES += $$PWD/targaFiles.qrc
    RESOURCES += $$PWD/toolbarFiles.qrc
    RESOURCES += $$PWD/variousImageFiles.qrc
    RESOURCES += $$PWD/iconFiles.qrc
    RESOURCES += $$PWD/imageFiles.qrc
    RESOURCES += $$PWD/qdarkstyle/style.qrc
} else {
    QT -= gui
}

*-msvc* {
    QMAKE_CXXFLAGS += /std:c++17
    QMAKE_CFLAGS += -O2
    QMAKE_CFLAGS += -fp:precise
    QMAKE_CXXFLAGS += -O2
    QMAKE_CXXFLAGS += -fp:precise
    QMAKE_CXXFLAGS += -we4715 # "error when no return value"

    QMAKE_CFLAGS_WARN_ON = -W3
    QMAKE_CFLAGS_WARN_ON += -wd4100 # "unreferenced formal parameter"
    QMAKE_CFLAGS_WARN_ON += -wd4996 # "function or variable may be unsafe..."
    QMAKE_CFLAGS_WARN_ON += -wd4101 # "unreferenced local variable"

    QMAKE_CXXFLAGS_WARN_ON = -W3
    QMAKE_CXXFLAGS_WARN_ON += -wd4100 # "unreferenced formal parameter"
    QMAKE_CXXFLAGS_WARN_ON += -wd4996 # "function or variable may be unsafe..."
    QMAKE_CXXFLAGS_WARN_ON += -wd4101 # "unreferenced local variable"

    MSVC_VER = $$(VisualStudioVersion)
    equals(MSVC_VER,14.0){
        DEFINES += SIM_COMPILER_STR=\\\"MSVC2015\\\"
    }
    equals(MSVC_VER,15.0){
        DEFINES += SIM_COMPILER_STR=\\\"MSVC2017\\\"
    }
    equals(MSVC_VER,16.0){
        DEFINES += SIM_COMPILER_STR=\\\"MSVC2019\\\"
    }
}

*-g++* { #includes MinGW
    CONFIG(debug,debug|release) {
        QMAKE_CXXFLAGS += -O0 -g -ggdb
    } else {
        QMAKE_CFLAGS += -O3
        QMAKE_CXXFLAGS += -O3
    }
    CONFIG += c++17
    QMAKE_CFLAGS_WARN_ON = -Wall
    QMAKE_CFLAGS_WARN_ON += -Wno-strict-aliasing
    QMAKE_CFLAGS_WARN_ON += -Wno-unused-parameter
    QMAKE_CFLAGS_WARN_ON += -Wno-unused-but-set-variable
    QMAKE_CFLAGS_WARN_ON += -Wno-unused-local-typedefs

    QMAKE_CXXFLAGS_WARN_ON = -Wall
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
    QMAKE_CXXFLAGS_WARN_ON += -Wno-strict-aliasing
    QMAKE_CXXFLAGS_WARN_ON += -Wno-empty-body
    QMAKE_CXXFLAGS_WARN_ON += -Wno-write-strings
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-but-set-variable
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-local-typedefs
    QMAKE_CXXFLAGS_WARN_ON += -Wno-narrowing
    QMAKE_CXXFLAGS += -Werror=return-type

    DEFINES += SIM_COMPILER_STR=\\\"GCC\\\"
}

clang* {
    CONFIG(debug,debug|release) {
        QMAKE_CXXFLAGS += -g -ggdb
    } else {
        QMAKE_CFLAGS += -O3
        QMAKE_CXXFLAGS += -O3
    }

    CONFIG += c++17
    QMAKE_CFLAGS_WARN_ON = -Wall
    QMAKE_CFLAGS_WARN_ON += -Wno-strict-aliasing
    QMAKE_CFLAGS_WARN_ON += -Wno-unused-parameter
    QMAKE_CFLAGS_WARN_ON += -Wno-unused-but-set-variable
    QMAKE_CFLAGS_WARN_ON += -Wno-unused-local-typedefs

    QMAKE_CXXFLAGS_WARN_ON = -Wall
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
    QMAKE_CXXFLAGS_WARN_ON += -Wno-strict-aliasing
    QMAKE_CXXFLAGS_WARN_ON += -Wno-empty-body
    QMAKE_CXXFLAGS_WARN_ON += -Wno-write-strings
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-but-set-variable
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-local-typedefs
    QMAKE_CXXFLAGS_WARN_ON += -Wno-narrowing
    QMAKE_CXXFLAGS += -Werror=return-type
    QMAKE_CFLAGS += -mmacosx-version-min=10.7
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.7

    DEFINES += SIM_COMPILER_STR=\\\"Clang\\\"
}

win32 {
    LIBS += -lwinmm
    LIBS += -lShell32
    LIBS += -lopengl32
    LIBS += -luser32
    LIBS += -lDbghelp
    DEFINES += WIN_SIM
    DEFINES += SIM_PLATFORM=\\\"windows\\\"
}

macx {
    DEFINES += MAC_SIM
    DEFINES += SIM_PLATFORM=\\\"macOS\\\"
}

unix:!macx {
    DEFINES += LIN_SIM
    DEFINES += SIM_PLATFORM=\\\"linux\\\"
}

INCLUDEPATH += $$BOOST_INCLUDEPATH
INCLUDEPATH += $$LUA_INCLUDEPATH
INCLUDEPATH += $$EIGEN_INCLUDEPATH
LIBS += $$LUA_LIBS
LIBS += $$TINYXML2_LIBS
INCLUDEPATH += $$QSCINTILLA_INCLUDEPATH # somehow required to avoid a crash on exit (Windows), when copy was used in the code editor...
LIBS += $$QSCINTILLA_LIBS # somehow required to avoid a crash on exit (Windows), when copy was used in the code editor...


INCLUDEPATH += $$PWD/"sourceCode"
INCLUDEPATH += $$PWD/"sourceCode/communication"
INCLUDEPATH += $$PWD/"sourceCode/communication/serialPort"
INCLUDEPATH += $$PWD/"sourceCode/drawingObjects"
INCLUDEPATH += $$PWD/"sourceCode/platform"
INCLUDEPATH += $$PWD/"sourceCode/collections"
INCLUDEPATH += $$PWD/"sourceCode/collisions"
INCLUDEPATH += $$PWD/"sourceCode/distances"
INCLUDEPATH += $$PWD/"sourceCode/pathPlanning_old"
INCLUDEPATH += $$PWD/"sourceCode/sceneObjects"
INCLUDEPATH += $$PWD/"sourceCode/sceneObjects/related"
INCLUDEPATH += $$PWD/"sourceCode/sceneObjects/graphObjectRelated"
INCLUDEPATH += $$PWD/"sourceCode/sceneObjects/proximitySensorObjectRelated"
INCLUDEPATH += $$PWD/"sourceCode/sceneObjects/shapeObjectRelated"
INCLUDEPATH += $$PWD/"sourceCode/mainContainers"
INCLUDEPATH += $$PWD/"sourceCode/mainContainers/sceneContainers"
INCLUDEPATH += $$PWD/"sourceCode/mainContainers/applicationContainers"
INCLUDEPATH += $$PWD/"sourceCode/scripting"
INCLUDEPATH += $$PWD/"sourceCode/pagesAndViews"
INCLUDEPATH += $$PWD/"sourceCode/textures"
INCLUDEPATH += $$PWD/"sourceCode/serialization"
INCLUDEPATH += $$PWD/"sourceCode/strings"
INCLUDEPATH += $$PWD/"sourceCode/interfaces"
INCLUDEPATH += $$PWD/"sourceCode/interfaces/interfaceStack"
INCLUDEPATH += $$PWD/"sourceCode/menusAndSimilar"
INCLUDEPATH += $$PWD/"sourceCode/variousFunctions"
INCLUDEPATH += $$PWD/"sourceCode/geometricAlgorithms"
INCLUDEPATH += $$PWD/"sourceCode/various"
INCLUDEPATH += $$PWD/"sourceCode/visual"
INCLUDEPATH += $$PWD/"sourceCode/displ"
INCLUDEPATH += $$PWD/"sourceCode/utils"
INCLUDEPATH += $$PWD/"sourceCode/undoRedo"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/pathPlanning"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/collisions"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/distances"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/kinematics"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/tubes"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/wireless"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/customUserInterfaces"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/draw"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/geom"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/interfaces"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/sceneContainers"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/sceneObjects"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/scripting"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/scripting/customScriptFuncAndVar"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/utils"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/various"

INCLUDEPATH += $$PWD/"../programming/include"

WITH_GUI {
    INCLUDEPATH += $$PWD/"sourceCode/gui/rendering"
    INCLUDEPATH += $$PWD/"sourceCode/gui/dialogs"
    INCLUDEPATH += $$PWD/"sourceCode/gui/dialogs/other"
    INCLUDEPATH += $$PWD/"sourceCode/gui/platform"
    INCLUDEPATH += $$PWD/"sourceCode/gui/mainContainers"
    INCLUDEPATH += $$PWD/"sourceCode/gui/mainContainers/sceneContainers"
    INCLUDEPATH += $$PWD/"sourceCode/gui/mainContainers/applicationContainers"
    INCLUDEPATH += $$PWD/"sourceCode/gui/editModes"
    INCLUDEPATH += $$PWD/"sourceCode/gui/modelBrowser"
    INCLUDEPATH += $$PWD/"sourceCode/gui/sceneHierarchy"
    INCLUDEPATH += $$PWD/"sourceCode/gui/selectors"
    INCLUDEPATH += $$PWD/"sourceCode/gui/menusAndSimilar"
    INCLUDEPATH += $$PWD/"sourceCode/gui/various"
    INCLUDEPATH += $$PWD/"sourceCode/gui/libs"
    INCLUDEPATH += $$PWD/"sourceCode/gui/visual"
    INCLUDEPATH += $$PWD/"sourceCode/gui/sceneObjects/visionSensorObjectRelated"
    INCLUDEPATH += $$PWD/"sourceCode/gui/communication"
    INCLUDEPATH += $$PWD/"sourceCode/gui/communication/wireless"
}

*-msvc* {
    QMAKE_CXXFLAGS += /FI $$PWD/"sourceCode/various/simMainHeader.h"
} else {
    QMAKE_CXXFLAGS += -include $$PWD/"sourceCode/various/simMainHeader.h"
}

WITH_GUI {
    FORMS += $$PWD/ui/qdlgsettings.ui \
    $$PWD/ui/qdlglayers.ui \
    $$PWD/ui/qdlgavirecorder.ui \
    $$PWD/ui/qdlgabout.ui \
    $$PWD/ui/qdlgsimulation.ui \
    $$PWD/ui/qdlgcommonproperties.ui \
    $$PWD/ui/qdlgdetectableproperties.ui \
    $$PWD/ui/qdlgmodelproperties.ui \
    $$PWD/ui/qdlgassembly.ui \
    $$PWD/ui/qdlgslider.ui \
    $$PWD/ui/qdlgslider2.ui \
    $$PWD/ui/qdlgmodelthumbnailvisu.ui \
    $$PWD/ui/qdlgmodelthumbnail.ui \
    $$PWD/ui/qdlgenvironment.ui \
    $$PWD/ui/qdlgcolor.ui \
    $$PWD/ui/qdlgmaterial.ui \
    $$PWD/ui/qdlglightmaterial.ui \
    $$PWD/ui/qdlgcolorpulsation.ui \
    $$PWD/ui/qdlgfog.ui \
    $$PWD/ui/qdlgcollections.ui \
    $$PWD/ui/qdlgcollisions.ui \
    $$PWD/ui/qdlgdistances.ui \
    $$PWD/ui/qdlgentityvsentityselection.ui \
    $$PWD/ui/qdlgscripts.ui \
    $$PWD/ui/qdlgscaling.ui \
    $$PWD/ui/qdlgcameras.ui \
    $$PWD/ui/qdlglights.ui \
    $$PWD/ui/qdlgmirrors.ui \
    $$PWD/ui/qdlgdummies.ui \
    $$PWD/ui/qdlgoctrees.ui \
    $$PWD/ui/qdlgpointclouds.ui \
    $$PWD/ui/qdlguserparameters.ui \
    $$PWD/ui/qdlgforcesensors.ui \
    $$PWD/ui/qdlgprimitives.ui \
    $$PWD/ui/qdlgconvexdecomposition.ui \
    $$PWD/ui/qdlgvisionsensors.ui \
    $$PWD/ui/qdlgimagecolor.ui \
    $$PWD/ui/qdlgshapes.ui \
    $$PWD/ui/qdlgmultishapeedition.ui \
    $$PWD/ui/qdlgtextures.ui \
    $$PWD/ui/qdlgtextureselection.ui \
    $$PWD/ui/qdlggeometry.ui \
    $$PWD/ui/qdlgshapedyn.ui \
    $$PWD/ui/qdlgdetectionvolume.ui \
    $$PWD/ui/qdlgproximitysensors.ui \
    $$PWD/ui/qdlgproxsensdetectionparam.ui \
    $$PWD/ui/qdlgobjectdialogcontainer.ui \
    $$PWD/ui/qdlgshapeeditioncontainer.ui \
    $$PWD/ui/qdlgcalcdialogcontainer_OLD.ui \
    $$PWD/ui/qdlgheightfielddimension.ui \
    $$PWD/ui/qdlgstopscripts.ui \
    $$PWD/ui/qdlgtextureloadoptions.ui \
    $$PWD/ui/qdlgprogress.ui \
    $$PWD/ui/qdlgjoints.ui \
    $$PWD/ui/qdlgdependencyequation.ui \
    $$PWD/ui/qdlgjointdyn.ui \
    $$PWD/ui/qdlggraphs.ui \
    $$PWD/ui/qdlgdatastreamselection.ui \
    $$PWD/ui/qdlg2d3dgraphproperties.ui \
    $$PWD/ui/qdlgaddgraphcurve.ui \
    $$PWD/ui/qdlgik.ui \
    $$PWD/ui/qdlgikelements.ui \
    $$PWD/ui/qdlgikconditional.ui \
    $$PWD/ui/qdlgpaths.ui \
    $$PWD/ui/qdlgpathshaping.ui \
    $$PWD/ui/qdlgmessageandcheckbox.ui \
    $$PWD/ui/qdlgopenglsettings.ui \
    $$PWD/ui/qdlgtriangleedit.ui \
    $$PWD/ui/qdlgvertexedit.ui \
    $$PWD/ui/qdlgedgeedit.ui \
    $$PWD/ui/qdlgpathedit.ui \
    $$PWD/ui/qdlgtranslation.ui \
    $$PWD/ui/qdlgrotation.ui \
    $$PWD/ui/qdlgtranslationrotationcont.ui \
}

HEADERS += $$PWD/../programming/include/simMath/mathDefines.h \
    $$PWD/../programming/include/simMath/mathFuncs.h \
    $$PWD/../programming/include/simMath/mXnMatrix.h \
    $$PWD/../programming/include/simMath/7Vector.h \
    $$PWD/../programming/include/simMath/4X4Matrix.h \
    $$PWD/../programming/include/simMath/4Vector.h \
    $$PWD/../programming/include/simMath/3X3Matrix.h \
    $$PWD/../programming/include/simMath/3Vector.h \
    $$PWD/sourceCode/various/simMainHeader.h

HEADERS += $$PWD/../programming/include/simLib/simConst.h \
    $$PWD/../programming/include/simLib/simTypes.h \

HEADERS += $$PWD/sourceCode/backwardCompatibility/kinematics/ikGroup_old.h \
    $$PWD/sourceCode/backwardCompatibility/kinematics/_ikGroup_old.h \
    $$PWD/sourceCode/backwardCompatibility/kinematics/ikElement_old.h \
    $$PWD/sourceCode/backwardCompatibility/kinematics/ikGroupContainer.h \
    $$PWD/sourceCode/backwardCompatibility/kinematics/_ikGroupContainer_.h \
    $$PWD/sourceCode/backwardCompatibility/collisions/collisionObject_old.h \
    $$PWD/sourceCode/backwardCompatibility/distances/distanceObject_old.h \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/pathPlanning_old.h \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/holonomicPathPlanning_old.h \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/holonomicPathNode_old.h \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathPlanning_old.h \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathNode_old.h \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/pathPlanningTask_old.h \
    $$PWD/sourceCode/backwardCompatibility/tubes/commTube.h \
    $$PWD/sourceCode/backwardCompatibility/wireless/broadcastDataContainer.h \
    $$PWD/sourceCode/backwardCompatibility/wireless/broadcastData.h \
    $$PWD/sourceCode/backwardCompatibility/customUserInterfaces/buttonBlock.h \
    $$PWD/sourceCode/backwardCompatibility/customUserInterfaces/softButton.h \
    $$PWD/sourceCode/backwardCompatibility/draw/bannerObject.h \
    $$PWD/sourceCode/backwardCompatibility/draw/ptCloud_old.h \
    $$PWD/sourceCode/backwardCompatibility/geom/linMotionRoutines.h \
    $$PWD/sourceCode/backwardCompatibility/interfaces/sim-old.h \
    $$PWD/sourceCode/backwardCompatibility/interfaces/simInternal-old.h \
    $$PWD/sourceCode/backwardCompatibility/interfaces/pathPlanningInterface.h \
    $$PWD/sourceCode/backwardCompatibility/interfaces/dummyClasses.h \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/bannerContainer.h \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/registeredPathPlanningTasks.h \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/distanceObjectContainer_old.h \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/collisionObjectContainer_old.h \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/commTubeContainer.h \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/customData_old.h \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/ghostObjectContainer.h \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/pointCloudContainer_old.h \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/buttonBlockContainer.h \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/mill.h \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/path_old.h \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/bezierPathPoint_old.h \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/simplePathPoint_old.h \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/pathPoint_old.h \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/pathCont_old.h \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/simpleFilter.h \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/composedFilter.h \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/graphingRoutines_old.h \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/graphDataComb_old.h \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/graphData_old.h \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/staticGraphCurve_old.h \
    $$PWD/sourceCode/backwardCompatibility/scripting/userParameters.h \
    $$PWD/sourceCode/backwardCompatibility/scripting/customScriptFuncAndVar/scriptCustomFuncAndVarContainer.h \
    $$PWD/sourceCode/backwardCompatibility/scripting/customScriptFuncAndVar/scriptCustomFunction.h \
    $$PWD/sourceCode/backwardCompatibility/scripting/customScriptFuncAndVar/scriptCustomVariable.h \
    $$PWD/sourceCode/backwardCompatibility/utils/threadPool_old.h \
    $$PWD/sourceCode/backwardCompatibility/various/memorizedConf_old.h \
    $$PWD/sourceCode/backwardCompatibility/various/ghostObject.h \

HEADERS += $$PWD/sourceCode/drawingObjects/drawingObject.h \

HEADERS += $$PWD/sourceCode/platform/vVarious.h \
    $$PWD/sourceCode/platform/vThread.h \
    $$PWD/sourceCode/platform/vMutex.h \
    $$PWD/sourceCode/platform/vFileFinder.h \
    $$PWD/sourceCode/platform/vFile.h \
    $$PWD/sourceCode/platform/vDateTime.h \
    $$PWD/sourceCode/platform/vArchive.h

HEADERS += $$PWD/sourceCode/collections/collectionElement.h \
    $$PWD/sourceCode/collections/collection.h \

HEADERS += $$PWD/sourceCode/collisions/collisionRoutines.h \

HEADERS += $$PWD/sourceCode/distances/distanceRoutines.h \

HEADERS += $$PWD/sourceCode/sceneObjects/related/sceneObject.h \
    $$PWD/sourceCode/sceneObjects/related/convexVolume.h \
    $$PWD/sourceCode/sceneObjects/related/viewableBase.h \

HEADERS += $$PWD/sourceCode/sceneObjects/jointObject.h \
    $$PWD/sourceCode/sceneObjects/camera.h \
    $$PWD/sourceCode/sceneObjects/dummy.h \
    $$PWD/sourceCode/sceneObjects/octree.h \
    $$PWD/sourceCode/sceneObjects/pointCloud.h \
    $$PWD/sourceCode/sceneObjects/forceSensor.h \
    $$PWD/sourceCode/sceneObjects/graph.h \
    $$PWD/sourceCode/sceneObjects/light.h \
    $$PWD/sourceCode/sceneObjects/mirror.h \
    $$PWD/sourceCode/sceneObjects/shape.h \
    $$PWD/sourceCode/sceneObjects/proximitySensor.h \
    $$PWD/sourceCode/sceneObjects/visionSensor.h \

HEADERS += $$PWD/sourceCode/sceneObjects/graphObjectRelated/graphCurve.h \
    $$PWD/sourceCode/sceneObjects/graphObjectRelated/graphDataStream.h \

HEADERS += $$PWD/sourceCode/sceneObjects/proximitySensorObjectRelated/proxSensorRoutine.h \

HEADERS += $$PWD/sourceCode/sceneObjects/shapeObjectRelated/mesh.h \
    $$PWD/sourceCode/sceneObjects/shapeObjectRelated/meshWrapper.h \
    $$PWD/sourceCode/sceneObjects/shapeObjectRelated/volInt.h \

HEADERS += $$PWD/sourceCode/communication/serialPort/serialPort.h \
    $$PWD/sourceCode/communication/serialPort/serial_win_mac_linux.h \
    $$PWD/sourceCode/communication/serialPort/serialPortWin.h \

HEADERS += $$PWD/sourceCode/mainContainers/worldContainer.h \
    $$PWD/sourceCode/mainContainers/world.h \

HEADERS += $$PWD/sourceCode/mainContainers/sceneContainers/drawingContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/textureContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/signalContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/simulation.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/collectionContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/sceneObjectContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/mainSettings.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/embeddedScriptContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/environment.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/dynamicsContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/apiErrors.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/outsideCommandQueue.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/cacheCont.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/customData.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/undoBufferCont.h \

HEADERS += $$PWD/sourceCode/mainContainers/applicationContainers/copyBuffer.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/persistentDataContainer.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/simulatorMessageQueue.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/calculationInfo.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/interfaceStackContainer.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/addOnScriptContainer.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/pluginContainer.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/plugin.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/pluginCallbackContainer.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/pluginVariableContainer.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/codeEditorInfos.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/codeEditorFunctions.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/codeEditorVariables.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/serialPortContainer.h \

HEADERS += $$PWD/sourceCode/scripting/scriptObject.h \
    $$PWD/sourceCode/scripting/outsideCommandQueueForScript.h \
    $$PWD/sourceCode/scripting/luaWrapper.h \

HEADERS += $$PWD/sourceCode/pagesAndViews/pageContainer.h \
    $$PWD/sourceCode/pagesAndViews/sPage.h \
    $$PWD/sourceCode/pagesAndViews/sView.h \

HEADERS += $$PWD/sourceCode/textures/textureObject.h \
    $$PWD/sourceCode/textures/imgLoaderSaver.h \
    $$PWD/sourceCode/textures/tGAFormat.h \
    $$PWD/sourceCode/textures/stb_image.h \
    $$PWD/sourceCode/textures/textureProperty.h \

HEADERS += $$PWD/sourceCode/serialization/ser.h \
    $$PWD/sourceCode/serialization/huffman.h \

HEADERS += $$PWD/sourceCode/strings/simStringTable.h \
    $$PWD/sourceCode/strings/simStringTable_openGl.h \

HEADERS += $$PWD/sourceCode/interfaces/sim.h \
    $$PWD/sourceCode/interfaces/simInternal.h \
    $$PWD/sourceCode/interfaces/luaScriptFunctions.h \
    
HEADERS += $$PWD/sourceCode/interfaces/interfaceStack/interfaceStack.h \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackObject.h \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackNull.h \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackBool.h \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackNumber.h \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackInteger.h \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackString.h \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackTable.h \

HEADERS += $$PWD/sourceCode/menusAndSimilar/fileOperations.h \
    $$PWD/sourceCode/menusAndSimilar/addOperations.h \
    $$PWD/sourceCode/menusAndSimilar/moduleMenuItemContainer.h \
    $$PWD/sourceCode/menusAndSimilar/moduleMenuItem.h \

HEADERS += $$PWD/sourceCode/variousFunctions/sceneObjectOperations.h \

HEADERS += $$PWD/sourceCode/geometricAlgorithms/meshRoutines.h \
    $$PWD/sourceCode/geometricAlgorithms/meshManip.h \
    $$PWD/sourceCode/geometricAlgorithms/edgeElement.h \
    $$PWD/sourceCode/geometricAlgorithms/algos.h \

HEADERS += $$PWD/sourceCode/various/simConfig.h \
    $$PWD/sourceCode/various/global.h \
    $$PWD/sourceCode/various/embeddedFonts.h \
    $$PWD/sourceCode/various/userSettings.h \
    $$PWD/sourceCode/various/app.h \
    $$PWD/sourceCode/various/folderSystem.h \
    $$PWD/sourceCode/various/dynMaterialObject.h \
    $$PWD/sourceCode/various/sigHandler.h \

HEADERS += $$PWD/sourceCode/undoRedo/undoBufferArrays.h \
    $$PWD/sourceCode/undoRedo/undoBuffer.h \
    $$PWD/sourceCode/undoRedo/undoBufferCameras.h \

HEADERS += $$PWD/sourceCode/visual/thumbnail.h \

HEADERS += $$PWD/sourceCode/displ/colorObject.h \

HEADERS += $$PWD/sourceCode/utils/tt.h \
    $$PWD/sourceCode/utils/utils.h \
    $$PWD/sourceCode/utils/confReaderAndWriter.h \
    $$PWD/sourceCode/utils/base64.h \
    $$PWD/sourceCode/utils/cbor.h \
    $$PWD/sourceCode/utils/annJson.h \
    $$PWD/sourceCode/utils/sha256.h \

HEADERS += $$PWD/sourceCode/platform/vSimUiMutex.h \
    $$PWD/sourceCode/platform/wThread.h

HEADERS += $$PWD/sourceCode/various/simQApp.h \

WITH_GUI {
    HEADERS += $$PWD/sourceCode/gui/rendering/rendering.h \
        $$PWD/sourceCode/gui/rendering/cameraRendering.h \
        $$PWD/sourceCode/gui/rendering/visionSensorRendering.h \
        $$PWD/sourceCode/gui/rendering/forceSensorRendering.h \
        $$PWD/sourceCode/gui/rendering/proximitySensorRendering.h \
        $$PWD/sourceCode/gui/rendering/dummyRendering.h \
        $$PWD/sourceCode/gui/rendering/lightRendering.h \
        $$PWD/sourceCode/gui/rendering/pointCloudRendering.h \
        $$PWD/sourceCode/gui/rendering/octreeRendering.h \
        $$PWD/sourceCode/gui/rendering/jointRendering.h \
        $$PWD/sourceCode/gui/rendering/graphRendering.h \
        $$PWD/sourceCode/gui/rendering/millRendering.h \
        $$PWD/sourceCode/gui/rendering/mirrorRendering.h \
        $$PWD/sourceCode/gui/rendering/pathRendering.h \
        $$PWD/sourceCode/gui/rendering/shapeRendering.h \
        $$PWD/sourceCode/gui/rendering/ghostRendering.h \
        $$PWD/sourceCode/gui/rendering/drawingObjectRendering.h \
        $$PWD/sourceCode/gui/rendering/ptCloudRendering_old.h \
        $$PWD/sourceCode/gui/rendering/collisionContourRendering.h \
        $$PWD/sourceCode/gui/rendering/distanceRendering.h \
        $$PWD/sourceCode/gui/rendering/bannerRendering.h \
        $$PWD/sourceCode/gui/rendering/thumbnailRendering.h \
        $$PWD/sourceCode/gui/rendering/pathPlanningTaskRendering_old.h \
        $$PWD/sourceCode/gui/rendering/broadcastDataVisualRendering.h \
        $$PWD/sourceCode/gui/rendering/dynamicsRendering.h \
        $$PWD/sourceCode/gui/rendering/environmentRendering.h \
        $$PWD/sourceCode/gui/rendering/pageRendering.h \
        $$PWD/sourceCode/gui/rendering/viewRendering.h \

    HEADERS += $$PWD/sourceCode/gui/communication/wireless/broadcastDataVisual.h \

    HEADERS += $$PWD/sourceCode/gui/sceneObjects/visionSensorObjectRelated/offscreenGlContext.h \
        $$PWD/sourceCode/gui/sceneObjects/visionSensorObjectRelated/frameBufferObject.h \
        $$PWD/sourceCode/gui/sceneObjects/visionSensorObjectRelated/visionSensorGlStuff.h

    HEADERS += $$PWD/sourceCode/gui/visual/oGL.h \
        $$PWD/sourceCode/gui/visual/oglExt.h \
        $$PWD/sourceCode/gui/visual/glShader.h \
        $$PWD/sourceCode/gui/visual/glBufferObjects.h \

    HEADERS += $$PWD/sourceCode/gui/dialogs/qdlglayers.h \
        $$PWD/sourceCode/gui/dialogs/qdlgavirecorder.h \
        $$PWD/sourceCode/gui/dialogs/qdlgabout.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimulation.h \
        $$PWD/sourceCode/gui/dialogs/qdlgcommonproperties.h \
        $$PWD/sourceCode/gui/dialogs/qdlgdetectableproperties.h \
        $$PWD/sourceCode/gui/dialogs/qdlgmodelproperties.h \
        $$PWD/sourceCode/gui/dialogs/qdlgassembly.h \
        $$PWD/sourceCode/gui/dialogs/qdlgslider.h \
        $$PWD/sourceCode/gui/dialogs/qdlgslider2.h \
        $$PWD/sourceCode/gui/dialogs/qdlgmodelthumbnailvisu.h \
        $$PWD/sourceCode/gui/dialogs/qdlgmodelthumbnail.h \
        $$PWD/sourceCode/gui/dialogs/qdlgenvironment.h \
        $$PWD/sourceCode/gui/dialogs/qdlgcolor.h \
        $$PWD/sourceCode/gui/dialogs/qdlgmaterial.h \
        $$PWD/sourceCode/gui/dialogs/qdlglightmaterial.h \
        $$PWD/sourceCode/gui/dialogs/qdlgcolorpulsation.h \
        $$PWD/sourceCode/gui/dialogs/qdlgfog.h \
        $$PWD/sourceCode/gui/dialogs/qdlgcollections.h \
        $$PWD/sourceCode/gui/dialogs/qdlgcollisions.h \
        $$PWD/sourceCode/gui/dialogs/qdlgdistances.h \
        $$PWD/sourceCode/gui/dialogs/qdlgentityvsentityselection.h \
        $$PWD/sourceCode/gui/dialogs/qdlgscripts.h \
        $$PWD/sourceCode/gui/dialogs/qdlgscaling.h \
        $$PWD/sourceCode/gui/dialogs/qdlgcameras.h \
        $$PWD/sourceCode/gui/dialogs/qdlglights.h \
        $$PWD/sourceCode/gui/dialogs/qdlgmirrors.h \
        $$PWD/sourceCode/gui/dialogs/qdlgdummies.h \
        $$PWD/sourceCode/gui/dialogs/qdlgoctrees.h \
        $$PWD/sourceCode/gui/dialogs/qdlgpointclouds.h \
        $$PWD/sourceCode/gui/dialogs/qdlguserparameters.h \
        $$PWD/sourceCode/gui/dialogs/qdlgforcesensors.h \
        $$PWD/sourceCode/gui/dialogs/qdlgprimitives.h \
        $$PWD/sourceCode/gui/dialogs/qdlgconvexdecomposition.h \
        $$PWD/sourceCode/gui/dialogs/qdlgvisionsensors.h \
        $$PWD/sourceCode/gui/dialogs/qdlgimagecolor.h \
        $$PWD/sourceCode/gui/dialogs/qdlgshapes.h \
        $$PWD/sourceCode/gui/dialogs/qdlgmultishapeedition.h \
        $$PWD/sourceCode/gui/dialogs/qdlgtextures.h \
        $$PWD/sourceCode/gui/dialogs/qdlgtextureselection.h \
        $$PWD/sourceCode/gui/dialogs/qdlggeometry.h \
        $$PWD/sourceCode/gui/dialogs/qdlgshapedyn.h \
        $$PWD/sourceCode/gui/dialogs/qdlgdetectionvolume.h \
        $$PWD/sourceCode/gui/dialogs/qdlgproximitysensors.h \
        $$PWD/sourceCode/gui/dialogs/qdlgproxsensdetectionparam.h \
        $$PWD/sourceCode/gui/dialogs/qdlgobjectdialogcontainer.h \
        $$PWD/sourceCode/gui/dialogs/qdlgshapeeditioncontainer.h \
        $$PWD/sourceCode/gui/dialogs/qdlgcalcdialogcontainer_OLD.h \
        $$PWD/sourceCode/gui/dialogs/qdlgheightfielddimension.h \
        $$PWD/sourceCode/gui/dialogs/qdlgstopscripts.h \
        $$PWD/sourceCode/gui/dialogs/qdlgtextureloadoptions.h \
        $$PWD/sourceCode/gui/dialogs/qdlgprogress.h \
        $$PWD/sourceCode/gui/dialogs/qdlgjoints.h \
        $$PWD/sourceCode/gui/dialogs/qdlgdependencyequation.h \
        $$PWD/sourceCode/gui/dialogs/qdlgjointdyn.h \
        $$PWD/sourceCode/gui/dialogs/qdlggraphs.h \
        $$PWD/sourceCode/gui/dialogs/qdlgdatastreamselection.h \
        $$PWD/sourceCode/gui/dialogs/qdlg2d3dgraphproperties.h \
        $$PWD/sourceCode/gui/dialogs/qdlgaddgraphcurve.h \
        $$PWD/sourceCode/gui/dialogs/qdlgik.h \
        $$PWD/sourceCode/gui/dialogs/qdlgikelements.h \
        $$PWD/sourceCode/gui/dialogs/qdlgikconditional.h \
        $$PWD/sourceCode/gui/dialogs/qdlgpaths.h \
        $$PWD/sourceCode/gui/dialogs/qdlgpathshaping.h \
        $$PWD/sourceCode/gui/dialogs/qdlgmessageandcheckbox.h \
        $$PWD/sourceCode/gui/dialogs/qdlgopenglsettings.h \
        $$PWD/sourceCode/gui/dialogs/qdlgtriangleedit.h \
        $$PWD/sourceCode/gui/dialogs/qdlgvertexedit.h \
        $$PWD/sourceCode/gui/dialogs/qdlgedgeedit.h \
        $$PWD/sourceCode/gui/dialogs/qdlgpathedit.h \
        $$PWD/sourceCode/gui/dialogs/qdlgtranslation.h \
        $$PWD/sourceCode/gui/dialogs/qdlgrotation.h \
        $$PWD/sourceCode/gui/dialogs/qdlgtranslationrotationcont.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsettings.h \

    HEADERS += $$PWD/sourceCode/gui/dialogs/other/editboxdelegate.h \
        $$PWD/sourceCode/gui/dialogs/other/toolDlgWrapper.h \
        $$PWD/sourceCode/gui/dialogs/other/dlgEx.h \
        $$PWD/sourceCode/gui/dialogs/other/dlgCont.h \
        $$PWD/sourceCode/gui/dialogs/other/qsim_openglwidget.h \

    HEADERS += $$PWD/sourceCode/gui/platform/vMessageBox.h \
        $$PWD/sourceCode/gui/platform/vMenubar.h \
        $$PWD/sourceCode/gui/platform/vFileDialog.h \
        $$PWD/sourceCode/gui/platform/vDialog.h \

    HEADERS += $$PWD/sourceCode/gui/mainContainers/sceneContainers/oglSurface.h \

    HEADERS += $$PWD/sourceCode/gui/mainContainers/applicationContainers/globalGuiTextureContainer.h \
        $$PWD/sourceCode/gui/mainContainers/applicationContainers/codeEditorContainer.h \

    HEADERS += $$PWD/sourceCode/gui/editModes/editModeContainer.h \
        $$PWD/sourceCode/gui/editModes/shapeEditMode.h \
        $$PWD/sourceCode/gui/editModes/multishapeEditMode.h \
        $$PWD/sourceCode/gui/editModes/pathEditMode.h \
        $$PWD/sourceCode/gui/editModes/edgeCont.h \
        $$PWD/sourceCode/gui/editModes/pathPointManipulation.h \

    HEADERS += $$PWD/sourceCode/gui/sceneHierarchy/hierarchy.h \
        $$PWD/sourceCode/gui/sceneHierarchy/hierarchyElement.h \

    HEADERS += $$PWD/sourceCode/gui/modelBrowser/modelFolderWidget.h \
        $$PWD/sourceCode/gui/modelBrowser/modelListWidget.h \
        $$PWD/sourceCode/gui/modelBrowser/modelListWidgetBase.h \

    HEADERS += $$PWD/sourceCode/gui/selectors/viewSelector.h \
        $$PWD/sourceCode/gui/selectors/pageSelector.h \

    HEADERS += $$PWD/sourceCode/gui/menusAndSimilar/toolBarCommand.h \
        $$PWD/sourceCode/gui/menusAndSimilar/mainWindow.h \
        $$PWD/sourceCode/gui/menusAndSimilar/statusBar.h \
        $$PWD/sourceCode/gui/menusAndSimilar/helpMenu.h \

    HEADERS += $$PWD/sourceCode/gui/various/simRecorder.h \
        $$PWD/sourceCode/gui/various/engineProperties.h \
        $$PWD/sourceCode/gui/various/guiApp.h \
        $$PWD/sourceCode/gui/various/uiThread.h \
        $$PWD/sourceCode/gui/various/simThread.h \
        $$PWD/sourceCode/gui/various/simAndUiThreadSync.h \

    HEADERS += $$PWD/sourceCode/gui/libs/auxLibVideo.h \
}

SOURCES += $$PWD/../programming/include/simMath/mathFuncs.cpp \
    $$PWD/../programming/include/simMath/mXnMatrix.cpp \
    $$PWD/../programming/include/simMath/7Vector.cpp \
    $$PWD/../programming/include/simMath/4X4Matrix.cpp \
    $$PWD/../programming/include/simMath/4Vector.cpp \
    $$PWD/../programming/include/simMath/3X3Matrix.cpp \
    $$PWD/../programming/include/simMath/3Vector.cpp \

SOURCES += $$PWD/sourceCode/backwardCompatibility/kinematics/ikGroup_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/kinematics/_ikGroup_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/kinematics/ikElement_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/kinematics/ikGroupContainer.cpp \
    $$PWD/sourceCode/backwardCompatibility/kinematics/_ikGroupContainer_.cpp \
    $$PWD/sourceCode/backwardCompatibility/collisions/collisionObject_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/distances/distanceObject_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/pathPlanning_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/holonomicPathPlanning_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/holonomicPathNode_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathPlanning_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathNode_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/pathPlanningTask_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/tubes/commTube.cpp \
    $$PWD/sourceCode/backwardCompatibility/wireless/broadcastDataContainer.cpp \
    $$PWD/sourceCode/backwardCompatibility/wireless/broadcastData.cpp \
    $$PWD/sourceCode/backwardCompatibility/customUserInterfaces/buttonBlock.cpp \
    $$PWD/sourceCode/backwardCompatibility/customUserInterfaces/softButton.cpp \
    $$PWD/sourceCode/backwardCompatibility/draw/bannerObject.cpp \
    $$PWD/sourceCode/backwardCompatibility/draw/ptCloud_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/geom/linMotionRoutines.cpp \
    $$PWD/sourceCode/backwardCompatibility/interfaces/sim-old.cpp \
    $$PWD/sourceCode/backwardCompatibility/interfaces/simInternal-old.cpp \
    $$PWD/sourceCode/backwardCompatibility/interfaces/pathPlanningInterface.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/bannerContainer.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/registeredPathPlanningTasks.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/distanceObjectContainer_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/collisionObjectContainer_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/customData_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/commTubeContainer.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/ghostObjectContainer.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/pointCloudContainer_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneContainers/buttonBlockContainer.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/mill.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/path_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/bezierPathPoint_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/simplePathPoint_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/pathPoint_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/pathCont_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/simpleFilter.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/composedFilter.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/graphingRoutines_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/graphDataComb_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/graphData_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/sceneObjects/staticGraphCurve_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/scripting/userParameters.cpp \
    $$PWD/sourceCode/backwardCompatibility/scripting/customScriptFuncAndVar/scriptCustomFuncAndVarContainer.cpp \
    $$PWD/sourceCode/backwardCompatibility/scripting/customScriptFuncAndVar/scriptCustomFunction.cpp \
    $$PWD/sourceCode/backwardCompatibility/scripting/customScriptFuncAndVar/scriptCustomVariable.cpp \
    $$PWD/sourceCode/backwardCompatibility/utils/threadPool_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/various/memorizedConf_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/various/ghostObject.cpp \

SOURCES += $$PWD/sourceCode/drawingObjects/drawingObject.cpp \

SOURCES += $$PWD/sourceCode/platform/vVarious.cpp \
    $$PWD/sourceCode/platform/vThread.cpp \
    $$PWD/sourceCode/platform/vMutex.cpp \
    $$PWD/sourceCode/platform/vFileFinder.cpp \
    $$PWD/sourceCode/platform/vFile.cpp \
    $$PWD/sourceCode/platform/vDateTime.cpp \
    $$PWD/sourceCode/platform/vArchive.cpp

SOURCES += $$PWD/sourceCode/collections/collectionElement.cpp \
    $$PWD/sourceCode/collections/collection.cpp \

SOURCES += $$PWD/sourceCode/collisions/collisionRoutines.cpp \

SOURCES += $$PWD/sourceCode/distances/distanceRoutines.cpp \

SOURCES += $$PWD/sourceCode/sceneObjects/related/sceneObject.cpp \
    $$PWD/sourceCode/sceneObjects/related/convexVolume.cpp \
    $$PWD/sourceCode/sceneObjects/related/viewableBase.cpp \

SOURCES += $$PWD/sourceCode/sceneObjects/jointObject.cpp \
    $$PWD/sourceCode/sceneObjects/camera.cpp \
    $$PWD/sourceCode/sceneObjects/dummy.cpp \
    $$PWD/sourceCode/sceneObjects/octree.cpp \
    $$PWD/sourceCode/sceneObjects/pointCloud.cpp \
    $$PWD/sourceCode/sceneObjects/forceSensor.cpp \
    $$PWD/sourceCode/sceneObjects/graph.cpp \
    $$PWD/sourceCode/sceneObjects/light.cpp \
    $$PWD/sourceCode/sceneObjects/mirror.cpp \
    $$PWD/sourceCode/sceneObjects/proximitySensor.cpp \
    $$PWD/sourceCode/sceneObjects/shape.cpp \
    $$PWD/sourceCode/sceneObjects/visionSensor.cpp \

SOURCES += $$PWD/sourceCode/sceneObjects/graphObjectRelated/graphCurve.cpp \
    $$PWD/sourceCode/sceneObjects/graphObjectRelated/graphDataStream.cpp \

SOURCES += $$PWD/sourceCode/sceneObjects/proximitySensorObjectRelated/proxSensorRoutine.cpp \

SOURCES += $$PWD/sourceCode/sceneObjects/shapeObjectRelated/mesh.cpp \
    $$PWD/sourceCode/sceneObjects/shapeObjectRelated/meshWrapper.cpp \
    $$PWD/sourceCode/sceneObjects/shapeObjectRelated/volInt.cpp \


SOURCES += $$PWD/sourceCode/communication/serialPort/serialPort.cpp \
    $$PWD/sourceCode/communication/serialPort/serial_win_mac_linux.cpp \
    $$PWD/sourceCode/communication/serialPort/serialPortWin.cpp \

SOURCES += $$PWD/sourceCode/mainContainers/worldContainer.cpp \
    $$PWD/sourceCode/mainContainers/world.cpp \

SOURCES += $$PWD/sourceCode/mainContainers/sceneContainers/drawingContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/textureContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/simulation.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/signalContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/collectionContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/outsideCommandQueue.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/sceneObjectContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/mainSettings.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/embeddedScriptContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/environment.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/dynamicsContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/customData.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/cacheCont.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/apiErrors.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/undoBufferCont.cpp \

SOURCES += $$PWD/sourceCode/mainContainers/applicationContainers/copyBuffer.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/persistentDataContainer.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/simulatorMessageQueue.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/calculationInfo.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/interfaceStackContainer.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/addOnScriptContainer.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/pluginContainer.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/plugin.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/pluginCallbackContainer.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/pluginVariableContainer.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/codeEditorInfos.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/codeEditorFunctions.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/codeEditorVariables.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/serialPortContainer.cpp \

SOURCES += $$PWD/sourceCode/scripting/scriptObject.cpp \
    $$PWD/sourceCode/scripting/outsideCommandQueueForScript.cpp \
    $$PWD/sourceCode/scripting/luaWrapper.cpp \

SOURCES += $$PWD/sourceCode/pagesAndViews/pageContainer.cpp \
    $$PWD/sourceCode/pagesAndViews/sPage.cpp \
    $$PWD/sourceCode/pagesAndViews/sView.cpp \

SOURCES += $$PWD/sourceCode/textures/textureObject.cpp \
    $$PWD/sourceCode/textures/imgLoaderSaver.cpp \
    $$PWD/sourceCode/textures/tGAFormat.cpp \
    $$PWD/sourceCode/textures/stb_image.c \
    $$PWD/sourceCode/textures/textureProperty.cpp \

SOURCES += $$PWD/sourceCode/serialization/ser.cpp \
    $$PWD/sourceCode/serialization/huffman.c \

SOURCES += $$PWD/sourceCode/interfaces/sim.cpp \
    $$PWD/sourceCode/interfaces/simInternal.cpp \
    $$PWD/sourceCode/interfaces/luaScriptFunctions.cpp \

!IS_PLUS {
    SOURCES += $$PWD/sourceCode/interfaces/simFlavor.cpp \
        $$PWD/sourceCode/interfaces/gm.cpp \
    }

SOURCES += $$PWD/sourceCode/interfaces/interfaceStack/interfaceStack.cpp \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackObject.cpp \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackNull.cpp \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackBool.cpp \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackNumber.cpp \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackInteger.cpp \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackString.cpp \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackTable.cpp \

SOURCES += $$PWD/sourceCode/menusAndSimilar/addOperations.cpp \
    $$PWD/sourceCode/menusAndSimilar/fileOperations.cpp \
    $$PWD/sourceCode/menusAndSimilar/moduleMenuItemContainer.cpp \
    $$PWD/sourceCode/menusAndSimilar/moduleMenuItem.cpp \

SOURCES += $$PWD/sourceCode/variousFunctions/sceneObjectOperations.cpp \

SOURCES += $$PWD/sourceCode/geometricAlgorithms/meshRoutines.cpp \
    $$PWD/sourceCode/geometricAlgorithms/meshManip.cpp \
    $$PWD/sourceCode/geometricAlgorithms/edgeElement.cpp \
    $$PWD/sourceCode/geometricAlgorithms/algos.cpp \

SOURCES += $$PWD/sourceCode/various/userSettings.cpp \
    $$PWD/sourceCode/various/folderSystem.cpp \
    $$PWD/sourceCode/various/app.cpp \
    $$PWD/sourceCode/various/dynMaterialObject.cpp \
    $$PWD/sourceCode/various/sigHandler.cpp \

SOURCES += $$PWD/sourceCode/undoRedo/undoBufferArrays.cpp \
    $$PWD/sourceCode/undoRedo/undoBuffer.cpp \
    $$PWD/sourceCode/undoRedo/undoBufferCameras.cpp \

SOURCES += $$PWD/sourceCode/displ/colorObject.cpp \

SOURCES += $$PWD/sourceCode/visual/thumbnail.cpp \

SOURCES += $$PWD/sourceCode/utils/utils.cpp \
    $$PWD/sourceCode/utils/tt.cpp \
    $$PWD/sourceCode/utils/confReaderAndWriter.cpp \
    $$PWD/sourceCode/utils/base64.cpp \
    $$PWD/sourceCode/utils/cbor.cpp \
    $$PWD/sourceCode/utils/annJson.cpp \
    $$PWD/sourceCode/utils/sha256.cpp \

SOURCES += $$PWD/sourceCode/platform/vSimUiMutex.cpp \
    $$PWD/sourceCode/platform/wThread.cpp

SOURCES += $$PWD/sourceCode/various/simQApp.cpp \

WITH_GUI {
    SOURCES += $$PWD/sourceCode/gui/rendering/rendering.cpp \
        $$PWD/sourceCode/gui/rendering/cameraRendering.cpp \
        $$PWD/sourceCode/gui/rendering/visionSensorRendering.cpp \
        $$PWD/sourceCode/gui/rendering/forceSensorRendering.cpp \
        $$PWD/sourceCode/gui/rendering/proximitySensorRendering.cpp \
        $$PWD/sourceCode/gui/rendering/dummyRendering.cpp \
        $$PWD/sourceCode/gui/rendering/lightRendering.cpp \
        $$PWD/sourceCode/gui/rendering/pointCloudRendering.cpp \
        $$PWD/sourceCode/gui/rendering/octreeRendering.cpp \
        $$PWD/sourceCode/gui/rendering/jointRendering.cpp \
        $$PWD/sourceCode/gui/rendering/graphRendering.cpp \
        $$PWD/sourceCode/gui/rendering/millRendering.cpp \
        $$PWD/sourceCode/gui/rendering/mirrorRendering.cpp \
        $$PWD/sourceCode/gui/rendering/pathRendering.cpp \
        $$PWD/sourceCode/gui/rendering/shapeRendering.cpp \
        $$PWD/sourceCode/gui/rendering/ghostRendering.cpp \
        $$PWD/sourceCode/gui/rendering/drawingObjectRendering.cpp \
        $$PWD/sourceCode/gui/rendering/ptCloudRendering_old.cpp \
        $$PWD/sourceCode/gui/rendering/collisionContourRendering.cpp \
        $$PWD/sourceCode/gui/rendering/distanceRendering.cpp \
        $$PWD/sourceCode/gui/rendering/bannerRendering.cpp \
        $$PWD/sourceCode/gui/rendering/thumbnailRendering.cpp \
        $$PWD/sourceCode/gui/rendering/pathPlanningTaskRendering_old.cpp \
        $$PWD/sourceCode/gui/rendering/broadcastDataVisualRendering.cpp \
        $$PWD/sourceCode/gui/rendering/dynamicsRendering.cpp \
        $$PWD/sourceCode/gui/rendering/environmentRendering.cpp \
        $$PWD/sourceCode/gui/rendering/pageRendering.cpp \
        $$PWD/sourceCode/gui/rendering/viewRendering.cpp \

    SOURCES += $$PWD/sourceCode/gui/communication/wireless/broadcastDataVisual.cpp \

    SOURCES += $$PWD/sourceCode/gui/sceneObjects/visionSensorObjectRelated/offscreenGlContext.cpp \
        $$PWD/sourceCode/gui/sceneObjects/visionSensorObjectRelated/frameBufferObject.cpp \
        $$PWD/sourceCode/gui/sceneObjects/visionSensorObjectRelated/visionSensorGlStuff.cpp \

    SOURCES += $$PWD/sourceCode/gui/visual/oGL.cpp \
        $$PWD/sourceCode/gui/visual/oglExt.cpp \
        $$PWD/sourceCode/gui/visual/glShader.cpp \
        $$PWD/sourceCode/gui/visual/glBufferObjects.cpp \

    SOURCES += $$PWD/sourceCode/gui/dialogs/qdlgsettings.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlglayers.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgavirecorder.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgabout.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimulation.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgcommonproperties.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgdetectableproperties.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgmodelproperties.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgassembly.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgslider.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgslider2.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgmodelthumbnailvisu.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgmodelthumbnail.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgenvironment.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgcolor.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgmaterial.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlglightmaterial.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgcolorpulsation.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgfog.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgcollections.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgcollisions.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgdistances.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgentityvsentityselection.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgscripts.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgscaling.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgcameras.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlglights.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgmirrors.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgdummies.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgoctrees.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgpointclouds.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlguserparameters.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgforcesensors.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgprimitives.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgconvexdecomposition.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgvisionsensors.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgimagecolor.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgshapes.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgmultishapeedition.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgtextures.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgtextureselection.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlggeometry.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgshapedyn.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgdetectionvolume.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgproximitysensors.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgproxsensdetectionparam.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgobjectdialogcontainer.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgshapeeditioncontainer.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgcalcdialogcontainer_OLD.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgheightfielddimension.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgstopscripts.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgtextureloadoptions.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgprogress.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgjoints.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgdependencyequation.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgjointdyn.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlggraphs.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgdatastreamselection.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlg2d3dgraphproperties.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgaddgraphcurve.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgik.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgikelements.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgikconditional.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgpaths.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgpathshaping.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgmessageandcheckbox.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgopenglsettings.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgtriangleedit.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgvertexedit.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgedgeedit.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgpathedit.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgtranslation.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgrotation.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgtranslationrotationcont.cpp \

    SOURCES += $$PWD/sourceCode/gui/dialogs/other/editboxdelegate.cpp \
        $$PWD/sourceCode/gui/dialogs/other/toolDlgWrapper.cpp \
        $$PWD/sourceCode/gui/dialogs/other/dlgEx.cpp \
        $$PWD/sourceCode/gui/dialogs/other/dlgCont.cpp \
        $$PWD/sourceCode/gui/dialogs/other/qsim_openglwidget.cpp \

    SOURCES += $$PWD/sourceCode/gui/platform/vMessageBox.cpp \
        $$PWD/sourceCode/gui/platform/vMenubar.cpp \
        $$PWD/sourceCode/gui/platform/vFileDialog.cpp \
        $$PWD/sourceCode/gui/platform/vDialog.cpp \

    SOURCES += $$PWD/sourceCode/gui/mainContainers/sceneContainers/oglSurface.cpp \

    SOURCES += $$PWD/sourceCode/gui/mainContainers/applicationContainers/globalGuiTextureContainer.cpp \
        $$PWD/sourceCode/gui/mainContainers/applicationContainers/codeEditorContainer.cpp \

    SOURCES += $$PWD/sourceCode/gui/editModes/editModeContainer.cpp \
        $$PWD/sourceCode/gui/editModes/shapeEditMode.cpp \
        $$PWD/sourceCode/gui/editModes/multishapeEditMode.cpp \
        $$PWD/sourceCode/gui/editModes/pathEditMode.cpp \
        $$PWD/sourceCode/gui/editModes/edgeCont.cpp \
        $$PWD/sourceCode/gui/editModes/pathPointManipulation.cpp \

    SOURCES += $$PWD/sourceCode/gui/sceneHierarchy/hierarchy.cpp \
        $$PWD/sourceCode/gui/sceneHierarchy/hierarchyElement.cpp \

    SOURCES += $$PWD/sourceCode/gui/modelBrowser/modelFolderWidget.cpp \
        $$PWD/sourceCode/gui/modelBrowser/modelListWidget.cpp \
        $$PWD/sourceCode/gui/modelBrowser/modelListWidgetBase.cpp \

    SOURCES += $$PWD/sourceCode/gui/selectors/pageSelector.cpp \
        $$PWD/sourceCode/gui/selectors/viewSelector.cpp \

    SOURCES += $$PWD/sourceCode/gui/menusAndSimilar/toolBarCommand.cpp \
        $$PWD/sourceCode/gui/menusAndSimilar/mainWindow.cpp \
        $$PWD/sourceCode/gui/menusAndSimilar/statusBar.cpp \
        $$PWD/sourceCode/gui/menusAndSimilar/helpMenu.cpp \

    SOURCES += $$PWD/sourceCode/gui/various/simRecorder.cpp \
        $$PWD/sourceCode/gui/various/engineProperties.cpp \
        $$PWD/sourceCode/gui/various/guiApp.cpp \
        $$PWD/sourceCode/gui/various/uiThread.cpp \
        $$PWD/sourceCode/gui/various/simThread.cpp \
        $$PWD/sourceCode/gui/various/simAndUiThreadSync.cpp \

    SOURCES += $$PWD/sourceCode/gui/libs/auxLibVideo.cpp \
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
