
#pragma once


class COutsideCommandQueueForScript  
{
public:
    COutsideCommandQueueForScript();
    virtual ~COutsideCommandQueueForScript();

    bool addCommand(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const float* aux2Vals,int aux2Count);
//  int extractOneCommand(int auxVals[4]);
    int extractOneCommand(int auxVals[4],float aux2Vals[8],int& aux2Count);
    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationEnded();

private:
    void _checkSelections();
    std::vector<int> commands; // for each command, there are 4 auxiliary values, and 8 aux2 values!
    std::vector<int> auxValues;
    std::vector<float> aux2Values;
    std::vector<int> aux2Cnt;

    std::vector<int> previousObjectSelection; // In order to generate the sim_message_object_selection_changed
    bool flagToCompareSelections;
};
