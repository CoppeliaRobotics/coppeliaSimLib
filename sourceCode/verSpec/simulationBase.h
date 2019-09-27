#pragma once

#ifndef SIM_WITHOUT_QT_AT_ALL
#ifdef SIM_WITH_GUI
    #include "vMenubar.h"
#endif
    #include "app.h"
#endif

class CSimulationBase
{
public:
#ifdef SIM_WITH_GUI
    static void handleVerSpec_addMenu(VMenu* menu)
    {
        bool noEditMode=(App::getEditModeType()==NO_EDIT_MODE);
        bool simRunning=App::ct->simulation->isSimulationRunning();
        bool simStopped=App::ct->simulation->isSimulationStopped();
        bool simPaused=App::ct->simulation->isSimulationPaused();
        bool canGoSlower=App::ct->simulation->canGoSlower();
        bool canGoFaster=App::ct->simulation->canGoFaster();
        bool canToggleThreadedRendering=App::ct->simulation->canToggleThreadedRendering();
        bool getThreadedRenderingIfSimulationWasRunning=App::ct->simulation->getThreadedRenderingIfSimulationWasRunning();

        if (simPaused)
            menu->appendMenuItem(App::mainWindow->getPlayViaGuiEnabled()&&noEditMode,false,SIMULATION_COMMANDS_START_RESUME_SIMULATION_REQUEST_SCCMD,IDS_RESUME_SIMULATION_MENU_ITEM);
        else
            menu->appendMenuItem(App::mainWindow->getPlayViaGuiEnabled()&&noEditMode&&(!simRunning),false,SIMULATION_COMMANDS_START_RESUME_SIMULATION_REQUEST_SCCMD,IDS_START_SIMULATION_MENU_ITEM);
        menu->appendMenuItem(App::mainWindow->getPauseViaGuiEnabled()&&noEditMode&&simRunning,false,SIMULATION_COMMANDS_PAUSE_SIMULATION_REQUEST_SCCMD,IDS_PAUSE_SIMULATION_MENU_ITEM);
        menu->appendMenuItem(App::mainWindow->getStopViaGuiEnabled()&&noEditMode&&(!simStopped),false,SIMULATION_COMMANDS_STOP_SIMULATION_REQUEST_SCCMD,IDS_STOP_SIMULATION_MENU_ITEM);
        menu->appendMenuSeparator();
        int version;
        int engine=App::ct->dynamicsContainer->getDynamicEngineType(&version);
        menu->appendMenuItem(noEditMode&&simStopped,(engine==sim_physics_bullet)&&(version==0),SIMULATION_COMMANDS_TOGGLE_TO_BULLET_2_78_ENGINE_SCCMD,IDS_SWITCH_TO_BULLET_2_78_ENGINE_MENU_ITEM,true);
        menu->appendMenuItem(noEditMode&&simStopped,(engine==sim_physics_bullet)&&(version==283),SIMULATION_COMMANDS_TOGGLE_TO_BULLET_2_83_ENGINE_SCCMD,IDS_SWITCH_TO_BULLET_2_83_ENGINE_MENU_ITEM,true);
        menu->appendMenuItem(noEditMode&&simStopped,engine==sim_physics_ode,SIMULATION_COMMANDS_TOGGLE_TO_ODE_ENGINE_SCCMD,IDS_SWITCH_TO_ODE_ENGINE_MENU_ITEM,true);
        menu->appendMenuItem(noEditMode&&simStopped,engine==sim_physics_vortex,SIMULATION_COMMANDS_TOGGLE_TO_VORTEX_ENGINE_SCCMD,IDS_SWITCH_TO_VORTEX_ENGINE_MENU_ITEM,true);
        menu->appendMenuItem(noEditMode&&simStopped,engine==sim_physics_newton,SIMULATION_COMMANDS_TOGGLE_TO_NEWTON_ENGINE_SCCMD,IDS_SWITCH_TO_NEWTON_ENGINE_MENU_ITEM,true);
        menu->appendMenuSeparator();
        menu->appendMenuItem(noEditMode&&simStopped,App::ct->simulation->getRealTimeSimulation(),SIMULATION_COMMANDS_TOGGLE_REAL_TIME_SIMULATION_SCCMD,IDSN_REAL_TIME_SIMULATION,true);
        menu->appendMenuItem(canGoSlower,false,SIMULATION_COMMANDS_SLOWER_SIMULATION_SCCMD,IDSN_SLOW_DOWN_SIMULATION);
        menu->appendMenuItem(canGoFaster,false,SIMULATION_COMMANDS_FASTER_SIMULATION_SCCMD,IDSN_SPEED_UP_SIMULATION);
        menu->appendMenuItem(canToggleThreadedRendering,getThreadedRenderingIfSimulationWasRunning,SIMULATION_COMMANDS_THREADED_RENDERING_SCCMD,IDSN_THREADED_RENDERING,true);
        menu->appendMenuItem(simRunning&&(!App::mainWindow->oglSurface->isSceneSelectionActive()||App::mainWindow->oglSurface->isPageSelectionActive()||App::mainWindow->oglSurface->isViewSelectionActive()),!App::mainWindow->getOpenGlDisplayEnabled(),SIMULATION_COMMANDS_TOGGLE_VISUALIZATION_SCCMD,IDSN_TOGGLE_VISUALIZATION,true);

        menu->appendMenuSeparator();
        if (App::mainWindow!=nullptr)
            menu->appendMenuItem(true,App::mainWindow->dlgCont->isVisible(SIMULATION_DLG),TOGGLE_SIMULATION_DLG_CMD,IDSN_SIMULATION_SETTINGS,true);
    }
#endif
};
