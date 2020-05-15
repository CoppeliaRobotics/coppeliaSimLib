
#pragma once

#include "graphData.h"

class CGraphingRoutines
{
public:
    CGraphingRoutines();
    virtual ~CGraphingRoutines();

    static void resetAllGraphs(bool exceptExplicitHandling);
    static void handleAllGraphs(bool exceptExplicitHandling,float time);

    static std::string getDataUnit(CGraphData* it);
    static void adjustDataToUserMetric(const CGraphData* it,float& v,int derivativeIntegralOrCumulative);
    static void getCyclicAndRangeValues(const CGraphData* it,bool& cyclic,float& range);

    static bool getDataName(int dataIndex,std::string& dataName);
    static bool getDataValue(int dataIndex,int objectID,float& value,const C7Vector* graphCTM=nullptr);
    static bool getGraphObjectName(int dataIndex,int objectID,std::string& objName);
    static bool getObjectsFromGraphCategory(int index,int dataIndex,int& objectID);

    // general:
    static bool loopThroughAllAndGetDataName(int dataIndex,std::string& dataName);
    static bool loopThroughAllAndGetDataValue(int dataIndex,int objectID,float& value,const C7Vector* graphCTM=nullptr);

    static bool loopThroughAllAndGetGraphObjectName(int dataIndex,int objectID,std::string& objName);
    static bool loopThroughAllAndGetObjectsFromGraphCategory(int index,int dataIndex,int& objectID);
};
