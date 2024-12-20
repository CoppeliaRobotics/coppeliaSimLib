#include <instance_id.h>

#ifdef _WIN32
#include <windows.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#endif

#include <iostream>
#include <QCoreApplication>
#include <QBuffer>
#include <QDataStream>
#include <QDebug>
#include <QSystemSemaphore>
#include <QThread>
#include <app.h>

#define SHAREDMEM_KEY "coppeliaSim_instance_shm"
#define SHAREDMEM_SIZE 4096
#define SEMAPHORE_KEY "coppeliaSim_instance_sem"

static bool pidExists(int pid)
{
    // Define platform-specific code using #ifdef directives
#if defined(__linux__) || defined(__APPLE__)
    // Send a null signal, fails only if the pid does not exist
    return kill(pid, 0) == 0;
#elif defined(_WIN32)
    // On Windows, use OpenProcess with specific access rights to check if the process exists
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, static_cast<DWORD>(pid));
    if (process != NULL)
    {
        CloseHandle(process);
        return true;
    }
    else
    {
        return false;
    }
#else
// Unsupported platform
#error Unsupported platform: pidExists function not implemented for this platform.
    return false;
#endif
}

static int thisInstancePid()
{
#if defined(__linux__) || defined(__APPLE__)
    return getpid();
#elif defined(_WIN32)
    return GetCurrentProcessId();
#else
// Unsupported platform
#error Unsupported platform: thisInstancePid function not implemented for this platform.
    return -1;
#endif
}

InstancesList::InstancesList()
    : sharedMemory(SHAREDMEM_KEY)
{
    int pid = thisInstancePid();

    //qDebug() << "InstancesList::InstancesList: this pid:" << pid;

    QSystemSemaphore semaphore(SEMAPHORE_KEY, 1);

    if (!sharedMemory.attach())
    {
        if (!sharedMemory.create(SHAREDMEM_SIZE))
        {
            std::string errStr("Failed to create shared memory segment: ");
            errStr += sharedMemory.errorString().toStdString();
            App::logMsg(sim_verbosity_errors, errStr.c_str());
        }
    }

    semaphore.acquire();

    QMap<int, int> instances = readInstancesList();
    if (!instances.contains(pid))
    {
        int nextId = nextInstanceId(instances);
        //qDebug() << "InstancesList::InstancesList: add" << pid << "->" << nextId;
        instances[pid] = nextId;
        writeInstancesList(instances);
    }

    semaphore.release();
}

InstancesList::~InstancesList()
{
    if (sharedMemory.isAttached())
    {
        //qDebug() << "InstancesList::~InstancesList: Shared memory is already attached. Detaching it.";
        sharedMemory.detach();
    }
}

int InstancesList::thisInstanceId()
{
    QMap<int, int> instances = readInstancesList();
    int pid = thisInstancePid();
    int instanceId = instances.value(pid, -1);
    return instanceId;
}

int InstancesList::numInstances()
{
    QMap<int, int> instances = readInstancesList();
    return instances.size();
}

QMap<int, int> InstancesList::readInstancesList()
{
    //qDebug() << "InstancesList::readInstancesList: Reading instances table...";

    QMap<int, int> instancesList;

    if (!sharedMemory.isAttached())
    {
        if (!sharedMemory.attach())
        {
            std::string errStr("Failed to attach to shared memory segment: ");
            errStr += sharedMemory.errorString().toStdString();
            App::logMsg(sim_verbosity_errors, errStr.c_str());
            return instancesList;
        }
    }

    // Lock the shared memory for reading
    if (!sharedMemory.lock())
    {
        App::logMsg(sim_verbosity_errors, "Failed to lock shared memory segment.");
        return instancesList;
    }

    QBuffer buffer;
    buffer.setData(static_cast<const char*>(sharedMemory.constData()), sharedMemory.size());
    buffer.open(QBuffer::ReadOnly);

    // Read data from the buffer using QDataStream
    QDataStream stream(&buffer);
    stream.setVersion(QDataStream::Qt_5_15); // Adjust version as needed

    stream >> instancesList;

    //qDebug() << "InstancesList::readInstancesList: Read a table of instances with" << instancesList.size() << "entries:";
    //for (auto it = instancesList.keyValueBegin(); it != instancesList.keyValueEnd(); ++it)
    //    qDebug() << it->first << it->second;

    // Unlock and detach shared memory
    sharedMemory.unlock();

    // remove "dead" instances:
    for (auto it = instancesList.begin(); it != instancesList.end();)
    {
        int pid = it.key();
        if (!pidExists(pid))
        {
            it = instancesList.erase(it);
            //qDebug() << "InstancesList::readInstancesList: removing \"dead\" pid" << pid;
        }
        else
            ++it;
    }

    return instancesList;
}

void InstancesList::writeInstancesList(const QMap<int, int>& instancesList)
{
    if (!sharedMemory.isAttached())
    {
        if (!sharedMemory.attach())
        {
            std::string errStr("Failed to attach to shared memory segment: ");
            errStr += sharedMemory.errorString().toStdString();
            App::logMsg(sim_verbosity_errors, errStr.c_str());
            return;
        }
    }

    //qDebug() << "InstancesList::writeInstancesList: Writing an instances table of" << instancesList.size() << "entries:";
    //for (auto it = instancesList.keyValueBegin(); it != instancesList.keyValueEnd(); ++it)
    //    qDebug() << it->first << it->second;

    // Create a buffer to write to the shared memory
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);

    // Write data to the buffer using QDataStream
    QDataStream stream(&buffer);
    stream.setVersion(QDataStream::Qt_5_15); // Adjust version as needed

    stream << instancesList;

    // Lock the shared memory for writing
    if (!sharedMemory.lock())
    {
        App::logMsg(sim_verbosity_errors, "Failed to lock shared memory segment.");
        return;
    }

    // Copy the buffer data to shared memory
    memcpy(sharedMemory.data(), buffer.data().constData(), qMin(qint64(sharedMemory.size()), buffer.size()));

    //qDebug() << "InstancesList::writeInstancesList: Wrote" << buffer.size() << "bytes";

    sharedMemory.unlock();
}

int InstancesList::nextInstanceId(const QMap<int, int>& m)
{
    int nextId = 0;
    auto values = m.values();
    QSet<int> valuesSet(values.begin(), values.end());
    while (valuesSet.contains(nextId))
        ++nextId;
    return nextId;
}

#ifdef TEST
int main()
{
    InstancesList instancesList; // keep this alive until application quits
    qDebug() << "constructed a InstancesList object";

    std::cout << "instance id = " << instancesList.thisInstanceId() << std::endl;
    std::cout << "number of live instances = " << instancesList.numInstances() << std::endl;
    QThread::sleep(100);

    return 0;
}
#endif
