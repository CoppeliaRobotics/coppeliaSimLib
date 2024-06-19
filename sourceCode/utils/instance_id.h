#pragma once

#include <QMap>
#include <QSharedMemory>

class InstancesList
{
public:
    InstancesList();
    ~InstancesList();

    int thisInstanceId();
    int numInstances();

private:
    QSharedMemory sharedMemory;

    QMap<int, int> readInstancesList();
    void writeInstancesList(const QMap<int, int> &instancesList);
    int nextInstanceId(const QMap<int, int> &m);
};
