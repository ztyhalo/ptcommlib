#ifndef MSGMNGBASE_H
#define MSGMNGBASE_H

#include "app.h"
#include "driver.h"
#include "mutex_class.h"




typedef QMap< uint32_t, app* >    mAppMap;
typedef QMap< int,  driver * >    mDrivMap;

class DrivAppMap
{

public:
    mDrivMap        m_drivMap;
    mAppMap         m_appMap;
    // MUTEX_CLASS     m_drivMutex;
    MUTEX_CLASS     m_mapMutex;
public:
    DrivAppMap();
    ~DrivAppMap();
    bool findApp(uint32_t id, app** ppapp);
    bool deleteApp(uint32_t id);
    bool findDriver(uint8_t id, driver** ppdriver);
    bool isAppMapExist(uint32_t id);
};
#endif // MSGMNGBASE_H
