#include "msgmngbase.h"
#include <mutex>

DrivAppMap::DrivAppMap()
{

}
DrivAppMap::~DrivAppMap()
{
    zprintf3("DrivAppMap destruct!\n");
}

bool DrivAppMap::findApp(uint32_t id, app** ppapp)
{
    mAppMap::iterator item;

    item = m_appMap.find(id);
    if ((item != m_appMap.end()) && (item.key() == id))
    {
        *ppapp = item.value();
        return true;
    }
    *ppapp = (app*) 0;
    return false;
}

bool DrivAppMap::deleteApp(uint32_t id)
{
    mAppMap::iterator item;
    lock_guard<MUTEX_CLASS> lock(m_mapMutex);
    item = m_appMap.find(id);
    if ((item != m_appMap.end()) && (item.key() == id))
    {
        m_appMap.erase(item);
        return true;
    }
    return false;
}

bool DrivAppMap::findDriver(uint8_t id, driver** ppdriver)
{
    mDrivMap::iterator item;

    item = m_drivMap.find(id);
    if ((item != m_drivMap.end()) && (item.key() == id))
    {
        *ppdriver = item.value();
        return true;
    }
    *ppdriver = (driver*) 0;
    return false;
}

bool DrivAppMap::isAppMapExist(uint32_t id)
{
    mAppMap::iterator item;
    // lock_guard<MUTEX_CLASS> lock(m_appMutex);
    item = m_appMap.find(id);
    if ((item != m_appMap.end()) && (item.key() == id))
    {
        return true;
    }
    return false;
}
