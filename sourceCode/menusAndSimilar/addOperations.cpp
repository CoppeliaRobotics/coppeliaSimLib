
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "addOperations.h"
#include "tt.h"
#include "meshRoutines.h"
#include "sceneObjectOperations.h"
#include "pluginContainer.h"
#include "app.h"
#include "meshManip.h"
#include "geometric.h"
#include "v_repStrings.h"
#include <boost/lexical_cast.hpp>
#include "miscBase.h"

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
            App::addStatusbarMessage(IDSNS_ADDING_A_PRIMITIVE_SHAPE);

            CShape* newShape=addPrimitive_withDialog(commandID,nullptr);
            int shapeHandle=-1;
            if (newShape!=nullptr)
                shapeHandle=newShape->getObjectHandle();
            if (shapeHandle!=-1)
            {
                App::ct->objCont->deselectObjects();
                App::ct->objCont->selectObject(shapeHandle);
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                App::addStatusbarMessage(IDSNS_DONE);
            }
            else
                App::addStatusbarMessage(IDSNS_OPERATION_ABORTED);
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
    if (commandID==ADD_COMMANDS_MAKE_GRAPH_CURVE_STATIC_ACCMD)
    { // can be executed via the UI or NON-UI thread!
        if (subView!=nullptr)
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                int lo=-1;
                lo=subView->getLinkedObjectID();
                CGraph* graph=App::ct->objCont->getGraph(lo);
                int val=0;
                if (!subView->getTimeGraph())
                    val=1;
                if ((graph!=nullptr)&&(subView->getTrackedGraphCurveIndex()!=-1))
                {
                    App::addStatusbarMessage(IDSNS_ADDING_STATIC_DUPLICATE_OF_CURVE);
                    graph->makeCurveStatic(subView->getTrackedGraphCurveIndex(),val);
                    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                    App::addStatusbarMessage(IDSNS_DONE);
                }
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                cmd.objectParams.push_back(subView);
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }

    if (commandID==COPY_GRAPH_CURVE_TO_CLIPBOARD_CMD)
    { // can be executed via the UI or NON-UI thread!
#ifdef SIM_WITH_GUI
        if (subView!=nullptr)
        {
            int lo=-1;
            lo=subView->getLinkedObjectID();
            CGraph* graph=App::ct->objCont->getGraph(lo);
            int val=0;
            if (!subView->getTimeGraph())
                val=1;
            if ((graph!=nullptr)&&(subView->getTrackedGraphCurveIndex()!=-1))
            {
                App::addStatusbarMessage(IDSNS_CURVE_DATA_COPIED_TO_CLIPBOARD);
                graph->copyCurveToClipboard(subView->getTrackedGraphCurveIndex(),val);
                App::addStatusbarMessage(IDSNS_DONE);
            }
        }
#endif
        return(true);
    }
    if (commandID==ADD_COMMANDS_ADD_FLOATING_VIEW_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::addStatusbarMessage(IDSNS_ADDING_A_FLOATING_VIEW);
#ifdef SIM_WITH_GUI
            App::ct->pageContainer->getPage(App::ct->pageContainer->getActivePageIndex())->addFloatingView();
#endif
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::addStatusbarMessage(IDSNS_DONE);
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
            App::addStatusbarMessage(IDSNS_ADDING_A_JOINT);
            CJoint* newObject=nullptr;
            if (commandID==ADD_COMMANDS_ADD_REVOLUTE_JOINT_ACCMD)
                newObject=new CJoint(sim_joint_revolute_subtype);
            if (commandID==ADD_COMMANDS_ADD_PRISMATIC_JOINT_ACCMD)
                newObject=new CJoint(sim_joint_prismatic_subtype);
            if (commandID==ADD_COMMANDS_ADD_SPHERICAL_JOINT_ACCMD)
                newObject=new CJoint(sim_joint_spherical_subtype);
            App::ct->objCont->addObjectToScene(newObject,false,true);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::ct->objCont->selectObject(newObject->getObjectHandle());
            App::addStatusbarMessage(IDSNS_DONE);
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
            CCamera* camera=App::ct->objCont->getCamera(lo);
            CGraph* graph=App::ct->objCont->getGraph(lo);
            if (graph!=nullptr)
                return(true);
            CCamera* myNewCamera=nullptr;
            CLight* myNewLight=nullptr;
            if (commandID==ADD_COMMANDS_ADD_CAMERA_ACCMD)
            {
                App::addStatusbarMessage(IDSNS_ADDING_A_CAMERA);
                myNewCamera=new CCamera();
                App::ct->objCont->addObjectToScene(myNewCamera,false,true);
                App::addStatusbarMessage(IDSNS_DONE);
            }
            else
            {
                App::addStatusbarMessage(IDSNS_ADDING_A_LIGHT);
                int tp;
                if (commandID==ADD_COMMANDS_ADD_OMNI_LIGHT_ACCMD)
                    tp=sim_light_omnidirectional_subtype;
                if (commandID==ADD_COMMANDS_ADD_SPOT_LIGHT_ACCMD)
                    tp=sim_light_spot_subtype;
                if (commandID==ADD_COMMANDS_ADD_DIR_LIGHT_ACCMD)
                    tp=sim_light_directional_subtype;
                myNewLight=new CLight(tp);
                App::ct->objCont->addObjectToScene(myNewLight,false,true);
                App::addStatusbarMessage(IDSNS_DONE);
            }
            C3DObject* addedObject=myNewCamera;
            if (addedObject==nullptr)
                addedObject=myNewLight;
            addedObject->setLocalTransformation(C3Vector(0.0f,0.0f,1.0f));
            addedObject->setLocalTransformation(C4Vector(piValue_f*0.5f,0.0f,0.0f));
            if (camera!=nullptr)
            {
                if (myNewCamera!=nullptr)
                {
                    App::ct->objCont->selectObject(myNewCamera->getObjectHandle());
                    C7Vector m(camera->getCumulativeTransformation());
                    myNewCamera->setLocalTransformation(m);
                    myNewCamera->scaleObject(camera->getCameraSize()/myNewCamera->getCameraSize());
                    C3Vector minV,maxV;
                    myNewCamera->getFullBoundingBox(minV,maxV);
                    m=myNewCamera->getLocalTransformation();
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
            App::ct->objCont->selectObject(addedObject->getObjectHandle());
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
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
            App::addStatusbarMessage(IDSNS_ADDING_A_MIRROR);
            CMirror* newObject=new CMirror();
            App::ct->objCont->addObjectToScene(newObject,false,true);
            App::ct->objCont->setAbsoluteAngles(newObject->getObjectHandle(),C3Vector(piValD2_f,0.0f,0.0f));
            App::ct->objCont->setAbsolutePosition(newObject->getObjectHandle(),newObject->getMirrorHeight()*0.5f,2);
            App::ct->objCont->selectObject(newObject->getObjectHandle());
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::addStatusbarMessage(IDSNS_DONE);
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
            App::addStatusbarMessage(IDSNS_ADDING_A_DUMMY);
            CDummy* newObject=new CDummy();
            App::ct->objCont->addObjectToScene(newObject,false,true);
            App::ct->objCont->selectObject(newObject->getObjectHandle());
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::addStatusbarMessage(IDSNS_DONE);
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
            App::addStatusbarMessage(IDSNS_ADDING_AN_OCTREE);
            COctree* newObject=new COctree();
            App::ct->objCont->addObjectToScene(newObject,false,true);
            App::ct->objCont->selectObject(newObject->getObjectHandle());
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::addStatusbarMessage(IDSNS_DONE);
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
            App::addStatusbarMessage(IDSNS_ADDING_A_POINTCLOUD);
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
            App::ct->objCont->addObjectToScene(newObject,false,true);
            App::ct->objCont->selectObject(newObject->getObjectHandle());
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::addStatusbarMessage(IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==ADD_COMMANDS_ADD_PATH_SEGMENT_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::addStatusbarMessage(IDSNS_ADDING_A_PATH);
            CPath* newObject=new CPath();
            CSimplePathPoint* it=new CSimplePathPoint();
            C7Vector trtmp(it->getTransformation());
            trtmp.X(0)=-0.25f;
            it->setTransformation(trtmp,newObject->pathContainer->getAttributes());
            newObject->pathContainer->addSimplePathPoint(it);
            it=it->copyYourself();
            trtmp=it->getTransformation();
            trtmp.X(0)=+0.25f;
            it->setTransformation(trtmp,newObject->pathContainer->getAttributes());
            newObject->pathContainer->addSimplePathPoint(it);
            App::ct->objCont->addObjectToScene(newObject,false,true);
            App::ct->objCont->selectObject(newObject->getObjectHandle());
            int atr=newObject->pathContainer->getAttributes();
            if (atr&sim_pathproperty_endpoints_at_zero_deprecated)
                atr-=sim_pathproperty_endpoints_at_zero_deprecated;
            newObject->pathContainer->setAttributes(atr);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::addStatusbarMessage(IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if ((commandID==ADD_COMMANDS_ADD_AND_ASSOCIATE_NON_THREADED_CHILD_SCRIPT_ACCMD)||(commandID==ADD_COMMANDS_ADD_AND_ASSOCIATE_THREADED_CHILD_SCRIPT_ACCMD))
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (App::ct->objCont->getSelSize()==1)
            {
                int scriptID=App::ct->luaScriptContainer->insertDefaultScript_mainAndChildScriptsOnly(sim_scripttype_childscript,commandID==ADD_COMMANDS_ADD_AND_ASSOCIATE_THREADED_CHILD_SCRIPT_ACCMD);
                CLuaScriptObject* script=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptID);
                if (script!=nullptr)
                    script->setObjectIDThatScriptIsAttachedTo(App::ct->objCont->getSelID(0));
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
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

    if (commandID==ADD_COMMANDS_ADD_AND_ASSOCIATE_CUSTOMIZATION_SCRIPT_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (App::ct->objCont->getSelSize()==1)
            {
                C3DObject* it=App::ct->objCont->getLastSelection_object();
                if (it!=nullptr)
                {
                    if (!it->getEnableCustomizationScript())
                    { // we don't yet have a customization script
                        std::string filenameAndPath(App::directories->systemDirectory+VREP_SLASH);
                        filenameAndPath+=DEFAULT_CUSTOMIZATIONSCRIPT_NAME;
                        if (VFile::doesFileExist(filenameAndPath))
                        {
                            try
                            {
                                VFile file(filenameAndPath,VFile::READ|VFile::SHARE_DENY_NONE);
                                VArchive archive(&file,VArchive::LOAD);
                                unsigned int archiveLength=(unsigned int)file.getLength();
                                char* defaultScript=new char[archiveLength+1];
                                for (int i=0;i<int(archiveLength);i++)
                                    archive >> defaultScript[i];
                                defaultScript[archiveLength]=0;
                                it->setEnableCustomizationScript(true,defaultScript);
                                delete[] defaultScript;
                                archive.close();
                                file.close();
                            }
                            catch(VFILE_EXCEPTION_TYPE e)
                            {
                                VFile::reportAndHandleFileExceptionError(e);
                                char defaultMessage[]="Default customization script file could not be found!"; // do not use comments ("--"), we want to cause an execution error!
                                it->setEnableCustomizationScript(true,defaultMessage);
                            }
                        }
                        else
                        {
                            char defaultMessage[]="Default customization script file could not be found!"; // do not use comments ("--"), we want to cause an execution error!
                            it->setEnableCustomizationScript(true,defaultMessage);
                        }
                        POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                        App::setFullDialogRefreshFlag();
                    }
                }
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

    if (commandID==ADD_COMMANDS_ADD_PATH_CIRCLE_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::addStatusbarMessage(IDSNS_ADDING_A_PATH);
            CPath* newObject=new CPath();
            CSimplePathPoint* it=nullptr;
            float a=0.0f;
            float da=piValTimes2_f/16.0f;
            float r=0.25f/cos(360.0*degToRad/32.0);
            for (int i=0;i<16;i++)
            {
                it=new CSimplePathPoint();
                it->setBezierFactors(0.95f,0.95f);
                C7Vector trtmp(it->getTransformation());
                trtmp.X(0)=r*cos(a);
                trtmp.X(1)=r*sin(a);
                it->setTransformation(trtmp,newObject->pathContainer->getAttributes());
                newObject->pathContainer->addSimplePathPoint(it);
                a+=da;
            }
            newObject->pathContainer->setAttributes(newObject->pathContainer->getAttributes()|sim_pathproperty_closed_path);
            App::ct->objCont->addObjectToScene(newObject,false,true);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::ct->objCont->selectObject(newObject->getObjectHandle());
            App::addStatusbarMessage(IDSNS_DONE);
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
            App::addStatusbarMessage(IDSNS_ADDING_A_GRAPH);
            CGraph* newObject=new CGraph();
            App::ct->objCont->addObjectToScene(newObject,false,true);

            // Following 3 on 24/3/2017
            CLuaScriptObject* scriptObj=new CLuaScriptObject(sim_scripttype_customizationscript);
            App::ct->luaScriptContainer->insertScript(scriptObj);
            scriptObj->setObjectIDThatScriptIsAttachedTo(newObject->getObjectHandle());
            scriptObj->setScriptText("require('graph_customization')");

            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::ct->objCont->selectObject(newObject->getObjectHandle());
            App::addStatusbarMessage(IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==ADD_COMMANDS_ADD_ORTHOGONAL_VISION_SENSOR_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::addStatusbarMessage(IDSNS_ADDING_A_VISION_SENSOR);
            CVisionSensor* newObject=new CVisionSensor();
            App::ct->objCont->addObjectToScene(newObject,false,true);
            newObject->setLocalTransformation(C3Vector(0.0f,0.0f,newObject->getSize()(2)));
            newObject->setPerspectiveOperation(false);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::ct->objCont->selectObject(newObject->getObjectHandle());
            App::addStatusbarMessage(IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==ADD_COMMANDS_ADD_PERSPECTIVE_VISION_SENSOR_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::addStatusbarMessage(IDSNS_ADDING_A_VISION_SENSOR);
            CVisionSensor* newObject=new CVisionSensor();
            App::ct->objCont->addObjectToScene(newObject,false,true);
            newObject->setLocalTransformation(C3Vector(0.0f,0.0f,newObject->getSize()(2)));
            newObject->setPerspectiveOperation(true);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::ct->objCont->selectObject(newObject->getObjectHandle());
            App::addStatusbarMessage(IDSNS_DONE);
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
            App::addStatusbarMessage(IDSNS_ADDING_A_FORCE_SENSOR);
            CForceSensor* newObject=new CForceSensor();
            App::ct->objCont->addObjectToScene(newObject,false,true);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::ct->objCont->selectObject(newObject->getObjectHandle());
            App::addStatusbarMessage(IDSNS_DONE);
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
            App::addStatusbarMessage(IDSNS_ADDING_A_PROXIMITY_SENSOR);
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
            App::ct->objCont->addObjectToScene(newObject,false,true);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::ct->objCont->selectObject(newObject->getObjectHandle());
            App::addStatusbarMessage(IDSNS_DONE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if ( (commandID==ADD_COMMANDS_ADD_PYRAMID_MILL_ACCMD)||(commandID==ADD_COMMANDS_ADD_CYLINDER_MILL_ACCMD)||
        (commandID==ADD_COMMANDS_ADD_DISC_MILL_ACCMD)||(commandID==ADD_COMMANDS_ADD_CONE_MILL_ACCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::addStatusbarMessage(IDSNS_ADDING_A_MILL);
            CMill* newObject=new CMill();
            int type=sim_mill_pyramid_subtype;
            if (commandID==ADD_COMMANDS_ADD_CYLINDER_MILL_ACCMD)
                type=sim_mill_cylinder_subtype;
            if (commandID==ADD_COMMANDS_ADD_DISC_MILL_ACCMD)
                type=sim_mill_disc_subtype;
            if (commandID==ADD_COMMANDS_ADD_CONE_MILL_ACCMD)
                type=sim_mill_cone_subtype;
            newObject->setMillType(type);
            App::ct->objCont->addObjectToScene(newObject,false,true);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            App::ct->objCont->selectObject(newObject->getObjectHandle());
            App::addStatusbarMessage(IDSNS_DONE);
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
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
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
                App::addStatusbarMessage(IDSNS_ADDING_CONVEX_DECOMPOSITION);
                for (int obji=0;obji<int(sel.size());obji++)
                {
                    CShape* oldShape=App::ct->objCont->getShape(sel[obji]);
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
                            C7Vector absCOM(oldShape->getCumulativeTransformation());
                            absCOM=absCOM*oldShape->geomData->geomInfo->getLocalInertiaFrame();
                            float mass=oldShape->geomData->geomInfo->getMass();
                            C7Vector absCOMNoShift(absCOM);
                            absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
                            C3X3Matrix tensor(CGeomWrap::getNewTensor(oldShape->geomData->geomInfo->getPrincipalMomentsOfInertia(),absCOMNoShift));

                            // Fix the name and add the new shape to the new selection vector:
                            CShape* newShape=App::ct->objCont->getShape(newShapeHandle);
                            std::string baseN("generated_part");
                            std::string n(baseN);
                            int suff=0;
                            while (App::ct->objCont->getObjectFromName(n.c_str())!=nullptr)
                                n=baseN+boost::lexical_cast<std::string>(suff++);
                            newShape->setObjectName_objectNotYetInScene(n);
                            n=tt::getObjectAltNameFromObjectName(baseN);
                            suff=0;
                            while (App::ct->objCont->getObjectFromAltName(n.c_str())!=nullptr)
                                n=baseN+boost::lexical_cast<std::string>(suff++);
                            newShape->setObjectAltName_objectNotYetInScene(n);

                            newSelection.push_back(newShapeHandle);

                            // Transfer the mass and inertia info to the new shape:
                            newShape->geomData->geomInfo->setMass(mass);
                            C4Vector rot;
                            C3Vector pmoi;
                            CGeomWrap::findPrincipalMomentOfInertia(tensor,rot,pmoi);
                            newShape->geomData->geomInfo->setPrincipalMomentsOfInertia(pmoi);
                            absCOM.Q=rot;
                            C7Vector relCOM(newShape->getCumulativeTransformation().getInverse()*absCOM);
                            newShape->geomData->geomInfo->setLocalInertiaFrame(relCOM);
                        }
                    }
                }

                App::uiThread->showOrHideProgressBar(false);

                App::ct->objCont->deselectObjects();
                for (size_t i=0;i<newSelection.size();i++)
                    App::ct->objCont->addObjectToSelection(newSelection[i]);
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                App::addStatusbarMessage(IDSNS_DONE);
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
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            std::vector<int> newSelection;
            std::vector<int> tmp;
            for (int i=0;i<int(sel.size());i++)
                tmp.push_back(sel[i]);
            CSceneObjectOperations::addRootObjectChildrenToSelection(tmp);

            // Now keep only visible objects:
            std::vector<int> rootSel;
            for (int i=0;i<int(tmp.size());i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(tmp[i]);
                if ( (it!=nullptr)&&(!it->isObjectPartOfInvisibleModel())&&(App::ct->mainSettings->getActiveLayers()&it->layer) )
                    rootSel.push_back(tmp[i]);
            }


            std::vector<float> allHullVertices;
            allHullVertices.reserve(40000*3);
            std::vector<bool> selectedDummies(App::ct->objCont->getHighestObjectHandle()+1,false);

            App::uiThread->showOrHideProgressBar(true,-1,"Computing convex hull...");
            App::addStatusbarMessage(IDSNS_ADDING_CONVEX_HULL);
            bool printQHullFail=false;
            std::vector<CShape*> originalShapes;
            int dummyCount=0;
            for (int i=0;i<int(rootSel.size());i++)
            {
                CShape* it=App::ct->objCont->getShape(rootSel[i]);
                if (it!=nullptr)
                {
                    C7Vector transf(it->getCumulativeTransformation());
                    std::vector<float> vert;
                    std::vector<float> vertD;
                    std::vector<int> ind;
                    it->geomData->geomInfo->getCumulativeMeshes(vertD,&ind,nullptr);
                    originalShapes.push_back(it);
                    for (int j=0;j<int(vertD.size())/3;j++)
                    {
                        C3Vector v(&vertD[3*j+0]);
                        v=transf*v;
                        allHullVertices.push_back(v(0));
                        allHullVertices.push_back(v(1));
                        allHullVertices.push_back(v(2));
                    }
                }
                // here we take care of dummies
                CDummy* dum=App::ct->objCont->getDummy(rootSel[i]);
                if (dum!=nullptr)
                {
                    if (!selectedDummies[dum->getObjectHandle()])
                    {
                        dummyCount++;
                        C3Vector v(dum->getCumulativeTransformation().X);
                        allHullVertices.push_back(v(0));
                        allHullVertices.push_back(v(1));
                        allHullVertices.push_back(v(2));
                        selectedDummies[dum->getObjectHandle()]=true;
                    }
                }
            }
            App::ct->objCont->deselectObjects();
            if (allHullVertices.size()!=0)
            {
                std::vector<float> hull;
                std::vector<int> indices;
                std::vector<float> normals;
                if (CMeshRoutines::getConvexHull(&allHullVertices,&hull,&indices))
                {
                    CGeomProxy* geom=new CGeomProxy(nullptr,hull,indices,nullptr,nullptr);
                    CShape* it=new CShape();
                    it->setLocalTransformation(geom->getCreationTransformation());
                    geom->setCreationTransformation(C7Vector::identityTransformation);
                    it->geomData=geom;
                    ((CGeometric*)geom->geomInfo)->setConvexVisualAttributes();

                    // Set some visual parameters:
                    ((CGeometric*)geom->geomInfo)->color.colors[0]=1.0f;
                    ((CGeometric*)geom->geomInfo)->color.colors[1]=0.7f;
                    ((CGeometric*)geom->geomInfo)->color.colors[2]=0.7f;
                    ((CGeometric*)geom->geomInfo)->setEdgeThresholdAngle(0.0f);
                    ((CGeometric*)geom->geomInfo)->setGouraudShadingAngle(0.0f);
                    ((CGeometric*)geom->geomInfo)->setVisibleEdges(true);

                    if ( (dummyCount==0)&&(originalShapes.size()==1) )
                    { // Since we extracted the convex hull from a single shape, we take over the inertia and mass parameters
                        // Get the mass and inertia info from the old shape:
                        C7Vector absCOM(originalShapes[0]->getCumulativeTransformation());
                        absCOM=absCOM*originalShapes[0]->geomData->geomInfo->getLocalInertiaFrame();
                        float mass=originalShapes[0]->geomData->geomInfo->getMass();
                        C7Vector absCOMNoShift(absCOM);
                        absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
                        C3X3Matrix tensor(CGeomWrap::getNewTensor(originalShapes[0]->geomData->geomInfo->getPrincipalMomentsOfInertia(),absCOMNoShift));

                        // Transfer the mass and inertia info to the new shape:
                        geom->geomInfo->setMass(mass);
                        C4Vector rot;
                        C3Vector pmoi;
                        CGeomWrap::findPrincipalMomentOfInertia(tensor,rot,pmoi);
                        geom->geomInfo->setPrincipalMomentsOfInertia(pmoi);
                        absCOM.Q=rot;
                        C7Vector relCOM(it->getCumulativeTransformation().getInverse()*absCOM);
                        geom->geomInfo->setLocalInertiaFrame(relCOM);

                    }
                    else
                        geom->geomInfo->setLocalInertiaFrame(C7Vector::identityTransformation);
                    App::ct->objCont->addObjectToScene(it,false,true);
                    App::ct->objCont->addObjectToSelection(it->getObjectHandle());
                }
                else
                    printQHullFail=true;
            }

            App::uiThread->showOrHideProgressBar(false);

            if (printQHullFail)
                App::addStatusbarMessage(IDSNS_FAILED_IS_THE_QHULL_PLUGIN_LOADED);
            else
            {
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                App::addStatusbarMessage(IDSNS_DONE);
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

    if (commandID==ADD_COMMANDS_ADD_GROWN_CONVEX_HULL_ACCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            std::vector<int> newSelection;
            std::vector<int> tmp;
            for (int i=0;i<int(sel.size());i++)
                tmp.push_back(sel[i]);
            CSceneObjectOperations::addRootObjectChildrenToSelection(tmp);

            // Now keep only visible shapes:
            std::vector<int> rootSel;
            for (int i=0;i<int(tmp.size());i++)
            {
                CShape* it=App::ct->objCont->getShape(tmp[i]);
                if ( (it!=nullptr)&&(!it->isObjectPartOfInvisibleModel())&&(App::ct->mainSettings->getActiveLayers()&it->layer) )
                    rootSel.push_back(tmp[i]);
            }
            float grow=0.03f;
            bool doIt=true;
#ifdef SIM_WITH_GUI
            doIt=(rootSel.size()>0)&&App::uiThread->dialogInputGetFloat(App::mainWindow,"Convex hull","Inflation parameter",0.05f,0.001f,10.0f,3,&grow);
#endif
            App::ct->objCont->deselectObjects();
            if (doIt)
            {
                App::uiThread->showOrHideProgressBar(true,-1,"Computing inflated convex hull...");
                App::addStatusbarMessage(IDSNS_ADDING_CONVEX_HULL);
                bool printQHullFail=true;
                std::vector<int> newObjectHandles;
                for (size_t shapeI=0;shapeI<rootSel.size();shapeI++)
                {
                    CShape* theShape=App::ct->objCont->getShape(rootSel[shapeI]);
                    if (theShape!=nullptr)
                    {
                        C7Vector transf(theShape->getCumulativeTransformation());
                        std::vector<float> vertD;
                        std::vector<float> vert;
                        std::vector<int> ind;
                        theShape->geomData->geomInfo->getCumulativeMeshes(vertD,&ind,nullptr);
                        for (size_t j=0;j<ind.size()/3;j++)
                        {
                            int indd[3]={ind[3*j+0],ind[3*j+1],ind[3*j+2]};
                            C3Vector w[3]={C3Vector(&vertD[0]+3*indd[0]),C3Vector(&vertD[0]+3*indd[1]),C3Vector(&vertD[0]+3*indd[2])};
                            C3Vector v12(w[0]-w[1]);
                            C3Vector v13(w[0]-w[2]);
                            C3Vector n(v12^v13);
                            n.normalize();
                            n*=grow;
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
                        for (int j=0;j<int(vert.size())/3;j++)
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
                            CGeomProxy* geom=new CGeomProxy(nullptr,hull,indices,nullptr,nullptr);
                            CShape* it=new CShape();
                            it->setLocalTransformation(geom->getCreationTransformation());
                            geom->setCreationTransformation(C7Vector::identityTransformation);
                            it->geomData=geom;
                            ((CGeometric*)geom->geomInfo)->setConvexVisualAttributes();

                            // Set some visual parameters:
                            ((CGeometric*)geom->geomInfo)->color.colors[0]=1.0f;
                            ((CGeometric*)geom->geomInfo)->color.colors[1]=0.7f;
                            ((CGeometric*)geom->geomInfo)->color.colors[2]=0.7f;
                            ((CGeometric*)geom->geomInfo)->setEdgeThresholdAngle(0.0f);
                            ((CGeometric*)geom->geomInfo)->setGouraudShadingAngle(0.0f);
                            ((CGeometric*)geom->geomInfo)->setVisibleEdges(true);

                            // Since we extracted the convex hull from a single shape, we take over the inertia and mass parameters
                            // Get the mass and inertia info from the old shape:
                            C7Vector absCOM(theShape->getCumulativeTransformation());
                            absCOM=absCOM*theShape->geomData->geomInfo->getLocalInertiaFrame();
                            float mass=theShape->geomData->geomInfo->getMass();
                            C7Vector absCOMNoShift(absCOM);
                            absCOMNoShift.X.clear(); // we just wanna get the orientation of the inertia matrix, no shift info!
                            C3X3Matrix tensor(CGeomWrap::getNewTensor(theShape->geomData->geomInfo->getPrincipalMomentsOfInertia(),absCOMNoShift));

                            // Transfer the mass and inertia info to the new shape:
                            geom->geomInfo->setMass(mass);
                            C4Vector rot;
                            C3Vector pmoi;
                            CGeomWrap::findPrincipalMomentOfInertia(tensor,rot,pmoi);
                            geom->geomInfo->setPrincipalMomentsOfInertia(pmoi);
                            absCOM.Q=rot;
                            C7Vector relCOM(it->getCumulativeTransformation().getInverse()*absCOM);
                            geom->geomInfo->setLocalInertiaFrame(relCOM);
                            App::ct->objCont->addObjectToScene(it,false,false);
                            newObjectHandles.push_back(it->getObjectHandle());
                            newSelection.push_back(it->getObjectHandle());
                            printQHullFail=false;
                        }
                    }
                }
                App::uiThread->showOrHideProgressBar(false);

                if (printQHullFail)
                    App::addStatusbarMessage(IDSNS_FAILED_IS_THE_QHULL_PLUGIN_LOADED);
                else
                {
                    for (size_t i=0;i<newSelection.size();i++)
                        App::ct->objCont->addObjectToSelection(newSelection[i]);
                    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                    App::addStatusbarMessage(IDSNS_DONE);
                }

                if (newObjectHandles.size()>0)
                {
                    CInterfaceStack stack;
                    stack.pushTableOntoStack();
                    stack.pushStringOntoStack("objectHandles",0);
                    stack.pushIntArrayTableOntoStack(&newObjectHandles[0],(int)newObjectHandles.size());
                    stack.insertDataIntoStackTable();
                    App::ct->luaScriptContainer->callChildMainCustomizationAddonSandboxScriptWithData(sim_syscb_aftercreate,&stack);
                }
            }
            else
                App::addStatusbarMessage(IDSNS_ABORTED);
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

CShape* CAddOperations::addPrimitiveShape(int type,const C3Vector& sizes,const int subdiv[3],int faces,int sides,int discSubdiv,bool smooth,int openEnds,bool dynamic,bool pure,bool cone,float density,bool negVolume,float negVolumeScaling)
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

        CGeomProxy* geom=new CGeomProxy(nullptr,vertices,indices,nullptr,nullptr);
        CShape* it=new CShape();
        it->setLocalTransformation(geom->getCreationTransformation());
        ((CGeometric*)geom->geomInfo)->color.setDefaultValues();
        ((CGeometric*)geom->geomInfo)->color.setColor(1.0f,1.0f,1.0f,sim_colorcomponent_ambient_diffuse);
        ((CGeometric*)geom->geomInfo)->setVisibleEdges(true);
        geom->setCreationTransformation(C7Vector::identityTransformation);
        ((CGeometric*)geom->geomInfo)->setGouraudShadingAngle(20.0f*degToRad_f);
        ((CGeometric*)geom->geomInfo)->setEdgeThresholdAngle(20.0f*degToRad_f);
        it->geomData=geom;
        it->setObjectName_objectNotYetInScene(IDSOGL_PLANE);
        it->setObjectAltName_objectNotYetInScene(tt::getObjectAltNameFromObjectName(it->getObjectName()));
        App::ct->objCont->addObjectToScene(it,false,true);
        it->setLocalTransformation(C3Vector(0.0f,0.0f,0.002f)); // we shift the plane so that it is above the floor
        it->alignBoundingBoxWithWorld();
        if (pure)
            ((CGeometric*)geom->geomInfo)->setPurePrimitiveType(sim_pure_primitive_plane,sizes(0),sizes(1),0.0001f);
        if (dynamic)
        {
            int propToRemove=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable_all|sim_objectspecialproperty_renderable|sim_objectspecialproperty_cuttable;
            it->setLocalObjectSpecialProperty((it->getLocalObjectSpecialProperty()|propToRemove)-propToRemove);
            it->setRespondable(true);
            it->setShapeIsDynamicallyStatic(false);
            ((CGeometric*)it->geomData->geomInfo)->color.setColor(0.85f,0.85f,1.0f,sim_colorcomponent_ambient_diffuse);
        }
        geom->geomInfo->setMass(sizes(0)*sizes(1)*density*0.001f); // we assume 1mm thickness
        geom->geomInfo->setPrincipalMomentsOfInertia(C3Vector(sizes(1)*sizes(1)/12.0f,sizes(0)*sizes(0)/12.0f,(sizes(0)*sizes(0)+sizes(1)*sizes(1))/12.0f));

        // make sure that object rests, stacks and can be easily grasped:
        it->getDynMaterial()->generateDefaultMaterial(sim_dynmat_reststackgrasp);

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

        float sca=negVolumeScaling;
        CMeshRoutines::createCube(vertices,indices,C3Vector(xhSize*2.0f,yhSize*2.0f,zhSize*2.0f),theDiv);
        if (negVolume)
        {
            std::vector<float> auxVert;
            std::vector<int> auxInd;
            auxVert.reserve(((divX+1)*(2*divY+2*divZ)+2*(divY-1)*(divZ-1))*3);
            auxInd.reserve(divX*divY*12+divX*divZ*12+divY*divZ*12);
            CMeshRoutines::createCube(auxVert,auxInd,C3Vector(xhSize*2.0f*sca,yhSize*2.0f*sca,zhSize*2.0f*sca),theDiv);
            CMeshManip::mergeWith(&vertices,&indices,nullptr,&auxVert,&auxInd,nullptr);
        }

        CGeomProxy* geom=new CGeomProxy(nullptr,vertices,indices,nullptr,nullptr);
        CShape* it=new CShape();
        it->setLocalTransformation(geom->getCreationTransformation());
        ((CGeometric*)geom->geomInfo)->color.setDefaultValues();
        ((CGeometric*)geom->geomInfo)->color.setColor(1.0f,1.0f,1.0f,sim_colorcomponent_ambient_diffuse);
        ((CGeometric*)geom->geomInfo)->setVisibleEdges(true);
        geom->setCreationTransformation(C7Vector::identityTransformation);
        ((CGeometric*)geom->geomInfo)->setGouraudShadingAngle(20.0f*degToRad_f);
        ((CGeometric*)geom->geomInfo)->setEdgeThresholdAngle(20.0f*degToRad_f);
        it->geomData=geom;
        it->setObjectName_objectNotYetInScene(IDSOGL_RECTANGLE);
        it->setObjectAltName_objectNotYetInScene(tt::getObjectAltNameFromObjectName(it->getObjectName()));
        App::ct->objCont->addObjectToScene(it,false,true);
        it->setLocalTransformation(C3Vector(0.0f,0.0f,zhSize)); // we shift the rectangle so that it sits on the floor
        it->alignBoundingBoxWithWorld();
        if (pure)
            ((CGeometric*)geom->geomInfo)->setPurePrimitiveType(sim_pure_primitive_cuboid,sizes(0),sizes(1),sizes(2));
        if (dynamic)
        {
            int propToRemove=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable_all|sim_objectspecialproperty_renderable|sim_objectspecialproperty_cuttable;
            it->setLocalObjectSpecialProperty((it->getLocalObjectSpecialProperty()|propToRemove)-propToRemove);
            it->setRespondable(true);
            it->setShapeIsDynamicallyStatic(false);
            ((CGeometric*)it->geomData->geomInfo)->color.setColor(0.85f,0.85f,1.0f,sim_colorcomponent_ambient_diffuse);
        }

        if (negVolume)
        {
            // For now, keep a mass and moment of inertia as if the shape was not hollow!
            ((CGeometric*)it->geomData->geomInfo)->setCulling(true);
            ((CGeometric*)it->geomData->geomInfo)->flipFaces();
            ((CGeometric*)it->geomData->geomInfo)->setPurePrimitiveInsideScaling(negVolumeScaling);
        }

        geom->geomInfo->setMass(sizes(0)*sizes(1)*sizes(2)*density);
        geom->geomInfo->setPrincipalMomentsOfInertia(C3Vector((sizes(1)*sizes(1)+sizes(2)*sizes(2))/12.0f,(sizes(0)*sizes(0)+sizes(2)*sizes(2))/12.0f,(sizes(0)*sizes(0)+sizes(1)*sizes(1))/12.0f));

        // make sure that object rests, stacks and can be easily grasped:
        it->getDynMaterial()->generateDefaultMaterial(sim_dynmat_reststackgrasp);

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

        float sca=negVolumeScaling;
        CMeshRoutines::createSphere(vertices,indices,C3Vector(xhSize*2.0f,yhSize*2.0f,zhSize*2.0f),sides,faces);
        if (negVolume)
        {
            std::vector<float> auxVert;
            std::vector<int> auxInd;
            auxVert.reserve(((faces-1)*sides+2)*3);
            auxInd.reserve((sides*2+2*(faces-2)*sides)*3);
            CMeshRoutines::createSphere(auxVert,auxInd,C3Vector(xhSize*2.0f*sca,yhSize*2.0f*sca,zhSize*2.0f*sca),sides,faces);
            CMeshManip::mergeWith(&vertices,&indices,nullptr,&auxVert,&auxInd,nullptr);
        }

        CGeomProxy* geom=new CGeomProxy(nullptr,vertices,indices,nullptr,nullptr);
        CShape* it=new CShape();
        it->setLocalTransformation(geom->getCreationTransformation());
        ((CGeometric*)geom->geomInfo)->color.setDefaultValues();
        ((CGeometric*)geom->geomInfo)->color.setColor(1.0f,1.0f,1.0f,sim_colorcomponent_ambient_diffuse);
        geom->setCreationTransformation(C7Vector::identityTransformation);
        if (smooth)
        {
            ((CGeometric*)geom->geomInfo)->setGouraudShadingAngle(20.0f*degToRad_f);
            ((CGeometric*)geom->geomInfo)->setEdgeThresholdAngle(20.0f*degToRad_f);
        }
        it->geomData=geom;
        it->setObjectName_objectNotYetInScene(IDSOGL_SPHERE);
        it->setObjectAltName_objectNotYetInScene(tt::getObjectAltNameFromObjectName(it->getObjectName()));
        App::ct->objCont->addObjectToScene(it,false,true);
        it->setLocalTransformation(C3Vector(0.0f,0.0f,zhSize)); // we shift the sphere so that it sits on the floor
        it->alignBoundingBoxWithWorld();
        if (pure)
            ((CGeometric*)geom->geomInfo)->setPurePrimitiveType(sim_pure_primitive_spheroid,sizes(0),sizes(1),sizes(2));
        if (dynamic)
        {
            int propToRemove=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable_all|sim_objectspecialproperty_renderable|sim_objectspecialproperty_cuttable;
            it->setLocalObjectSpecialProperty((it->getLocalObjectSpecialProperty()|propToRemove)-propToRemove);
            it->setRespondable(true);
            it->setShapeIsDynamicallyStatic(false);
            ((CGeometric*)it->geomData->geomInfo)->color.setColor(0.85f,0.85f,1.0f,sim_colorcomponent_ambient_diffuse);
        }
        float avR=(sizes(0)+sizes(1)+sizes(2))/6.0f;

        if (negVolume)
        {
            // For now, keep a mass and moment of inertia as if the shape was not hollow!
            ((CGeometric*)it->geomData->geomInfo)->setCulling(true);
            ((CGeometric*)it->geomData->geomInfo)->flipFaces();
            ((CGeometric*)it->geomData->geomInfo)->setPurePrimitiveInsideScaling(negVolumeScaling);
        }

        geom->geomInfo->setMass((4.0f*piValue_f/3.0f)*avR*avR*avR*density);
        geom->geomInfo->setPrincipalMomentsOfInertia(C3Vector(2.0f*avR*avR/5.0f,2.0f*avR*avR/5.0f,2.0f*avR*avR/5.0f));
        float avr2=avR*2.0f;
        geom->geomInfo->scaleMassAndInertia(sizes(0)/avr2,sizes(1)/avr2,sizes(2)/avr2);

        // make sure that object rests, stacks and can be easily grasped:
        it->getDynMaterial()->generateDefaultMaterial(sim_dynmat_reststackgrasp);

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
        float sca=negVolumeScaling;
        if (negVolume)
        {
            CMeshRoutines::createCylinder(vertices,indices,C3Vector(xhSize*2.0f,yhSize*2.0f,zhSize*2.0f),sides,faces,discDiv,3,false);
            std::vector<float> auxVert;
            std::vector<int> auxInd;
            auxVert.reserve((sides*(1+faces)+2+(discDiv-1)*sides*2)*3);
            auxInd.reserve((sides*2+sides*faces*2+(discDiv-1)*4*sides)*3);
            CMeshRoutines::createCylinder(auxVert,auxInd,C3Vector(xhSize*2.0f*sca,yhSize*2.0f*sca,zhSize*2.0f),sides,faces,discDiv,3,false);
            CMeshManip::mergeWith(&vertices,&indices,nullptr,&auxVert,&auxInd,nullptr);
            CMeshRoutines::createAnnulus(auxVert,auxInd,xhSize*2.0f,xhSize*2.0f*sca,-zhSize,sides,true);
            CMeshManip::mergeWith(&vertices,&indices,nullptr,&auxVert,&auxInd,nullptr);
            CMeshRoutines::createAnnulus(auxVert,auxInd,xhSize*2.0f,xhSize*2.0f*sca,+zhSize,sides,false);
            CMeshManip::mergeWith(&vertices,&indices,nullptr,&auxVert,&auxInd,nullptr);
        }
        else
            CMeshRoutines::createCylinder(vertices,indices,C3Vector(xhSize*2.0f,yhSize*2.0f,zhSize*2.0f),sides,faces,discDiv,openEnds,cone);

        CGeomProxy* geom=new CGeomProxy(nullptr,vertices,indices,nullptr,nullptr);
        CShape* it=new CShape();
        it->setLocalTransformation(geom->getCreationTransformation());
        ((CGeometric*)geom->geomInfo)->color.setDefaultValues();
        ((CGeometric*)geom->geomInfo)->color.setColor(1.0f,1.0f,1.0f,sim_colorcomponent_ambient_diffuse);
        ((CGeometric*)geom->geomInfo)->setVisibleEdges(true);
        geom->setCreationTransformation(C7Vector::identityTransformation);
        if (smooth)
        {
            ((CGeometric*)geom->geomInfo)->setGouraudShadingAngle(20.0f*degToRad_f);
            ((CGeometric*)geom->geomInfo)->setEdgeThresholdAngle(20.0f*degToRad_f);
        }
        it->geomData=geom;
        it->setObjectName_objectNotYetInScene(IDSOGL_CYLINDER);
        it->setObjectAltName_objectNotYetInScene(tt::getObjectAltNameFromObjectName(it->getObjectName()));
        App::ct->objCont->addObjectToScene(it,false,true);
        it->alignBoundingBoxWithWorld();
        it->setLocalTransformation(C3Vector(0.0f,0.0f,zhSize)); // Now we shift the cylinder so it sits on the floor
        if (pure)
        {
            if (cone)
                ((CGeometric*)geom->geomInfo)->setPurePrimitiveType(sim_pure_primitive_cone,sizes(0),sizes(1),sizes(2));
            else
                ((CGeometric*)geom->geomInfo)->setPurePrimitiveType(sim_pure_primitive_cylinder,sizes(0),sizes(1),sizes(2));
        }
        if (dynamic)
        {
            int propToRemove=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable_all|sim_objectspecialproperty_renderable|sim_objectspecialproperty_cuttable;
            it->setLocalObjectSpecialProperty((it->getLocalObjectSpecialProperty()|propToRemove)-propToRemove);
            it->setRespondable(true);
            it->setShapeIsDynamicallyStatic(false);
            ((CGeometric*)it->geomData->geomInfo)->color.setColor(0.85f,0.85f,1.0f,sim_colorcomponent_ambient_diffuse);
        }
        float avR=(sizes(0)+sizes(1))/4.0f;
        float divider=1.0f;
        if (cone)
            divider=3.0f;


        if (negVolume)
        {
            // For now, keep a mass and moment of inertia as if the shape was not hollow!
            ((CGeometric*)it->geomData->geomInfo)->setCulling(true);
            ((CGeometric*)it->geomData->geomInfo)->flipFaces();
            ((CGeometric*)it->geomData->geomInfo)->setPurePrimitiveInsideScaling(negVolumeScaling);
        }
        geom->geomInfo->setMass(piValue_f*avR*avR*divider*sizes(2)*density);
        if (cone)
            geom->geomInfo->setPrincipalMomentsOfInertia(C3Vector(3.0f*(0.25f*avR*avR+sizes(2)*sizes(2))/5.0f,3.0f*(0.25f*avR*avR+sizes(2)*sizes(2))/5.0f,3.0f*avR*avR/10.0f));
        else
            geom->geomInfo->setPrincipalMomentsOfInertia(C3Vector((3.0f*avR*avR+sizes(2)*sizes(2))/12.0f,(3.0f*avR*avR+sizes(2)*sizes(2))/12.0f,avR*avR/2.0f));
        float avR2=avR*2.0f;
        geom->geomInfo->scaleMassAndInertia(sizes(0)/avR2,sizes(1)/avR2,1.0f);

        // make sure that object rests, stacks and can be easily grasped:
        it->getDynMaterial()->generateDefaultMaterial(sim_dynmat_reststackgrasp);

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

        CGeomProxy* geom=new CGeomProxy(nullptr,vertices,indices,nullptr,nullptr);
        CShape* it=new CShape();
        it->setLocalTransformation(geom->getCreationTransformation());
        ((CGeometric*)geom->geomInfo)->color.setDefaultValues();
        ((CGeometric*)geom->geomInfo)->color.setColor(1.0f,1.0f,1.0f,sim_colorcomponent_ambient_diffuse);
        ((CGeometric*)geom->geomInfo)->setVisibleEdges(true);
        geom->setCreationTransformation(C7Vector::identityTransformation);
        ((CGeometric*)geom->geomInfo)->setGouraudShadingAngle(20.0f*degToRad_f);
        ((CGeometric*)geom->geomInfo)->setEdgeThresholdAngle(20.0f*degToRad_f);
        it->geomData=geom;
        it->setObjectName_objectNotYetInScene(IDSOGL_DISC);
        it->setObjectAltName_objectNotYetInScene(tt::getObjectAltNameFromObjectName(it->getObjectName()));
        App::ct->objCont->addObjectToScene(it,false,true);
        it->setLocalTransformation(C3Vector(0.0f,0.0f,0.002f)); // Now we shift the disc so it sits just above the floor
        it->alignBoundingBoxWithWorld();
        if (pure)
            ((CGeometric*)geom->geomInfo)->setPurePrimitiveType(sim_pure_primitive_disc,sizes(0),sizes(1),0.0001f);
        if (dynamic)
        {
            int propToRemove=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable_all|sim_objectspecialproperty_renderable|sim_objectspecialproperty_cuttable;
            it->setLocalObjectSpecialProperty((it->getLocalObjectSpecialProperty()|propToRemove)-propToRemove);
            it->setRespondable(true);
            it->setShapeIsDynamicallyStatic(false);
            ((CGeometric*)it->geomData->geomInfo)->color.setColor(0.85f,0.85f,1.0f,sim_colorcomponent_ambient_diffuse);
        }
        float avR=(sizes(0)+sizes(1))/4.0f;
        geom->geomInfo->setMass(piValue_f*avR*avR*density*0.001f); // we assume 1mm thickness
        geom->geomInfo->setPrincipalMomentsOfInertia(C3Vector(3.0f*(avR*avR)/12.0f,3.0f*(avR*avR)/12.0f,avR*avR/2.0f));
        float avR2=avR*2.0f;
        geom->geomInfo->scaleMassAndInertia(sizes(0)/avR2,sizes(1)/avR2,1.0f);

        // make sure that object rests, stacks and can be easily grasped:
        it->getDynMaterial()->generateDefaultMaterial(sim_dynmat_reststackgrasp);

        return(it);
    }

    return(nullptr);
}

#ifdef SIM_WITH_GUI
void CAddOperations::addMenu(VMenu* menu,CSView* subView,bool onlyCamera)
{
    // subView can be null
    bool canAddChildScript=false;
    bool canAddCustomizationScript=false;
    if (App::ct->objCont->getSelSize()==1)
    {
        canAddChildScript=(App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(App::ct->objCont->getSelID(0))==nullptr)&&App::ct->simulation->isSimulationStopped();
        canAddCustomizationScript=(App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(App::ct->objCont->getSelID(0))==nullptr)&&App::ct->simulation->isSimulationStopped();
    }

    std::vector<int> rootSel;
    for (int i=0;i<App::ct->objCont->getSelSize();i++)
        rootSel.push_back(App::ct->objCont->getSelID(i));
    CSceneObjectOperations::addRootObjectChildrenToSelection(rootSel);
    int shapesInRootSel=App::ct->objCont->getShapeNumberInSelection(&rootSel);
    int shapesAndDummiesInRootSel=App::ct->objCont->getShapeNumberInSelection(&rootSel)+App::ct->objCont->getDummyNumberInSelection(&rootSel);

    bool linkedObjIsInexistentOrNotGraphNorRenderingSens=true;
    if (subView!=nullptr)
        linkedObjIsInexistentOrNotGraphNorRenderingSens=((App::ct->objCont->getGraph(subView->getLinkedObjectID())==nullptr)&&(App::ct->objCont->getVisionSensor(subView->getLinkedObjectID())==nullptr));
    bool itemsPresent=false;
    if (subView!=nullptr)
    {
        menu->appendMenuItem(true,false,ADD_COMMANDS_ADD_FLOATING_VIEW_ACCMD,IDS_FLOATING_VIEW_MENU_ITEM);
        itemsPresent=true;
        if (onlyCamera)
            menu->appendMenuItem(linkedObjIsInexistentOrNotGraphNorRenderingSens,false,ADD_COMMANDS_ADD_CAMERA_ACCMD,IDS_CAMERA_MENU_ITEM);
    }
    C3DObject* associatedViewable=nullptr;
    if (subView!=nullptr)
        associatedViewable=App::ct->objCont->getObjectFromHandle(subView->getLinkedObjectID());
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

            if (CMiscBase::handleVerSpec_fullEditionAdd())
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

        if (CMiscBase::handleVerSpec_fullEditionAdd()&&App::userSettings->enableOldMirrorObjects)
            menu->appendMenuItem(true,false,ADD_COMMANDS_ADD_MIRROR_ACCMD,IDS_MIRROR_MENU_ITEM);

        itemsPresent=true;

        if ( CMiscBase::handleVerSpec_fullEditionAdd()&&(!onlyCamera) )
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

            VMenu* visSens=new VMenu();
            visSens->appendMenuItem(true,false,ADD_COMMANDS_ADD_ORTHOGONAL_VISION_SENSOR_ACCMD,IDS_VISION_SENSOR_ORTHOGONAL_TYPE_MENU_ITEM);
            visSens->appendMenuItem(true,false,ADD_COMMANDS_ADD_PERSPECTIVE_VISION_SENSOR_ACCMD,IDS_VISION_SENSOR_PERSPECTIVE_TYPE_MENU_ITEM);
            menu->appendMenuAndDetach(visSens,true,IDSN_VISION_SENSOR);

            menu->appendMenuItem(true,false,ADD_COMMANDS_ADD_FORCE_SENSOR_ACCMD,IDSN_FORCE_SENSOR);

            if (App::userSettings->enableOldMillObjects)
            {
                VMenu* mill=new VMenu();
                mill->appendMenuItem(true,false,ADD_COMMANDS_ADD_PYRAMID_MILL_ACCMD,IDS_PYRAMID_TYPE_MENU_ITEM);
                mill->appendMenuItem(true,false,ADD_COMMANDS_ADD_CYLINDER_MILL_ACCMD,IDS_CYLINDER_TYPE_MENU_ITEM);
                mill->appendMenuItem(true,false,ADD_COMMANDS_ADD_DISC_MILL_ACCMD,IDS_DISC_TYPE_MENU_ITEM);
                mill->appendMenuItem(true,false,ADD_COMMANDS_ADD_CONE_MILL_ACCMD,IDS_CONE_TYPE_MENU_ITEM);
                menu->appendMenuAndDetach(mill,true,IDSN_MILL);
            }

            VMenu* pathM=new VMenu();
            pathM->appendMenuItem(true,false,ADD_COMMANDS_ADD_PATH_SEGMENT_ACCMD,IDS_SEGMENT_TYPE_MENU_ITEM);
            pathM->appendMenuItem(true,false,ADD_COMMANDS_ADD_PATH_CIRCLE_ACCMD,IDS_CIRCLE_TYPE_MENU_ITEM);
            menu->appendMenuAndDetach(pathM,true,IDSN_PATH);

            VMenu* childScript=new VMenu();
            childScript->appendMenuItem(true,false,ADD_COMMANDS_ADD_AND_ASSOCIATE_NON_THREADED_CHILD_SCRIPT_ACCMD,IDS_NON_THREADED_MENU_ITEM);
            childScript->appendMenuItem(true,false,ADD_COMMANDS_ADD_AND_ASSOCIATE_THREADED_CHILD_SCRIPT_ACCMD,IDS_THREADED_MENU_ITEM);

            menu->appendMenuAndDetach(childScript,canAddChildScript,IDS_ASSOCIATED_CHILD_SCRIPT_MENU_ITEM);

            menu->appendMenuItem(canAddCustomizationScript,false,ADD_COMMANDS_ADD_AND_ASSOCIATE_CUSTOMIZATION_SCRIPT_ACCMD,IDS_ASSOCIATED_CUSTOMIZATION_SCRIPT_MENU_ITEM);


            menu->appendMenuSeparator();
            menu->appendMenuItem(shapesAndDummiesInRootSel>0,false,ADD_COMMANDS_ADD_CONVEX_HULL_ACCMD,IDS_CONVEX_HULL_OF_SELECTION_MENU_ITEM);
            menu->appendMenuItem(shapesInRootSel>0,false,ADD_COMMANDS_ADD_GROWN_CONVEX_HULL_ACCMD,IDS_GROWN_CONVEX_HULL_OF_SELECTED_SHAPE_MENU_ITEM);
            menu->appendMenuItem(shapesInRootSel>0,false,ADD_COMMANDS_ADD_CONVEX_DECOMPOSITION_ACCMD,IDS_CONVEX_DECOMPOSITION_OF_SELECTION_MENU_ITEM);
        }
    }

    if ((associatedViewable!=nullptr)&&(associatedViewable->getObjectType()==sim_object_graph_type)&&(subView!=nullptr))
    {
        menu->appendMenuSeparator();
        bool itemEnabled=(subView->getTrackedGraphCurveIndex()!=-1);
        menu->appendMenuItem(itemEnabled,false,ADD_COMMANDS_MAKE_GRAPH_CURVE_STATIC_ACCMD,IDS_MAKE_CURVE_STATIC_MENU_ITEM);
        menu->appendMenuItem(itemEnabled,false,COPY_GRAPH_CURVE_TO_CLIPBOARD_CMD,IDS_COPY_CURVE_TO_CLIPBOARD);
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
        bool smooth,dynamic,pure,cone,negVolume;
        float density,negVolumeScaling;
        if (App::uiThread->showPrimitiveShapeDialog(pType,optSizes,sizes,subdiv,faces,sides,discSubdiv,smooth,openEnds,dynamic,pure,cone,density,negVolume,negVolumeScaling))
            retVal=addPrimitiveShape(pType,sizes,subdiv,faces,sides,discSubdiv,smooth,openEnds,dynamic,pure,cone,density,negVolume,negVolumeScaling);
    }
    return(retVal);
}
#endif
