#include "calculationInfo.h"
#include "app.h"
#include "gV.h"
#include "tt.h"
#include "pluginContainer.h"
#include "threadPool.h"
#include <boost/lexical_cast.hpp>
#include "simStrings.h"
#include "vDateTime.h"

CCalculationInfo::CCalculationInfo()
{
    resetInfo(true);
}

CCalculationInfo::~CCalculationInfo()
{
}

void CCalculationInfo::simulationAboutToStart()
{
    resetInfo(true);
}

void CCalculationInfo::simulationAboutToStep()
{
    formatInfo();
    resetInfo(false);
}

void CCalculationInfo::simulationEnded()
{
    resetInfo(true);
}

void CCalculationInfo::resetInfo(bool clearDisp)
{
    _sensCalcCount=0;
    _sensDetectCount=0;
    _sensCalcDuration=0;
    _rendSensCalcCount=0;
    _rendSensDetectCount=0;
    _rendSensCalcDuration=0;
    _mainScriptDuration=0;
    _simulationScriptExecCount=0;
    _simulationPassDuration=0;

    _dynamicsCalcPasses=0;
    _dynamicsCalcDuration=0;

    _renderingDuration=0;
    if (clearDisp)
    {
        _scriptTxt[0]="";
        _scriptTxt[1]="";
        _sensTxt[0]="";
        _sensTxt[1]="";
        _visionSensTxt[0]="";
        _visionSensTxt[1]="";
        _dynamicsTxt[0]="";
        _dynamicsTxt[1]="";
    }
}

void CCalculationInfo::formatInfo()
{
    // Script functionality:
    _scriptTxt[0]="Simulation scripts called";
    _scriptTxt[1]=boost::lexical_cast<std::string>(_simulationScriptExecCount);
    _scriptTxt[1]+=" (";
    _scriptTxt[1]+=boost::lexical_cast<std::string>(_mainScriptDuration);
    _scriptTxt[1]+=" ms)";

    // Proximity sensor calculation:
    if (CPluginContainer::isGeomPluginAvailable())
    {
        if (!App::currentWorld->mainSettings->proximitySensorsEnabled)
            _sensTxt[0]="&&fg930Proximity sensor handling disabled";
        else
            _sensTxt[0]="Proximity sensor handling enabled";
    }
    else
        _sensTxt[0]="&&fg930'Geometric' plugin not found";

    _sensTxt[1]="Calculations: ";
    _sensTxt[1]+=boost::lexical_cast<std::string>(_sensCalcCount)+", detections: ";
    _sensTxt[1]+=boost::lexical_cast<std::string>(_sensDetectCount)+" (";
    _sensTxt[1]+=boost::lexical_cast<std::string>(_sensCalcDuration)+" ms)";

    // Vision sensor calculation:
    if (!App::currentWorld->mainSettings->visionSensorsEnabled)
        _visionSensTxt[0]="&&fg930Vision sensor handling disabled";
    else
        _visionSensTxt[0]="Vision sensor handling enabled (FBO)";
    _visionSensTxt[1]="Calculations: ";
    _visionSensTxt[1]+=boost::lexical_cast<std::string>(_rendSensCalcCount)+", detections: ";
    _visionSensTxt[1]+=boost::lexical_cast<std::string>(_rendSensDetectCount)+" (";
    _visionSensTxt[1]+=boost::lexical_cast<std::string>(_rendSensCalcDuration)+" ms)";

    // Dynamics calculation:
    if (!App::currentWorld->dynamicsContainer->getDynamicsEnabled())
        _dynamicsTxt[0]="&&fg930Dynamics handling disabled";
    else
    {
        if (App::currentWorld->dynamicsContainer->isWorldThere())
        {
            _dynamicsTxt[0]="Dynamics handling enabled (";
            int ver;
            int eng=App::currentWorld->dynamicsContainer->getDynamicEngineType(&ver);
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
}

float CCalculationInfo::getProximitySensorCalculationTime()
{
    return(float(_sensCalcDuration)*0.001f);
}

float CCalculationInfo::getVisionSensorCalculationTime()
{
    return(float(_rendSensCalcDuration)*0.001f);
}

float CCalculationInfo::getMainScriptExecutionTime()
{
    return(float(_mainScriptDuration)*0.001f);
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

void CCalculationInfo::setMainScriptExecutionTime(int duration)
{
    _mainScriptDuration=duration;
}

void CCalculationInfo::setSimulationScriptExecCount(int cnt)
{
    _simulationScriptExecCount=cnt;
}

void CCalculationInfo::simulationPassStart()
{
    _simulationPassStartTime=VDateTime::getTimeInMs();
}

void CCalculationInfo::simulationPassEnd()
{
    _simulationPassDuration+=VDateTime::getTimeDiffInMs(_simulationPassStartTime);
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

#ifdef SIM_WITH_GUI
void CCalculationInfo::printInformation()
{
    if (App::currentWorld->buttonBlockContainer==nullptr)
        return;
    if (App::currentWorld->sceneObjects==nullptr)
        return;
    for (int i=0;i<INFO_BOX_ROW_COUNT;i++)
    {
        if (App::currentWorld->buttonBlockContainer->getInfoBoxButton(i,0)==nullptr)
            return;
        if (App::currentWorld->buttonBlockContainer->getInfoBoxButton(i,1)==nullptr)
            return;
        App::currentWorld->buttonBlockContainer->getInfoBoxButton(i,0)->label="";
        App::currentWorld->buttonBlockContainer->getInfoBoxButton(i,1)->label="";
    }
    if (App::currentWorld->buttonBlockContainer!=nullptr)
    {
        int pos=0;
        std::string tmp;
        std::string txt;
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE_OLD)==0)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getLastSelectionObject();
            if (it!=nullptr)
            {

                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected objects:";
                if (App::currentWorld->sceneObjects->getSelectionCount()!=2)
                    tmp=boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getSelectionCount());
                else
                {
                    CSceneObject* it2=App::currentWorld->sceneObjects->getObjectFromHandle(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0));
                    C7Vector v0(it->getFullCumulativeTransformation());
                    C7Vector v1(it2->getFullCumulativeTransformation());
                    tmp="2 (frame-frame distance="+gv::getSizeStr(false,(v0.X-v1.X).getLength(),0)+")";
                }
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=tmp;

                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Last selected object alias:";
                tmp=it->getObjectAlias_printPath();
                tmp+="    (deprecated name:";
                tmp+=it->getObjectName_old();
                tmp+=")";
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=tmp;

                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Last selected object type:";
                tmp=it->getObjectTypeInfoExtended();
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=tmp;

                C7Vector m(it->getCumulativeTransformation());
                C3Vector euler(m.Q.getEulerAngles());
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Last selected object position:";
                txt="x: "+gv::getSizeStr(true,m.X(0))+"    y: "+gv::getSizeStr(true,m.X(1))+"    z: "+gv::getSizeStr(true,m.X(2));
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Last selected object orientation:";
                txt="a: "+gv::getAngleStr(true,euler(0))+"    b: "+gv::getAngleStr(true,euler(1))+"    g: "+gv::getAngleStr(true,euler(2));
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
            }
            else
            {
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected objects:";
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label="0";
            }
        }
        if (App::getEditModeType()&VERTEX_EDIT_MODE)
        {
            if (App::mainWindow->editModeContainer->getEditModeBufferSize()!=2)
            {
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected vertices:";
                tmp=boost::lexical_cast<std::string>(App::mainWindow->editModeContainer->getEditModeBufferSize());
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=tmp;
            }
            else
            {
                C3Vector p1(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(App::mainWindow->editModeContainer->getEditModeBufferValue(0)));
                C3Vector p2(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(App::mainWindow->editModeContainer->getEditModeBufferValue(1)));
                float dist=(p2-p1).getLength();
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected vertices:";
                txt="2 (distance="+gv::getSizeStr(false,dist,0)+")";
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
            }
            if (App::mainWindow->editModeContainer->getEditModeBufferSize()!=0)
            {
                CSceneObject* it=App::mainWindow->editModeContainer->getEditModeObject();
                if (it!=nullptr)
                {
                    C7Vector m(it->getCumulativeTransformation());
                    int lastV=App::mainWindow->editModeContainer->getLastEditModeBufferValue();
                    C3Vector v(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(lastV));
                    v*=m;
                    App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Last selected vertex position:";
                    txt="x: "+gv::getSizeStr(true,v(0))+"    y: "+gv::getSizeStr(true,v(1))+"    z: "+gv::getSizeStr(true,v(2));
                    App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
                }
            }
        }
        if (App::getEditModeType()&TRIANGLE_EDIT_MODE)
        {
            App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected triangles:";
            tmp=boost::lexical_cast<std::string>(App::mainWindow->editModeContainer->getEditModeBufferSize());
            App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=tmp;
        }
        if (App::getEditModeType()&EDGE_EDIT_MODE)
        {
            if (App::mainWindow->editModeContainer->getEditModeBufferSize()==0)
            {
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected edges:";
                tmp="0";
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=tmp;
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
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected edges:";
                txt=boost::lexical_cast<std::string>(App::mainWindow->editModeContainer->getEditModeBufferSize())+" (total edge length="+gv::getSizeStr(false,totLength,0)+")";
                App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
            }
        }



        if (App::getEditModeType()&PATH_EDIT_MODE_OLD)
        { // OLD
            CPathCont_old* pc=App::mainWindow->editModeContainer->getEditModePathContainer_old();
            CPath_old* path=App::mainWindow->editModeContainer->getEditModePath_old();
            if ( (pc!=nullptr)&&(path!=nullptr) )
            {
                if (App::mainWindow->editModeContainer->getEditModeBufferSize()!=2)
                {
                    App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected path points:";
                    txt=boost::lexical_cast<std::string>(App::mainWindow->editModeContainer->getEditModeBufferSize());
                    txt+=" (Bezier path length="+gv::getSizeStr(false,pc->getBezierNormalPathLength(),0)+")";
                    App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
                }
                else
                {
                    CSimplePathPoint_old* pt1(App::mainWindow->editModeContainer->getPathEditMode()->getSimplePathPoint(0));
                    CSimplePathPoint_old* pt2(App::mainWindow->editModeContainer->getPathEditMode()->getSimplePathPoint(1));
                    float dist=(pt2->getTransformation().X-pt1->getTransformation().X).getLength();
                    App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Selected path points:";
                    txt="2 (distance="+gv::getSizeStr(false,dist,0)+")";
                    App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
                }
                if (App::mainWindow->editModeContainer->getEditModeBufferSize()!=0)
                {
                    CSimplePathPoint_old* pt(App::mainWindow->editModeContainer->getPathEditMode()->getSimplePathPoint(App::mainWindow->editModeContainer->getEditModeBufferSize()-1));
//                  CSimplePathPoint_old* pt(pc->getSimplePathPoint(App::currentWorld->objCont->editModeBuffer[App::currentWorld->objCont->editModeBuffer.size()-1]));
                    C7Vector tr(path->getCumulativeTransformation());
                    C3Vector v(tr*pt->getTransformation().X);
                    App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Last selected path point position:";
                    txt="x: "+gv::getSizeStr(true,v(0))+"    y: "+gv::getSizeStr(true,v(1))+"    z: "+gv::getSizeStr(true,v(2));
                    App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
                    C3Vector euler((tr.Q*pt->getTransformation().Q).getEulerAngles());
                    App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label="Last selected path point orientation:";
                    txt="a: "+gv::getAngleStr(true,euler(0))+"    b: "+gv::getAngleStr(true,euler(1))+"    g: "+gv::getAngleStr(true,euler(2));
                    App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txt;
                }
            }
        }




        std::string txtLeft,txtRight;
        int index=0;
        while (App::currentWorld->simulation->getInfo(txtLeft,txtRight,index))
        {
            App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label=txtLeft;
            App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=txtRight;
        }
        if (App::currentWorld->simulation->isSimulationRunning())
        {
            // Script functionality:
            App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label=_scriptTxt[0];
            App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=_scriptTxt[1];
            // Proximity sensor simulation:
            App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label=_sensTxt[0];
            App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=_sensTxt[1];

            // Vision sensor simulation:
            App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label=_visionSensTxt[0];
            App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=_visionSensTxt[1];
            // Dynamics calculation:
            App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos,0)->label=_dynamicsTxt[0];
            App::currentWorld->buttonBlockContainer->getInfoBoxButton(pos++,1)->label=_dynamicsTxt[1];
        }
    }
}
#endif
