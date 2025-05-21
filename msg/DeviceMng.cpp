#include "DeviceMng.h"
#include "MsgMng.h"


DeviceMng::DeviceMng()
{
    SysMinKey   = 0;
    SysMaxKey   = 0;
    AppStartKey = SysMaxKey + 1;
    CfgList.clear();
    // DriverTable.clear();
    AppUseKeyList.clear();
    InitFinishFlag = false;
    m_pMsgMngServer = MsgMngServer::GetMsgMngServer();
}

DeviceMng::~DeviceMng()
{
    mDriverTable::iterator item;

    // sysLogQD() << "DeviceMng**************~DeviceMng";
    // for (item = DriverTable.begin(); item != DriverTable.end(); ++item)
    // {
    //     // item.value()->pmsg->delete_object();
    //     delete item.value();
    //     item.value() = (driver*) 0;
    // }
    // CfgList.clear();
    // DriverTable.clear();
    AppUseKeyList.clear();
    CfgList.clear();
    DELETE(m_pMsgMngServer);
}

DeviceMng* DeviceMng::pDeviceCmd = NULL;
DeviceMng* DeviceMng::GetDeviceMng()
{
    if (pDeviceCmd == NULL)
    {
        pDeviceCmd = new DeviceMng();
    }
    return pDeviceCmd;
}

void DeviceMng::loadCfgFile(const QString filePath)
{
    QDomDocument doc;
    QString      filepathname = filePath;
    QFile        file(filepathname);

    QString errorStr;
    int     errorLine;
    int     errorColumn;

    if (!doc.setContent(&file, true, &errorStr, &errorLine, &errorColumn))
    {
        zprintf1("DeviceMng Line errorLine %d column:%d err: %s!\n", errorLine,  errorColumn ,errorStr.toLatin1().data());
        return;
    }

    QDomElement doc_root = doc.documentElement();
    if (doc_root.tagName() != "DriverList")
    {
        zprintf1("DeviceMng The driver list config file is not valid");
        return;
    }

    CfgList.clear();
    QDomElement element;
    QDomNode    doc_node = doc_root.firstChild();
    while (!doc_node.isNull())
    {
        if (doc_node.toElement().tagName() == "Driver")
        {
            element = doc_node.toElement();
            sDeviceCfg tmpnode;
            tmpnode.id     = element.attribute("id").toInt();
            tmpnode.name   = element.attribute("drivername");
            tmpnode.script = element.attribute("startscript");
            CfgList.append(tmpnode);
        }
        doc_node = doc_node.nextSibling();
    }
}

void DeviceMng::loadShareParam(const QString filePath)
{
    QSettings settings(filePath, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    QString value;

    settings.beginGroup("MEMERY_PARAM");
    value     = QString("SYS_KEY_RESERVE");
    SysResKey = settings.value(value).toInt();
    value     = QString("SYS_KEY_MIN");
    SysMinKey = settings.value(value).toInt();
    value     = QString("SYS_KEY_MAX");
    SysMaxKey = settings.value(value).toInt();
    settings.endGroup();
    settings.deleteLater();
}

int DeviceMng::OperateAppMsgKey(eOperateKeyType mode, int key)
{
    int remain = SysMaxKey - AppStartKey + 1 - AppUseKeyList.size();
    if (mode == KEY_ADD)
    {
        if (remain <= 0)
            return 0;
        for (int i = AppStartKey; i < SysMaxKey; i++)
        {
            if (!AppUseKeyList.contains(i))
            {
                AppUseKeyList.append(i);
                return i;
            }
        }
    }
    else if (mode == KEY_SUB)
    {
        if (AppUseKeyList.size() == 0)
            return 0;
        for (int i = 0; i < AppUseKeyList.size(); i++)
        {
            if (AppUseKeyList.at(i) == key)
            {
                AppUseKeyList.removeAt(i);
                return 1;
            }
        }
    }
    return 0;
}

bool DeviceMng::CheckParamValidity(void)
{
    for (int i = 1; i < CfgList.size(); i++)
    {
        for (int j = 0; j < i; j++)
        {
            if (CfgList.at(j).id == CfgList.at(i).id)
            {
                zprintf3("DeviceMng cfglist id exist!\n");
                return false;
            }
        }
    }

    if (SysMaxKey < SysMinKey)
    {
        zprintf1("DeviceMng SysMaxKey < SysMinKey!\n");
        return false;
    }

    AppUseKeyList.clear();
    if ((SysMaxKey - SysMinKey + 1) <= (DRIVER_SHARE_KEY_NUM * CfgList.size() + DEVICEMNG_SHARE_KEY_NUM))
    {
       zprintf1("DeviceMng SysKey num fail,SysMaxKey: %d SysMinKey: %d CfgList.size() %d!\n",SysMaxKey ,SysMinKey
                                                       , CfgList.size());
        return false;
    }

    AppStartKey = SysMinKey + DRIVER_SHARE_KEY_NUM * CfgList.size() + DEVICEMNG_SHARE_KEY_NUM;
    return true;
}

int DeviceMng::GetDeviceMngKey(void)
{
    return SysMinKey;
}

int DeviceMng::GetDeviceMngResKey(void)
{
    return SysResKey;
}

void DeviceMng::ShellSetupDriver(sDeviceCfg& cfg, int key, int drivermsgkey)
{
    QString script;
    QString chmod = "chmod +x ";

    chmod = chmod + cfg.script;
    system(chmod.toLatin1().data());

    script = cfg.script + QString(" %1 %2 %3 %4 %5 %6 %7")
                              .arg(key)
                              .arg(key + 1)
                              .arg(key + 2)
                              .arg(key + 3)
                              .arg(drivermsgkey)
                              .arg(cfg.id)
                              .arg(key + 4);
    zprintf1("DeviceMng ShellSetupDriver : %s!\n" , script.toStdString().c_str());
    system(script.toLatin1().data());
}

bool DeviceMng::SetupDriver(void)
{
    driver*    pdriver;
    int        keytemp;
    sDeviceCfg cfg;
    int        usekey = SysMinKey + DEVICEMNG_SHARE_KEY_NUM;
    MsgMngServer*  pMsgMngServer = MsgMngServer::GetMsgMngServer();
    struct timeval tv;

    keytemp = SysMinKey + DEVICEMNG_SHARE_KEY_NUM;

    for (int i = 0; i < CfgList.size(); i++)
    {
        gettimeofday(&tv, NULL);
        keytemp += DRIVER_SHARE_KEY_NUM * i;

        cfg.id = CfgList.at(i).id;
        cfg.name = CfgList.at(i).name;
        pdriver  = new driver(cfg.id, cfg.name, keytemp, keytemp + 1, keytemp + 2, keytemp + 3, keytemp + 4);

        m_pMsgMngServer->m_driverTable.insert(cfg.id, pdriver);
        if (!pdriver->initMsg())
            return false;

        cfg.script = CfgList.at(i).script;
        ShellSetupDriver(
            cfg, SysMinKey + DEVICEMNG_SHARE_KEY_NUM + DRIVER_SHARE_KEY_NUM * i, SysMinKey + 1);
        usekey += DRIVER_SHARE_KEY_NUM;

        if(!pdriver->msgGetInfo())
        {
            zprintf1("DeviceMng device[%d] Msg_GetInfo error!\n", cfg.id);
            return false;
        }

        if(!pMsgMngServer->waitDriverInfo(pdriver))
        {
            zprintf1("DeviceMng device[%d] waitDriverInfo error!\n", cfg.id);
        }


        if (!pdriver->init())
        {
            gettimeofday(&tv, NULL);
            zprintf1("DeviceMng device[%d] Initmem error time: %d!\n", cfg.id , tv.tv_sec);
            return false;
        }
    }
    AppStartKey = usekey;

    gettimeofday(&tv, NULL);
    zprintf3("DeviceMng device SetupDriver end time: %d.\n" ,tv.tv_sec);

    pMsgMngServer->startRecvDrivMsgProcess();
    // m_recvAppMsg.z_pthread_init(msgmng_apprecv_back, this, "msgservapprecv");
    return true;
}

void DeviceMng::SendHeartToDriver(void)
{
    mDriverTable::iterator item;

    m_pMsgMngServer->m_recvDriMsg.lock();
    for (item = m_pMsgMngServer->m_driverTable.begin(); item != m_pMsgMngServer->m_driverTable.end(); ++item)
    {
        item.value()->msgSendHeart();
    }
    m_pMsgMngServer->m_recvDriMsg.unlock();
}

void DeviceMng::DriverHeartMng(void)
{
    // int                    ret;
    mDriverTable::iterator item;
    uint8_t                data;

    for (item = m_pMsgMngServer->m_driverTable.begin(); item != m_pMsgMngServer->m_driverTable.end(); ++item)
    {
        if(item.value()->m_heartMark) //心跳错误
        {
            if (item.value()->m_comState != COMSTATE_ABNORMAL)
            {
                item.value()->m_comState = COMSTATE_ABNORMAL;
                data                   = (uint8_t) item.key();
                // m_pMsgMngServer->BroadcastToApp(MSG_TYPE_AppReportDriverComAbnormal, &data, 1);
                zprintf1("DeviceMng report driver%d com abnormal!\n", item.value()->m_driverId);
            }

        }
        else
        {
            if (item.value()->m_comState != COMSTATE_NORMAL)
            {
                item.value()->m_comState = COMSTATE_NORMAL;
                data                   = (uint8_t) item.key();
                // pMsgMng->BroadcastToApp(MSG_TYPE_AppReportDriverComNormal, &data, 1);
                zprintf1("DeviceMng report driver%d com normal.\n", item.value()->m_driverId);
            }

        }
    }
}
