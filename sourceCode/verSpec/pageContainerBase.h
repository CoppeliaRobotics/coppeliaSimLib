#pragma once

#include "app.h"
#include "pluginContainer.h"

class CPageContainerBase
{
public:
#ifdef SIM_WITH_GUI
    static int handleVerSpec_getTag(bool hideWatermark)
    {   int tagId=-1;
        // If you are using an EDU license for the DYNAMICS plugin, MESH CALCULATION plugin or PATH PLANNING plugin, you have to keep the V-REP tag in place!!
        if ((!hideWatermark)&&(App::mainWindow!=nullptr)&&(!App::mainWindow->simulationRecorder->getIsRecording()))
        {
            // If you are using V-REP in conjunction with the DYNAMICS PLUGIN or the MESH CALCULATION PLUGIN, you have to keep the EDU tag!! See the license agreements for details.
            if (CPluginContainer::isMeshPluginAvailable())
                tagId=EDU_TAG;
            else
                tagId=VREP_TAG;
        }
        return(tagId);
    }
#endif
};
