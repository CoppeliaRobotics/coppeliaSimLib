TARGET = v_rep
TEMPLATE = lib

CONFIG += shared debug_and_release
!HEADLESS_TEST {
    CONFIG += WITH_GUI
    CONFIG += WITH_OPENGL # comment only if above line is commented
    CONFIG += WITH_QT # comment only if above 2 lines are commented. Without Qt uses some sub-optimal routines for now, check TODO_SIM_WITHOUT_QT_AT_ALL
    CONFIG += WITH_SERIAL
}
CONFIG(debug,debug|release) {
    CONFIG += force_debug_info
}

QT += printsupport

WITH_GUI {
    QT      += widgets
    DEFINES += SIM_WITH_GUI
    DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
    RESOURCES += $$PWD/targaFiles.qrc
    RESOURCES += $$PWD/toolbarFiles.qrc
    RESOURCES += $$PWD/variousImageFiles.qrc
    RESOURCES += $$PWD/iconFiles.qrc
    RESOURCES += $$PWD/imageFiles_vrep.qrc
}

WITH_OPENGL {
    QT      += opengl
    #QT      += 3dcore 3drender 3dinput 3dextras
    DEFINES += SIM_WITH_OPENGL
} else {
    QT -= gui
}

WITH_SERIAL {
    QT      += serialport
    DEFINES += SIM_WITH_SERIAL
}

!WITH_QT {
    DEFINES += SIM_WITHOUT_QT_AT_ALL
    QT -= core
}

*-msvc* {
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
        DEFINES += VREP_COMPILER_STR=\\\"MSVC2015\\\"
    }
    equals(MSVC_VER,15.0){
        DEFINES += VREP_COMPILER_STR=\\\"MSVC2017\\\"
    }
}

*-g++* { #includes MinGW
    CONFIG(debug,debug|release) {
        QMAKE_CXXFLAGS += -g -ggdb
    } else {
        QMAKE_CFLAGS += -O3
        QMAKE_CXXFLAGS += -O3
    }

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

    DEFINES += VREP_COMPILER_STR=\\\"GCC\\\"
}

clang* {
    CONFIG(debug,debug|release) {
        QMAKE_CXXFLAGS += -g -ggdb
    } else {
        QMAKE_CFLAGS += -O3
        QMAKE_CXXFLAGS += -O3
    }

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

    DEFINES += VREP_COMPILER_STR=\\\"Clang\\\"
}

win32 {
    LIBS += -lwinmm
    LIBS += -lShell32
    LIBS += -lopengl32
    LIBS += -luser32
    LIBS += -lDbghelp
    DEFINES += WIN_VREP
}

macx {
    DEFINES += MAC_VREP
}

unix:!macx {
    DEFINES += LIN_VREP
}


INCLUDEPATH += $$PWD/"sourceCode"
INCLUDEPATH += $$PWD/"sourceCode/inverseKinematics/geomConstraintSolver"
INCLUDEPATH += $$PWD/"sourceCode/inverseKinematics/ik"
INCLUDEPATH += $$PWD/"sourceCode/shared/sharedBufferFunctions"
INCLUDEPATH += $$PWD/"sourceCode/communication"
INCLUDEPATH += $$PWD/"sourceCode/communication/tubes"
INCLUDEPATH += $$PWD/"sourceCode/communication/wireless"
INCLUDEPATH += $$PWD/"sourceCode/drawingObjects"
INCLUDEPATH += $$PWD/"sourceCode/platform"
INCLUDEPATH += $$PWD/"sourceCode/collections"
INCLUDEPATH += $$PWD/"sourceCode/collisions"
INCLUDEPATH += $$PWD/"sourceCode/distances"
INCLUDEPATH += $$PWD/"sourceCode/pathPlanning_old"
INCLUDEPATH += $$PWD/"sourceCode/motionPlanning_old"
INCLUDEPATH += $$PWD/"sourceCode/3dObjects"
INCLUDEPATH += $$PWD/"sourceCode/3dObjects/related"
INCLUDEPATH += $$PWD/"sourceCode/3dObjects/graphObjectRelated"
INCLUDEPATH += $$PWD/"sourceCode/3dObjects/millObjectRelated"
INCLUDEPATH += $$PWD/"sourceCode/3dObjects/pathObjectRelated"
INCLUDEPATH += $$PWD/"sourceCode/3dObjects/proximitySensorObjectRelated"
INCLUDEPATH += $$PWD/"sourceCode/3dObjects/shapeObjectRelated"
INCLUDEPATH += $$PWD/"sourceCode/3dObjects/visionSensorObjectRelated"
INCLUDEPATH += $$PWD/"sourceCode/mainContainers"
INCLUDEPATH += $$PWD/"sourceCode/mainContainers/sceneContainers"
INCLUDEPATH += $$PWD/"sourceCode/mainContainers/applicationContainers"
INCLUDEPATH += $$PWD/"sourceCode/luaScripting"
INCLUDEPATH += $$PWD/"sourceCode/luaScripting/customLuaFuncAndVar"
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
INCLUDEPATH += $$PWD/"sourceCode/libsAndPlugins"
INCLUDEPATH += $$PWD/"sourceCode/visual"
INCLUDEPATH += $$PWD/"sourceCode/utils"
INCLUDEPATH += $$PWD/"sourceCode/sharedBufferFunctions"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/pathPlanning"
INCLUDEPATH += $$PWD/"sourceCode/backwardCompatibility/motionPlanning"
INCLUDEPATH += $$PWD/"sourceCode/customUserInterfaces"
INCLUDEPATH += $$PWD/"sourceCode/undoRedo"
INCLUDEPATH += $$PWD/"sourceCode/rendering"
INCLUDEPATH += $$PWD/"../programming/include"
INCLUDEPATH += $$PWD/"../programming/v_repMath"

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

WITH_GUI {
    FORMS += $$PWD/ui/qdlgsettings.ui \
    $$PWD/ui/qdlglayers.ui \
    $$PWD/ui/qdlgavirecorder.ui \
    $$PWD/ui/qdlgselection.ui \
    $$PWD/ui/qdlgabout.ui \
    $$PWD/ui/qdlgsimulation.ui \
    $$PWD/ui/qdlginteractiveik.ui \
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
    $$PWD/ui/qdlginsertscript.ui \
    $$PWD/ui/qdlgdynamics.ui \
    $$PWD/ui/qdlgscaling.ui \
    $$PWD/ui/qdlgcameras.ui \
    $$PWD/ui/qdlglights.ui \
    $$PWD/ui/qdlgmirrors.ui \
    $$PWD/ui/qdlgdummies.ui \
    $$PWD/ui/qdlgoctrees.ui \
    $$PWD/ui/qdlgpointclouds.ui \
    $$PWD/ui/qdlgscriptparameters.ui \
    $$PWD/ui/qdlgforcesensors.ui \
    $$PWD/ui/qdlgprimitives.ui \
    $$PWD/ui/qdlgconvexdecomposition.ui \
    $$PWD/ui/qdlgconstraintsolver.ui \
    $$PWD/ui/qdlgvisionsensors.ui \
    $$PWD/ui/qdlgimagecolor.ui \
    $$PWD/ui/qdlgsimpleSearch.ui \
    $$PWD/ui/qdlgfilters.ui \
    $$PWD/ui/qdlgsimplefilter_rotate.ui \
    $$PWD/ui/qdlgsimplefilter_uniformImage.ui \
    $$PWD/ui/qdlgsimplefilter_shift.ui \
    $$PWD/ui/qdlgsimplefilter_scaleCols.ui \
    $$PWD/ui/qdlgsimplefilter_resize.ui \
    $$PWD/ui/qdlgsimplefilter_coordExtraction.ui \
    $$PWD/ui/qdlgsimplefilter_velodyne.ui \
    $$PWD/ui/qdlgsimplefilter_pixelChange.ui \
    $$PWD/ui/qdlgsimplefilter_rectangularCut.ui \
    $$PWD/ui/qdlgsimplefilter_keepRemoveCols.ui \
    $$PWD/ui/qdlgsimplefilter_intensityScale.ui \
    $$PWD/ui/qdlgsimplefilter_edge.ui \
    $$PWD/ui/qdlgsimplefilter_correlation.ui \
    $$PWD/ui/qdlgsimplefilter_colorSegmentation.ui \
    $$PWD/ui/qdlgsimplefilter_circularCut.ui \
    $$PWD/ui/qdlgsimplefilter_blob.ui \
    $$PWD/ui/qdlgsimplefilter_binary.ui \
    $$PWD/ui/qdlgsimplefilter_5x5.ui \
    $$PWD/ui/qdlgsimplefilter_3x3.ui \
    $$PWD/ui/qdlgshapes.ui \
    $$PWD/ui/qdlgmultishapeedition.ui \
    $$PWD/ui/qdlgtextures.ui \
    $$PWD/ui/qdlgtextureselection.ui \
    $$PWD/ui/qdlggeometry.ui \
    $$PWD/ui/qdlgshapedyn.ui \
    $$PWD/ui/qdlgdetectionvolume.ui \
    $$PWD/ui/qdlgproximitysensors.ui \
    $$PWD/ui/qdlgproxsensdetectionparam.ui \
    $$PWD/ui/qdlgmills.ui \
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
    $$PWD/ui/qdlgikavoidance.ui \
    $$PWD/ui/qdlgui.ui \
    $$PWD/ui/qdlguidialogcontainer.ui \
    $$PWD/ui/qdlguibuttons.ui \
    $$PWD/ui/qdlgnewui.ui \
    $$PWD/ui/qdlguirolledup.ui \
    $$PWD/ui/qdlgpathplanning.ui \
    $$PWD/ui/qdlgmotionplanning.ui \
    $$PWD/ui/qdlgpathplanningparams.ui \
    $$PWD/ui/qdlgmotionplanningjoints.ui \
    $$PWD/ui/qdlgworkspacemetric.ui \
    $$PWD/ui/qdlgpathplanningaddnew.ui \
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

HEADERS += $$PWD/../programming/v_repMath/mathDefines.h \
    $$PWD/../programming/v_repMath/Vector.h \
    $$PWD/../programming/v_repMath/MyMath.h \
    $$PWD/../programming/v_repMath/MMatrix.h \
    $$PWD/../programming/v_repMath/7Vector.h \
    $$PWD/../programming/v_repMath/6X6Matrix.h \
    $$PWD/../programming/v_repMath/6Vector.h \
    $$PWD/../programming/v_repMath/4X4Matrix.h \
    $$PWD/../programming/v_repMath/4X4FullMatrix.h \
    $$PWD/../programming/v_repMath/4Vector.h \
    $$PWD/../programming/v_repMath/3X3Matrix.h \
    $$PWD/../programming/v_repMath/3Vector.h \
    $$PWD/sourceCode/various/vrepMainHeader.h

HEADERS += $$PWD/../programming/include/v_repConst.h \
    $$PWD/../programming/include/v_repTypes.h \

HEADERS += $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKObject.h \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKObjCont.h \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKMesh.h \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKJoint.h \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKGraphObject.h \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKGraphObjCont.h \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKGraphNode.h \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKGraphJoint.h \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKDummy.h \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKChainCont.h \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKChain.h \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/geometricConstraintSolverInt.h \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/geometricConstraintSolver.h \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/gCSDefs.h \

HEADERS += $$PWD/sourceCode/inverseKinematics/ik/ikRoutine.h \
    $$PWD/sourceCode/inverseKinematics/ik/ikGroup.h \
    $$PWD/sourceCode/inverseKinematics/ik/ikEl.h \

HEADERS += $$PWD/sourceCode/shared/sharedBufferFunctions/sharedFloatVector.h \
    $$PWD/sourceCode/shared/sharedBufferFunctions/sharedIntVector.h \
    $$PWD/sourceCode/shared/sharedBufferFunctions/sharedUCharVector.h \

HEADERS += $$PWD/sourceCode/drawingObjects/bannerObject.h \
    $$PWD/sourceCode/drawingObjects/drawingObject2D.h \
    $$PWD/sourceCode/drawingObjects/drawingObject.h \
    $$PWD/sourceCode/drawingObjects/drawingContainer2D.h \
    $$PWD/sourceCode/drawingObjects/ptCloud_old.h \

HEADERS += $$PWD/sourceCode/platform/vVarious.h \
    $$PWD/sourceCode/platform/vThread.h \
    $$PWD/sourceCode/platform/vMutex.h \
    $$PWD/sourceCode/platform/vFileFinder.h \
    $$PWD/sourceCode/platform/vFile.h \
    $$PWD/sourceCode/platform/vDateTime.h \
    $$PWD/sourceCode/platform/vArchive.h

HEADERS += $$PWD/sourceCode/collections/regCollectionEl.h \
    $$PWD/sourceCode/collections/regCollection.h \

HEADERS += $$PWD/sourceCode/collisions/regCollision.h \
    $$PWD/sourceCode/collisions/collisionRoutine.h \

HEADERS += $$PWD/sourceCode/distances/regDist.h \
    $$PWD/sourceCode/distances/distanceRoutine.h \
    $$PWD/sourceCode/distances/statDistObj.h \

HEADERS += $$PWD/sourceCode/3dObjects/related/3DObject.h \
    $$PWD/sourceCode/3dObjects/related/convexVolume.h \
    $$PWD/sourceCode/3dObjects/related/viewableBase.h \

HEADERS += $$PWD/sourceCode/3dObjects/jointObject.h \
    $$PWD/sourceCode/3dObjects/camera.h \
    $$PWD/sourceCode/3dObjects/dummy.h \
    $$PWD/sourceCode/3dObjects/octree.h \
    $$PWD/sourceCode/3dObjects/pointCloud.h \
    $$PWD/sourceCode/3dObjects/forceSensor.h \
    $$PWD/sourceCode/3dObjects/graph.h \
    $$PWD/sourceCode/3dObjects/light.h \
    $$PWD/sourceCode/3dObjects/mill.h \
    $$PWD/sourceCode/3dObjects/mirror.h \
    $$PWD/sourceCode/3dObjects/shape.h \
    $$PWD/sourceCode/3dObjects/path.h \
    $$PWD/sourceCode/3dObjects/proximitySensor.h \
    $$PWD/sourceCode/3dObjects/visionSensor.h \

HEADERS += $$PWD/sourceCode/3dObjects/graphObjectRelated/graphingRoutines.h \
    $$PWD/sourceCode/3dObjects/graphObjectRelated/graphDataComb.h \
    $$PWD/sourceCode/3dObjects/graphObjectRelated/graphData.h \
    $$PWD/sourceCode/3dObjects/graphObjectRelated/staticGraphCurve.h \

HEADERS += $$PWD/sourceCode/3dObjects/millObjectRelated/cuttingRoutine.h \

HEADERS += $$PWD/sourceCode/3dObjects/pathObjectRelated/bezierPathPoint.h \
    $$PWD/sourceCode/3dObjects/pathObjectRelated/simplePathPoint.h \
    $$PWD/sourceCode/3dObjects/pathObjectRelated/pathPoint.h \
    $$PWD/sourceCode/3dObjects/pathObjectRelated/pathCont.h \

HEADERS += $$PWD/sourceCode/3dObjects/shapeObjectRelated/geometric.h \
    $$PWD/sourceCode/3dObjects/shapeObjectRelated/geomWrap.h \
    $$PWD/sourceCode/3dObjects/shapeObjectRelated/geomProxy.h \
    $$PWD/sourceCode/3dObjects/shapeObjectRelated/volInt.h \

HEADERS += $$PWD/sourceCode/backwardCompatibility/pathPlanning/pathPlanning.h \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/holonomicPathPlanning.h \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/holonomicPathNode.h \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathPlanning.h \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathNode.h \
    $$PWD/sourceCode/backwardCompatibility/motionPlanning/mpPhase1Node.h \
    $$PWD/sourceCode/backwardCompatibility/motionPlanning/mpPhase2Node.h \
    $$PWD/sourceCode/backwardCompatibility/motionPlanning/mpObject.h \

HEADERS += $$PWD/sourceCode/communication/tubes/commTube.h \

HEADERS += $$PWD/sourceCode/communication/wireless/broadcastDataContainer.h \
    $$PWD/sourceCode/communication/wireless/broadcastData.h \
    $$PWD/sourceCode/communication/wireless/broadcastDataVisual.h \

HEADERS += $$PWD/sourceCode/mainContainers/mainContainer.h \

HEADERS += $$PWD/sourceCode/mainContainers/sceneContainers/drawingContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/bannerContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/textureContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/signalContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/simulation.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/registeredPathPlanningTasks.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/registeredMotionPlanningTasks.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/registerediks.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/registeredCollections.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/registeredDistances.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/registeredCollisions.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/objCont.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/memorizedConfContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/mainSettings.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/mainCont.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/luaScriptContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/environment.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/dynamicsContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/constraintSolverContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/commTubeContainer.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/apiErrors.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/outsideCommandQueue.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/cacheCont.h \
    $$PWD/sourceCode/mainContainers/sceneContainers/customData.h \
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

HEADERS += $$PWD/sourceCode/3dObjects/visionSensorObjectRelated/imageProcess.h \
    $$PWD/sourceCode/3dObjects/visionSensorObjectRelated/simpleFilter.h \
    $$PWD/sourceCode/3dObjects/visionSensorObjectRelated/composedFilter.h \

HEADERS += $$PWD/sourceCode/pathPlanning_old/pathPlanningTask_old.h \

HEADERS += $$PWD/sourceCode/motionPlanning_old/motionPlanningTask_old.h \

HEADERS += $$PWD/sourceCode/luaScripting/luaScriptParameters.h \
    $$PWD/sourceCode/luaScripting/luaScriptObject.h \
    $$PWD/sourceCode/luaScripting/outsideCommandQueueForScript.h \
    $$PWD/sourceCode/luaScripting/luaWrapper.h \

HEADERS += $$PWD/sourceCode/luaScripting/customLuaFuncAndVar/luaCustomFuncAndVarContainer.h \
    $$PWD/sourceCode/luaScripting/customLuaFuncAndVar/luaCustomFunction.h \
    $$PWD/sourceCode/luaScripting/customLuaFuncAndVar/luaCustomVariable.h \

HEADERS += $$PWD/sourceCode/pagesAndViews/pageContainer.h \
    $$PWD/sourceCode/pagesAndViews/sPage.h \
    $$PWD/sourceCode/pagesAndViews/sView.h \

HEADERS += $$PWD/sourceCode/textures/textureObject.h \
    $$PWD/sourceCode/textures/imgLoaderSaver.h \
    $$PWD/sourceCode/textures/tGAFormat.h \
    $$PWD/sourceCode/textures/stb_image.h \
    $$PWD/sourceCode/textures/textureProperty.h \

HEADERS += $$PWD/sourceCode/serialization/ser.h \
    $$PWD/sourceCode/serialization/xmlSer.h \
    $$PWD/sourceCode/serialization/extIkSer.h \
    $$PWD/sourceCode/serialization/huffman.h \
    $$PWD/sourceCode/serialization/tinyxml2.cpp \

HEADERS += $$PWD/sourceCode/strings/v_repStringTable.h \
    $$PWD/sourceCode/strings/v_repStringTable_openGl.h \
    $$PWD/sourceCode/strings/v_repStringTable_noTranslation.h \

HEADERS += $$PWD/sourceCode/interfaces/v_rep.h \
    $$PWD/sourceCode/interfaces/v_rep_internal.h \
    $$PWD/sourceCode/interfaces/luaScriptFunctions.h \
    $$PWD/sourceCode/interfaces/pathPlanningInterface.h \
    $$PWD/sourceCode/interfaces/dummyClasses.h \

HEADERS += $$PWD/sourceCode/interfaces/interfaceStack/interfaceStack.h \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackObject.h \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackNull.h \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackBool.h \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackNumber.h \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackString.h \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackTable.h \

HEADERS += $$PWD/sourceCode/menusAndSimilar/fileOperations.h \
    $$PWD/sourceCode/menusAndSimilar/addOperations.h \

HEADERS += $$PWD/sourceCode/variousFunctions/sceneObjectOperations.h \

HEADERS += $$PWD/sourceCode/geometricAlgorithms/linMotionRoutines.h \
    $$PWD/sourceCode/geometricAlgorithms/meshRoutines.h \
    $$PWD/sourceCode/geometricAlgorithms/meshManip.h \
    $$PWD/sourceCode/geometricAlgorithms/edgeElement.h \
    $$PWD/sourceCode/geometricAlgorithms/algos.h \

HEADERS += $$PWD/sourceCode/various/vrepConfig.h \
    $$PWD/sourceCode/various/gV.h \
    $$PWD/sourceCode/various/global.h \
    $$PWD/sourceCode/various/embeddedFonts.h \
    $$PWD/sourceCode/various/userSettings.h \
    $$PWD/sourceCode/various/memorizedConf.h \
    $$PWD/sourceCode/various/uiThread.h \
    $$PWD/sourceCode/various/simThread.h \
    $$PWD/sourceCode/various/app.h \
    $$PWD/sourceCode/various/directoryPaths.h \
    $$PWD/sourceCode/various/constraintSolverObject.h \
    $$PWD/sourceCode/various/dynMaterialObject.h \
    $$PWD/sourceCode/various/easyLock.h \
    $$PWD/sourceCode/various/funcDebug.h \
    $$PWD/sourceCode/various/ghostObject.h \
    $$PWD/sourceCode/various/debugLogFile.h \
    $$PWD/sourceCode/various/vrepConfig.h \
    $$PWD/sourceCode/various/sigHandler.h \

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
    $$PWD/sourceCode/rendering/drawingObject2DRendering.h \
    $$PWD/sourceCode/rendering/ptCloudRendering_old.h \
    $$PWD/sourceCode/rendering/collisionContourRendering.h \
    $$PWD/sourceCode/rendering/distanceRendering.h \
    $$PWD/sourceCode/rendering/bannerRendering.h \
    $$PWD/sourceCode/rendering/thumbnailRendering.h \
    $$PWD/sourceCode/rendering/pathPlanningTaskRendering_old.h \
    $$PWD/sourceCode/rendering/motionPlanningTaskRendering_old.h \
    $$PWD/sourceCode/rendering/broadcastDataVisualRendering.h \
    $$PWD/sourceCode/rendering/dynamicsRendering.h \
    $$PWD/sourceCode/rendering/environmentRendering.h \
    $$PWD/sourceCode/rendering/pageRendering.h \
    $$PWD/sourceCode/rendering/viewRendering.h \

HEADERS += $$PWD/sourceCode/libsAndPlugins/pluginContainer.h \

HEADERS += $$PWD/sourceCode/visual/visualParam.h \
    $$PWD/sourceCode/visual/thumbnail.h \

HEADERS += $$PWD/sourceCode/utils/threadPool.h \
    $$PWD/sourceCode/utils/tt.h \
    $$PWD/sourceCode/utils/ttUtil.h \
    $$PWD/sourceCode/utils/confReaderAndWriter.h \
    $$PWD/sourceCode/utils/base64.h \

HEADERS += $$PWD/sourceCode/customUserInterfaces/buttonBlock.h \
    $$PWD/sourceCode/customUserInterfaces/softButton.h \

WITH_QT {
    HEADERS += $$PWD/sourceCode/platform/vSimUiMutex.h \
        $$PWD/sourceCode/platform/wThread.h

    HEADERS += $$PWD/sourceCode/various/simAndUiThreadSync.h \
        $$PWD/sourceCode/various/vrepQApp.h
}

WITH_SERIAL {
    HEADERS += $$PWD/sourceCode/mainContainers/applicationContainers/serialPortContainer.h

    HEADERS += $$PWD/sourceCode/communication/serialPort/serialPort.h \
        $$PWD/sourceCode/communication/serialPort/serial_win_mac_linux.h \
        $$PWD/sourceCode/communication/serialPort/serialPortWin.h
}

WITH_OPENGL {
    HEADERS += $$PWD/sourceCode/3dObjects/visionSensorObjectRelated/offscreenGlContext.h \
        $$PWD/sourceCode/3dObjects/visionSensorObjectRelated/frameBufferObject.h \
        $$PWD/sourceCode/3dObjects/visionSensorObjectRelated/visionSensorGlStuff.h

    HEADERS += $$PWD/sourceCode/visual/oGL.h \
        $$PWD/sourceCode/visual/oglExt.h \
        $$PWD/sourceCode/visual/glShader.h \
        $$PWD/sourceCode/visual/glBufferObjects.h \
}

WITH_GUI {
    HEADERS += $$PWD/sourceCode/gui/dialogs/qdlglayers.h \
        $$PWD/sourceCode/gui/dialogs/qdlgavirecorder.h \
        $$PWD/sourceCode/gui/dialogs/qdlgselection.h \
        $$PWD/sourceCode/gui/dialogs/qdlgabout.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimulation.h \
        $$PWD/sourceCode/gui/dialogs/qdlginteractiveik.h \
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
        $$PWD/sourceCode/gui/dialogs/qdlginsertscript.h \
        $$PWD/sourceCode/gui/dialogs/qdlgdynamics.h \
        $$PWD/sourceCode/gui/dialogs/qdlgscaling.h \
        $$PWD/sourceCode/gui/dialogs/qdlgcameras.h \
        $$PWD/sourceCode/gui/dialogs/qdlglights.h \
        $$PWD/sourceCode/gui/dialogs/qdlgmirrors.h \
        $$PWD/sourceCode/gui/dialogs/qdlgdummies.h \
        $$PWD/sourceCode/gui/dialogs/qdlgoctrees.h \
        $$PWD/sourceCode/gui/dialogs/qdlgpointclouds.h \
        $$PWD/sourceCode/gui/dialogs/qdlgscriptparameters.h \
        $$PWD/sourceCode/gui/dialogs/qdlgforcesensors.h \
        $$PWD/sourceCode/gui/dialogs/qdlgprimitives.h \
        $$PWD/sourceCode/gui/dialogs/qdlgconvexdecomposition.h \
        $$PWD/sourceCode/gui/dialogs/qdlgconstraintsolver.h \
        $$PWD/sourceCode/gui/dialogs/qdlgvisionsensors.h \
        $$PWD/sourceCode/gui/dialogs/qdlgimagecolor.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimpleSearch.h \
        $$PWD/sourceCode/gui/dialogs/qdlgfilters.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_rotate.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_uniformImage.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_shift.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_coordExtraction.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_velodyne.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_pixelChange.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_scaleCols.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_resize.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_rectangularCut.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_keepRemoveCols.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_intensityScale.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_edge.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_correlation.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_colorSegmentation.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_circularCut.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_blob.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_binary.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_5x5.h \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_3x3.h \
        $$PWD/sourceCode/gui/dialogs/qdlgshapes.h \
        $$PWD/sourceCode/gui/dialogs/qdlgmultishapeedition.h \
        $$PWD/sourceCode/gui/dialogs/qdlgtextures.h \
        $$PWD/sourceCode/gui/dialogs/qdlgtextureselection.h \
        $$PWD/sourceCode/gui/dialogs/qdlggeometry.h \
        $$PWD/sourceCode/gui/dialogs/qdlgshapedyn.h \
        $$PWD/sourceCode/gui/dialogs/qdlgdetectionvolume.h \
        $$PWD/sourceCode/gui/dialogs/qdlgproximitysensors.h \
        $$PWD/sourceCode/gui/dialogs/qdlgproxsensdetectionparam.h \
        $$PWD/sourceCode/gui/dialogs/qdlgmills.h \
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
        $$PWD/sourceCode/gui/dialogs/qdlgikavoidance.h \
        $$PWD/sourceCode/gui/dialogs/qdlgui.h \
        $$PWD/sourceCode/gui/dialogs/qdlguidialogcontainer.h \
        $$PWD/sourceCode/gui/dialogs/qdlguibuttons.h \
        $$PWD/sourceCode/gui/dialogs/qdlgnewui.h \
        $$PWD/sourceCode/gui/dialogs/qdlguirolledup.h \
        $$PWD/sourceCode/gui/dialogs/qdlgpathplanning.h \
        $$PWD/sourceCode/gui/dialogs/qdlgmotionplanning.h \
        $$PWD/sourceCode/gui/dialogs/qdlgpathplanningparams.h \
        $$PWD/sourceCode/gui/dialogs/qdlgmotionplanningjoints.h \
        $$PWD/sourceCode/gui/dialogs/qdlgworkspacemetric.h \
        $$PWD/sourceCode/gui/dialogs/qdlgpathplanningaddnew.h \
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
        $$PWD/sourceCode/gui/dialogs/other/qvrep_openglwidget.h \

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
        $$PWD/sourceCode/gui/editModes/uiEditMode.h \
        $$PWD/sourceCode/gui/editModes/edgeCont.h \
        $$PWD/sourceCode/gui/editModes/pathPointManipulation.h \

    HEADERS += $$PWD/sourceCode/gui/sceneHierarchy/hierarchy.h \
        $$PWD/sourceCode/gui/sceneHierarchy/hierarchyElement.h \
        $$PWD/sourceCode/gui/sceneHierarchy/sceneHierarchyWidget.h \
        $$PWD/sourceCode/gui/sceneHierarchy/sceneHierarchyItemDelegate.h \

    HEADERS += $$PWD/sourceCode/gui/modelBrowser/modelFolderWidget.h \
        $$PWD/sourceCode/gui/modelBrowser/modelListWidget.h \
        $$PWD/sourceCode/gui/modelBrowser/modelListWidgetBase.h \

    HEADERS += $$PWD/sourceCode/gui/selectors/viewSelector.h \
        $$PWD/sourceCode/gui/selectors/pageSelector.h \
        $$PWD/sourceCode/gui/selectors/sceneSelector.h \

    HEADERS += $$PWD/sourceCode/gui/menusAndSimilar/toolBarCommand.h \
        $$PWD/sourceCode/gui/menusAndSimilar/customMenuBarItemContainer.h \
        $$PWD/sourceCode/gui/menusAndSimilar/customMenuBarItem.h \
        $$PWD/sourceCode/gui/menusAndSimilar/mainWindow.h \
        $$PWD/sourceCode/gui/menusAndSimilar/statusBar.h \
        $$PWD/sourceCode/gui/menusAndSimilar/helpMenu.h \

    HEADERS += $$PWD/sourceCode/gui/various/simRecorder.h \

    HEADERS += $$PWD/sourceCode/gui/libs/auxLibVideo.h \
}

SOURCES += $$PWD/../programming/v_repMath/Vector.cpp \
    $$PWD/../programming/v_repMath/MyMath.cpp \
    $$PWD/../programming/v_repMath/MMatrix.cpp \
    $$PWD/../programming/v_repMath/7Vector.cpp \
    $$PWD/../programming/v_repMath/6X6Matrix.cpp \
    $$PWD/../programming/v_repMath/6Vector.cpp \
    $$PWD/../programming/v_repMath/4X4Matrix.cpp \
    $$PWD/../programming/v_repMath/4X4FullMatrix.cpp \
    $$PWD/../programming/v_repMath/4Vector.cpp \
    $$PWD/../programming/v_repMath/3X3Matrix.cpp \
    $$PWD/../programming/v_repMath/3Vector.cpp \

SOURCES += $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKObject.cpp \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKObjCont.cpp \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKMesh.cpp \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKJoint.cpp \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKGraphObject.cpp \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKGraphObjCont.cpp \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKGraphNode.cpp \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKGraphJoint.cpp \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKDummy.cpp \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKChainCont.cpp \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/iKChain.cpp \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/geometricConstraintSolverInt.cpp \
    $$PWD/sourceCode/inverseKinematics/geomConstraintSolver/geometricConstraintSolver.cpp \

SOURCES += $$PWD/sourceCode/inverseKinematics/ik/ikRoutine.cpp \
    $$PWD/sourceCode/inverseKinematics/ik/ikGroup.cpp \
    $$PWD/sourceCode/inverseKinematics/ik/ikEl.cpp \

SOURCES += $$PWD/sourceCode/shared/sharedBufferFunctions/sharedFloatVector.cpp \
    $$PWD/sourceCode/shared/sharedBufferFunctions/sharedIntVector.cpp \
    $$PWD/sourceCode/shared/sharedBufferFunctions/sharedUCharVector.cpp \

SOURCES += $$PWD/sourceCode/drawingObjects/bannerObject.cpp \
    $$PWD/sourceCode/drawingObjects/drawingObject2D.cpp \
    $$PWD/sourceCode/drawingObjects/drawingObject.cpp \
    $$PWD/sourceCode/drawingObjects/drawingContainer2D.cpp \
    $$PWD/sourceCode/drawingObjects/ptCloud_old.cpp \

SOURCES += $$PWD/sourceCode/platform/vVarious.cpp \
    $$PWD/sourceCode/platform/vThread.cpp \
    $$PWD/sourceCode/platform/vMutex.cpp \
    $$PWD/sourceCode/platform/vFileFinder.cpp \
    $$PWD/sourceCode/platform/vFile.cpp \
    $$PWD/sourceCode/platform/vDateTime.cpp \
    $$PWD/sourceCode/platform/vArchive.cpp

SOURCES += $$PWD/sourceCode/collections/regCollectionEl.cpp \
    $$PWD/sourceCode/collections/regCollection.cpp \

SOURCES += $$PWD/sourceCode/collisions/regCollision.cpp \
    $$PWD/sourceCode/collisions/collisionRoutine.cpp \

SOURCES += $$PWD/sourceCode/distances/regDist.cpp \
    $$PWD/sourceCode/distances/distanceRoutine.cpp \
    $$PWD/sourceCode/distances/statDistObj.cpp \

SOURCES += $$PWD/sourceCode/3dObjects/related/3DObject.cpp \
    $$PWD/sourceCode/3dObjects/related/convexVolume.cpp \
    $$PWD/sourceCode/3dObjects/related/viewableBase.cpp \

SOURCES += $$PWD/sourceCode/3dObjects/jointObject.cpp \
    $$PWD/sourceCode/3dObjects/camera.cpp \
    $$PWD/sourceCode/3dObjects/dummy.cpp \
    $$PWD/sourceCode/3dObjects/octree.cpp \
    $$PWD/sourceCode/3dObjects/pointCloud.cpp \
    $$PWD/sourceCode/3dObjects/forceSensor.cpp \
    $$PWD/sourceCode/3dObjects/graph.cpp \
    $$PWD/sourceCode/3dObjects/light.cpp \
    $$PWD/sourceCode/3dObjects/mirror.cpp \
    $$PWD/sourceCode/3dObjects/mill.cpp \
    $$PWD/sourceCode/3dObjects/path.cpp \
    $$PWD/sourceCode/3dObjects/proximitySensor.cpp \
    $$PWD/sourceCode/3dObjects/shape.cpp \
    $$PWD/sourceCode/3dObjects/visionSensor.cpp \

SOURCES += $$PWD/sourceCode/3dObjects/graphObjectRelated/graphingRoutines.cpp \
    $$PWD/sourceCode/3dObjects/graphObjectRelated/graphDataComb.cpp \
    $$PWD/sourceCode/3dObjects/graphObjectRelated/graphData.cpp \
    $$PWD/sourceCode/3dObjects/graphObjectRelated/staticGraphCurve.cpp \

SOURCES += $$PWD/sourceCode/3dObjects/millObjectRelated/cuttingRoutine.cpp \

SOURCES += $$PWD/sourceCode/3dObjects/pathObjectRelated/bezierPathPoint.cpp \
    $$PWD/sourceCode/3dObjects/pathObjectRelated/simplePathPoint.cpp \
    $$PWD/sourceCode/3dObjects/pathObjectRelated/pathPoint.cpp \
    $$PWD/sourceCode/3dObjects/pathObjectRelated/pathCont.cpp \

SOURCES += $$PWD/sourceCode/3dObjects/proximitySensorObjectRelated/proxSensorRoutine.cpp \

SOURCES += $$PWD/sourceCode/3dObjects/shapeObjectRelated/geometric.cpp \
    $$PWD/sourceCode/3dObjects/shapeObjectRelated/geomWrap.cpp \
    $$PWD/sourceCode/3dObjects/shapeObjectRelated/geomProxy.cpp \
    $$PWD/sourceCode/3dObjects/shapeObjectRelated/volInt.cpp \


SOURCES += $$PWD/sourceCode/backwardCompatibility/pathPlanning/pathPlanning.cpp \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/holonomicPathPlanning.cpp \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/holonomicPathNode.cpp \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathPlanning.cpp \
    $$PWD/sourceCode/backwardCompatibility/pathPlanning/nonHolonomicPathNode.cpp \
    $$PWD/sourceCode/backwardCompatibility/motionPlanning/mpPhase1Node.cpp \
    $$PWD/sourceCode/backwardCompatibility/motionPlanning/mpPhase2Node.cpp \
    $$PWD/sourceCode/backwardCompatibility/motionPlanning/mpObject.cpp \

SOURCES += $$PWD/sourceCode/communication/tubes/commTube.cpp \

SOURCES += $$PWD/sourceCode/communication/wireless/broadcastDataContainer.cpp \
    $$PWD/sourceCode/communication/wireless/broadcastData.cpp \
    $$PWD/sourceCode/communication/wireless/broadcastDataVisual.cpp \

SOURCES += $$PWD/sourceCode/mainContainers/mainContainer.cpp \

SOURCES += $$PWD/sourceCode/mainContainers/sceneContainers/bannerContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/drawingContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/textureContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/simulation.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/signalContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/registeredPathPlanningTasks.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/registeredMotionPlanningTasks.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/registerediks.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/registeredCollections.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/registeredDistances.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/registeredCollisions.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/outsideCommandQueue.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/objCont.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/memorizedConfContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/mainSettings.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/mainCont.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/luaScriptContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/environment.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/dynamicsContainer.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/customData.cpp \
    $$PWD/sourceCode/mainContainers/sceneContainers/constraintSolverContainer.cpp \
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

SOURCES += $$PWD/sourceCode/3dObjects/visionSensorObjectRelated/imageProcess.cpp \
    $$PWD/sourceCode/3dObjects/visionSensorObjectRelated/simpleFilter.cpp \
    $$PWD/sourceCode/3dObjects/visionSensorObjectRelated/composedFilter.cpp \

SOURCES += $$PWD/sourceCode/pathPlanning_old/pathPlanningTask_old.cpp \

SOURCES += $$PWD/sourceCode/motionPlanning_old/motionPlanningTask_old.cpp \

SOURCES += $$PWD/sourceCode/luaScripting/luaScriptParameters.cpp \
    $$PWD/sourceCode/luaScripting/luaScriptObject.cpp \
    $$PWD/sourceCode/luaScripting/outsideCommandQueueForScript.cpp \
    $$PWD/sourceCode/luaScripting/luaWrapper.cpp \

SOURCES += $$PWD/sourceCode/luaScripting/customLuaFuncAndVar/luaCustomFuncAndVarContainer.cpp \
    $$PWD/sourceCode/luaScripting/customLuaFuncAndVar/luaCustomFunction.cpp \
    $$PWD/sourceCode/luaScripting/customLuaFuncAndVar/luaCustomVariable.cpp \

SOURCES += $$PWD/sourceCode/pagesAndViews/pageContainer.cpp \
    $$PWD/sourceCode/pagesAndViews/sPage.cpp \
    $$PWD/sourceCode/pagesAndViews/sView.cpp \

SOURCES += $$PWD/sourceCode/textures/textureObject.cpp \
    $$PWD/sourceCode/textures/imgLoaderSaver.cpp \
    $$PWD/sourceCode/textures/tGAFormat.cpp \
    $$PWD/sourceCode/textures/stb_image.c \
    $$PWD/sourceCode/textures/textureProperty.cpp \

SOURCES += $$PWD/sourceCode/serialization/ser.cpp \
    $$PWD/sourceCode/serialization/xmlSer.cpp \
    $$PWD/sourceCode/serialization/extIkSer.cpp \
    $$PWD/sourceCode/serialization/huffman.c \
    $$PWD/sourceCode/serialization/tinyxml2.cpp \

SOURCES += $$PWD/sourceCode/interfaces/v_rep.cpp \
    $$PWD/sourceCode/interfaces/v_rep_internal.cpp \
    $$PWD/sourceCode/interfaces/luaScriptFunctions.cpp \
    $$PWD/sourceCode/interfaces/pathPlanningInterface.cpp \

SOURCES += $$PWD/sourceCode/interfaces/interfaceStack/interfaceStack.cpp \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackObject.cpp \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackNull.cpp \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackBool.cpp \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackNumber.cpp \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackString.cpp \
    $$PWD/sourceCode/interfaces/interfaceStack/interfaceStackTable.cpp \

SOURCES += $$PWD/sourceCode/menusAndSimilar/addOperations.cpp \
    $$PWD/sourceCode/menusAndSimilar/fileOperations.cpp \

SOURCES += $$PWD/sourceCode/variousFunctions/sceneObjectOperations.cpp \

SOURCES += $$PWD/sourceCode/geometricAlgorithms/linMotionRoutines.cpp \
    $$PWD/sourceCode/geometricAlgorithms/meshRoutines.cpp \
    $$PWD/sourceCode/geometricAlgorithms/meshManip.cpp \
    $$PWD/sourceCode/geometricAlgorithms/edgeElement.cpp \
    $$PWD/sourceCode/geometricAlgorithms/algos.cpp \

SOURCES += $$PWD/sourceCode/various/gV.cpp \
    $$PWD/sourceCode/various/memorizedConf.cpp \
    $$PWD/sourceCode/various/userSettings.cpp \
    $$PWD/sourceCode/various/directoryPaths.cpp \
    $$PWD/sourceCode/various/constraintSolverObject.cpp \
    $$PWD/sourceCode/various/uiThread.cpp \
    $$PWD/sourceCode/various/simThread.cpp \
    $$PWD/sourceCode/various/app.cpp \
    $$PWD/sourceCode/various/dynMaterialObject.cpp \
    $$PWD/sourceCode/various/easyLock.cpp \
    $$PWD/sourceCode/various/funcDebug.cpp \
    $$PWD/sourceCode/various/ghostObject.cpp \
    $$PWD/sourceCode/various/debugLogFile.cpp \
    $$PWD/sourceCode/various/sigHandler.cpp \

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
    $$PWD/sourceCode/rendering/drawingObject2DRendering.cpp \
    $$PWD/sourceCode/rendering/ptCloudRendering_old.cpp \
    $$PWD/sourceCode/rendering/collisionContourRendering.cpp \
    $$PWD/sourceCode/rendering/distanceRendering.cpp \
    $$PWD/sourceCode/rendering/bannerRendering.cpp \
    $$PWD/sourceCode/rendering/thumbnailRendering.cpp \
    $$PWD/sourceCode/rendering/pathPlanningTaskRendering_old.cpp \
    $$PWD/sourceCode/rendering/motionPlanningTaskRendering_old.cpp \
    $$PWD/sourceCode/rendering/broadcastDataVisualRendering.cpp \
    $$PWD/sourceCode/rendering/dynamicsRendering.cpp \
    $$PWD/sourceCode/rendering/environmentRendering.cpp \
    $$PWD/sourceCode/rendering/pageRendering.cpp \
    $$PWD/sourceCode/rendering/viewRendering.cpp \

SOURCES += $$PWD/sourceCode/libsAndPlugins/pluginContainer.cpp \

SOURCES += $$PWD/sourceCode/visual/visualParam.cpp \
    $$PWD/sourceCode/visual/thumbnail.cpp \

SOURCES += $$PWD/sourceCode/utils/threadPool.cpp \
    $$PWD/sourceCode/utils/ttUtil.cpp \
    $$PWD/sourceCode/utils/tt.cpp \
    $$PWD/sourceCode/utils/confReaderAndWriter.cpp \
    $$PWD/sourceCode/utils/base64.cpp \

SOURCES += $$PWD/sourceCode/customUserInterfaces/buttonBlock.cpp \
    $$PWD/sourceCode/customUserInterfaces/softButton.cpp \

WITH_QT {
    SOURCES += $$PWD/sourceCode/platform/vSimUiMutex.cpp \
        $$PWD/sourceCode/platform/wThread.cpp

    SOURCES += $$PWD/sourceCode/various/simAndUiThreadSync.cpp \
        $$PWD/sourceCode/various/vrepQApp.cpp
}

WITH_SERIAL {
    SOURCES += $$PWD/sourceCode/mainContainers/applicationContainers/serialPortContainer.cpp

    SOURCES += $$PWD/sourceCode/communication/serialPort/serialPort.cpp \
        $$PWD/sourceCode/communication/serialPort/serial_win_mac_linux.cpp \
        $$PWD/sourceCode/communication/serialPort/serialPortWin.cpp
}

WITH_OPENGL {
    SOURCES += $$PWD/sourceCode/3dObjects/visionSensorObjectRelated/offscreenGlContext.cpp \
        $$PWD/sourceCode/3dObjects/visionSensorObjectRelated/frameBufferObject.cpp \
        $$PWD/sourceCode/3dObjects/visionSensorObjectRelated/visionSensorGlStuff.cpp \

    SOURCES += $$PWD/sourceCode/visual/oGL.cpp \
        $$PWD/sourceCode/visual/oglExt.cpp \
        $$PWD/sourceCode/visual/glShader.cpp \
        $$PWD/sourceCode/visual/glBufferObjects.cpp \
}

WITH_GUI {
    SOURCES += $$PWD/sourceCode/gui/dialogs/qdlgsettings.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlglayers.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgavirecorder.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgselection.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgabout.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimulation.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlginteractiveik.cpp \
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
        $$PWD/sourceCode/gui/dialogs/qdlginsertscript.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgdynamics.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgscaling.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgcameras.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlglights.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgmirrors.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgdummies.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgoctrees.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgpointclouds.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgscriptparameters.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgforcesensors.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgprimitives.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgconvexdecomposition.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgconstraintsolver.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgvisionsensors.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgimagecolor.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimpleSearch.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgfilters.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_rotate.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_uniformImage.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_shift.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_coordExtraction.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_velodyne.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_pixelChange.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_scaleCols.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_resize.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_rectangularCut.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_keepRemoveCols.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_intensityScale.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_edge.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_correlation.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_colorSegmentation.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_circularCut.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_blob.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_binary.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_5x5.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgsimplefilter_3x3.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgshapes.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgmultishapeedition.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgtextures.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgtextureselection.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlggeometry.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgshapedyn.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgdetectionvolume.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgproximitysensors.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgproxsensdetectionparam.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgmills.cpp \
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
        $$PWD/sourceCode/gui/dialogs/qdlgikavoidance.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgui.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlguidialogcontainer.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlguibuttons.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgnewui.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlguirolledup.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgpathplanning.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgmotionplanning.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgpathplanningparams.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgmotionplanningjoints.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgworkspacemetric.cpp \
        $$PWD/sourceCode/gui/dialogs/qdlgpathplanningaddnew.cpp \
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
        $$PWD/sourceCode/gui/dialogs/other/qvrep_openglwidget.cpp \

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
        $$PWD/sourceCode/gui/editModes/uiEditMode.cpp \
        $$PWD/sourceCode/gui/editModes/edgeCont.cpp \
        $$PWD/sourceCode/gui/editModes/pathPointManipulation.cpp \

    SOURCES += $$PWD/sourceCode/gui/sceneHierarchy/hierarchy.cpp \
        $$PWD/sourceCode/gui/sceneHierarchy/hierarchyElement.cpp \
        $$PWD/sourceCode/gui/sceneHierarchy/sceneHierarchyWidget.cpp \
        $$PWD/sourceCode/gui/sceneHierarchy/sceneHierarchyItemDelegate.cpp \

    SOURCES += $$PWD/sourceCode/gui/modelBrowser/modelFolderWidget.cpp \
        $$PWD/sourceCode/gui/modelBrowser/modelListWidget.cpp \
        $$PWD/sourceCode/gui/modelBrowser/modelListWidgetBase.cpp \

    SOURCES += $$PWD/sourceCode/gui/selectors/pageSelector.cpp \
        $$PWD/sourceCode/gui/selectors/viewSelector.cpp \
        $$PWD/sourceCode/gui/selectors/sceneSelector.cpp \

    SOURCES += $$PWD/sourceCode/gui/menusAndSimilar/toolBarCommand.cpp \
        $$PWD/sourceCode/gui/menusAndSimilar/customMenuBarItemContainer.cpp \
        $$PWD/sourceCode/gui/menusAndSimilar/customMenuBarItem.cpp \
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
