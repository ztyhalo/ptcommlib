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

class DeviceMng
{
  private:
    lCfgType     CfgList;
    // mDriverTable DriverTable;
    // int          SysResKey;
    // int          SysMinKey;
    // int          SysMaxKey;
    // int          AppStartKey;
    // QList< int > AppUseKeyList;

    DeviceMng();
    void ShellSetupDriver(sDeviceCfg& cfg, int key, int drivermsgkey);

  public:
    bool              InitFinishFlag;
    MsgMngServer *    m_pMsgMngServer;
    static DeviceMng* pDeviceCmd;

    static DeviceMng* GetDeviceMng(void);
    ~DeviceMng();
    void loadCfgFile(const QString filePath);
    void loadShareParam(const QString filePath);
    // int  OperateAppMsgKey(eOperateKeyType mode, int key);

    bool CheckParamValidity(void);
    int  GetDeviceMngKey(void);
    int  GetDeviceMngResKey(void);
    bool SetupDriver(void);
    void SendHeartToDriver(void);
    void DriverHeartMng(void);
};

#endif // DEVICEMNG_H
