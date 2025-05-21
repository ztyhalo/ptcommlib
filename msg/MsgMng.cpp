#include "MsgMng.h"
// #include "candata.h"
#include "zprint.h"
#include <signal.h>



MsgMngBase::MsgMngBase()
{
    ;
}

MsgMngBase::~MsgMngBase()
{
    zprintf3("msgmngbase destruct!\n");
}

bool MsgMngBase::sendMsg(sMsgUnit *pdata, uint16_t size)
{
    bool ret;
    ret = m_SendMsg.send_object(pdata, (int)(size+MSG_UNIT_HEAD_LEN), 1);
    return ret;
}



/*************************************************************************************
 * MsgMngDriver 驱动消息管理类
 * *************************************************************************************/


MsgMngDriver* MsgMngDriver::m_pMsgMngDriver = NULL;
MsgMngDriver* MsgMngDriver::GetMsgMngDriver()
{
    if (m_pMsgMngDriver == NULL)
    {
        m_pMsgMngDriver = new MsgMngDriver();
    }
    return m_pMsgMngDriver;
}

MsgMngDriver::MsgMngDriver():dest_id(0),m_pDriver(NULL)
{
    soure_id.app = 0;
}

MsgMngDriver::~MsgMngDriver()
{
    zprintf3("MsgMngDriver destruct!\n");
}

bool MsgMngDriver::Init(int recvkey,int sendkey, PtDriverBase * pdriver)
{
    this->msg_init(recvkey, 1);
    if(!this->create_object())
    {
        zprintf1("MsgMngDriver create recvkey %d error!\n", recvkey);
        return false;
    }
    m_SendMsg.msg_init(sendkey, 1);

    if(!m_SendMsg.get_msg())
    {
        zprintf1("MsgMngDriver get sendkey %d error!\n", sendkey);
        if(!m_SendMsg.create_object())
        {
            zprintf1("MsgMngDriver create sendkey %d error!\n", sendkey);
            return false;
        }
    }
    m_pDriver = pdriver;
    this->start("MsgMngDriRev");

    return true;
}


void MsgMngDriver::msgRecvProcess(sMsgUnit pkt)
{
    // sMsgUnit   pkt;
    uint16_t   pkt_len;
    uint32_t   addr;
    // Can_Data *pdriver = (Can_Data *) arg;
    zprintf1("msgmngdriver msg receiv pkt.dest.driver.id_driver %d driverid %d!\n", pkt.dest.driver.id_driver, m_pDriver->devkey.driverid);
    if((pkt.dest.driver.id_driver != m_pDriver->devkey.driverid) && (pkt.dest.app != BROADCAST_ID))
        return;

    switch(pkt.type)
    {
        case MSG_TYPE_DriverGetInfo:
            // uint8_t midchang ;
            soure_id.driver.id_driver = pkt.dest.driver.id_driver;

            dest_id = pkt.source.app;
            zprintf3("receive dest id is %d\n",dest_id);
            addr = pkt.dest.app;
            pkt.dest.app = pkt.source.app;
            pkt.source.app =addr;
            //            zprintf1("receive driver info id \n");
            zprintf1("msg driver totalincnt %d outcnt %d state%d!\n", m_pDriver->m_paramInfo.TotalInCnt, m_pDriver->m_paramInfo.TotalOutCnt,
                m_pDriver->m_paramInfo.TotalStateCnt);
            memcpy(&pkt.data[0] ,&(m_pDriver->m_paramInfo), sizeof(sDriverInfoType));
            printf("sDriverInfoType %d!\n", sizeof(sDriverInfoType));
            printf("send 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x!\n", pkt.data[0],pkt.data[1],pkt.data[2],pkt.data[3],pkt.data[4],pkt.data[5]);
            pkt_len = sizeof(sDriverInfoType);

            sendMsg(&pkt, pkt_len);
            break;

        case MSG_TYPE_DriverSendHeart:

            addr = pkt.dest.app;
            pkt.dest.app = pkt.source.app;
            pkt.source.app =addr;
            // pSendMsg->SendMsg(&pkt,0);
            sendMsg(&pkt, 0);
            break;

        case MSG_TYPE_AppGetIOParam:
            addr = pkt.dest.app;
            pkt.dest.app = pkt.source.app;
            pkt.source.app =addr;
            // can_inode_info ininfo;
            // pdriver->get_innode_info(pkt.source.driver.id_parent, pkt.source.driver.id_child,
            //                          pkt.source.driver.id_point, ininfo);
            // memcpy(pkt.data, &ininfo,sizeof(ininfo));
            // pSendMsg->SendMsg(&pkt,sizeof(ininfo));
            break;


        default:
            break;
    }
    return;
}

void MsgMngDriver::msgmng_send_msg(sMsgUnit *pdata, uint16_t size)
{
    sendMsg(pdata, size);
}

/*-----------------------------------------------------------------------------------------------------------------------------*/
//MsgMngServer 消息管理服务类
/******************************************************************************************************************************/

MsgMngServer* MsgMngServer::m_pMsgMngServ = NULL;
MsgMngServer* MsgMngServer::GetMsgMngServer()
{
    if (m_pMsgMngServ == NULL)
    {
        m_pMsgMngServ = new MsgMngServer();
    }
    return m_pMsgMngServ;
}

MsgMngServer::MsgMngServer():m_pInitSem(NULL)
{
    ;
}
MsgMngServer::~MsgMngServer()
{
    zprintf3("MsgMngServer destruct!\n");
    QMap< int,  driver * >::iterator it;
    for(it = m_driverTable.begin(); it != m_driverTable.end(); ++it)
    {
        if(it.value() != NULL)
        {
            delete it.value();
            it.value() = NULL;
        }
    }
    m_driverTable.clear();
    DELETE(m_pInitSem);
    m_pMsgMngServ = NULL;
}
int  msgmng_drirecv_back(MsgMngServer * pro, const sMsgUnit pkt)
{
    driver * pdriver;
    if (pkt.dest.app == GET_DEVMNG_ID)
    {
        switch (pkt.type)
        {
        case MSG_TYPE_DriverGetInfo:

            break;

        case MSG_TYPE_DriverSendHeart:

            pro->m_recvDriMsg.lock();
            if (pro->findDriver(pkt.source.driver.id_driver, &pdriver))
            {
                pdriver->m_heartMark = 0;
            }
            pro->m_recvDriMsg.unlock();
            break;
        }
    }
    else if (pkt.dest.app == BROADCAST_ID)
    {
        // sysLogQD() << "+++++++++++++++++++++++++++pkt.dest.app == BROADCAST_ID";
        // mAppTable::iterator item;

        // for (item = AppTable.begin(); item != AppTable.end(); ++item)
        // {
        //     pkt.dest.app = item.key();
        //     item.value()->pmsg->SendMsg(&pkt, pkt_len);
        // }
    }
    else
    {

        // mAppTable::iterator item;
        // for (item = AppTable.begin(); item != AppTable.end(); ++item)
        // {
        //     if (item.value()->isRecv)
        //     {
        //         pkt.dest.app = item.key();
        //         item.value()->pmsg->SendMsg(&pkt, pkt_len);
        //     }
        // }
    }
    return 0;
}

int  msgmng_apprecv_back(MsgMngServer * pro,  sMsgUnit pkt)
{

    // sMsgUnit pkt;
    uint16_t pkt_len;
    app*     papp;
    driver*  pdriver;
    int      key;

    // if (!pAppMsg->ReceiveMsg(&pkt, &pkt_len, RECV_NOWAIT))
    // {
    //     USLEEP(10000);
    //     return;
    // }

    // DeviceMng* pdevice = DeviceMng::GetDeviceMng();
    // if (!pdevice->InitFinishFlag)
    //     return;

    // pthread_mutex_lock(&RevTaskMutex);
    switch (pkt.type)
    {
    case MSG_TYPE_AppLogIn:
        zprintf3("DeviceMng MSG_TYPE_AppLogIn enter!\n");
        if (pkt.source.app == GET_DEVMNG_ID)
        {
            zprintf3("DeviceMng pkt.source.app == GET_DEVMNG_ID!\n");
            return 0;
        }
        if (pro->isAppMapExist(pkt.source.app))
        {
            pkt.dest.app   = pkt.source.app;
            pkt.source.app = BROADCAST_ID;
            pkt.data[0]    = MSG_ERROR_LogIn_Exist;
            zprintf1("DeviceMng app login error:exist!");
            if(pro->findApp(pkt.dest.app, &papp))
            {
                // papp->pmsg->SendMsg(&pkt, ABNORMAL_MSG_LEN);
            }
            break;
        }
        if (pro->m_appTable.size() < APP_LOGIN_MAX)
        {
            // DeviceMng* pdevice = DeviceMng::GetDeviceMng();
            key                = pro->operateAppMsgKey(KEY_ADD, 0);
            if (key == 0)
            {
                pkt.dest.app   = pkt.source.app;
                pkt.source.app = BROADCAST_ID;
                pkt.data[0]    = MSG_ERROR_LogIn_NOAPPID;
                zprintf1("DeviceMng app login error:no id!\n");
                // pAppTotalMsg->SendMsg(&pkt, ABNORMAL_MSG_LEN);
                break;
            }

            bool isRecv = (bool) pkt.data[0];
            zprintf3("isRecv: %d app msg key %d!\n", isRecv, key);

            app* apphandle = new app(pkt.source.app, key, isRecv);
            if(apphandle == NULL)
            {
                zprintf1("msg mng server app new error!\n");
                return -1;
            }
            // 先清除之前废除的app 感觉不对 这样是不是不能支持多app
            for (auto iter = pro->m_appTable.begin(); iter != pro->m_appTable.end();)
            {
                if (!pro->isProcessExists(iter.key()))
                {
                    iter = pro->m_appTable.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }

            pro->m_appTable.insert(pkt.source.app, apphandle);

            for (auto iter : pro->m_appTable.keys())
            {
                zprintf3("AppTable key: %d!\n" ,iter);
            }

            pkt.dest.app   = pkt.source.app;
            pkt.source.app = BROADCAST_ID;
            pkt.data[0]    = MSG_ERROR_NoError;
            pkt.data[1]    = (uint8_t) ((key & 0xff000000) >> 24);
            pkt.data[2]    = (uint8_t) ((key & 0x00ff0000) >> 16);
            pkt.data[3]    = (uint8_t) ((key & 0x0000ff00) >> 8);
            pkt.data[4]    = (uint8_t) (key & 0x000000ff);
            pkt.data[5]    = (uint8_t) ((GET_DEVMNG_ID & 0xff000000) >> 24);
            pkt.data[6]    = (uint8_t) ((GET_DEVMNG_ID & 0x00ff0000) >> 16);
            pkt.data[7]    = (uint8_t) ((GET_DEVMNG_ID & 0x0000ff00) >> 8);
            pkt.data[8]    = (uint8_t) (GET_DEVMNG_ID & 0x000000ff);
            pro->m_sendToAppMsg.sendMsg(&pkt, NORMAL_MSG_LEN);
            zprintf3("DeviceMng app login sucess!\n");
            break;
        }
        else
            zprintf1("DeviceMng app login appid full!\n");
        break;

    case MSG_TYPE_AppLogOut:
        zprintf3("DeviceMng app logout enter!\n");
        if (pkt.source.app == GET_DEVMNG_ID)
            break;
        if (pro->findApp(pkt.source.app, &papp))
        {
            // DeviceMng* pdevice = DeviceMng::GetDeviceMng();
            // pdevice->OperateAppMsgKey(KEY_SUB, papp->pmsg->GetMsgKey());
            delete papp;
            pro->deleteApp(pkt.source.app);
            zprintf3("DeviceMng app logout sucess!\n");
        }
        break;

    default:
        zprintf3("DeviceMng default enter!\n");
        // DeviceMng* pdevice = DeviceMng::GetDeviceMng();
        // if (!pro->findApp(pkt.source.app, &papp))
        //     break;

        // if (!pdevice->FindDriver(pkt.dest.driver.id_driver, &pdriver))
        // {
        //     pkt.dest.app   = pkt.source.app;
        //     pkt.source.app = GET_DEVMNG_ID;
        //     pkt.data[0]    = MSG_ERROR_Driver_NotExist;
        //     papp->pmsg->SendMsg(&pkt, ABNORMAL_MSG_LEN);
        // }
        // else
        // {
        //     pdriver->pmsg->SendMsg(&pkt, pkt_len);
        // }
        // break;
    }
    return 0;
}

/***********************************
 * appkey: receive app msg key 30
 * driverkey: driver msg key 31
 * totalkey: send to app msg key 32
 * initsemkey:               33
 * reskey:                   29
 * ****************************************/

bool MsgMngServer::init(int appkey, int driverkey, int totalkey, int initsemkey, int reskey)
{

    m_recvDriMsg.msg_init(driverkey, 1);
    if(!m_recvDriMsg.create_object())
    {
        zprintf1("MsgMngServer m_recvDriMsg create driverkey %d error!\n", driverkey);
        return false;
    }

    m_recvAppMsg.msg_init(appkey, 1);
    if(!m_recvAppMsg.create_object())
    {
        zprintf1("MsgMngServer m_recvAppMsg create appkey %d error!\n", appkey);
        return false;
    }

    m_sendToAppMsg.msg_init(totalkey, 1);
    if(!m_sendToAppMsg.create_object())
    {
        zprintf1("MsgMngServer m_sendToAppMsg create totalkey %d error!\n", totalkey);
        return false;
    }


    m_pInitSem = new LSystemSem();
    if (m_pInitSem->createSem(initsemkey, 1) != 0)
    {
        zprintf1("MsgMngServer pinitsem create_sem initsemkey: %d!\n",initsemkey) ;
        return false;
    }
    zprintf3("MsgMngServer msgmng init finish!\n");
    // m_recvDriMsg.z_pthread_init(msgmng_drirecv_back, this, "msgservdrirecv");
    // m_recvAppMsg.z_pthread_init(msgmng_apprecv_back, this, "msgservapprecv");



    return true;
}

int MsgMngServer::operateAppMsgKey(eOperateKeyType mode, int key)
{
    int remain = m_sysMaxMsgKey - m_appStMsgKey + 1 - m_appUseKeyList.size();
    if (mode == KEY_ADD)
    {
        if (remain <= 0)
            return 0;
        for (int i = m_appStMsgKey; i < m_sysMaxMsgKey; i++)
        {
            if (!m_appUseKeyList.contains(i))
            {
                m_appUseKeyList.append(i);
                return i;
            }
        }
    }
    else if (mode == KEY_SUB)
    {
        if (m_appUseKeyList.size() == 0)
            return 0;
        for (int i = 0; i < m_appUseKeyList.size(); i++)
        {
            if (m_appUseKeyList.at(i) == key)
            {
                m_appUseKeyList.removeAt(i);
                return 1;
            }
        }
    }
    return 0;
}

bool MsgMngServer::isProcessExists(qint64 pid)
{
    if (kill(pid, 0) == -1)
    {
        if (errno == ESRCH)
        {
            return false;
        }
    }
    return true;
}

void MsgMngServer::startRecvDrivMsgProcess(void)
{
     m_recvDriMsg.z_pthread_init(msgmng_drirecv_back, this, "msgservdrirecv");
}

void MsgMngServer::startRecvAppMsgProcess(void)
{
    m_recvAppMsg.z_pthread_init(msgmng_apprecv_back, this, "msgservapprecv");
}

bool MsgMngServer::findApp(uint32_t id, app** ppapp)
{
    mAppTable::iterator item;

    item = m_appTable.find(id);
    if ((item != m_appTable.end()) && (item.key() == id))
    {
        *ppapp = item.value();
        return true;
    }
    *ppapp = (app*) 0;
    return false;
}

bool MsgMngServer::deleteApp(uint32_t id)
{
    mAppTable::iterator item;

    item = m_appTable.find(id);
    if ((item != m_appTable.end()) && (item.key() == id))
    {
        m_appTable.erase(item);
        return true;
    }
    return false;
}

bool MsgMngServer::findDriver(uint8_t id, driver** ppdriver)
{
    mDriverTable::iterator item;

    item = m_driverTable.find(id);
    if ((item != m_driverTable.end()) && (item.key() == id))
    {
        *ppdriver = item.value();
        return true;
    }
    *ppdriver = (driver*) 0;
    return false;
}

bool MsgMngServer::isAppMapExist(uint32_t id)
{
    mAppTable::iterator item;

    item = m_appTable.find(id);
    if ((item != m_appTable.end()) && (item.key() == id))
    {
        return true;
    }
    return false;
}


bool MsgMngServer::waitDriverInfo(driver *pdirv)
{
    sMsgUnit pkt;
    int len;
    while(m_recvDriMsg.receive_object(pkt, 0, len))
    {
        zprintf3("msg mng server dir id %d my dirv id %d len %d!\n", pkt.source.driver.id_driver, pdirv->m_driverId, len);
        if (pkt.source.driver.id_driver == pdirv->m_driverId)
        {
            if(pkt.type == MSG_TYPE_DriverGetInfo && (len == (MSG_UNIT_HEAD_LEN +sizeof(sDriverInfoType))))
            {
                memcpy(&pdirv->m_driverInfo, &pkt.data[0], sizeof(sDriverInfoType));
                zprintf3("msg wait totalincnt %d outcnt %d state%d!\n", pdirv->m_driverInfo.TotalInCnt, pdirv->m_driverInfo.TotalOutCnt,
                    pdirv->m_driverInfo.TotalStateCnt);
                return true;
            }
            else
                zprintf3("pdirv%d recv msg type %d len %d!\n", pdirv->m_driverId, pkt.type, len);

        }

    }
    zprintf1("wait DriverInfo error!\n");
    return false;

}
/*-----------------------------------------------------------------------------------------------------------------------------*/
//MsgMngApp类
MsgMngApp::MsgMngApp()
{
    m_deviceMngId = BROADCAST_ID;
}

MsgMngApp::~MsgMngApp()
{
    ;
}

/***************************************
 * senkey: app send msg to server 30
 * totalkey:app receive msg from server 32
 * totalmutexkey:
 *
 * ***************************************************/

bool MsgMngApp::initSendMail(int sendkey, int totalkey, int totalmutexkey)
{

    m_sendToServMsg.msg_init(sendkey, 1);
    if(!m_sendToServMsg.get_msg())
    {
        zprintf1("MsgMngApp m_sendToServMsg get msg sendkey %d error!\n", sendkey);
        return false;
    }

    m_recvServMsg.msg_init(totalkey, 1);
    if(!m_recvServMsg.get_msg())
    {
        zprintf1("MsgMngApp m_recvServMsg get msg totalkey %d error!\n", totalkey);
        return false;
    }

    m_pInitSem = new LSystemSem();
    if (m_pInitSem->readSemKey(totalmutexkey) != 0)
    {
        zprintf1("MsgMngApp pinitsem read sem key totalmutexkey: %d!\n",totalmutexkey) ;
        return false;
    }

    // pthread_create(&TotalMsg_id, NULL, TotalMsg_task, NULL);
    // freshTimer->start(MSG_CHECK_TIME);
    return true;
}


bool MsgMngApp::waitServerInfo(sMsgUnit & pkt)
{
    // sMsgUnit pkt;
    int len;
    while(m_recvServMsg.receive_object(pkt, 0, len))
    {
        zprintf3("MsgMngApp app id %d  id %d len %d!\n", pkt.dest.app, GET_APP_ID, len);
        if (pkt.dest.app == GET_APP_ID && pkt.dest.app == BROADCAST_ID)
        {
            if(pkt.type == MSG_TYPE_AppLogIn && (len == (MSG_UNIT_HEAD_LEN + LOGIN_ACK_MSG_LEN)) && (pkt.data[0] == MSG_ERROR_NoError))
            {
                m_deviceMngId = ((int) pkt.data[5] << 24) | ((int) pkt.data[6] << 16) | ((int) pkt.data[7] << 8) |
                              (int) pkt.data[8];
                m_loginKeyId = ((int) pkt.data[1] << 24) | ((int) pkt.data[2] << 16) | ((int) pkt.data[3] << 8) |
                             (int) pkt.data[4];
                zprintf3("app login ok m_deviceMngId 0x%x m_loginKeyId 0x%x!\n", m_deviceMngId, m_loginKeyId);
                m_loginOkFlag = 1;
                return true;
            }
            else
                zprintf3("recv msg type %d len %d!\n", pkt.type, len);

        }

    }
    zprintf1("wait MsgMng server info error!\n");
    return false;

}

bool MsgMngApp::loginRecvMail(void)
{
    sMsgUnit pkt;
    // sWaitMsg msg;

    zprintf3("MsgMngApp %d LoginRecvMail pend!\n", GET_APP_ID);
    m_loginOkFlag = 0;
    m_pInitSem->acquire();      //app注册互斥

    // sem_post(&TotalProcessSem);
    pkt.dest.app   = m_deviceMngId;
    pkt.source.app = GET_APP_ID;
    pkt.type       = MSG_TYPE_AppLogIn;
    pkt.data[0]    = m_isRecv;

    zprintf3("LibDeviceMng IsRecv: %d" , m_isRecv);

    m_loginKeyId = 0;

    if (!m_sendToServMsg.sendMsg(&pkt, LOGIN_MSG_LEN))
    {
        zprintf1("MsgMngApp LoginRecvMail post app%d SendMsg fail!", GET_APP_ID);
        m_pInitSem->release();
        return false;
    }

    if(!waitServerInfo(pkt))
    {
        zprintf1("app login wait fail!\n");
        m_pInitSem->release();
        return false;
    }

    if ((m_loginKeyId == 0) || (m_loginOkFlag == 0))
    {
        zprintf1("MsgMngApp LoginRecvMail login key fail!\n");
        m_pInitSem->release();
        return false;
    }

    qDebug() << "LoginRecvMail post!" << GET_APP_ID << "success!";
    m_pInitSem->release();
    return true;
}

int  msgmngapp_apprecv_back(MsgMngApp * pro,  sMsgUnit pkt)
{

    // sMsgUnit    pkt;
    uint16_t    pkt_len;
    driver*     pdriver;
    // ackfunctype func;
    // sNotifyMsg  notifypkt;

    // if (!pRecvMsg->ReceiveMsg(&pkt, &pkt_len, RECV_WAIT))
    // {
    //     sysLogQE() << "LibDeviceMng Test ReceiveMsg fail!";
    //     USLEEP(10000);
    //     return;
    // }

    // DeviceMng* pdevice = DeviceMng::GetDeviceMng();
    if ((pkt.dest.app != GET_APP_ID) && (pkt.dest.app != BROADCAST_ID))
    {
        zprintf1("MsgMngApp Test pkt.dest.app fail,pkt.dest.app: %d.\n" ,pkt.dest.app);
        return -1;
    }

    switch (pkt.type)
    {
    case MSG_TYPE_DriverGetInfo:
    {
        // sysLogQE() << "LibDeviceMng Test MSG_TYPE_DriverGetInfo";
        // bool ret_checkwait = CheckWaitMsg(pkt.source, pkt.type);
        // //            qDebug()<<"Test MSG_TYPE_DriverGetInfo ret_checkwait:"<<ret_checkwait;
        // if (!ret_checkwait)
        //     break;

        // if (!pdevice->InitFinishFlag)
        // {
        //     //                qDebug()<<"Test MSG_TYPE_DriverGetInfo --1"<<" --
        //     //                "<<QDateTime::currentMSecsSinceEpoch();
        //     if (pdevice->FindDriver(pkt.source.driver.id_driver, &pdriver))
        //     {
        //         pdriver->DriverInfo.TotalInCnt    = (uint16_t) (((uint16_t) pkt.data[0] << 8) | pkt.data[1]);
        //         pdriver->DriverInfo.TotalOutCnt   = (uint16_t) (((uint16_t) pkt.data[2] << 8) | pkt.data[3]);
        //         pdriver->DriverInfo.TotalStateCnt = (uint16_t) (((uint16_t) pkt.data[4] << 8) | pkt.data[5]);
        //         //                    qDebug()<<" pdriver->DriverInfo.TotalStateCnt =
        //         //                    "<(uint16_t)(((uint16_t)pkt.data[4]<<8)|pkt.data[5]);
        //         // memcpy(&pdriver->DriverInfo,&pkt.data[0],sizeof(sDriverInfoType));
        //         /*bool ret_act = */ AckWaitMsg(pkt.source, pkt.type, 0);
        //         //                    qDebug()<<"Test MSG_TYPE_DriverGetInfo --2 ret_act:"<<ret_act<<" --
        //         //                    "<<QDateTime::currentMSecsSinceEpoch();
        //     }
        // }
        // else
        // {
        //     //              qDebug()<<"Test MSG_TYPE_DriverGetInfo --3";
        //     func = GetWaitFunc(pkt.source, pkt.type);
        //     //               qDebug()<<"Test MSG_TYPE_DriverGetInfo --4";
        //     if (func != 0)
        //         func((void*) (&pkt.data[0]), pkt_len);
        //     //              qDebug()<<"Test MSG_TYPE_DriverGetInfo --5";
        // }
    }
    break;

    case MSG_TYPE_AppReportDriverComNormal:
        // sysLogQE() << "LibDeviceMng Test MSG_TYPE_AppReportDriverComNormal";
        // if ((pkt.source.app == DeviceMngId) && (pkt_len == 1))
        // {
        //     if (pdevice->FindDriver(pkt.data[0], &pdriver))
        //     {
        //         pdriver->ComState = COMSTATE_NORMAL;
        //     }
        // }
        break;

    case MSG_TYPE_AppGetIOParam:
    case MSG_TYPE_AppSetIOParam:
        // sysLogQE() << "LibDeviceMng Test MSG_TYPE_AppGetIOParam";
        // if (!CheckWaitMsg(pkt.source, pkt.type))
        //     break;
        // func = GetWaitFunc(pkt.source, pkt.type);
        // if (func != 0)
        //     func((void*) (&pkt.data[0]), pkt_len);
        break;

    case MSG_TYPE_AppReportDriverComAbnormal:
        // sysLogQE() << "LibDeviceMng Test MSG_TYPE_AppReportDriverComAbnormal";
        // if ((pkt.source.app == DeviceMngId) && (pkt_len == 1))
        // {
        //     if (pdevice->FindDriver(pkt.data[0], &pdriver))
        //     {
        //         pdriver->ComState = COMSTATE_ABNORMAL;
        //     }
        // }

    default:
        // if (NotifyList.size() < WAIT_MSG_MAX)
        // {
        //     pthread_mutex_lock(&NotifyListMutex);
        //     memcpy(&notifypkt.MsgData, &pkt, sizeof(sMsgUnit));
        //     notifypkt.MsgLen = pkt_len;
        //     NotifyList.append(notifypkt);
        //     pthread_mutex_unlock(&NotifyListMutex);
        //     sem_post(&NotifySem);

        //     // sysLogQD() << "------------------------------------NotifyList.append";
        // }
        break;
    }
    return 0;
}
bool MsgMngApp::initRecvMail(void)
{
    m_appRecvMsg.msg_init(m_loginKeyId, 1);

    if(!m_appRecvMsg.get_msg())
    {
        zprintf1("MsgMngApp get sendkey %d error!\n", m_loginKeyId);
        // if(!m_appRecvMsg.create_object())
        // {
        //     zprintf1("MsgMngApp create appRecvMsg %d error!\n", m_loginKeyId);
        //     return false;
        // }
        return false;
    }
    m_appRecvMsg.z_pthread_init(msgmngapp_apprecv_back, this, "msgAppRecv");
}
