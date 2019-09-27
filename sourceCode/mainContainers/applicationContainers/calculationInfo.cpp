
#include "vrepMainHeader.h"
#include "calculationInfo.h"
#include "app.h"
#include "gV.h"
#include "tt.h"
#include "pluginContainer.h"
#include "threadPool.h"
#include <boost/lexical_cast.hpp>
#include "v_repStrings.h"
#include "vDateTime.h"

CCalculationInfo::CCalculationInfo()
{
    _threadedScriptCount=0;

    _scriptTxt[0]="";
    _scriptTxt[1]="";
    _collTxt[0]="";
    _collTxt[1]="";
    _distTxt[0]="";
    _distTxt[1]="";
    _sensTxt[0]="";
    _sensTxt[1]="";
    _visionSensTxt[0]="";
    _visionSensTxt[1]="";
    _ikTxt[0]="";
    _ikTxt[1]="";
    _millTxt[0]="";
    _millTxt[1]="";
    _dynamicsTxt[0]="";
    _dynamicsTxt[1]="";
}

CCalculationInfo::~CCalculationInfo()
{
}

void CCalculationInfo::simulationAboutToStart()
{
    resetInfo();
}

void CCalculationInfo::simulationEnded()
{
    resetInfo();
}

void CCalculationInfo::resetInfo()
{
    _collCalcCount=0;
    _collDetectCount=0;
    _collCalcDuration=0;
    _distCalcCount=0;
    _distCalcDuration=0;
    _sensCalcCount=0;
    _sensDetectCount=0;
    _sensCalcDuration=0;
    _rendSensCalcCount=0;
    _rendSensDetectCount=0;
    _rendSensCalcDuration=0;
    _ikCalcCount=0;
    _ikCalcDuration=0;
    _mainScriptExecuted=false;
    _mainScriptDuration=0;
    _mainScriptMessage="";
    _regularScriptDuration=0;
    _regularScriptCount=0;
    _threadedScriptDuration=0;
    _threadedScriptCount=0;
    _simulationPassDuration=0;

    _dynamicsCalcPasses=0;
    _dynamicsCalcDuration=0;

    _renderingDuration=0;

    _millCalcCount=0;
    _millRemovedSurface=0.0f;
    _millRemovedVolume=0.0f;
    _millCalcDuration=0;
}

void CCalculationInfo::formatInfo()
{
    // Script functionality:
    if (_mainScriptExecuted)
    {
        _scriptTxt[0]="Simulation scripts called/resumed";
        _scriptTxt[1]="main: 1 (";
    }
    else
    {
        _scriptTxt[0]="&&fg930Main script not called";
        _scriptTxt[1]="main: 0 (";
    }
    _scriptTxt[1]+=boost::lexical_cast<std::string>(_mainScriptDuration);
    _scriptTxt[1]+=" ms), non-threaded: ";
    _scriptTxt[1]+=boost::lexical_cast<std::string>(_regularScriptCount)+" (";
    _scriptTxt[1]+=boost::lexical_cast<std::string>(_regularScriptDuration)+" ms), running threads: ";
    _scriptTxt[1]+=boost::lexical_cast<std::string>(_threadedScriptCount);
    _scriptTxt[1]+=" (";
    _scriptTxt[1]+=boost::lexical_cast<std::string>(_threadedScriptDuration)+" ms) ";
    _scriptTxt[1]+=_mainScriptMessage;

    // Collision detection:
    if (CPluginContainer::isMeshPluginAvailable())
    {
        if (!App::ct->mainSettings->collisionDetectionEnabled)
            _collTxt[0]="&&fg930Collision handling disabled";
        else
            _collTxt[0]="Collision handling enabled";
    }
    else
        _collTxt[0]="&&fg930'MeshCalc' plugin not found";

    _collTxt[1]="Calculations: ";
    _collTxt[1]+=boost::lexical_cast<std::string>(_collCalcCount)+", detections: ";
    _collTxt[1]+=boost::lexical_cast<std::string>(_collDetectCount)+" (";
    _collTxt[1]+=boost::lexical_cast<std::string>(_collCalcDuration)+" ms)";

    // Distance calculation:
    if (CPluginContainer::isMeshPluginAvailable())
    {
        if (!App::ct->mainSettings->distanceCalculationEnabled)
            _distTxt[0]="&&fg930Distance handling disabled";
        else
            _distTxt[0]="Distance handling enabled";
    }
    else
        _distTxt[0]="&&fg930'MeshCalc' plugin not found";

    _distTxt[1]="Calculations: ";
    _distTxt[1]+=boost::lexical_cast<std::string>(_distCalcCount)+" (";
    _distTxt[1]+=boost::lexical_cast<std::string>(_distCalcDuration)+" ms)";


    // Proximity sensor calculation:
    if (CPluginContainer::isMeshPluginAvailable())
    {
        if (!App::ct->mainSettings->proximitySensorsEnabled)
            _sensTxt[0]="&&fg930Proximity sensor handling disabled";
        else
            _sensTxt[0]="Proximity sensor handling enabled";
    }
    else
        _sensTxt[0]="&&fg930'MeshCalc' plugin not found";

    _sensTxt[1]="Calculations: ";
    _sensTxt[1]+=boost::lexical_cast<std::string>(_sensCalcCount)+", detections: ";
    _sensTxt[1]+=boost::lexical_cast<std::string>(_sensDetectCount)+" (";
    _sensTxt[1]+=boost::lexical_cast<std::string>(_sensCalcDuration)+" ms)";

    // Vision sensor calculation:
    if (!App::ct->mainSettings->visionSensorsEnabled)
        _visionSensTxt[0]="&&fg930Vision sensor handling disabled";
    else
        _visionSensTxt[0]="Vision sensor handling enabled (FBO)";
    _visionSensTxt[1]="Calculations: ";
    _visionSensTxt[1]+=boost::lexical_cast<std::string>(_rendSensCalcCount)+", detections: ";
    _visionSensTxt[1]+=boost::lexical_cast<std::string>(_rendSensDetectCount)+" (";
    _visionSensTxt[1]+=boost::lexical_cast<std::string>(_rendSensCalcDuration)+" ms)";

    // IK calculation:
    if (!App::ct->mainSettings->ikCalculationEnabled)
        _ikTxt[0]="&&fg930IK group handling disabled";
    else
        _ikTxt[0]="IK group handling enabled";
    _ikTxt[1]="Calculations: ";
    _ikTxt[1]+=boost::lexical_cast<std::string>(_ikCalcCount)+" (";
    _ikTxt[1]+=boost::lexical_cast<std::string>(_ikCalcDuration)+" ms)";

    // Dynamics calculation:
    if (!App::ct->dynamicsContainer->getDynamicsEnabled())
        _dynamicsTxt[0]="&&fg930Dynamics handling disabled";
    else
    {
        if (App::ct->dynamicsContainer->isWorldThere())
        {
            _dynamicsTxt[0]="Dynamics handling enabled (";
            int ver;
            int eng=App::ct->dynamicsContainer->getDynamicEngineType(&ver);
            if (eng==sim_physics_ode)
                _dynamicsTxt[0]+=IDS_ODE;
            if ( (eng==sim_physics_bullet)&&(ver==0) )
                _dynamicsTxt[0]+=IDS_BULLET_2_78;
            if ( (eng==sim_physics_bullet)&&(ver==283) )
                _dynamicsTxt[0]+=IDS_BULLET_2_83;
            if (eng==sim_physics_vortex)
                _dynamicsTxt[0]+=IDS_VORTEX;
            if (eng==sim_physics_newton)
                _dynamicsTxt[0]+=IDS_NEWTON;
            _dynamicsTxt[0]+=")";
        }
        else
            _dynamicsTxt[0]="&&fg930Appropriate dynamics plugin not found";
    }

    _dynamicsTxt[1]="Calculation passes: ";
    if (_dynamicsContentAvailable)
    {
        _dynamicsTxt[1]+=boost::lexical_cast<std::string>(_dynamicsCalcPasses)+" (";
        _dynamicsTxt[1]+=boost::lexical_cast<std::string>(_dynamicsCalcDuration)+" ms)";
    }
    else
        _dynamicsTxt[1]+="0 (no dynamic content)";
/*
    // Milling calculation:
    if (CPluginContainer::isMeshPluginAvailable())
    {
        if (!App::ct->mainSettings->millsEnabled)
            _millTxt[0]="&&fg930Mill handling disabled";
        else
            _millTxt[0]="Mill handling enabled";
    }
    else
        _millTxt[0]="&&fg930'MeshCalc' plugin not found";

    _millTxt[1]="Calculations: ";
    _millTxt[1]+=boost::lexical_cast<std::string>(_millCalcCount)+", surface cut: ";
    _millTxt[1]+=boost::lexical_cast<std::string>(_millRemovedSurface*1000000.0f)+" mm^2 (";
    _millTxt[1]+=boost::lexical_cast<std::string>(_millCalcDuration)+" ms)";
    */
}

float CCalculationInfo::getCollisionCalculationTime()
{
    return(float(_collCalcDuration)*0.001f);
}

float CCalculationInfo::getDistanceCalculationTime()
{
    return(float(_distCalcDuration)*0.001f);
}

float CCalculationInfo::getProximitySensorCalculationTime()
{
    return(float(_sensCalcDuration)*0.001f);
}

float CCalculationInfo::getMillingCalculationTime()
{
    return(float(_millCalcDuration)*0.001f);
}

float CCalculationInfo::getVisionSensorCalculationTime()
{
    return(float(_rendSensCalcDuration)*0.001f);
}

float CCalculationInfo::getIkCalculationTime()
{
    return(float(_ikCalcDuration)*0.001f);
}

float CCalculationInfo::getChildScriptExecutionTime()
{
    return(float(_regularScriptDuration+_threadedScriptDuration)*0.001f);
}

float CCalculationInfo::getDynamicsCalculationTime()
{
    return(float(_dynamicsCalcDuration)*0.001f);
}

float CCalculationInfo::getSimulationPassExecutionTime()
{
    return(float(VDateTime::getTimeDiffInMs(_simulationPassStartTime))*0.001f);
}

float CCalculationInfo::getRenderingDuration()
{
    return(float(_renderingDuration)*0.001f);
}

void CCalculationInfo::addChildScriptCalcTime(int duration,bool threaded)
{
    if (!threaded)
        _regularScriptDuration+=duration;
    else
        _threadedScriptDuration+=duration;
}

void CCalculationInfo::addChildScriptExecCnt(int cnt,bool threaded)
{
    if (!threaded)
        _regularScriptCount+=cnt;
    else
        _threadedScriptCount+=cnt;
}

void CCalculationInfo::setMainScriptExecutionTime(int duration)
{
    _mainScriptExecuted=true;
    _mainScriptDuration=duration;
}

void CCalculationInfo::setMainScriptMessage(const char* msg)
{
    _mainScriptMessage=msg;
}

void CCalculationInfo::simulationPassStart()
{
    _simulationPassStartTime=VDateTime::getTimeInMs();
}

void CCalculationInfo::simulationPassEnd()
{
    _simulationPassDuration+=VDateTime::getTimeDiffInMs(_simulationPassStartTime);
}

void CCalculationInfo::collisionDetectionStart()
{
    _collStartTime=VDateTime::getTimeInMs();
}

void CCalculationInfo::collisionDetectionEnd(bool detected)
{
    _collCalcCount++;
    if (detected)
        _collDetectCount++;
    _collCalcDuration+=VDateTime::getTimeDiffInMs(_collStartTime);
}

void CCalculationInfo::distanceCalculationStart()
{
    _distStartTime=VDateTime::getTimeInMs();
}

void CCalculationInfo::distanceCalculationEnd()
{
    _distCalcCount++;
    _distCalcDuration+=VDateTime::getTimeDiffInMs(_distStartTime);
}

void CCalculationInfo::proximitySensorSimulationStart()
{
    _sensStartTime=VDateTime::getTimeInMs();
}

void CCalculationInfo::proximitySensorSimulationEnd(bool detected)
{
    _sensCalcCount++;
    if (detected)
        _sensDetectCount++;
    _sensCalcDuration+=VDateTime::getTimeDiffInMs(_sensStartTime);
}

void CCalculationInfo::visionSensorSimulationStart()
{
    _rendSensStartTime=VDateTime::getTimeInMs();
}

void CCalculationInfo::visionSensorSimulationEnd(bool detected)
{
    _rendSensCalcCount++;
    if (detected)
        _rendSensDetectCount++;
    _rendSensCalcDuration+=VDateTime::getTimeDiffInMs(_rendSensStartTime);
}

void CCalculationInfo::inverseKinematicsStart()
{
    _ikStartTime=VDateTime::getTimeInMs();
}

void CCalculationInfo::inverseKinematicsEnd()
{
    _ikCalcCount++;
    _ikCalcDuration+=VDateTime::getTimeDiffInMs(_ikStartTime);
}

void CCalculationInfo::renderingStart()
{
    _renderingStartTime=VDateTime::getTimeInMs();
}

void CCalculationInfo::renderingEnd()
{
    _renderingDuration+=VDateTime::getTimeDiffInMs(_renderingStartTime);
}

void CCalculationInfo::clearRenderingTime()
{
    _renderingDuration=0;
}

void CCalculationInfo::dynamicsStart()
{
    _dynamicsStartTime=VDateTime::getTimeInMs();
}

void CCalculationInfo::dynamicsEnd(int calcPasses,bool dynamicContent)
{
    _dynamicsCalcPasses=calcPasses;
    _dynamicsCalcDuration+=VDateTime::getTimeDiffInMs(_dynamicsStartTime);
    _dynamicsContentAvailable=dynamicContent;
}

void CCalculationInfo::millSimulationStart()
{
    _millStartTime=VDateTime::getTimeInMs();
}

void CCalculationInfo::millSimulationEnd(float surfaceRemoved,float volumeRemoved)
{
    _millCalcCount++;
    _millRemovedSurface+=surfaceRemoved;
    _millRemovedVolume+=volumeRemoved;
    _millCalcDuration+=VDateTime::getTimeDiffInMs(_millStartTime);
}

#ifdef SIM_WITH_GUI
void CCalculationInfo::printInformation()
{
    if (App::ct->buttonBlockContainer==nullptr)
        return;
    if (App::ct->objCont==nullptr)
        return;
    for (int i=0;i<INFO_BOX_ROW_COUNT;i++)
    {
        if (App::ct->buttonBlockContainer->getInfoBoxButton(i,0)==nullptr)
            return;
        if (App::ct->buttonBlockContainer->getInfoBoxButton(i,1)==nullptr)
            return;
        App::ct->buttonBlockContainer->getInfoBoxButton(i,0)->label="";
        App::ct->buttonBlockContainer->getInfoBoxButton(i,1)->label="";
    }
    if (App::ct->buttonBlockContainer!=nullptr)
    {
        int pos=0;
        std::string tmp;
        std::string txt;
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
        {
            C3DObject* it=App::ct->objCont->getLastSelection_object();
            if (it!=nullptr)
            {

                App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected objects:";
                if (App::ct->objCont->getSelSize()!=2)
                    tmp=boost::lexical_cast<std::string>(App::ct->objCont->getSelSize());
                else
                {
                    C3DObject* it2=App::ct->objCont->getObjectFromHandle(App::ct->objCont->getSelID(0));
                    C7Vector v0(it->getCumulativeTransformation());
                    C7Vector v1(it2->getCumulativeTransformation());
                    tmp="2 (frame-frame distance="+gv::getSizeStr(false,(v0.X-v1.X).getLength(),0)+")";
                }
                App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=tmp;

                App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Last selected object name:";
                tmp=it->getObjectName();
                App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=tmp;

                App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Last selected object type:";
                tmp=it->getObjectTypeInfoExtended();
                App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=tmp;

                C7Vector m(it->getCumulativeTransformationPart1());
                C3Vector euler(m.Q.getEulerAngles());
                App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Last selected object position:";
                txt="x: "+gv::getSizeStr(true,m.X(0))+"    y: "+gv::getSizeStr(true,m.X(1))+"    z: "+gv::getSizeStr(true,m.X(2));
                App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
                App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Last selected object orientation:";
                txt="a: "+gv::getAngleStr(true,euler(0))+"    b: "+gv::getAngleStr(true,euler(1))+"    g: "+gv::getAngleStr(true,euler(2));
                App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
            }
            else
            {
                App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected objects:";
                App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label="0";
            }

            int selSize=(int)App::ct->collections->selectedCollections.size();

            if (selSize==1)
            {
                CRegCollection* aGroup=App::ct->collections->getCollection(App::ct->collections->selectedCollections[0]);
                if (aGroup!=nullptr)
                {
                    App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected collection:";
                    tmp=aGroup->getCollectionName()+" (containing ";
                    tmp+=boost::lexical_cast<std::string>(aGroup->collectionObjects.size())+" objects)";
                    App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=tmp;
                }
            }
        }
        if (App::getEditModeType()&VERTEX_EDIT_MODE)
        {
            if (App::mainWindow->editModeContainer->getEditModeBufferSize()!=2)
            {
                App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected vertices:";
                tmp=boost::lexical_cast<std::string>(App::mainWindow->editModeContainer->getEditModeBufferSize());
                App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=tmp;
            }
            else
            {
                C3Vector p1(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(App::mainWindow->editModeContainer->getEditModeBufferValue(0)));
                C3Vector p2(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(App::mainWindow->editModeContainer->getEditModeBufferValue(1)));
                float dist=(p2-p1).getLength();
                App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected vertices:";
                txt="2 (distance="+gv::getSizeStr(false,dist,0)+")";
                App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
            }
            if (App::mainWindow->editModeContainer->getEditModeBufferSize()!=0)
            {
                C3DObject* it=App::mainWindow->editModeContainer->getEditModeObject();
                if (it!=nullptr)
                {
                    C7Vector m(it->getCumulativeTransformationPart1());
                    int lastV=App::mainWindow->editModeContainer->getLastEditModeBufferValue();
                    C3Vector v(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(lastV));
                    v*=m;
                    App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Last selected vertex position:";
                    txt="x: "+gv::getSizeStr(true,v(0))+"    y: "+gv::getSizeStr(true,v(1))+"    z: "+gv::getSizeStr(true,v(2));
                    App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
                }
            }
        }
        if (App::getEditModeType()&TRIANGLE_EDIT_MODE)
        {
            App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected triangles:";
            tmp=boost::lexical_cast<std::string>(App::mainWindow->editModeContainer->getEditModeBufferSize());
            App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=tmp;
        }
        if (App::getEditModeType()&EDGE_EDIT_MODE)
        {
            if (App::mainWindow->editModeContainer->getEditModeBufferSize()==0)
            {
                App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected edges:";
                tmp="0";
                App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=tmp;
            }
            else
            {
                float totLength=0.0f;
                for (int i=0;i<App::mainWindow->editModeContainer->getEditModeBufferSize();i++)
                {
                    int v=App::mainWindow->editModeContainer->getEditModeBufferValue(i);
                    int ind[2];
                    App::mainWindow->editModeContainer->getShapeEditMode()->getEditionEdge(v,ind);
                    C3Vector p1(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind[0]));
                    C3Vector p2(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(ind[1]));
                    float dist=(p2-p1).getLength();
                    totLength+=dist;
                }
                App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected edges:";
                txt=boost::lexical_cast<std::string>(App::mainWindow->editModeContainer->getEditModeBufferSize())+" (total edge length="+gv::getSizeStr(false,totLength,0)+")";
                App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
            }
        }



        if (App::getEditModeType()&PATH_EDIT_MODE)
        {
            CPathCont* pc=App::mainWindow->editModeContainer->getEditModePathContainer();
            CPath* path=App::mainWindow->editModeContainer->getEditModePath();
            if ( (pc!=nullptr)&&(path!=nullptr) )
            {
                if (App::mainWindow->editModeContainer->getEditModeBufferSize()!=2)
                {
                    App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected path points:";
                    txt=boost::lexical_cast<std::string>(App::mainWindow->editModeContainer->getEditModeBufferSize());
                    txt+=" (Bezier path length="+gv::getSizeStr(false,pc->getBezierNormalPathLength(),0)+")";
                    App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
                }
                else
                {
                    CSimplePathPoint* pt1(App::mainWindow->editModeContainer->getPathEditMode()->getSimplePathPoint(0));
                    CSimplePathPoint* pt2(App::mainWindow->editModeContainer->getPathEditMode()->getSimplePathPoint(1));
                    float dist=(pt2->getTransformation().X-pt1->getTransformation().X).getLength();
                    App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected path points:";
                    txt="2 (distance="+gv::getSizeStr(false,dist,0)+")";
                    App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
                }
                if (App::mainWindow->editModeContainer->getEditModeBufferSize()!=0)
                {
                    CSimplePathPoint* pt(App::mainWindow->editModeContainer->getPathEditMode()->getSimplePathPoint(App::mainWindow->editModeContainer->getEditModeBufferSize()-1));
//                  CSimplePathPoint* pt(pc->getSimplePathPoint(App::ct->objCont->editModeBuffer[App::ct->objCont->editModeBuffer.size()-1]));
                    C7Vector tr(path->getCumulativeTransformationPart1());
                    C3Vector v(tr*pt->getTransformation().X);
                    App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Last selected path point position:";
                    txt="x: "+gv::getSizeStr(true,v(0))+"    y: "+gv::getSizeStr(true,v(1))+"    z: "+gv::getSizeStr(true,v(2));
                    App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
                    C3Vector euler((tr.Q*pt->getTransformation().Q).getEulerAngles());
                    App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Last selected path point orientation:";
                    txt="a: "+gv::getAngleStr(true,euler(0))+"    b: "+gv::getAngleStr(true,euler(1))+"    g: "+gv::getAngleStr(true,euler(2));
                    App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
                }
            }
        }




        std::string txtLeft,txtRight;
        int index=0;
        while (App::ct->simulation->getInfo(txtLeft,txtRight,index))
        {
            App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label=txtLeft;
            App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txtRight;
        }
        if (App::ct->simulation->isSimulationRunning())
        {
            // Script functionality:
            App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label=_scriptTxt[0];
            App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=_scriptTxt[1];
            // Collision detection:
            App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label=_collTxt[0];
            App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=_collTxt[1];
            // Distance calculation:
            App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label=_distTxt[0];
            App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=_distTxt[1];
            // Proximity sensor simulation:
            App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label=_sensTxt[0];
            App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=_sensTxt[1];

            // Vision sensor simulation:
            App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label=_visionSensTxt[0];
            App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=_visionSensTxt[1];
            // IK calculation:
            App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label=_ikTxt[0];
            App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=_ikTxt[1];
            // Dynamics calculation:
            App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label=_dynamicsTxt[0];
            App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=_dynamicsTxt[1];
            // Milling calculation:
            App::ct->buttonBlockContainer->getInfoBoxButton(pos,0)->label=_millTxt[0];
            App::ct->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=_millTxt[1];
        }
    }
}
#endif
