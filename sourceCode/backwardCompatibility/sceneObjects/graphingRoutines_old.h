#pragma once

#include <graphData_old.h>

class CGraphingRoutines_old
{
public:
    CGraphingRoutines_old();
    virtual ~CGraphingRoutines_old();

    static std::string getDataUnit(CGraphData_old* it);
    static void adjustDataToUserMetric(const CGraphData_old* it,double& v,int derivativeIntegralOrCumulative);
    static void getCyclicAndRangeValues(const CGraphData_old* it,bool& cyclic,double& range);

    static bool getDataName(int dataIndex,std::string& dataName);
    static bool getDataValue(int dataIndex,int objectID,double& value,const C7Vector* graphCTM=nullptr);
    static bool getGraphObjectName(int dataIndex,int objectID,std::string& objName);
    static bool getObjectsFromGraphCategory(int index,int dataIndex,int& objectID);

    // general:
    static bool loopThroughAllAndGetDataName(int dataIndex,std::string& dataName);
    static bool loopThroughAllAndGetDataValue(int dataIndex,int objectID,double& value,const C7Vector* graphCTM=nullptr);

    static bool loopThroughAllAndGetGraphObjectName(int dataIndex,int objectID,std::string& objName);
    static bool loopThroughAllAndGetObjectsFromGraphCategory(int index,int dataIndex,int& objectID);
};
