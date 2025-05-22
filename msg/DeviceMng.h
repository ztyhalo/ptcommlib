#ifndef DEVICEMNG_H
#define DEVICEMNG_H

#include <QObject>
#include <QDomDocument>
#include <QFile>
#include <QMap>
#include "driver.h"
#include "MsgMng.h"
#include <QSettings>

#ifdef I386
#define CFGXML_FILE_PATH "/opt/config/devicemng/DeviceManager.xml"
#define CFGINI_FILE_PATH "/opt/config/devicemng/devicemng.ini"
#else
#define CFGXML_FILE_PATH "/opt/config/devicemng/DeviceManager.xml"
#define CFGINI_FILE_PATH "/opt/config/devicemng/devicemng.ini"
#endif



typedef struct
{
    int     id;
    QString name;
    QString script;
} sDeviceCfg;
typedef QList< sDeviceCfg>  lCfgType;
// typedef QMap< int, driver * > mDriverTable;


class DeviceMngBase
{
  public:
    lCfgType     CfgList;

  public:
    bool              m_initOk;
    MsgKeyClass *     m_pKey;
    DeviceMngBase();
    ~DeviceMngBase();
    void deviceMngBaseInit(MsgKeyClass * key);
    void loadCfgFile(const QString & filePath);
    void loadShareParam(const QString & filePath);

    bool CheckParamValidity(void);
    int  GetDeviceMngKey(void);
    int  GetDeviceMngResKey(void);
};

class DeviceMng:public DeviceMngBase
{
  private:
    DeviceMng();
    void ShellSetupDriver(sDeviceCfg& cfg, int key, int drivermsgkey);

  public:

    MsgMngServer *    m_pMngServ;

    static DeviceMng* pDeviceCmd;

    static DeviceMng* GetDeviceMng(void);
    ~DeviceMng();

    bool SetupDriver(void);
    void SendHeartToDriver(void);
    void DriverHeartMng(void);

};


class DeviceMngApp:public DeviceMngBase
{
  private:
    DeviceMngApp();


  public:

    MsgMngApp *    m_pMngApp;

    static DeviceMngApp* pAppCmd;

    static DeviceMngApp* getDeviceMngApp(void);
    ~DeviceMngApp();
    int  initApp(void);
    bool setupDriver(uint32_t timeout);
    uint32_t getAppid(uint8_t DriverID, uint8_t ParentDeviceID, uint8_t ChildDeviceID, uint8_t PointID, uint8_t type);
    void changeAppid(uint32_t appid, uint8_t* DriverID, uint8_t* ParentDeviceID, uint8_t* ChildDeviceID,
            uint8_t* PointID, uint8_t* type);

};


#endif // DEVICEMNG_H
