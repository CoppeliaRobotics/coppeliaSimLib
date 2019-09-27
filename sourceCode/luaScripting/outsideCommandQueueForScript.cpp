
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "outsideCommandQueueForScript.h"
#include "app.h"

const unsigned int MAX_QUEUE_SIZE=64; // no more than 64 messages at a given time!

COutsideCommandQueueForScript::COutsideCommandQueueForScript()
{
//  flagToCompareSelections=true;
}

COutsideCommandQueueForScript::~COutsideCommandQueueForScript()
{
}

void COutsideCommandQueueForScript::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    commands.clear();
    auxValues.clear();
    aux2Values.clear();
    aux2Cnt.clear();
}

void COutsideCommandQueueForScript::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    commands.clear();
    auxValues.clear();
    aux2Values.clear();
    aux2Cnt.clear();
}

bool COutsideCommandQueueForScript::addCommand(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const float* aux2Vals,int aux2Count)
{ // the queue can't be bigger than 64! (for now)
    // For the Lua-API:
    if (commandID<sim_message_for_c_api_only_start)
    {
        if (commands.size()>=MAX_QUEUE_SIZE)
        { // We remove the last command:
            commands.erase(commands.begin()+0);
            auxValues.erase(auxValues.begin()+0,auxValues.begin()+4);
            aux2Values.erase(aux2Values.begin()+0,aux2Values.begin()+8);
            aux2Cnt.erase(aux2Cnt.begin()+0);
        }
        // REMOVED ON 2010/02/22:
        //for (int i=0;i<int(commands_lua.size());i++)
        //{ // We remove a same command if present:
        //  if (commands_lua[i]==commandID)
        //  {
        //      commands_lua.erase(commands_lua.begin()+i);
        //      auxValues_lua.erase(auxValues_lua.begin()+4*i+0,auxValues_lua.begin()+4*i+4);
        //      break;
        //  }
        //}
        // We add the command:
        commands.push_back(commandID);
        auxValues.push_back(auxVal1);
        auxValues.push_back(auxVal2);
        auxValues.push_back(auxVal3);
        auxValues.push_back(auxVal4);
        int tot=0;
        if ((aux2Vals!=nullptr)&&(aux2Count>0))
        {
            if (aux2Count>8)
                aux2Count=8;
            for (int i=0;i<aux2Count;i++)
                aux2Values.push_back(aux2Vals[i]);
            tot+=aux2Count;
        }
        aux2Cnt.push_back(aux2Count);
        for (int i=0;i<8-tot;i++)
            aux2Values.push_back(0.0f);
    }
    return(true);
}

int COutsideCommandQueueForScript::extractOneCommand(int auxVals[4],float aux2Vals[8],int& aux2Count)
{
    if (flagToCompareSelections)
        _checkSelections();
    if (commands.size()==0)
    {
        flagToCompareSelections=true; // next time this function is called, we compare selections
        return(-1);
    }
    int retVal=commands[0];
    aux2Count=aux2Cnt[0];
    for (int i=0;i<4;i++)
        auxVals[i]=auxValues[i];
    auxValues.erase(auxValues.begin()+0,auxValues.begin()+4);
    for (int i=0;i<8;i++)
        aux2Vals[i]=aux2Values[i];
    aux2Values.erase(aux2Values.begin()+0,aux2Values.begin()+8);
    aux2Cnt.erase(aux2Cnt.begin());
    commands.erase(commands.begin());
    return(retVal);
}

void COutsideCommandQueueForScript::_checkSelections()
{
    if (!App::ct->objCont->isSelectionSame(previousObjectSelection,true))
    {
        int l=-1;
        int t=-1;
        if (previousObjectSelection.size()!=0)
        {
            l=previousObjectSelection[previousObjectSelection.size()-1];
            C3DObject* it=App::ct->objCont->getLastSelection_object();
            if (it!=nullptr)
                t=it->getObjectType();
        }
        addCommand(sim_message_object_selection_changed,(int)previousObjectSelection.size(),l,t,0,nullptr,0);
    }
    flagToCompareSelections=false;
}
