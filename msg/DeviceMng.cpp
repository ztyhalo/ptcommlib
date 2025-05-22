#include "DeviceMng.h"
#include "MsgMng.h"

DeviceMngBase::DeviceMngBase():m_initOk(false),m_pKey(NULL)
{
    ;
}
DeviceMngBase::~DeviceMngBase()
{
    zprintf3("DeviceMngBase destruct!\n");
}

void DeviceMngBase::deviceMngBaseInit(MsgKeyClass * key)
{
    m_pKey = key;
}
void DeviceMngBase::loadCfgFile(const QString & filePath)
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

void DeviceMngBase::loadShareParam(const QString & filePath)
{
    QSettings settings(filePath, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    if(m_pKey == NULL)
    {
        zprintf1("DeviceMngBase m_pKey is NULL!\n");
        return;
    }
    QString value;



    settings.beginGroup("MEMERY_PARAM");
    value     = QString("SYS_KEY_RESERVE");
    m_pKey->m_resMsgKey = settings.value(value).toInt();
    value     = QString("SYS_KEY_MIN");
    m_pKey->m_minMsgKey = settings.value(value).toInt();
    value     = QString("SYS_KEY_MAX");
    m_pKey->m_maxMsgKey = settings.value(value).toInt();
    settings.endGroup();
    settings.deleteLater();
}


bool DeviceMngBase::CheckParamValidity(void)
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
    if(m_pKey == NULL)
        return false;
    return m_pKey->keyCheckInit(CfgList.size());

}

int DeviceMngBase::GetDeviceMngKey(void)
{
    return m_pKey->m_minMsgKey;
}

int DeviceMngBase::GetDeviceMngResKey(void)
{
    return m_pKey->m_resMsgKey;
}


DeviceMng::DeviceMng()
{

    CfgList.clear();
    m_pMngServ = MsgMngServer::GetMsgMngServer();
    this->deviceMngBaseInit(m_pMngServ);
}

DeviceMng::~DeviceMng()
{
    // mDriverTable::iterator item;

    CfgList.clear();
    DELETE(m_pMngServ);
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
    int        usekey = m_pMngServ->m_minMsgKey + DEVICEMNG_SHARE_KEY_NUM;

    struct timeval tv;

    keytemp = m_pMngServ->m_minMsgKey + DEVICEMNG_SHARE_KEY_NUM;

    for (int i = 0; i < CfgList.size(); i++)
    {
        gettimeofday(&tv, NULL);
        keytemp += DRIVER_SHARE_KEY_NUM * i;

        cfg.id = CfgList.at(i).id;
        cfg.name = CfgList.at(i).name;
        pdriver  = new driver(cfg.id, cfg.name, keytemp, keytemp + 1, keytemp + 2, keytemp + 3, keytemp + 4);

        m_pMngServ->m_driverTable.insert(cfg.id, pdriver);
        if (!pdriver->initMsg())
            return false;

        cfg.script = CfgList.at(i).script;
        ShellSetupDriver(
            cfg, m_pMngServ->m_minMsgKey + DEVICEMNG_SHARE_KEY_NUM + DRIVER_SHARE_KEY_NUM * i, m_pMngServ->m_minMsgKey + 1);
        usekey += DRIVER_SHARE_KEY_NUM;

        if(!pdriver->msgGetInfo())
        {
            zprintf1("DeviceMng device[%d] Msg_GetInfo error!\n", cfg.id);
            return false;
        }

        if(!m_pMngServ->waitDriverInfo(pdriver))
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
    m_pMngServ->m_appMsgKey = usekey;

    gettimeofday(&tv, NULL);
    zprintf3("DeviceMng device SetupDriver end time: %d.\n" ,tv.tv_sec);

    m_pMngServ->startRecvDrivMsgProcess();
    // m_recvAppMsg.z_pthread_init(msgmng_apprecv_back, this, "msgservapprecv");
    return true;
}

void DeviceMng::SendHeartToDriver(void)
{
    mDriverTable::iterator item;

    m_pMngServ->m_recvDriMsg.lock();
    for (item = m_pMngServ->m_driverTable.begin(); item != m_pMngServ->m_driverTable.end(); ++item)
    {
        item.value()->msgSendHeart();
    }
    m_pMngServ->m_recvDriMsg.unlock();
}

void DeviceMng::DriverHeartMng(void)
{
    // int                    ret;
    mDriverTable::iterator item;
    uint8_t                data;

    for (item = m_pMngServ->m_driverTable.begin(); item != m_pMngServ->m_driverTable.end(); ++item)
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


/*******************************************************************************************************************************************
 *
 * DeviceMngApp
 * **************************************************************************************************************************************/

DeviceMngApp* DeviceMngApp::pAppCmd = NULL;
DeviceMngApp* DeviceMngApp::getDeviceMngApp()
{
    if (pAppCmd == NULL)
    {
        pAppCmd = new DeviceMngApp();
    }
    return pAppCmd;
}

DeviceMngApp::DeviceMngApp()
{
    CfgList.clear();
    m_pMngApp = MsgMngApp::getMsgMngApp();
    this->deviceMngBaseInit(m_pMngApp);
}
DeviceMngApp::~DeviceMngApp()
{
    ;
}

bool DeviceMngApp::setupDriver(uint32_t timeout)
{
    driver*        pdriver;
    int            keytemp;
    sDeviceCfg     cfg;
    struct timeval reporttime;

    gettimeofday(&reporttime, NULL);
    for (int i = 0; i < CfgList.size(); i++)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        keytemp  = m_pKey->m_minMsgKey + DEVICEMNG_SHARE_KEY_NUM + DRIVER_SHARE_KEY_NUM * i;
        cfg.id   = CfgList.at(i).id;
        cfg.name = CfgList.at(i).name;

        // pdriver = new driver(cfg.id, cfg.name, keytemp, keytemp + 1, keytemp + 2, keytemp + 4);
        pdriver  = new driver(cfg.id, cfg.name, keytemp, keytemp + 1, keytemp + 2, keytemp + 3, keytemp + 4);
        m_pMngApp->m_driverTable.insert(cfg.id, pdriver);
        if (!m_pMngApp->initGetInfo(cfg.id, timeout))
        {
            gettimeofday(&tv, NULL);
            zprintf1("LibDeviceMng device[%d]=%d InitGetInfo error time: %d!\n" ,i, cfg.id, tv.tv_sec);
            return false;
        }
        if (!pdriver->init())
        {
            gettimeofday(&tv, NULL);
            zprintf1("LibDeviceMng device[%d]= %d Initmem error time: %d!\n",i, cfg.id, tv.tv_sec) ;
            return false;
        }
        if (timeout != 0)
        {
            // if (timeout > timeruseMS(&reporttime))
            //     timeout = timeout - timeruseMS(&reporttime);
            // else
            // {
            //     timeout = 0;
            //     gettimeofday(&tv, NULL);
            //     sysLogQE() << "LibDeviceMng device[" << i << "]=" << cfg.id << " timeout error time:" << tv.tv_sec;
            //     return false;
            // }
        }
        gettimeofday(&tv, NULL);
        zprintf3("LibDeviceMng device[%d]= %d  Init finish time: %d!\n", i, cfg.id, tv.tv_sec);
    }

    return true;
}

int  DeviceMngApp::initApp(void)
{
    struct timeval reporttime;


    loadCfgFile(CFGXML_FILE_PATH);
    loadShareParam(CFGINI_FILE_PATH);
    if (!CheckParamValidity())
    {
        zprintf1("LibDeviceMng param check fail!\n");
        return -2;
    }

    if (!m_pMngApp->initSendMail(m_pKey->m_minMsgKey, m_pKey->m_minMsgKey + 2, m_pKey->m_minMsgKey + 3))
    {
        zprintf1("LibDeviceMng self msg init fail!\n");
        return -3;
    }

    gettimeofday(&reporttime, NULL);
    if (!m_pMngApp->loginRecvMail())
    {
        zprintf1("LibDeviceMng login recv msg fail!\n");
        return -4;
    }

    if (!m_pMngApp->initRecvMail())
    {
        zprintf1("LibDeviceMng recv msg init fail!\n");
        return -5;
    }

    if (!setupDriver(0))
    {
        zprintf1("LibDeviceMng driver init fail!\n");
        return -6;
    }
    m_initOk = true;
    zprintf3("LibDeviceMng driver init InitFinishFlag true!\n");
    return 1;
}

uint32_t DeviceMngApp::getAppid(
    uint8_t DriverID, uint8_t ParentDeviceID, uint8_t ChildDeviceID, uint8_t PointID, uint8_t type)
{
    if ((DriverID >= 64) || (type > 3))
        return 0;

    return (((uint32_t) (type << 30)) | ((uint32_t) (DriverID << 24)) | ((uint32_t) (ParentDeviceID << 16)) |
            ((uint32_t) (ChildDeviceID << 8)) | PointID);
}


void DeviceMngApp::changeAppid(
    uint32_t appid, uint8_t* DriverID, uint8_t* ParentDeviceID, uint8_t* ChildDeviceID, uint8_t* PointID, uint8_t* type)
{
    *DriverID       = (uint8_t) ((appid & 0x3f000000) >> 24);
    *ParentDeviceID = (uint8_t) ((appid & 0x00ff0000) >> 16);
    *ChildDeviceID  = (uint8_t) ((appid & 0x0000ff00) >> 8);
    *PointID        = (uint8_t) (appid & 0x000000ff);
    *type           = (uint8_t) ((appid & 0xc0000000) >> 30);
}
