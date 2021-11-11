#include "simInternal.h"
#include "addOperations.h"
#include "fileOperations.h"
#include "tt.h"
#include "meshRoutines.h"
#include "sceneObjectOperations.h"
#include "simFlavor.h"
#include "app.h"
#include "meshManip.h"
#include "mesh.h"
#include "simStrings.h"
#include <boost/lexical_cast.hpp>

CAddOperations::CAddOperations()
{

}

CAddOperations::~CAddOperations()
{

}

bool CAddOperations::processCommand(int commandID,CSView* subView)
{ // Return value is true if the command belonged to Add menu and was executed
#ifdef SIM_WITH_GUI
    if ( (commandID==ADD_COMMANDS_ADD_PRIMITIVE_PLANE_ACCMD)||(commandID==ADD_COMMANDS_ADD_PRIMITIVE_DISC_ACCMD)||
        (commandID==ADD_COMMANDS_ADD_PRIMITIVE_RECTANGLE_ACCMD)||(commandID==ADD_COMMANDS_ADD_PRIMITIVE_SPHERE_ACCMD)||
        (commandID==ADD_COMMANDS_ADD_PRIMITIVE_CYLINDER_ACCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_A_PRIMITIVE_SHAPE);

            CShape* newShape=addPrimitive_withDialog(commandID,nullptr);
            int shapeHandle=-1;
            if (newShape!=nullptr)
                shapeHandle=newShape->getObjectHandle();
            if (shapeHandle!=-1)
            {
                App::currentWorld->sceneObjects->deselectObjects();
                App::currentWorld->sceneObjects->selectObject(shapeHandle);
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
            }
            else
                App::logMsg(sim_verbosity_msgs,IDSNS_OPERATION_ABORTED);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
#endif

    if (commandID==ADD_COMMANDS_ADD_FLOATING_VIEW_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_A_FLOATING_VIEW);
#ifdef SIM_WITH_GUI
            App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex())->addFloatingView();
#endif
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if ( (commandID==ADD_COMMANDS_ADD_REVOLUTE_JOINT_ACCMD)||(commandID==ADD_COMMANDS_ADD_PRISMATIC_JOINT_ACCMD)||
        (commandID==ADD_COMMANDS_ADD_SPHERICAL_JOINT_ACCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_A_JOINT);
            CJoint* newObject=nullptr;
            if (commandID==ADD_COMMANDS_ADD_REVOLUTE_JOINT_ACCMD)
                newObject=new CJoint(sim_joint_revolute_subtype);
            if (commandID==ADD_COMMANDS_ADD_PRISMATIC_JOINT_ACCMD)
                newObject=new CJoint(sim_joint_prismatic_subtype);
            if (commandID==ADD_COMMANDS_ADD_SPHERICAL_JOINT_ACCMD)
                newObject=new CJoint(sim_joint_spherical_subtype);
            App::currentWorld->sceneObjects->addObjectToScene(newObject,false,true);
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if ( (commandID==ADD_COMMANDS_ADD_CAMERA_ACCMD)||(commandID==ADD_COMMANDS_ADD_OMNI_LIGHT_ACCMD)||
        (commandID==ADD_COMMANDS_ADD_SPOT_LIGHT_ACCMD)||(commandID==ADD_COMMANDS_ADD_DIR_LIGHT_ACCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            int lo=-1;
            if (subView!=nullptr)
                lo=subView->getLinkedObjectID();
            CCamera* camera=App::currentWorld->sceneObjects->getCameraFromHandle(lo);
            CGraph* graph=App::currentWorld->sceneObjects->getGraphFromHandle(lo);
            if (graph!=nullptr)
                return(true);
            CCamera* myNewCamera=nullptr;
            CLight* myNewLight=nullptr;
            if (commandID==ADD_COMMANDS_ADD_CAMERA_ACCMD)
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_A_CAMERA);
                myNewCamera=new CCamera();
                myNewCamera->setPerspectiveOperation(true);
                App::currentWorld->sceneObjects->addObjectToScene(myNewCamera,false,true);
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
            }
            else
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_A_LIGHT);
                int tp;
                if (commandID==ADD_COMMANDS_ADD_OMNI_LIGHT_ACCMD)
                    tp=sim_light_omnidirectional_subtype;
                if (commandID==ADD_COMMANDS_ADD_SPOT_LIGHT_ACCMD)
                    tp=sim_light_spot_subtype;
                if (commandID==ADD_COMMANDS_ADD_DIR_LIGHT_ACCMD)
                    tp=sim_light_directional_subtype;
                myNewLight=new CLight(tp);
                App::currentWorld->sceneObjects->addObjectToScene(myNewLight,false,true);
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
            }
            CSceneObject* addedObject=myNewCamera;
            if (addedObject==nullptr)
                addedObject=myNewLight;
            addedObject->setLocalTransformation(C3Vector(0.0f,0.0f,1.0f));
            addedObject->setLocalTransformation(C4Vector(piValue_f*0.5f,0.0f,0.0f));
            if (camera!=nullptr)
            {
                if (myNewCamera!=nullptr)
                {
                    App::currentWorld->sceneObjects->selectObject(myNewCamera->getObjectHandle());
                    C7Vector m(camera->getFullCumulativeTransformation());
                    myNewCamera->setLocalTransformation(m);
                    myNewCamera->scaleObject(camera->getCameraSize()/myNewCamera->getCameraSize());
                    C3Vector minV,maxV;
                    myNewCamera->getBoundingBox(minV,maxV);
                    m=myNewCamera->getFullLocalTransformation();
                    maxV-=minV;
                    float averageSize=(maxV(0)+maxV(1)+maxV(2))/3.0f;
                    float shiftForward=camera->getNearClippingPlane()-minV(2)+3.0f*averageSize;
                    m.X+=(m.Q.getAxis(2)*shiftForward);
                    myNewCamera->setLocalTransformation(m.X);
                }
            }
            else if (subView!=nullptr)
            {   // When we want to add a camera to an empty window
                if (myNewCamera!=nullptr)
                {
                    C7Vector m;
                    m.X=C3Vector(-1.12f,1.9f,1.08f);
                    m.Q.setEulerAngles(C3Vector(110.933f*degToRad_f,28.703f*degToRad_f,-10.41f*degToRad_f));
                    myNewCamera->setLocalTransformation(m);
                    subView->setLinkedObjectID(myNewCamera->getObjectHandle(),false);
                }
            }
            App::currentWorld->sceneObjects->selectObject(addedObject->getObjectHandle());
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.objectParams.push_back(subView);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==ADD_COMMANDS_ADD_MIRROR_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_A_MIRROR);
            CMirror* newObject=new CMirror();
            App::currentWorld->sceneObjects->addObjectToScene(newObject,false,true);
            App::currentWorld->sceneObjects->setObjectAbsoluteOrientation(newObject->getObjectHandle(),C3Vector(piValD2_f,0.0f,0.0f));
            App::currentWorld->sceneObjects->setObjectAbsolutePosition(newObject->getObjectHandle(),C3Vector(0.0f,0.0f,newObject->getMirrorHeight()*0.5f));
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==ADD_COMMANDS_ADD_DUMMY_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_A_DUMMY);
            CDummy* newObject=new CDummy();
            App::currentWorld->sceneObjects->addObjectToScene(newObject,false,true);
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==ADD_COMMANDS_ADD_OCTREE_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_AN_OCTREE);
            COctree* newObject=new COctree();
            App::currentWorld->sceneObjects->addObjectToScene(newObject,false,true);
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==ADD_COMMANDS_ADD_POINTCLOUD_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_A_POINTCLOUD);
            CPointCloud* newObject=new CPointCloud();
/*
            std::vector<float> v;
            for (size_t i=0;i<50000;i++)
            {
                float x=sin(float(i)/500.0);
                float y=cos(float(i)/500.0);
                float z=float(i)/50000.0;
                v.push_back(x);
                v.push_back(y);
                v.push_back(z);
            }
            newObject->insertPoints(&v[0],v.size()/3,true,nullptr);
            //*/
            App::currentWorld->sceneObjects->addObjectToScene(newObject,false,true);
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if ((commandID==ADD_COMMANDS_ADD_AND_ASSOCIATE_NON_THREADED_CHILD_SCRIPT_ACCMD)||(commandID==ADD_COMMANDS_ADD_AND_ASSOCIATE_THREADED_CHILD_SCRIPT_ACCMD)||(commandID==ADD_COMMANDS_ADD_AND_ASSOCIATE_oldTHREADED_CHILD_SCRIPT_ACCMD))
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (App::currentWorld->sceneObjects->getSelectionCount()==1)
            {
                int scriptID=App::currentWorld->embeddedScriptContainer->insertDefaultScript(sim_scripttype_childscript,commandID==ADD_COMMANDS_ADD_AND_ASSOCIATE_THREADED_CHILD_SCRIPT_ACCMD,commandID==ADD_COMMANDS_ADD_AND_ASSOCIATE_oldTHREADED_CHILD_SCRIPT_ACCMD);
                CScriptObject* script=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptID);
                if (script!=nullptr)
                    script->setObjectHandleThatScriptIsAttachedTo(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0));
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                App::setFullDialogRefreshFlag();
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if ((commandID==ADD_COMMANDS_ADD_AND_ASSOCIATE_NON_THREADED_CUSTOMIZATION_SCRIPT_ACCMD)||(commandID==ADD_COMMANDS_ADD_AND_ASSOCIATE_THREADED_CUSTOMIZATION_SCRIPT_ACCMD))
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (App::currentWorld->sceneObjects->getSelectionCount()==1)
            {
                int scriptID=App::currentWorld->embeddedScriptContainer->insertDefaultScript(sim_scripttype_customizationscript,commandID==ADD_COMMANDS_ADD_AND_ASSOCIATE_THREADED_CUSTOMIZATION_SCRIPT_ACCMD,false);
                CScriptObject* script=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptID);
                if (script!=nullptr)
                    script->setObjectHandleThatScriptIsAttachedTo(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0));
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                App::setFullDialogRefreshFlag();
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if ( (commandID==ADD_COMMANDS_ADD_PATH_SEGMENT_ACCMD)||(commandID==ADD_COMMANDS_ADD_PATH_CIRCLE_ACCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_A_PATH);
            std::string txt;
            if (commandID==ADD_COMMANDS_ADD_PATH_SEGMENT_ACCMD)
                txt+="local pathData={-2.5000e-01,0.0000e+00,0.0000e+00,0.0000e+00,0.0000e+00,0.0000e+00,1.0000e+00,2.5000e-01,0.0000e+00,0.0000e+00,0.0000e+00,0.0000e+00,0.0000e+00,1.0000e+00}";
            else
                txt+="local pathData={2.5490e-01,0.0000e+00,0.0000e+00,-2.1073e-08,-2.9802e-08,2.1073e-08,1.0000e+00,2.3549e-01,9.7545e-02,0.0000e+00,0.0000e+00,0.0000e+00,1.9509e-01,9.8079e-01,1.8024e-01,1.8024e-01,0.0000e+00,4.4703e-08,0.0000e+00,3.8268e-01,9.2388e-01,9.7545e-02,2.3549e-01,0.0000e+00,-2.9802e-08,-2.9802e-08,5.5557e-01,8.3147e-01,-1.1142e-08,2.5490e-01,0.0000e+00,0.0000e+00,0.0000e+00,7.0711e-01,7.0711e-01,-9.7545e-02,2.3549e-01,0.0000e+00,-2.9802e-08,-5.9605e-08,8.3147e-01,5.5557e-01,-1.8024e-01,1.8024e-01,0.0000e+00,-8.9407e-08,-4.4703e-08,9.2388e-01,3.8268e-01,-2.3549e-01,9.7545e-02,0.0000e+00,0.0000e+00,0.0000e+00,9.8079e-01,1.9509e-01,-2.5490e-01,3.8488e-08,0.0000e+00,-2.9802e-08,-2.1073e-08,1.0000e+00,2.1073e-08,-2.3549e-01,-9.7545e-02,0.0000e+00,-5.9605e-08,4.4703e-08,9.8079e-01,-1.9509e-01,-1.8024e-01,-1.8024e-01,0.0000e+00,0.0000e+00,-4.4703e-08,9.2388e-01,-3.8268e-01,-9.7545e-02,-2.3549e-01,0.0000e+00,-5.9605e-08,5.9605e-08,8.3147e-01,-5.5557e-01,3.0396e-09,-2.5490e-01,0.0000e+00,0.0000e+00,0.0000e+00,7.0711e-01,-7.0711e-01,9.7545e-02,-2.3549e-01,0.0000e+00,0.0000e+00,0.0000e+00,5.5557e-01,-8.3147e-01,1.8024e-01,-1.8024e-01,0.0000e+00,0.0000e+00,0.0000e+00,3.8268e-01,-9.2388e-01,2.3549e-01,-9.7545e-02,0.0000e+00,-2.2352e-08,0.0000e+00,1.9509e-01,-9.8079e-01}";
            txt+="\nlocal path=sim.createPath(pathData,";
            int opt=0;
            if (commandID==ADD_COMMANDS_ADD_PATH_CIRCLE_ACCMD)
                opt+=2;
            txt+=std::to_string(opt)+",100)\nsim.setObjectSelection({path})";
            App::worldContainer->sandboxScript->executeScriptString(txt.c_str(),nullptr);
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==ADD_COMMANDS_ADD_GRAPH_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_A_GRAPH);
            CGraph* newObject=new CGraph();
            App::currentWorld->sceneObjects->addObjectToScene(newObject,false,true);

            // Following 3 on 24/3/2017
            CScriptObject* scriptObj=new CScriptObject(sim_scripttype_customizationscript);
            App::currentWorld->embeddedScriptContainer->insertScript(scriptObj);
            scriptObj->setObjectHandleThatScriptIsAttachedTo(newObject->getObjectHandle());
            scriptObj->setScriptText("graph=require('graph_customization')");
            scriptObj->setExecutionPriority(sim_scriptexecorder_last);

            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==ADD_COMMANDS_ADD_VISION_SENSOR_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_A_VISION_SENSOR);
            CVisionSensor* newObject=new CVisionSensor();
            App::currentWorld->sceneObjects->addObjectToScene(newObject,false,true);
            newObject->setLocalTransformation(C3Vector(0.0f,0.0f,newObject->getSize()(2)));
            newObject->setPerspectiveOperation(true);
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==ADD_COMMANDS_ADD_FORCE_SENSOR_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_A_FORCE_SENSOR);
            CForceSensor* newObject=new CForceSensor();
            App::currentWorld->sceneObjects->addObjectToScene(newObject,false,true);
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if ( (commandID==ADD_COMMANDS_ADD_RAY_PROXSENSOR_ACCMD)||(commandID==ADD_COMMANDS_ADD_PYRAMID_PROXSENSOR_ACCMD)||(commandID==ADD_COMMANDS_ADD_CYLINDER_PROXSENSOR_ACCMD)||
        (commandID==ADD_COMMANDS_ADD_DISC_PROXSENSOR_ACCMD)||(commandID==ADD_COMMANDS_ADD_CONE_PROXSENSOR_ACCMD)||(commandID==ADD_COMMANDS_ADD_RANDOMIZED_RAY_PROXSENSOR_ACCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_A_PROXIMITY_SENSOR);
            CProxSensor* newObject=new CProxSensor();
            int type=sim_proximitysensor_pyramid_subtype;
            if (commandID==ADD_COMMANDS_ADD_CYLINDER_PROXSENSOR_ACCMD)
                type=sim_proximitysensor_cylinder_subtype;
            if (commandID==ADD_COMMANDS_ADD_DISC_PROXSENSOR_ACCMD)
                type=sim_proximitysensor_disc_subtype;
            if (commandID==ADD_COMMANDS_ADD_CONE_PROXSENSOR_ACCMD)
                type=sim_proximitysensor_cone_subtype;
            if (commandID==ADD_COMMANDS_ADD_RAY_PROXSENSOR_ACCMD)
                type=sim_proximitysensor_ray_subtype;
            if (commandID==ADD_COMMANDS_ADD_RANDOMIZED_RAY_PROXSENSOR_ACCMD)
                type=sim_proximitysensor_ray_subtype;
            newObject->setSensorType(type);
            if (commandID==ADD_COMMANDS_ADD_RANDOMIZED_RAY_PROXSENSOR_ACCMD)
                newObject->setRandomizedDetection(true);
            App::currentWorld->sceneObjects->addObjectToScene(newObject,false,true);
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==ADD_COMMANDS_ADD_CONVEX_DECOMPOSITION_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            // CSceneObjectOperations::addRootObjectChildrenToSelection(sel);

            std::vector<int> newSelection;

            SUIThreadCommand cmdIn; // leave empty for default parameters
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=DISPLAY_CONVEX_DECOMPOSITION_DIALOG_UITHREADCMD;
            App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
            bool addExtraDistPoints=cmdOut.boolParams[0];
            bool addFacesPoints=cmdOut.boolParams[1];
            int nClusters=cmdOut.intParams[0];
            int maxHullVertices=cmdOut.intParams[1];
            float maxConcavity=cmdOut.floatParams[0];
            float smallClusterThreshold=cmdOut.floatParams[1];
            int maxTrianglesInDecimatedMesh=cmdOut.intParams[2];
            float maxConnectDist=cmdOut.floatParams[2];
            bool individuallyConsiderMultishapeComponents=cmdOut.boolParams[2];
            int maxIterations=cmdOut.intParams[3];
            bool cancel=cmdOut.boolParams[4];
            bool useHACD=cmdOut.boolParams[5];
            bool pca=cmdOut.boolParams[6];
            bool voxelBased=cmdOut.boolParams[7];
            int resolution=cmdOut.intParams[4];
            int depth=cmdOut.intParams[5];
            int planeDownsampling=cmdOut.intParams[6];
            int convexHullDownsampling=cmdOut.intParams[7];
            int maxNumVerticesPerCH=cmdOut.intParams[8];
            float concavity=cmdOut.floatParams[3];
            float alpha=cmdOut.floatParams[4];
            float beta=cmdOut.floatParams[5];
            float gamma=cmdOut.floatParams[6];
            float minVolumePerCH=cmdOut.floatParams[7];
            if (!cancel)
            {
                App::uiThread->showOrHideProgressBar(true,-1,"Computing convex decomposed shape(s)...");
                App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_CONVEX_DECOMPOSITION);
                for (int obji=0;obji<int(sel.size());obji++)
                {
                    CShape* oldShape=App::currentWorld->sceneObjects->getShapeFromHandle(sel[obji]);
                    if (oldShape!=nullptr)
                    {
                        int newShapeHandle=CSceneObjectOperations::generateConvexDecomposed(sel[obji],nClusters,maxConcavity,addExtraDistPoints,addFacesPoints,
                                                                                    maxConnectDist,maxTrianglesInDecimatedMesh,maxHullVertices,
                                                                                    smallClusterThreshold,individuallyConsiderMultishapeComponents,
                                                                                    maxIterations,useHACD,resolution,depth,concavity,planeDownsampling,
                                                                                    convexHullDownsampling,alpha,beta,gamma,pca,voxelBased,
                                                                                    maxNumVerticesPerCH,minVolumePerCH);
                        if (newShapeHandle!=-1)
                        {
                            // Get the mass and inertia info from the old shape:
                            C7Vector absCOM(oldShape->getFullCumulativeTransformation());
                            absCOM=absCOM*oldShape->getMeshWrapper()->getLocalInertiaFrame();
                            float mass=oldShape->getMeshWrapper()->getMass();
                            C7Vector absCOMNoShift(absCOM);
                            absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
                            C3X3Matrix tensor(CMeshWrapper::getNewTensor(oldShape->getMeshWrapper()->getPrincipalMomentsOfInertia(),absCOMNoShift));

                            // Fix the name and add the new shape to the new selection vector:
                            CShape* newShape=App::currentWorld->sceneObjects->getShapeFromHandle(newShapeHandle);
                            App::currentWorld->sceneObjects->setObjectAlias(newShape,"generated_part",true);
                            App::currentWorld->sceneObjects->setObjectName_old(newShape,"generated_part",true);
                            App::currentWorld->sceneObjects->setObjectAltName_old(newShape,"generated_part",true);
                            newSelection.push_back(newShapeHandle);

                            // Transfer the mass and inertia info to the new shape:
                            newShape->getMeshWrapper()->setMass(mass);
                            C4Vector rot;
                            C3Vector pmoi;
                            CMeshWrapper::findPrincipalMomentOfInertia(tensor,rot,pmoi);
                            newShape->getMeshWrapper()->setPrincipalMomentsOfInertia(pmoi);
                            absCOM.Q=rot;
                            C7Vector relCOM(newShape->getFullCumulativeTransformation().getInverse()*absCOM);
                            newShape->getMeshWrapper()->setLocalInertiaFrame(relCOM);
                        }
                    }
                }

                App::uiThread->showOrHideProgressBar(false);

                App::currentWorld->sceneObjects->deselectObjects();
                for (size_t i=0;i<newSelection.size();i++)
                    App::currentWorld->sceneObjects->addObjectToSelection(newSelection[i]);
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==ADD_COMMANDS_ADD_CONVEX_HULL_ACCMD)
    { 
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            CSceneObjectOperations::addRootObjectChildrenToSelection(sel);

            // Now keep only visible objects:
            std::vector<CSceneObject*> inputObjects;
            for (size_t i=0;i<sel.size();i++)
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
                if ( (it!=nullptr)&&(!it->isObjectPartOfInvisibleModel())&&(App::currentWorld->environment->getActiveLayers()&it->getVisibilityLayer()) )
                    inputObjects.push_back(it);
            }

            App::uiThread->showOrHideProgressBar(true,-1,"Computing convex hull...");
            App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_CONVEX_HULL);
            App::currentWorld->sceneObjects->deselectObjects();

            CShape* hull=addConvexHull(inputObjects,true);

            App::uiThread->showOrHideProgressBar(false);

            if (hull!=nullptr)
            {
                App::currentWorld->sceneObjects->addObjectToSelection(hull->getObjectHandle());
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
            }
            else
                App::logMsg(sim_verbosity_errors,IDSNS_FAILED_IS_THE_QHULL_PLUGIN_LOADED);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==ADD_COMMANDS_ADD_GROWN_CONVEX_HULL_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            CSceneObjectOperations::addRootObjectChildrenToSelection(sel);

            // Now keep only visible objects:
            std::vector<CSceneObject*> inputObjects;
            for (size_t i=0;i<sel.size();i++)
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(sel[i]);
                if ( (it!=nullptr)&&(!it->isObjectPartOfInvisibleModel())&&(App::currentWorld->environment->getActiveLayers()&it->getVisibilityLayer()) )
                    inputObjects.push_back(it);
            }
            float grow=0.03f;
            bool doIt=true;
#ifdef SIM_WITH_GUI
            doIt=(inputObjects.size()>0)&&App::uiThread->dialogInputGetFloat(App::mainWindow,"Convex hull","Inflation parameter",0.05f,0.001f,10.0f,3,&grow);
#endif
            App::currentWorld->sceneObjects->deselectObjects();

            if (doIt)
            {
                App::uiThread->showOrHideProgressBar(true,-1,"Computing inflated convex hull...");
                App::logMsg(sim_verbosity_msgs,IDSNS_ADDING_CONVEX_HULL);

                CShape* hull=addInflatedConvexHull(inputObjects,grow);

                App::uiThread->showOrHideProgressBar(false);

                if (hull==nullptr)
                    App::logMsg(sim_verbosity_errors,IDSNS_FAILED_IS_THE_QHULL_PLUGIN_LOADED);
                else
                {
                    App::currentWorld->sceneObjects->addObjectToSelection(hull->getObjectHandle());
                    App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                    App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
                }
            }
            else
                App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    return(false);
}

CShape* CAddOperations::addPrimitiveShape(int type,const C3Vector& sizes,const int subdiv[3],int faces,int sides,int discSubdiv,bool smooth,int openEnds,bool dynamic,bool pure,bool cone,float density)
{ // subdiv can be nullptr
    int sdiv[3]={0,0,0};
    if (subdiv!=nullptr)
    {
        sdiv[0]=subdiv[0];
        sdiv[1]=subdiv[1];
        sdiv[2]=subdiv[2];
    }

    if (type==0)
    { // plane
        int divX=sdiv[0]+1;
        int divY=sdiv[1]+1;
        float xhSize=sizes(0)/2.0f;
        float yhSize=sizes(1)/2.0f;
        float xs=sizes(0)/((float)divX);
        float ys=sizes(1)/((float)divY);
        std::vector<float> vertices;
        std::vector<int> indices;
        std::vector<float> normals;
        vertices.reserve((divX+1)*(divY+1)*3);
        indices.reserve(divX*divY*6);
        normals.reserve(divX*divY*6*3);
        // We first create the vertices:
        for (int i=0;i<(divY+1);i++)
        { // along y
            for (int j=0;j<(divX+1);j++)
            { // along x
                tt::addToFloatArray(&vertices,-xhSize+j*xs,-yhSize+i*ys,0.0f);
            }
        }
        // Now we create the indices:
        for (int i=0;i<divY;i++)
        { // along y
            for (int j=0;j<divX;j++)
            { // along x
                tt::addToIntArray(&indices,i*(divX+1)+j,i*(divX+1)+j+1,(i+1)*(divX+1)+j+1);
                tt::addToIntArray(&indices,i*(divX+1)+j,(i+1)*(divX+1)+j+1,(i+1)*(divX+1)+j);
            }
        }
        // And now the normals:
        for (int i=0;i<divX*divY*6;i++)
            tt::addToFloatArray(&normals,0.0f,0.0f,1.0f);

        CShape* it=new CShape(nullptr,vertices,indices,nullptr,nullptr);
        it->getSingleMesh()->color.setDefaultValues();
        it->setColor(nullptr,sim_colorcomponent_ambient_diffuse,1.0f,1.0f,1.0f);
        it->getSingleMesh()->setVisibleEdges(false);
        it->getSingleMesh()->setShadingAngle(20.0f*degToRad_f);
        it->getSingleMesh()->setEdgeThresholdAngle(20.0f*degToRad_f);
        it->setObjectAlias_direct(IDSOGL_PLANE);
        it->setObjectName_direct_old(IDSOGL_PLANE);
        it->setObjectAltName_direct_old(tt::getObjectAltNameFromObjectName(it->getObjectName_old().c_str()).c_str());
        it->setLocalTransformation(C3Vector(0.0f,0.0f,0.002f)); // we shift the plane so that it is above the floor
        it->alignBoundingBoxWithWorld();
        if (pure)
            it->getSingleMesh()->setPurePrimitiveType(sim_pure_primitive_plane,sizes(0),sizes(1),0.0001f);
        if (dynamic)
        {
            int propToRemove=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable;
            it->setLocalObjectSpecialProperty((it->getLocalObjectSpecialProperty()|propToRemove)-propToRemove);
            it->setRespondable(true);
            it->setShapeIsDynamicallyStatic(false);
            it->setColor(nullptr,sim_colorcomponent_ambient_diffuse,0.85f,0.85f,1.0f);
        }
        it->getMeshWrapper()->setMass(sizes(0)*sizes(1)*density*0.001f); // we assume 1mm thickness
        it->getMeshWrapper()->setPrincipalMomentsOfInertia(C3Vector(sizes(1)*sizes(1)/12.0f,sizes(0)*sizes(0)/12.0f,(sizes(0)*sizes(0)+sizes(1)*sizes(1))/12.0f));

        // make sure that object rests, stacks and can be easily grasped:
        it->getDynMaterial()->generateDefaultMaterial(sim_dynmat_reststackgrasp);

        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        return(it);
    }

    if (type==1)
    { // rectangle
        int divX=sdiv[0]+1;
        int divY=sdiv[1]+1;
        int divZ=sdiv[2]+1;
        float xhSize=sizes(0)/2.0f;
        float yhSize=sizes(1)/2.0f;
        float zhSize=sizes(2)/2.0f;
        std::vector<float> vertices;
        std::vector<int> indices;
        vertices.reserve(((divX+1)*(2*divY+2*divZ)+2*(divY-1)*(divZ-1))*3);
        indices.reserve(divX*divY*12+divX*divZ*12+divY*divZ*12);
        int theDiv[3]={divX,divY,divZ};

        CMeshRoutines::createCube(vertices,indices,C3Vector(xhSize*2.0f,yhSize*2.0f,zhSize*2.0f),theDiv);

        CShape* it=new CShape(nullptr,vertices,indices,nullptr,nullptr);
        it->getSingleMesh()->color.setDefaultValues();
        it->setColor(nullptr,sim_colorcomponent_ambient_diffuse,1.0f,1.0f,1.0f);
        it->getSingleMesh()->setVisibleEdges(false);
        it->getSingleMesh()->setShadingAngle(20.0f*degToRad_f);
        it->getSingleMesh()->setEdgeThresholdAngle(20.0f*degToRad_f);
        it->setObjectAlias_direct(IDSOGL_RECTANGLE);
        it->setObjectName_direct_old(IDSOGL_RECTANGLE);
        it->setObjectAltName_direct_old(tt::getObjectAltNameFromObjectName(it->getObjectName_old().c_str()).c_str());
        it->setLocalTransformation(C3Vector(0.0f,0.0f,zhSize)); // we shift the rectangle so that it sits on the floor
        it->alignBoundingBoxWithWorld();
        if (pure)
            it->getSingleMesh()->setPurePrimitiveType(sim_pure_primitive_cuboid,sizes(0),sizes(1),sizes(2));
        if (dynamic)
        {
            int propToRemove=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable;
            it->setLocalObjectSpecialProperty((it->getLocalObjectSpecialProperty()|propToRemove)-propToRemove);
            it->setRespondable(true);
            it->setShapeIsDynamicallyStatic(false);
            it->setColor(nullptr,sim_colorcomponent_ambient_diffuse,0.85f,0.85f,1.0f);
        }

        it->getMeshWrapper()->setMass(sizes(0)*sizes(1)*sizes(2)*density);
        it->getMeshWrapper()->setPrincipalMomentsOfInertia(C3Vector((sizes(1)*sizes(1)+sizes(2)*sizes(2))/12.0f,(sizes(0)*sizes(0)+sizes(2)*sizes(2))/12.0f,(sizes(0)*sizes(0)+sizes(1)*sizes(1))/12.0f));

        // make sure that object rests, stacks and can be easily grasped:
        it->getDynMaterial()->generateDefaultMaterial(sim_dynmat_reststackgrasp);
        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        return(it);
    }

    if (type==2)
    { // sphere
        float xhSize=sizes(0)/2.0f;
        float yhSize=sizes(1)/2.0f;
        float zhSize=sizes(2)/2.0f;
        std::vector<float> vertices;
        std::vector<int> indices;
        vertices.reserve(((faces-1)*sides+2)*3);
        indices.reserve((sides*2+2*(faces-2)*sides)*3);
        if (sides<3)
            sides=3;
        if (faces<2)
            faces=2;

        CMeshRoutines::createSphere(vertices,indices,C3Vector(xhSize*2.0f,yhSize*2.0f,zhSize*2.0f),sides,faces);

        CShape* it=new CShape(nullptr,vertices,indices,nullptr,nullptr);
        it->getSingleMesh()->color.setDefaultValues();
        it->setColor(nullptr,sim_colorcomponent_ambient_diffuse,1.0f,1.0f,1.0f);
        if (smooth)
        {
            it->getSingleMesh()->setShadingAngle(20.0f*degToRad_f);
            it->getSingleMesh()->setEdgeThresholdAngle(20.0f*degToRad_f);
        }
        it->setObjectAlias_direct(IDSOGL_SPHERE);
        it->setObjectName_direct_old(IDSOGL_SPHERE);
        it->setObjectAltName_direct_old(tt::getObjectAltNameFromObjectName(it->getObjectName_old().c_str()).c_str());
        it->setLocalTransformation(C3Vector(0.0f,0.0f,zhSize)); // we shift the sphere so that it sits on the floor
        it->alignBoundingBoxWithWorld();
        if (pure)
            it->getSingleMesh()->setPurePrimitiveType(sim_pure_primitive_spheroid,sizes(0),sizes(1),sizes(2));
        if (dynamic)
        {
            int propToRemove=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable;
            it->setLocalObjectSpecialProperty((it->getLocalObjectSpecialProperty()|propToRemove)-propToRemove);
            it->setRespondable(true);
            it->setShapeIsDynamicallyStatic(false);
            it->setColor(nullptr,sim_colorcomponent_ambient_diffuse,0.85f,0.85f,1.0f);
        }
        float avR=(sizes(0)+sizes(1)+sizes(2))/6.0f;

        it->getMeshWrapper()->setMass((4.0f*piValue_f/3.0f)*avR*avR*avR*density);
        it->getMeshWrapper()->setPrincipalMomentsOfInertia(C3Vector(2.0f*avR*avR/5.0f,2.0f*avR*avR/5.0f,2.0f*avR*avR/5.0f));
        float avr2=avR*2.0f;
        it->getMeshWrapper()->scaleMassAndInertia(sizes(0)/avr2,sizes(1)/avr2,sizes(2)/avr2);

        // make sure that object rests, stacks and can be easily grasped:
        it->getDynMaterial()->generateDefaultMaterial(sim_dynmat_reststackgrasp);

        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        return(it);
    }

    if (type==3)
    { // cylinder
        faces++;
        float xhSize=sizes(0)/2.0f;
        float yhSize=sizes(1)/2.0f;
        float zhSize=sizes(2)/2.0f;
        int discDiv=discSubdiv+1;
        std::vector<float> vertices;
        std::vector<int> indices;
        vertices.reserve((sides*(1+faces)+2+(discDiv-1)*sides*2)*3);
        indices.reserve((sides*2+sides*faces*2+(discDiv-1)*4*sides)*3);
        if (sides<3)
            sides=3;

        CMeshRoutines::createCylinder(vertices,indices,C3Vector(xhSize*2.0f,yhSize*2.0f,zhSize*2.0f),sides,faces,discDiv,openEnds,cone);

        CShape* it=new CShape(nullptr,vertices,indices,nullptr,nullptr);
        it->getSingleMesh()->color.setDefaultValues();
        it->setColor(nullptr,sim_colorcomponent_ambient_diffuse,1.0f,1.0f,1.0f);
        it->getSingleMesh()->setVisibleEdges(false);
        if (smooth)
        {
            it->getSingleMesh()->setShadingAngle(20.0f*degToRad_f);
            it->getSingleMesh()->setEdgeThresholdAngle(20.0f*degToRad_f);
        }
        it->setObjectAlias_direct(IDSOGL_CYLINDER);
        it->setObjectName_direct_old(IDSOGL_CYLINDER);
        it->setObjectAltName_direct_old(tt::getObjectAltNameFromObjectName(it->getObjectName_old().c_str()).c_str());
        it->setLocalTransformation(C3Vector(0.0f,0.0f,zhSize)); // Now we shift the cylinder so it sits on the floor
        it->alignBoundingBoxWithWorld();
        if (pure)
        {
            if (cone)
                it->getSingleMesh()->setPurePrimitiveType(sim_pure_primitive_cone,sizes(0),sizes(1),sizes(2));
            else
                it->getSingleMesh()->setPurePrimitiveType(sim_pure_primitive_cylinder,sizes(0),sizes(1),sizes(2));
        }
        if (dynamic)
        {
            int propToRemove=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable;
            it->setLocalObjectSpecialProperty((it->getLocalObjectSpecialProperty()|propToRemove)-propToRemove);
            it->setRespondable(true);
            it->setShapeIsDynamicallyStatic(false);
            it->setColor(nullptr,sim_colorcomponent_ambient_diffuse,0.85f,0.85f,1.0f);
        }
        float avR=(sizes(0)+sizes(1))/4.0f;
        float divider=1.0f;
        if (cone)
            divider=3.0f;

        it->getMeshWrapper()->setMass(piValue_f*avR*avR*divider*sizes(2)*density);
        if (cone)
            it->getMeshWrapper()->setPrincipalMomentsOfInertia(C3Vector(3.0f*(0.25f*avR*avR+sizes(2)*sizes(2))/5.0f,3.0f*(0.25f*avR*avR+sizes(2)*sizes(2))/5.0f,3.0f*avR*avR/10.0f));
        else
            it->getMeshWrapper()->setPrincipalMomentsOfInertia(C3Vector((3.0f*avR*avR+sizes(2)*sizes(2))/12.0f,(3.0f*avR*avR+sizes(2)*sizes(2))/12.0f,avR*avR/2.0f));
        float avR2=avR*2.0f;
        it->getMeshWrapper()->scaleMassAndInertia(sizes(0)/avR2,sizes(1)/avR2,1.0f);

        // make sure that object rests, stacks and can be easily grasped:
        it->getDynMaterial()->generateDefaultMaterial(sim_dynmat_reststackgrasp);

        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        return(it);
    }

    if (type==4)
    { // disc
        float xhSize=sizes(0)/2.0f;
        float yhSize=sizes(1)/2.0f;
        int discDiv=discSubdiv+1;
        std::vector<float> vertices;
        std::vector<int> indices;
        vertices.reserve((discDiv*sides*2+2)*3);
        indices.reserve(((discDiv-1)*2*sides)*3);
        float dd=1.0f/((float)discDiv);
        if (sides<3) sides=3;
        float sa=2.0f*piValue_f/((float)sides);
        // The two middle vertices:
        int sideStart=1;
        tt::addToFloatArray(&vertices,0.0f,0.0f,0.0f);
        for (int i=0;i<sides;i++)
        { // The side vertices:
            tt::addToFloatArray(&vertices,(float)cos(sa*i),(float)sin(sa*i),0.0f);
        }
        int dstStart=(int)vertices.size()/3;
        // The disc subdivision vertices:
        for (int i=1;i<discDiv;i++)
            for (int j=0;j<sides;j++)
                tt::addToFloatArray(&vertices,(1.0f-dd*i)*(float)cos(sa*j),(1.0f-dd*i)*(float)sin(sa*j),0.0f);
        // We set up the indices:
        for (int i=0;i<sides-1;i++)
        {
            if (discDiv==1)
                tt::addToIntArray(&indices,0,i+sideStart,(i+1)+sideStart);
            else
            {
                tt::addToIntArray(&indices,0,dstStart+i+sides*(discDiv-2),dstStart+i+sides*(discDiv-2)+1);
                for (int j=0;j<discDiv-2;j++)
                {
                    tt::addToIntArray(&indices,dstStart+j*sides+i,dstStart+j*sides+i+1,dstStart+(j+1)*sides+i);
                    tt::addToIntArray(&indices,dstStart+j*sides+i+1,dstStart+(j+1)*sides+i+1,dstStart+(j+1)*sides+i);
                }
                tt::addToIntArray(&indices,sideStart+i,sideStart+(i+1),dstStart+i);
                tt::addToIntArray(&indices,sideStart+(i+1),dstStart+i+1,dstStart+i);
            }
        }

        // We have to close the cylinder here:
        // First top and bottom part:
        if (discDiv==1)
            tt::addToIntArray(&indices,0,(sides-1)+sideStart,sideStart);
        else
        {
            tt::addToIntArray(&indices,0,dstStart+sides*(discDiv-1)-1,dstStart+sides*(discDiv-2));
            for (int j=0;j<discDiv-2;j++)
            {
                tt::addToIntArray(&indices,dstStart+j*sides+sides-1,dstStart+j*sides,dstStart+(j+2)*sides-1);
                tt::addToIntArray(&indices,dstStart+j*sides,dstStart+(j+1)*sides,dstStart+(j+2)*sides-1);
            }
            tt::addToIntArray(&indices,sideStart+(sides-1),sideStart,dstStart+sides-1);
            tt::addToIntArray(&indices,sideStart,dstStart,dstStart+sides-1);
        }
        // Now we scale the disc:
        for (int i=0;i<int(vertices.size())/3;i++)
        {
            C3Vector p(vertices[3*i+0],vertices[3*i+1],0.0f);
            p(0)=p(0)*xhSize;
            p(1)=p(1)*yhSize;
            vertices[3*i+0]=p(0);
            vertices[3*i+1]=p(1);
        }

        CShape* it=new CShape(nullptr,vertices,indices,nullptr,nullptr);
        it->getSingleMesh()->color.setDefaultValues();
        it->setColor(nullptr,sim_colorcomponent_ambient_diffuse,1.0f,1.0f,1.0f);
        it->getSingleMesh()->setVisibleEdges(false);
        it->getSingleMesh()->setShadingAngle(20.0f*degToRad_f);
        it->getSingleMesh()->setEdgeThresholdAngle(20.0f*degToRad_f);
        it->setObjectAlias_direct(IDSOGL_DISC);
        it->setObjectName_direct_old(IDSOGL_DISC);
        it->setObjectAltName_direct_old(tt::getObjectAltNameFromObjectName(it->getObjectName_old().c_str()).c_str());
        it->setLocalTransformation(C3Vector(0.0f,0.0f,0.002f)); // Now we shift the disc so it sits just above the floor
        it->alignBoundingBoxWithWorld();
        if (pure)
            it->getSingleMesh()->setPurePrimitiveType(sim_pure_primitive_disc,sizes(0),sizes(1),0.0001f);
        if (dynamic)
        {
            int propToRemove=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable;
            it->setLocalObjectSpecialProperty((it->getLocalObjectSpecialProperty()|propToRemove)-propToRemove);
            it->setRespondable(true);
            it->setShapeIsDynamicallyStatic(false);
            it->setColor(nullptr,sim_colorcomponent_ambient_diffuse,0.85f,0.85f,1.0f);
        }
        float avR=(sizes(0)+sizes(1))/4.0f;
        it->getMeshWrapper()->setMass(piValue_f*avR*avR*density*0.001f); // we assume 1mm thickness
        it->getMeshWrapper()->setPrincipalMomentsOfInertia(C3Vector(3.0f*(avR*avR)/12.0f,3.0f*(avR*avR)/12.0f,avR*avR/2.0f));
        float avR2=avR*2.0f;
        it->getMeshWrapper()->scaleMassAndInertia(sizes(0)/avR2,sizes(1)/avR2,1.0f);

        // make sure that object rests, stacks and can be easily grasped:
        it->getDynMaterial()->generateDefaultMaterial(sim_dynmat_reststackgrasp);

        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        return(it);
    }

    return(nullptr);
}

CShape* CAddOperations::addInflatedConvexHull(const std::vector<CSceneObject*>& inputObjects,float margin)
{
    CShape* retVal=nullptr;
    CShape* ch=addConvexHull(inputObjects,false);
    if (ch!=nullptr)
    {
        C7Vector transf(ch->getFullCumulativeTransformation());
        std::vector<float> vertD;
        std::vector<float> vert;
        std::vector<int> ind;
        ch->getMeshWrapper()->getCumulativeMeshes(vertD,&ind,nullptr);
        for (size_t j=0;j<ind.size()/3;j++)
        {
            int indd[3]={ind[3*j+0],ind[3*j+1],ind[3*j+2]};
            C3Vector w[3]={C3Vector(&vertD[0]+3*indd[0]),C3Vector(&vertD[0]+3*indd[1]),C3Vector(&vertD[0]+3*indd[2])};
            C3Vector v12(w[0]-w[1]);
            C3Vector v13(w[0]-w[2]);
            C3Vector n(v12^v13);
            n.normalize();
            n*=margin;
            for (float k=-1.0;k<1.2f;k+=2.0)
            {
                for (int l=0;l<3;l++)
                {
                    C3Vector vv(w[l]+n*k);
                    vert.push_back(vv(0));
                    vert.push_back(vv(1));
                    vert.push_back(vv(2));
                }
            }
        }
        for (size_t j=0;j<vert.size()/3;j++)
        {
            C3Vector v(&vert[3*j+0]);
            v=transf*v;
            vert[3*j+0]=v(0);
            vert[3*j+1]=v(1);
            vert[3*j+2]=v(2);
        }

        std::vector<float> hull;
        std::vector<int> indices;
        if (CMeshRoutines::getConvexHull(&vert,&hull,&indices))
        {
            retVal=new CShape(nullptr,hull,indices,nullptr,nullptr);
            retVal->getSingleMesh()->setConvexVisualAttributes();
            retVal->setColor(nullptr,sim_colorcomponent_ambient_diffuse,1.0f,0.7f,0.7f);
            retVal->getSingleMesh()->setEdgeThresholdAngle(0.0f);
            retVal->getSingleMesh()->setShadingAngle(0.0f);
            retVal->getSingleMesh()->setVisibleEdges(false);

            // Since we extracted the convex hull from a single shape, we take over the inertia and mass parameters
            // Get the mass and inertia info from the old shape:
            C7Vector absCOM(ch->getFullCumulativeTransformation());
            absCOM=absCOM*ch->getMeshWrapper()->getLocalInertiaFrame();
            float mass=ch->getMeshWrapper()->getMass();
            C7Vector absCOMNoShift(absCOM);
            absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
            C3X3Matrix tensor(CMeshWrapper::getNewTensor(ch->getMeshWrapper()->getPrincipalMomentsOfInertia(),absCOMNoShift));

            // Transfer the mass and inertia info to the new shape:
            retVal->getMeshWrapper()->setMass(mass);
            C4Vector rot;
            C3Vector pmoi;
            CMeshWrapper::findPrincipalMomentOfInertia(tensor,rot,pmoi);
            retVal->getMeshWrapper()->setPrincipalMomentsOfInertia(pmoi);
            absCOM.Q=rot;
            C7Vector relCOM(retVal->getFullCumulativeTransformation().getInverse()*absCOM);
            retVal->getMeshWrapper()->setLocalInertiaFrame(relCOM);
            App::currentWorld->sceneObjects->eraseObject(ch,false);
            App::currentWorld->sceneObjects->addObjectToScene(retVal,false,true);
        }
        else
            App::currentWorld->sceneObjects->eraseObject(ch,false);
    }
    return(retVal);
}


CShape* CAddOperations::addConvexHull(const std::vector<CSceneObject*>& inputObjects,bool generateAfterCreateCallback)
{
    CShape* retVal=nullptr;

    std::vector<float> allHullVertices;
    CShape* oneShape=nullptr;
    for (size_t i=0;i<inputObjects.size();i++)
    {
        CSceneObject* it=inputObjects[i];
        if (it->getObjectType()==sim_object_shape_type)
        {
            CShape* shape=(CShape*)it;
            oneShape=shape;
            C7Vector transf(shape->getFullCumulativeTransformation());
            std::vector<float> vert;
            std::vector<float> vertD;
            std::vector<int> ind;
            shape->getMeshWrapper()->getCumulativeMeshes(vertD,&ind,nullptr);
            for (size_t j=0;j<vertD.size()/3;j++)
            {
                C3Vector v(&vertD[3*j+0]);
                v=transf*v;
                allHullVertices.push_back(v(0));
                allHullVertices.push_back(v(1));
                allHullVertices.push_back(v(2));
            }
        }
        if (it->getObjectType()==sim_object_dummy_type)
        {
            CDummy* dum=(CDummy*)it;
            C3Vector v(dum->getFullCumulativeTransformation().X);
            allHullVertices.push_back(v(0));
            allHullVertices.push_back(v(1));
            allHullVertices.push_back(v(2));
        }
    }
    if (allHullVertices.size()!=0)
    {
        std::vector<float> hull;
        std::vector<int> indices;
        std::vector<float> normals;
        if (CMeshRoutines::getConvexHull(&allHullVertices,&hull,&indices))
        {
            retVal=new CShape(nullptr,hull,indices,nullptr,nullptr);
            retVal->setObjectAlias_direct("convexHull");
            retVal->setObjectName_direct_old("convexHull");
            retVal->setObjectAltName_direct_old(tt::getObjectAltNameFromObjectName(retVal->getObjectName_old().c_str()).c_str());
            retVal->getSingleMesh()->setConvexVisualAttributes();
            retVal->setColor(nullptr,sim_colorcomponent_ambient_diffuse,1.0f,0.7f,0.7f);
            retVal->getSingleMesh()->setEdgeThresholdAngle(0.0f);
            retVal->getSingleMesh()->setShadingAngle(0.0f);
            retVal->getSingleMesh()->setVisibleEdges(false);

            if ( (oneShape!=nullptr)&&(inputObjects.size()==1) )
            { // Since we extracted the convex hull from a single shape, we take over the inertia and mass parameters
                // Get the mass and inertia info from the old shape:
                C7Vector absCOM(oneShape->getFullCumulativeTransformation());
                absCOM=absCOM*oneShape->getMeshWrapper()->getLocalInertiaFrame();
                float mass=oneShape->getMeshWrapper()->getMass();
                C7Vector absCOMNoShift(absCOM);
                absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
                C3X3Matrix tensor(CMeshWrapper::getNewTensor(oneShape->getMeshWrapper()->getPrincipalMomentsOfInertia(),absCOMNoShift));

                // Transfer the mass and inertia info to the new shape:
                retVal->getMeshWrapper()->setMass(mass);
                C4Vector rot;
                C3Vector pmoi;
                CMeshWrapper::findPrincipalMomentOfInertia(tensor,rot,pmoi);
                retVal->getMeshWrapper()->setPrincipalMomentsOfInertia(pmoi);
                absCOM.Q=rot;
                C7Vector relCOM(retVal->getFullCumulativeTransformation().getInverse()*absCOM);
                retVal->getMeshWrapper()->setLocalInertiaFrame(relCOM);
            }
            else
                retVal->getMeshWrapper()->setLocalInertiaFrame(C7Vector::identityTransformation);
            App::currentWorld->sceneObjects->addObjectToScene(retVal,false,generateAfterCreateCallback);
        }
    }
    return(retVal);
}

#ifdef SIM_WITH_GUI
void CAddOperations::addMenu(VMenu* menu,CSView* subView,bool onlyCamera)
{
    // subView can be null
    bool canAddChildScript=false;
    bool canAddCustomizationScript=false;
    if (App::currentWorld->sceneObjects->getSelectionCount()==1)
    {
        canAddChildScript=(App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_child(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0))==nullptr)&&App::currentWorld->simulation->isSimulationStopped();
        canAddCustomizationScript=(App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_customization(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0))==nullptr)&&App::currentWorld->simulation->isSimulationStopped();
    }

    std::vector<int> rootSel;
    for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
        rootSel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
    CSceneObjectOperations::addRootObjectChildrenToSelection(rootSel);
    size_t shapesInRootSel=App::currentWorld->sceneObjects->getShapeCountInSelection(&rootSel);
    size_t shapesAndDummiesInRootSel=App::currentWorld->sceneObjects->getShapeCountInSelection(&rootSel)+App::currentWorld->sceneObjects->getDummyCountInSelection(&rootSel);

    bool linkedObjIsInexistentOrNotGraphNorRenderingSens=true;
    if (subView!=nullptr)
        linkedObjIsInexistentOrNotGraphNorRenderingSens=((App::currentWorld->sceneObjects->getGraphFromHandle(subView->getLinkedObjectID())==nullptr)&&(App::currentWorld->sceneObjects->getVisionSensorFromHandle(subView->getLinkedObjectID())==nullptr));
    bool itemsPresent=false;
    if (subView!=nullptr)
    {
        menu->appendMenuItem(true,false,ADD_COMMANDS_ADD_FLOATING_VIEW_ACCMD,IDS_FLOATING_VIEW_MENU_ITEM);
        itemsPresent=true;
        if (onlyCamera)
            menu->appendMenuItem(linkedObjIsInexistentOrNotGraphNorRenderingSens,false,ADD_COMMANDS_ADD_CAMERA_ACCMD,IDS_CAMERA_MENU_ITEM);
    }
    CSceneObject* associatedViewable=nullptr;
    if (subView!=nullptr)
        associatedViewable=App::currentWorld->sceneObjects->getObjectFromHandle(subView->getLinkedObjectID());
    if ( (subView==nullptr)||((associatedViewable!=nullptr)&&(associatedViewable->getObjectType()==sim_object_camera_type) ) )
    {
        if (!onlyCamera)
        {
            if (itemsPresent)
                menu->appendMenuSeparator();
            VMenu* prim=new VMenu();
            prim->appendMenuItem(true,false,ADD_COMMANDS_ADD_PRIMITIVE_PLANE_ACCMD,IDS_PLANE_MENU_ITEM);
            prim->appendMenuItem(true,false,ADD_COMMANDS_ADD_PRIMITIVE_DISC_ACCMD,IDS_DISC_MENU_ITEM);
            prim->appendMenuItem(true,false,ADD_COMMANDS_ADD_PRIMITIVE_RECTANGLE_ACCMD,IDS_RECTANGLE_MENU_ITEM);
            prim->appendMenuItem(true,false,ADD_COMMANDS_ADD_PRIMITIVE_SPHERE_ACCMD,IDS_SPHERE_MENU_ITEM);
            prim->appendMenuItem(true,false,ADD_COMMANDS_ADD_PRIMITIVE_CYLINDER_ACCMD,IDS_CYLINDER_MENU_ITEM);
            menu->appendMenuAndDetach(prim,true,IDS_PRIMITIVE_SHAPE_MENU_ITEM);
            itemsPresent=true;

            if (CSimFlavor::getBoolVal(7))
            {
                VMenu* joint=new VMenu();
                joint->appendMenuItem(true,false,ADD_COMMANDS_ADD_REVOLUTE_JOINT_ACCMD,IDS_REVOLUTE_MENU_ITEM);
                joint->appendMenuItem(true,false,ADD_COMMANDS_ADD_PRISMATIC_JOINT_ACCMD,IDS_PRISMATIC_MENU_ITEM);
                joint->appendMenuItem(true,false,ADD_COMMANDS_ADD_SPHERICAL_JOINT_ACCMD,IDS_SPHERICAL_MENU_ITEM);
                menu->appendMenuAndDetach(joint,true,IDS_JOINT_MENU_ITEM);
                itemsPresent=true;
            }
        }
        else
        {
            if (itemsPresent)
                menu->appendMenuSeparator();
        }

        menu->appendMenuItem(linkedObjIsInexistentOrNotGraphNorRenderingSens,false,ADD_COMMANDS_ADD_CAMERA_ACCMD,IDS_CAMERA_MENU_ITEM);

        if (CSimFlavor::getBoolVal(7)&&App::userSettings->enableOldMirrorObjects)
            menu->appendMenuItem(true,false,ADD_COMMANDS_ADD_MIRROR_ACCMD,IDS_MIRROR_MENU_ITEM);

        itemsPresent=true;

        if ( CSimFlavor::getBoolVal(7)&&(!onlyCamera) )
        {
            VMenu* light=new VMenu();
            light->appendMenuItem(linkedObjIsInexistentOrNotGraphNorRenderingSens,false,ADD_COMMANDS_ADD_OMNI_LIGHT_ACCMD,IDS_OMNIDIRECTIONAL_MENU_ITEM);
            light->appendMenuItem(linkedObjIsInexistentOrNotGraphNorRenderingSens,false,ADD_COMMANDS_ADD_SPOT_LIGHT_ACCMD,IDS_SPOTLIGHT_MENU_ITEM);
            light->appendMenuItem(linkedObjIsInexistentOrNotGraphNorRenderingSens,false,ADD_COMMANDS_ADD_DIR_LIGHT_ACCMD,IDS_DIRECTIONAL_MENU_ITEM);
            if (linkedObjIsInexistentOrNotGraphNorRenderingSens)
                menu->appendMenuAndDetach(light,true,IDS_LIGHT_MENU_ITEM);
            else
            {
                menu->appendMenuItem(false,false,0,IDS_LIGHT_MENU_ITEM);
                delete light;
            }

            menu->appendMenuItem(true,false,ADD_COMMANDS_ADD_DUMMY_ACCMD,IDS_DUMMY_MENU_ITEM);
            menu->appendMenuItem(true,false,ADD_COMMANDS_ADD_POINTCLOUD_ACCMD,IDS_POINTCLOUD_MENU_ITEM);
            menu->appendMenuItem(true,false,ADD_COMMANDS_ADD_OCTREE_ACCMD,IDS_OCTREE_MENU_ITEM);

            menu->appendMenuItem(true,false,ADD_COMMANDS_ADD_GRAPH_ACCMD,IDS_GRAPH_MENU_ITEM);

            VMenu* sens=new VMenu();
            sens->appendMenuItem(true,false,ADD_COMMANDS_ADD_RAY_PROXSENSOR_ACCMD,IDS_RAY_TYPE_MENU_ITEM);
            sens->appendMenuItem(true,false,ADD_COMMANDS_ADD_RANDOMIZED_RAY_PROXSENSOR_ACCMD,IDS_RANDOMIZED_RAY_TYPE_MENU_ITEM);
            sens->appendMenuItem(true,false,ADD_COMMANDS_ADD_PYRAMID_PROXSENSOR_ACCMD,IDS_PYRAMID_TYPE_MENU_ITEM);
            sens->appendMenuItem(true,false,ADD_COMMANDS_ADD_CYLINDER_PROXSENSOR_ACCMD,IDS_CYLINDER_TYPE_MENU_ITEM);
            sens->appendMenuItem(true,false,ADD_COMMANDS_ADD_DISC_PROXSENSOR_ACCMD,IDS_DISC_TYPE_MENU_ITEM);
            sens->appendMenuItem(true,false,ADD_COMMANDS_ADD_CONE_PROXSENSOR_ACCMD,IDS_CONE_TYPE_MENU_ITEM);

            menu->appendMenuAndDetach(sens,true,IDS_PROXSENSOR_MENU_ITEM);

            menu->appendMenuItem(true,false,ADD_COMMANDS_ADD_VISION_SENSOR_ACCMD,IDSN_VISION_SENSOR);

            menu->appendMenuItem(true,false,ADD_COMMANDS_ADD_FORCE_SENSOR_ACCMD,IDSN_FORCE_SENSOR);


            VMenu* pathM=new VMenu();
            pathM->appendMenuItem(true,false,ADD_COMMANDS_ADD_PATH_SEGMENT_ACCMD,IDS_SEGMENT_TYPE_MENU_ITEM);
            pathM->appendMenuItem(true,false,ADD_COMMANDS_ADD_PATH_CIRCLE_ACCMD,IDS_CIRCLE_TYPE_MENU_ITEM);
            menu->appendMenuAndDetach(pathM,true,IDSN_PATH);

            VMenu* childScript=new VMenu();
            childScript->appendMenuItem(true,false,ADD_COMMANDS_ADD_AND_ASSOCIATE_NON_THREADED_CHILD_SCRIPT_ACCMD,"Non threaded");
            childScript->appendMenuItem(true,false,ADD_COMMANDS_ADD_AND_ASSOCIATE_THREADED_CHILD_SCRIPT_ACCMD,"Threaded");
            if (App::userSettings->keepOldThreadedScripts)
                childScript->appendMenuItem(true,false,ADD_COMMANDS_ADD_AND_ASSOCIATE_oldTHREADED_CHILD_SCRIPT_ACCMD,"Threaded (deprecated, compatibility version)");
            menu->appendMenuAndDetach(childScript,canAddChildScript,"Associated child script");

            VMenu* customizationScript=new VMenu();
            customizationScript->appendMenuItem(true,false,ADD_COMMANDS_ADD_AND_ASSOCIATE_NON_THREADED_CUSTOMIZATION_SCRIPT_ACCMD,"Non threaded");
            customizationScript->appendMenuItem(true,false,ADD_COMMANDS_ADD_AND_ASSOCIATE_THREADED_CUSTOMIZATION_SCRIPT_ACCMD,"Threaded");
            menu->appendMenuAndDetach(customizationScript,canAddCustomizationScript,"Associated customization script");

            menu->appendMenuSeparator();
            menu->appendMenuItem(shapesAndDummiesInRootSel>0,false,ADD_COMMANDS_ADD_CONVEX_HULL_ACCMD,IDS_CONVEX_HULL_OF_SELECTION_MENU_ITEM);
            menu->appendMenuItem(shapesInRootSel>0,false,ADD_COMMANDS_ADD_GROWN_CONVEX_HULL_ACCMD,IDS_GROWN_CONVEX_HULL_OF_SELECTED_SHAPE_MENU_ITEM);
            menu->appendMenuItem(shapesInRootSel>0,false,ADD_COMMANDS_ADD_CONVEX_DECOMPOSITION_ACCMD,IDS_CONVEX_DECOMPOSITION_OF_SELECTION_MENU_ITEM);
        }
    }
}

CShape* CAddOperations::addPrimitive_withDialog(int command,const C3Vector* optSizes)
{ // if optSizes is not nullptr, then sizes are locked in the dialog
    CShape* retVal=nullptr;
    int pType=-1;
    if (command==ADD_COMMANDS_ADD_PRIMITIVE_PLANE_ACCMD)
        pType=0;
    if (command==ADD_COMMANDS_ADD_PRIMITIVE_RECTANGLE_ACCMD)
        pType=1;
    if (command==ADD_COMMANDS_ADD_PRIMITIVE_SPHERE_ACCMD)
        pType=2;
    if (command==ADD_COMMANDS_ADD_PRIMITIVE_CYLINDER_ACCMD)
        pType=3;
    if (command==ADD_COMMANDS_ADD_PRIMITIVE_DISC_ACCMD)
        pType=4;
    if (pType!=-1)
    {
        C3Vector sizes;
        int subdiv[3];
        int faces,sides,discSubdiv,openEnds;
        bool smooth,dynamic,pure,cone;
        float density;
        if (App::uiThread->showPrimitiveShapeDialog(pType,optSizes,sizes,subdiv,faces,sides,discSubdiv,smooth,openEnds,dynamic,pure,cone,density))
            retVal=addPrimitiveShape(pType,sizes,subdiv,faces,sides,discSubdiv,smooth,openEnds,dynamic,pure,cone,density);
    }
    return(retVal);
}
#endif
