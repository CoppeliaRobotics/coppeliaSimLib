TARGET = coppeliaSim
TEMPLATE = lib
DEFINES += SIM_LIB

CONFIG += shared plugin debug_and_release
CONFIG += WITH_QT # can be compiled without Qt, but then it should be headless, and some functionality won't be there, check TODO_SIM_WITH_QT

!HEADLESS {
    CONFIG += WITH_GUI
    CONFIG += WITH_OPENGL # comment only if above line is commented
    CONFIG += WITH_SERIAL
}

CONFIG(debug,debug|release) {
    CONFIG += force_debug_info
}

QT += printsupport
QT += network

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
DEFINES += LUA_STACK_COMPATIBILITY_MODE # 06.11.2020, will avoid using Lua INTEGER values at interfaces (using DOUBLE type instead)

WITH_GUI {
    QT      += widgets
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

WITH_OPENGL {
    QT      += opengl
    #QT      += 3dcore 3drender 3dinput 3dextras
    DEFINES += SIM_WITH_OPENGL
}

WITH_SERIAL {
    QT      += serialport
    DEFINES += SIM_WITH_SERIAL
}

WITH_QT {
    DEFINES += SIM_WITH_QT
} else {
    QT -= core
}

*-msvc* {
    QMAKE_CXXFLAGS += /std:c++17
    QMAKE_CFLAGS += -O2
    QMAKE_CFLAGS += -fp:precise
    QMAKE_CXXFLAGS += -O2
    QMAKE_CXXFLAGS += -fp:precise

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
}

macx {
    DEFINES += MAC_SIM
}

unix:!macx {
    DEFINES += LIN_SIM
}

INCLUDEPATH += $$BOOST_INCLUDEPATH
INCLUDEPATH += $$LUA_INCLUDEPATH
INCLUDEPATH += $$EIGEN_INCLUDEPATH
LIBS += $$LUA_LIBS
INCLUDEPATH += $$QSCINTILLA_INCLUDEPATH # somehow required to avoid a crash on exit (Windows), when copy was used in the code editor...
LIBS += $$QSCINTILLA_LIBS # somehow required to avoid a crash on exit (Windows), when copy was used in the code editor...


INCLUDEPATH += $$PWD/"sourceCode"
INCLUDEPATH += $$PWD/"sourceCode/shared"
INCLUDEPATH += $$PWD/"sourceCode/shared/backwardCompatibility/kinematics"
INCLUDEPATH += $$PWD/"sourceCode/communication"
INCLUDEPATH += $$PWD/"sourceCode/communication/tubes"
INCLUDEPATH += $$PWD/"sourceCode/communication/wireless"
INCLUDEPATH += $$PWD/"sourceCode/drawingObjects"
INCLUDEPATH += $$PWD/"sourceCode/platform"
INCLUDEPATH += $$PWD/"sourceCode/collections"
INCLUDEPATH += $$PWD/"sourceCode/collisions"
INCLUDEPATH += $$PWD/"sourceCode/distances"
INCLUDEPATH += $$PWD/"sourceCode/pathPlanning_old"
INCLUDEPATH += $$PWD/"sourceCode/sceneObjects"
INCLUDEPATH += $$PWD/"sourceCode/shared/sceneObjects"
INCLUDEPATH += $$PWD/"sourceCode/sceneObjects/related"
INCLUDEPATH += $$PWD/"sourceCode/shared/sceneObjects/related"
INCLUDEPATH += $$PWD/"sourceCode/sceneObjects/graphObjectRelated"
INCLUDEPATH += $$PWD/"sourceCode/sceneObjects/pathObjectRelated"
INCLUDEPATH += $$PWD/"sourceCode/sceneObjects/proximitySensorObjectRelated"
INCLUDEPATH += $$PWD/"sourceCode/sceneObjects/shapeObjectRelated"
INCLUDEPATH += $$PWD/"sourceCode/sceneObjects/visionSensorObjectRelated"
INCLUDEPATH += $$PWD/"sourceCode/mainContainers"
INCLUDEPATH += $$PWD/"sourceCode/shared/mainContainers"
INCLUDEPATH += $$PWD/"sourceCode/mainContainers/sceneContainers"
INCLUDEPATH += $$PWD/"sourceCode/shared/mainContainers/sceneContainers"
INCLUDEPATH += $$PWD/"sourceCode/mainContainers/applicationContainers"
INCLUDEPATH += $$PWD/"sourceCode/scripting"
INCLUDEPATH += $$PWD/"sourceCode/scripting/customScriptFuncAndVar"
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
INCLUDEPATH += $$PWD/"sourceCode/shared/various"
INCLUDEPATH += $$PWD/"sourceCode/libsAndPlugins"
INCLUDEPATH += $$PWD/"sourceCode/visual"
INCLUDEPATH += $$PWD/"sourceCode/displ"
INCLUDEPATH += $$PWD/"sourceCode/utils"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/pathPlanning"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/collisions"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/distances"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/kinematics"
INCLUDEPATH += $$PWD/"sourceCode/customUserInterfaces"
INCLUDEPATH += $$PWD/"sourceCode/undoRedo"
INCLUDEPATH += $$PWD/"sourceCode/rendering"
INCLUDEPATH += $$PWD/"../programming/include"
INCLUDEPATH += $$PWD/"../programming/simMath"

WITH_SERIAL {
    INCLUDEPATH += $$PWD/"sourceCode/communication/serialPort"
}

WITH_GUI {
    INCLUDEPATH += $$PWD/"sourceCode/gui/qtPropertyBrowserRelated"
    INCLUDEPATH += $$PWD/"sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles"
    INCLUDEPATH += $$PWD/"sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserRelatedFiles"
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
    $$PWD/ui/qdlginertiatensor.ui \
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
    $$PWD/ui/qdlgdynamics.ui \
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
    $$PWD/ui/qdlgcalcdialogcontainer.ui \
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

HEADERS += $$PWD/../programming/simMath/mathDefines.h \
    $$PWD/../programming/simMath/Vector.h \
    $$PWD/../programming/simMath/MyMath.h \
    $$PWD/../programming/simMath/MMatrix.h \
    $$PWD/../programming/simMath/7Vector.h \
    $$PWD/../programming/simMath/6X6Matrix.h \
    $$PWD/../programming/simMath/6Vector.h \
    $$PWD/../programming/simMath/4X4Matrix.h \
    $$PWD/../programming/simMath/4X4FullMatrix.h \
    $$PWD/../programming/simMath/4Vector.h \
    $$PWD/../programming/simMath/3X3Matrix.h \
    $$PWD/../programming/simMath/3Vector.h \
    $$PWD/sourceCode/various/simMainHeader.h

HEADERS += $$PWD/../programming/include/simConst.h \
    $$PWD/../programming/include/simTypes.h \

HEADERS += $$PWD/sourceCode/backwardCompatibility/kinematics/ikGroup_old.h \
    $$PWD/sourceCode/shared/backwardCompatibility/kinematics/_ikGroup_old.h \
    $$PWD/sourceCode/backwardCompatibility/kinematics/ikElement_old.h \
    $$PWD/sourceCode/shared/backwardCompatibility/kinematics/_ikElement_old.h

HEADERS += $$PWD/sourceCode/drawingObjects/bannerObject.h \
    $$PWD/sourceCode/drawingObjects/drawingObject.h \
    $$PWD/sourceCode/drawingObjects/ptCloud_old.h \

HEADERS += $$PWD/sourceCode/platform/vVarious.h \
    $$PWD/sourceCode/platform/vThread.h \
    $$PWD/sourceCode/platform/vMutex.h \
    $$PWD/sourceCode/platform/vFileFinder.h \
    $$PWD/sourceCode/platform/vFile.h \
    $$PWD/sourceCode/platform/vDateTime.h \
    $$PWD/sourceCode/platform/vArchive.h

HEADERS += $$PWD/sourceCode/collections/collectionElement.h \
    $$PWD/sourceCode/collections/collection.h \

HEADERS += $$PWD/sourceCode/backwardCompatibility/collisions/collisionObject_old.h \
    $$PWD/sourceCode/collisions/collisionRoutines.h \

HEADERS += $$PWD/sourceCode/backwardCompatibility/distances/distanceObject_old.h \
    $$PWD/sourceCode/distances/distanceRoutines.h \

HEADERS += $$PWD/sourceCode/sceneObjects/related/sceneObject.h \
    $$PWD/sourceCode/shared/sceneObjects/related/_sceneObject_.h \
    $$PWD/sourceCode/sceneObjects/related/convexVolume.h \
    $$PWD/sourceCode/sceneObjects/related/viewableBase.h \

HEADERS += $$PWD/sourceCode/sceneObjects/jointObject.h \
    $$PWD/sourceCode/shared/sceneObjects/_jointObject_.h \
    $$PWD/sourceCode/sceneObjects/camera.h \
    $$PWD/sourceCode/sceneObjects/dummy.h \
    $$PWD/sourceCode/sceneObjects/octree.h \
    $$PWD/sourceCode/sceneObjects/pointCloud.h \
    $$PWD/sourceCode/sceneObjects/forceSensor.h \
    $$PWD/sourceCode/sceneObjects/graph.h \
    $$PWD/sourceCode/sceneObjects/light.h \
    $$PWD/sourceCode/sceneObjects/mill.h \
    $$PWD/sourceCode/sceneObjects/mirror.h \
    $$PWD/sourceCode/sceneObjects/shape.h \
    $$PWD/sourceCode/sceneObjects/path_old.h \
    $$PWD/sourceCode/sceneObjects/proximitySensor.h \
    $$PWD/sourceCode/sceneObjects/visionSensor.h \

HEADERS += $$PWD/sourceCode/sceneObjects/graphObjectRelated/graphCurve.h \
    $$PWD/sourceCode/sceneObjects/graphObjectRelated/graphDataStream.h \
    $$PWD/sourceCode/sceneObjects/graphObjectRelated/graphingRoutines_old.h \
    $$PWD/sourceCode/sceneObjects/graphObjectRelated/graphDataComb_old.h \
    $$PWD/sourceCode/sceneObjects/graphObjectRelated/graphData_old.h \
    $$PWD/sourceCode/sceneObjects/graphObjectRelated/staticGraphCurve_old.h \

HEADERS += $$PWD/sourceCode/sceneObjects/pathObjectRelated/bezierPathPoint_old.h \
    $$PWD/sourceCode/sceneObjects/pathObjectRelated/simplePathPoint_old.h \
    $$PWD/sourceCode/sceneObjects/pathObjectRelated/pathPoint_old.h \
    $$PWD/sourceCode/sceneObjects/pathObjectRelated/pathCont_old.h \

HEADERS += $$PWD/sourceCode/sceneObjects/shapeObjectRelated/mesh.h \
    $$PWD/sourceCode/sceneObjects/shapeObjectRelated/meshWrapper.h \
    $$PWD/sourceCode/sceneObjects/shapeObjectRelated/volInt.h \

HEADERS += $$PWD/sourceCode/backwardCompatibility/pathPlanning/pathPlanning_old.h \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/holonomicPathPlanning_old.h \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/holonomicPathNode_old.h \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathPlanning_old.h \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathNode_old.h \

HEADERS += $$PWD/sourceCode/communication/tubes/commTube.h \

HEADERS += $$PWD/sourceCode/communication/wireless/broadcastDataContainer.h \
    $$PWD/sourceCode/communication/wireless/broadcastData.h \
    $$PWD/sourceCode/communication/wireless/broadcastDataVisual.h \

HEADERS += $$PWD/sourceCode/mainContainers/worldContainer.h \
    $$PWD/sourceCode/shared/mainContainers/_worldContainer_.h \
    $$PWD/sourceCode/mainContainers/world.h \
    $$PWD/sourceCode/shared/mainContainers/_world_.h \

HEADERS += $$PWD/sourceCode/mainContainers/sceneContainers/drawingContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/bannerContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/textureContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/signalContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/simulation.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/registeredPathPlanningTasks.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/ikGroupContainer.h \
    $$PWD/sourceCode/shared/mainContainers/sceneContainers/_ikGroupContainer_.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/collectionContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/distanceObjectContainer_old.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/collisionObjectContainer_old.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/sceneObjectContainer.h \
    $$PWD/sourceCode/shared/mainContainers/sceneContainers/_sceneObjectContainer_.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/mainSettings.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/embeddedScriptContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/environment.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/dynamicsContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/commTubeContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/apiErrors.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/outsideCommandQueue.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/cacheCont.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/customData.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/customData_old.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/ghostObjectContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/pointCloudContainer_old.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/buttonBlockContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/undoBufferCont.h \

HEADERS += $$PWD/sourceCode/mainContainers/applicationContainers/copyBuffer.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/persistentDataContainer.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/simulatorMessageQueue.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/calculationInfo.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/interfaceStackContainer.h \
    $$PWD/sourceCode/mainContainers/applicationContainers/addOnScriptContainer.h \

HEADERS += $$PWD/sourceCode/sceneObjects/visionSensorObjectRelated/simpleFilter.h \
    $$PWD/sourceCode/sceneObjects/visionSensorObjectRelated/composedFilter.h \

HEADERS += $$PWD/sourceCode/pathPlanning_old/pathPlanningTask_old.h \

HEADERS += $$PWD/sourceCode/scripting/userParameters.h \
    $$PWD/sourceCode/scripting/scriptObject.h \
    $$PWD/sourceCode/scripting/outsideCommandQueueForScript.h \
    $$PWD/sourceCode/scripting/luaWrapper.h \

HEADERS += $$PWD/sourceCode/scripting/customScriptFuncAndVar/scriptCustomFuncAndVarContainer.h \
    $$PWD/sourceCode/scripting/customScriptFuncAndVar/scriptCustomFunction.h \
    $$PWD/sourceCode/scripting/customScriptFuncAndVar/scriptCustomVariable.h \

HEADERS += $$PWD/sourceCode/pagesAndViews/pageContainer.h \
    $$PWD/sourceCode/pagesAndViews/sPage.h \
    $$PWD/sourceCode/pagesAndViews/sView.h \

HEADERS += $$PWD/sourceCode/textures/textureObject.h \
    $$PWD/sourceCode/textures/imgLoaderSaver.h \
    $$PWD/sourceCode/textures/tGAFormat.h \
    $$PWD/sourceCode/textures/stb_image.h \
    $$PWD/sourceCode/textures/textureProperty.h \

HEADERS += $$PWD/sourceCode/serialization/ser.h \
    $$PWD/sourceCode/serialization/extIkSer.h \
    $$PWD/sourceCode/serialization/huffman.h \
    $$PWD/sourceCode/serialization/tinyxml2.cpp \

HEADERS += $$PWD/sourceCode/strings/simStringTable.h \
    $$PWD/sourceCode/strings/simStringTable_openGl.h \
    $$PWD/sourceCode/strings/simStringTable_noTranslation.h \

HEADERS += $$PWD/sourceCode/interfaces/sim.h \
    $$PWD/sourceCode/interfaces/simInternal.h \
    $$PWD/sourceCode/interfaces/luaScriptFunctions.h \
    $$PWD/sourceCode/interfaces/pathPlanningInterface.h \
    $$PWD/sourceCode/interfaces/dummyClasses.h \
    $$PWD/sourceCode/interfaces/simFlavor.h \
    
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

HEADERS += $$PWD/sourceCode/geometricAlgorithms/linMotionRoutines.h \
    $$PWD/sourceCode/geometricAlgorithms/meshRoutines.h \
    $$PWD/sourceCode/geometricAlgorithms/meshManip.h \
    $$PWD/sourceCode/geometricAlgorithms/edgeElement.h \
    $$PWD/sourceCode/geometricAlgorithms/algos.h \

HEADERS += $$PWD/sourceCode/various/simConfig.h \
    $$PWD/sourceCode/various/gV.h \
    $$PWD/sourceCode/various/global.h \
    $$PWD/sourceCode/various/embeddedFonts.h \
    $$PWD/sourceCode/various/userSettings.h \
    $$PWD/sourceCode/various/memorizedConf.h \
    $$PWD/sourceCode/various/uiThread.h \
    $$PWD/sourceCode/various/simThread.h \
    $$PWD/sourceCode/various/app.h \
    $$PWD/sourceCode/various/folderSystem.h \
    $$PWD/sourceCode/various/dynMaterialObject.h \
    $$PWD/sourceCode/various/easyLock.h \
    $$PWD/sourceCode/various/ghostObject.h \
    $$PWD/sourceCode/various/sigHandler.h \
    $$PWD/sourceCode/various/syncObject.h \
    $$PWD/sourceCode/shared/various/_syncObject_.h \

HEADERS += $$PWD/sourceCode/undoRedo/undoBufferArrays.h \
    $$PWD/sourceCode/undoRedo/undoBuffer.h \
    $$PWD/sourceCode/undoRedo/undoBufferCameras.h \

HEADERS += $$PWD/sourceCode/rendering/rendering.h \
    $$PWD/sourceCode/rendering/cameraRendering.h \
    $$PWD/sourceCode/rendering/visionSensorRendering.h \
    $$PWD/sourceCode/rendering/forceSensorRendering.h \
    $$PWD/sourceCode/rendering/proximitySensorRendering.h \
    $$PWD/sourceCode/rendering/dummyRendering.h \
    $$PWD/sourceCode/rendering/lightRendering.h \
    $$PWD/sourceCode/rendering/pointCloudRendering.h \
    $$PWD/sourceCode/rendering/octreeRendering.h \
    $$PWD/sourceCode/rendering/jointRendering.h \
    $$PWD/sourceCode/rendering/graphRendering.h \
    $$PWD/sourceCode/rendering/millRendering.h \
    $$PWD/sourceCode/rendering/mirrorRendering.h \
    $$PWD/sourceCode/rendering/pathRendering.h \
    $$PWD/sourceCode/rendering/shapeRendering.h \
    $$PWD/sourceCode/rendering/ghostRendering.h \
    $$PWD/sourceCode/rendering/drawingObjectRendering.h \
    $$PWD/sourceCode/rendering/ptCloudRendering_old.h \
    $$PWD/sourceCode/rendering/collisionContourRendering.h \
    $$PWD/sourceCode/rendering/distanceRendering.h \
    $$PWD/sourceCode/rendering/bannerRendering.h \
    $$PWD/sourceCode/rendering/thumbnailRendering.h \
    $$PWD/sourceCode/rendering/pathPlanningTaskRendering_old.h \
    $$PWD/sourceCode/rendering/broadcastDataVisualRendering.h \
    $$PWD/sourceCode/rendering/dynamicsRendering.h \
    $$PWD/sourceCode/rendering/environmentRendering.h \
    $$PWD/sourceCode/rendering/pageRendering.h \
    $$PWD/sourceCode/rendering/viewRendering.h \

HEADERS += $$PWD/sourceCode/libsAndPlugins/pluginContainer.h \

HEADERS += $$PWD/sourceCode/visual/thumbnail.h \

HEADERS += $$PWD/sourceCode/displ/colorObject.h \

HEADERS += $$PWD/sourceCode/utils/threadPool_old.h \
    $$PWD/sourceCode/utils/tt.h \
    $$PWD/sourceCode/utils/ttUtil.h \
    $$PWD/sourceCode/utils/confReaderAndWriter.h \
    $$PWD/sourceCode/utils/base64.h \
    $$PWD/sourceCode/utils/cbor.h \

HEADERS += $$PWD/sourceCode/customUserInterfaces/buttonBlock.h \
    $$PWD/sourceCode/customUserInterfaces/softButton.h \

WITH_QT {
    HEADERS += $$PWD/sourceCode/platform/vSimUiMutex.h \
        $$PWD/sourceCode/platform/wThread.h

    HEADERS += $$PWD/sourceCode/various/simAndUiThreadSync.h \
        $$PWD/sourceCode/various/simQApp.h
}

WITH_SERIAL {
    HEADERS += $$PWD/sourceCode/mainContainers/applicationContainers/serialPortContainer.h

    HEADERS += $$PWD/sourceCode/communication/serialPort/serialPort.h \
        $$PWD/sourceCode/communication/serialPort/serial_win_mac_linux.h \
        $$PWD/sourceCode/communication/serialPort/serialPortWin.h
}

WITH_OPENGL {
    HEADERS += $$PWD/sourceCode/sceneObjects/visionSensorObjectRelated/offscreenGlContext.h \
        $$PWD/sourceCode/sceneObjects/visionSensorObjectRelated/frameBufferObject.h \
        $$PWD/sourceCode/sceneObjects/visionSensorObjectRelated/visionSensorGlStuff.h

    HEADERS += $$PWD/sourceCode/visual/oGL.h \
        $$PWD/sourceCode/visual/oglExt.h \
        $$PWD/sourceCode/visual/glShader.h \
        $$PWD/sourceCode/visual/glBufferObjects.h \
}

WITH_GUI {
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
        $$PWD/sourceCode/gui/dialogs/qdlginertiatensor.h \
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
        $$PWD/sourceCode/gui/dialogs/qdlgdynamics.h \
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
        $$PWD/sourceCode/gui/dialogs/qdlgcalcdialogcontainer.h \
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

    HEADERS += $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtbuttonpropertybrowser.h \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qteditorfactory.h \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtgroupboxpropertybrowser.h \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtpropertybrowser.h \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtpropertybrowserutils_p.h \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtpropertymanager.h \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qttreepropertybrowser.h \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtvariantproperty.h \

    HEADERS += $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserRelatedFiles/buttoneditfactory.h \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserRelatedFiles/buttoneditmanager.h \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserRelatedFiles/pushbuttonedit.h \

    HEADERS += $$PWD/sourceCode/gui/qtPropertyBrowserRelated/propBrowser_engineProp_material.h \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/propBrowser_engineProp_general.h \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/propBrowser_engineProp_joint.h \

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

    HEADERS += $$PWD/sourceCode/gui/libs/auxLibVideo.h \
}

SOURCES += $$PWD/../programming/simMath/Vector.cpp \
    $$PWD/../programming/simMath/MyMath.cpp \
    $$PWD/../programming/simMath/MMatrix.cpp \
    $$PWD/../programming/simMath/7Vector.cpp \
    $$PWD/../programming/simMath/6X6Matrix.cpp \
    $$PWD/../programming/simMath/6Vector.cpp \
    $$PWD/../programming/simMath/4X4Matrix.cpp \
    $$PWD/../programming/simMath/4X4FullMatrix.cpp \
    $$PWD/../programming/simMath/4Vector.cpp \
    $$PWD/../programming/simMath/3X3Matrix.cpp \
    $$PWD/../programming/simMath/3Vector.cpp \

SOURCES += $$PWD/sourceCode/backwardCompatibility/kinematics/ikGroup_old.cpp \
    $$PWD/sourceCode/shared/backwardCompatibility/kinematics/_ikGroup_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/kinematics/ikElement_old.cpp \
    $$PWD/sourceCode/shared/backwardCompatibility/kinematics/_ikElement_old.cpp

SOURCES += $$PWD/sourceCode/drawingObjects/bannerObject.cpp \
    $$PWD/sourceCode/drawingObjects/drawingObject.cpp \
    $$PWD/sourceCode/drawingObjects/ptCloud_old.cpp \

SOURCES += $$PWD/sourceCode/platform/vVarious.cpp \
    $$PWD/sourceCode/platform/vThread.cpp \
    $$PWD/sourceCode/platform/vMutex.cpp \
    $$PWD/sourceCode/platform/vFileFinder.cpp \
    $$PWD/sourceCode/platform/vFile.cpp \
    $$PWD/sourceCode/platform/vDateTime.cpp \
    $$PWD/sourceCode/platform/vArchive.cpp

SOURCES += $$PWD/sourceCode/collections/collectionElement.cpp \
    $$PWD/sourceCode/collections/collection.cpp \

SOURCES += $$PWD/sourceCode/backwardCompatibility/collisions/collisionObject_old.cpp \
    $$PWD/sourceCode/collisions/collisionRoutines.cpp \

SOURCES += $$PWD/sourceCode/backwardCompatibility/distances/distanceObject_old.cpp \
    $$PWD/sourceCode/distances/distanceRoutines.cpp \

SOURCES += $$PWD/sourceCode/sceneObjects/related/sceneObject.cpp \
    $$PWD/sourceCode/shared/sceneObjects/related/_sceneObject_.cpp \
    $$PWD/sourceCode/sceneObjects/related/convexVolume.cpp \
    $$PWD/sourceCode/sceneObjects/related/viewableBase.cpp \

SOURCES += $$PWD/sourceCode/sceneObjects/jointObject.cpp \
    $$PWD/sourceCode/shared/sceneObjects/_jointObject_.cpp \
    $$PWD/sourceCode/sceneObjects/camera.cpp \
    $$PWD/sourceCode/sceneObjects/dummy.cpp \
    $$PWD/sourceCode/sceneObjects/octree.cpp \
    $$PWD/sourceCode/sceneObjects/pointCloud.cpp \
    $$PWD/sourceCode/sceneObjects/forceSensor.cpp \
    $$PWD/sourceCode/sceneObjects/graph.cpp \
    $$PWD/sourceCode/sceneObjects/light.cpp \
    $$PWD/sourceCode/sceneObjects/mirror.cpp \
    $$PWD/sourceCode/sceneObjects/mill.cpp \
    $$PWD/sourceCode/sceneObjects/path_old.cpp \
    $$PWD/sourceCode/sceneObjects/proximitySensor.cpp \
    $$PWD/sourceCode/sceneObjects/shape.cpp \
    $$PWD/sourceCode/sceneObjects/visionSensor.cpp \

SOURCES += $$PWD/sourceCode/sceneObjects/graphObjectRelated/graphCurve.cpp \
    $$PWD/sourceCode/sceneObjects/graphObjectRelated/graphDataStream.cpp \
    $$PWD/sourceCode/sceneObjects/graphObjectRelated/graphingRoutines_old.cpp \
    $$PWD/sourceCode/sceneObjects/graphObjectRelated/graphDataComb_old.cpp \
    $$PWD/sourceCode/sceneObjects/graphObjectRelated/graphData_old.cpp \
    $$PWD/sourceCode/sceneObjects/graphObjectRelated/staticGraphCurve_old.cpp \

SOURCES += $$PWD/sourceCode/sceneObjects/pathObjectRelated/bezierPathPoint_old.cpp \
    $$PWD/sourceCode/sceneObjects/pathObjectRelated/simplePathPoint_old.cpp \
    $$PWD/sourceCode/sceneObjects/pathObjectRelated/pathPoint_old.cpp \
    $$PWD/sourceCode/sceneObjects/pathObjectRelated/pathCont_old.cpp \

SOURCES += $$PWD/sourceCode/sceneObjects/proximitySensorObjectRelated/proxSensorRoutine.cpp \

SOURCES += $$PWD/sourceCode/sceneObjects/shapeObjectRelated/mesh.cpp \
    $$PWD/sourceCode/sceneObjects/shapeObjectRelated/meshWrapper.cpp \
    $$PWD/sourceCode/sceneObjects/shapeObjectRelated/volInt.cpp \


SOURCES += $$PWD/sourceCode/backwardCompatibility/pathPlanning/pathPlanning_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/holonomicPathPlanning_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/holonomicPathNode_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathPlanning_old.cpp \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathNode_old.cpp \

SOURCES += $$PWD/sourceCode/communication/tubes/commTube.cpp \

SOURCES += $$PWD/sourceCode/communication/wireless/broadcastDataContainer.cpp \
    $$PWD/sourceCode/communication/wireless/broadcastData.cpp \
    $$PWD/sourceCode/communication/wireless/broadcastDataVisual.cpp \

SOURCES += $$PWD/sourceCode/mainContainers/worldContainer.cpp \
    $$PWD/sourceCode/shared/mainContainers/_worldContainer_.cpp \
    $$PWD/sourceCode/mainContainers/world.cpp \
    $$PWD/sourceCode/shared/mainContainers/_world_.cpp \

SOURCES += $$PWD/sourceCode/mainContainers/sceneContainers/bannerContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/drawingContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/textureContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/simulation.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/signalContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/registeredPathPlanningTasks.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/ikGroupContainer.cpp \
    $$PWD/sourceCode/shared/mainContainers/sceneContainers/_ikGroupContainer_.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/collectionContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/distanceObjectContainer_old.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/collisionObjectContainer_old.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/outsideCommandQueue.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/sceneObjectContainer.cpp \
    $$PWD/sourceCode/shared/mainContainers/sceneContainers/_sceneObjectContainer_.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/mainSettings.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/embeddedScriptContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/environment.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/dynamicsContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/customData.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/customData_old.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/commTubeContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/cacheCont.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/apiErrors.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/ghostObjectContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/pointCloudContainer_old.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/buttonBlockContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/undoBufferCont.cpp \

SOURCES += $$PWD/sourceCode/mainContainers/applicationContainers/copyBuffer.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/persistentDataContainer.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/simulatorMessageQueue.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/calculationInfo.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/interfaceStackContainer.cpp \
    $$PWD/sourceCode/mainContainers/applicationContainers/addOnScriptContainer.cpp \

SOURCES += $$PWD/sourceCode/sceneObjects/visionSensorObjectRelated/simpleFilter.cpp \
    $$PWD/sourceCode/sceneObjects/visionSensorObjectRelated/composedFilter.cpp \

SOURCES += $$PWD/sourceCode/pathPlanning_old/pathPlanningTask_old.cpp \

SOURCES += $$PWD/sourceCode/scripting/userParameters.cpp \
    $$PWD/sourceCode/scripting/scriptObject.cpp \
    $$PWD/sourceCode/scripting/outsideCommandQueueForScript.cpp \
    $$PWD/sourceCode/scripting/luaWrapper.cpp \

SOURCES += $$PWD/sourceCode/scripting/customScriptFuncAndVar/scriptCustomFuncAndVarContainer.cpp \
    $$PWD/sourceCode/scripting/customScriptFuncAndVar/scriptCustomFunction.cpp \
    $$PWD/sourceCode/scripting/customScriptFuncAndVar/scriptCustomVariable.cpp \

SOURCES += $$PWD/sourceCode/pagesAndViews/pageContainer.cpp \
    $$PWD/sourceCode/pagesAndViews/sPage.cpp \
    $$PWD/sourceCode/pagesAndViews/sView.cpp \

SOURCES += $$PWD/sourceCode/textures/textureObject.cpp \
    $$PWD/sourceCode/textures/imgLoaderSaver.cpp \
    $$PWD/sourceCode/textures/tGAFormat.cpp \
    $$PWD/sourceCode/textures/stb_image.c \
    $$PWD/sourceCode/textures/textureProperty.cpp \

SOURCES += $$PWD/sourceCode/serialization/ser.cpp \
    $$PWD/sourceCode/serialization/extIkSer.cpp \
    $$PWD/sourceCode/serialization/huffman.c \
    $$PWD/sourceCode/serialization/tinyxml2.cpp \

SOURCES += $$PWD/sourceCode/interfaces/sim.cpp \
    $$PWD/sourceCode/interfaces/simInternal.cpp \
    $$PWD/sourceCode/interfaces/luaScriptFunctions.cpp \
    $$PWD/sourceCode/interfaces/pathPlanningInterface.cpp \

SOURCES += $$PWD/sourceCode/interfaces/simFlavor.cpp

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

SOURCES += $$PWD/sourceCode/geometricAlgorithms/linMotionRoutines.cpp \
    $$PWD/sourceCode/geometricAlgorithms/meshRoutines.cpp \
    $$PWD/sourceCode/geometricAlgorithms/meshManip.cpp \
    $$PWD/sourceCode/geometricAlgorithms/edgeElement.cpp \
    $$PWD/sourceCode/geometricAlgorithms/algos.cpp \

SOURCES += $$PWD/sourceCode/various/gV.cpp \
    $$PWD/sourceCode/various/memorizedConf.cpp \
    $$PWD/sourceCode/various/userSettings.cpp \
    $$PWD/sourceCode/various/folderSystem.cpp \
    $$PWD/sourceCode/various/uiThread.cpp \
    $$PWD/sourceCode/various/simThread.cpp \
    $$PWD/sourceCode/various/app.cpp \
    $$PWD/sourceCode/various/dynMaterialObject.cpp \
    $$PWD/sourceCode/various/easyLock.cpp \
    $$PWD/sourceCode/various/ghostObject.cpp \
    $$PWD/sourceCode/various/sigHandler.cpp \
    $$PWD/sourceCode/various/syncObject.cpp \
    $$PWD/sourceCode/shared/various/_syncObject_.cpp \

SOURCES += $$PWD/sourceCode/undoRedo/undoBufferArrays.cpp \
    $$PWD/sourceCode/undoRedo/undoBuffer.cpp \
    $$PWD/sourceCode/undoRedo/undoBufferCameras.cpp \

SOURCES += $$PWD/sourceCode/rendering/rendering.cpp \
    $$PWD/sourceCode/rendering/cameraRendering.cpp \
    $$PWD/sourceCode/rendering/visionSensorRendering.cpp \
    $$PWD/sourceCode/rendering/forceSensorRendering.cpp \
    $$PWD/sourceCode/rendering/proximitySensorRendering.cpp \
    $$PWD/sourceCode/rendering/dummyRendering.cpp \
    $$PWD/sourceCode/rendering/lightRendering.cpp \
    $$PWD/sourceCode/rendering/pointCloudRendering.cpp \
    $$PWD/sourceCode/rendering/octreeRendering.cpp \
    $$PWD/sourceCode/rendering/jointRendering.cpp \
    $$PWD/sourceCode/rendering/graphRendering.cpp \
    $$PWD/sourceCode/rendering/millRendering.cpp \
    $$PWD/sourceCode/rendering/mirrorRendering.cpp \
    $$PWD/sourceCode/rendering/pathRendering.cpp \
    $$PWD/sourceCode/rendering/shapeRendering.cpp \
    $$PWD/sourceCode/rendering/ghostRendering.cpp \
    $$PWD/sourceCode/rendering/drawingObjectRendering.cpp \
    $$PWD/sourceCode/rendering/ptCloudRendering_old.cpp \
    $$PWD/sourceCode/rendering/collisionContourRendering.cpp \
    $$PWD/sourceCode/rendering/distanceRendering.cpp \
    $$PWD/sourceCode/rendering/bannerRendering.cpp \
    $$PWD/sourceCode/rendering/thumbnailRendering.cpp \
    $$PWD/sourceCode/rendering/pathPlanningTaskRendering_old.cpp \
    $$PWD/sourceCode/rendering/broadcastDataVisualRendering.cpp \
    $$PWD/sourceCode/rendering/dynamicsRendering.cpp \
    $$PWD/sourceCode/rendering/environmentRendering.cpp \
    $$PWD/sourceCode/rendering/pageRendering.cpp \
    $$PWD/sourceCode/rendering/viewRendering.cpp \

SOURCES += $$PWD/sourceCode/libsAndPlugins/pluginContainer.cpp \

SOURCES += $$PWD/sourceCode/displ/colorObject.cpp \

SOURCES += $$PWD/sourceCode/visual/thumbnail.cpp \

SOURCES += $$PWD/sourceCode/utils/threadPool_old.cpp \
    $$PWD/sourceCode/utils/ttUtil.cpp \
    $$PWD/sourceCode/utils/tt.cpp \
    $$PWD/sourceCode/utils/confReaderAndWriter.cpp \
    $$PWD/sourceCode/utils/base64.cpp \
    $$PWD/sourceCode/utils/cbor.cpp \

SOURCES += $$PWD/sourceCode/customUserInterfaces/buttonBlock.cpp \
    $$PWD/sourceCode/customUserInterfaces/softButton.cpp \

WITH_QT {
    SOURCES += $$PWD/sourceCode/platform/vSimUiMutex.cpp \
        $$PWD/sourceCode/platform/wThread.cpp

    SOURCES += $$PWD/sourceCode/various/simAndUiThreadSync.cpp \
        $$PWD/sourceCode/various/simQApp.cpp
}

WITH_SERIAL {
    SOURCES += $$PWD/sourceCode/mainContainers/applicationContainers/serialPortContainer.cpp

    SOURCES += $$PWD/sourceCode/communication/serialPort/serialPort.cpp \
        $$PWD/sourceCode/communication/serialPort/serial_win_mac_linux.cpp \
        $$PWD/sourceCode/communication/serialPort/serialPortWin.cpp
}

WITH_OPENGL {
    SOURCES += $$PWD/sourceCode/sceneObjects/visionSensorObjectRelated/offscreenGlContext.cpp \
        $$PWD/sourceCode/sceneObjects/visionSensorObjectRelated/frameBufferObject.cpp \
        $$PWD/sourceCode/sceneObjects/visionSensorObjectRelated/visionSensorGlStuff.cpp \

    SOURCES += $$PWD/sourceCode/visual/oGL.cpp \
        $$PWD/sourceCode/visual/oglExt.cpp \
        $$PWD/sourceCode/visual/glShader.cpp \
        $$PWD/sourceCode/visual/glBufferObjects.cpp \
}

WITH_GUI {
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
        $$PWD/sourceCode/gui/dialogs/qdlginertiatensor.cpp \
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
        $$PWD/sourceCode/gui/dialogs/qdlgdynamics.cpp \
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
        $$PWD/sourceCode/gui/dialogs/qdlgcalcdialogcontainer.cpp \
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

    SOURCES += $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtbuttonpropertybrowser.cpp \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qteditorfactory.cpp \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtgroupboxpropertybrowser.cpp \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtpropertybrowser.cpp \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtpropertybrowserutils.cpp \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtpropertymanager.cpp \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qttreepropertybrowser.cpp \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserFiles/qtvariantproperty.cpp \

    SOURCES += $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserRelatedFiles/buttoneditfactory.cpp \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserRelatedFiles/buttoneditmanager.cpp \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/qtPropertyBrowserRelatedFiles/pushbuttonedit.cpp \

    SOURCES += $$PWD/sourceCode/gui/qtPropertyBrowserRelated/propBrowser_engineProp_material.cpp \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/propBrowser_engineProp_general.cpp \
        $$PWD/sourceCode/gui/qtPropertyBrowserRelated/propBrowser_engineProp_joint.cpp \

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
