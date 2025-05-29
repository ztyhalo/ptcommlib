#include "appmsgmng.h"
#include <mutex>

/*-----------------------------------------------------------------------------------------------------------------------------*/
//MsgMngApp类
/*-----------------------------------------------------------------------------------------------------------------------------*/

AppMsgMng* AppMsgMng::m_pMsgMngApp = NULL;
AppMsgMng* AppMsgMng::getMsgMngApp()
{
    if (m_pMsgMngApp == NULL)
    {
        m_pMsgMngApp = new AppMsgMng();
    }
    return m_pMsgMngApp;
}

AppMsgMng::AppMsgMng():m_pInitSem(NULL),m_loginOkFlag(0),m_deviceMngId(BROADCAST_ID),m_isRecv(0),m_loginKeyId(0)
{

    sem_init(&m_notifySem, 0, 0);
}

AppMsgMng::~AppMsgMng()
{
    zprintf3("MsgMngServer destruct!\n");
    lock_guard<MUTEX_CLASS> lock(m_mapMutex);
    QMap< int,  driver * >::iterator it;
    for(it = m_drivMap.begin(); it != m_drivMap.end(); ++it)
    {
        if(it.value() != NULL)
        {
            delete it.value();
            it.value() = NULL;
        }
    }
    m_drivMap.clear();

    mAppMap::iterator appit;
    for(appit = m_appMap.begin(); appit != m_appMap.end(); ++appit)
    {
        if(appit.value() != NULL)
        {
            delete appit.value();
            appit.value() = NULL;
        }
    }
    m_appMap.clear();

    DELETE(m_pInitSem);
    m_pMsgMngApp = NULL;
}

/***********************************************************************************************
 * senkey: app send msg to server       30
 * totalkey:app receive msg from server 32
 * totalmutexkey:  互斥信号量             33
 *
 * **********************************************************************************************/

bool AppMsgMng::initSendMail(int sendkey, int totalkey, int totalmutexkey)
{

    m_sendToServMsg.msg_init(sendkey, 1);
    if(!m_sendToServMsg.get_msg())
    {
        zprintf1("AppMsgMng m_sendToServMsg get msg sendkey %d error!\n", sendkey);
        return false;
    }

    m_recvServMsg.msg_init(totalkey, 1);
    if(!m_recvServMsg.get_msg())
    {
        zprintf1("AppMsgMng m_recvServMsg get msg totalkey %d error!\n", totalkey);
        return false;
    }

    m_pInitSem = new LSystemSem();
    if (m_pInitSem->readSemKey(totalmutexkey) != 0)
    {
        zprintf1("MsgMngApp pinitsem read sem key totalmutexkey: %d!\n",totalmutexkey) ;
        return false;
    }

    return true;
}


bool AppMsgMng::waitServerInfo(sMsgUnit & pkt)
{

    int len;
    while(m_recvServMsg.receive_object(pkt, 0, len))
    {
        zprintf3("MsgMngApp app id %d  id %d len %d type %d data %d!\n", pkt.dest.app, GET_APP_ID, len, pkt.type, pkt.data[0]);
        if (pkt.dest.app == GET_APP_ID)// && pkt.dest.app == BROADCAST_ID)
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



bool AppMsgMng::waitDriverInfo(sMsgUnit & pkt)
{
    int         len;
    driver*     pdriver;
    while(m_appRecvMsg.receive_object(pkt, 0, len))
    {
        zprintf3("MsgMngApp driver app id %d  id %d len %d!\n", pkt.dest.app, GET_APP_ID, len);
        if (pkt.dest.app == GET_APP_ID) // && pkt.dest.app == BROADCAST_ID)
        {
            if(pkt.type == MSG_TYPE_DriverGetInfo)
            {
                if (findDriver(pkt.source.driver.id_driver, &pdriver))
                {
                    // pdriver->m_driverInfo.TotalInCnt    = (uint16_t) (((uint16_t) pkt.data[0] << 8) | pkt.data[1]);
                    // pdriver->m_driverInfo.TotalOutCnt   = (uint16_t) (((uint16_t) pkt.data[2] << 8) | pkt.data[3]);
                    // pdriver->m_driverInfo.TotalStateCnt = (uint16_t) (((uint16_t) pkt.data[4] << 8) | pkt.data[5]);
                    memcpy(&pdriver->m_driverInfo, &pkt.data[0], sizeof(sDriverInfoType));
                    zprintf3("appmsgmng diriver %d totalincunt %d out %d state %d!\n", pkt.source.driver.id_driver,
                        pdriver->m_driverInfo.TotalInCnt, pdriver->m_driverInfo.TotalOutCnt, pdriver->m_driverInfo.TotalStateCnt);
                }
                return true;
            }
            else
                zprintf3("recv msg type %d len %d!\n", pkt.type, len);

        }

    }
    zprintf1("wait MsgMng server info error!\n");
    return false;

}

bool AppMsgMng::loginRecvMail(void)
{
    sMsgUnit pkt;

    zprintf3("MsgMngApp %d LoginRecvMail pend!\n", GET_APP_ID);
    m_loginOkFlag = 0;
    m_pInitSem->acquire();      //app注册互斥

    pkt.dest.app   = m_deviceMngId;
    pkt.source.app = GET_APP_ID;
    pkt.type       = MSG_TYPE_AppLogIn;
    pkt.data[0]    = m_isRecv;

    zprintf3("LibDeviceMng IsRecv: %d!\n" , m_isRecv);

    m_loginKeyId = 0;

    if(!m_sendToServMsg.sendMsg(&pkt, LOGIN_MSG_LEN))
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

int  appRecvMsgBack(AppMsgMng * pro,  sMsgUnit pkt, int len)
{

    // driver*     pdriver;
    // ackfunctype func;
    sNotifyMsg  notifypkt;

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
    case MSG_TYPE_DriverGetInfo:      //目前不用
    {
        ;
    }
    break;

    case MSG_TYPE_AppReportDriverComNormal:

        break;

    case MSG_TYPE_AppGetIOParam:
    case MSG_TYPE_AppSetIOParam:

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
        if (pro->m_notifyList.size() < WAIT_MSG_MAX)
        {
            pro->m_notifyMutex.lock();
            memcpy(&notifypkt.MsgData, &pkt, sizeof(sMsgUnit));
            notifypkt.MsgLen = len;
            pro->m_notifyList.append(notifypkt);
            pro->m_notifyMutex.unlock();
            sem_post(&pro->m_notifySem);

        }
        break;
    }
    return 0;
}

bool AppMsgMng::initRecvMail(void)
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
    // m_appRecvMsg.msgPthreadInit(msgmngapp_apprecv_back, this, "msgAppRecv");
    return true;
}


bool AppMsgMng::initGetInfo(int driver_id, uint32_t timeout_ms)
{
    sMsgUnit pkt;

    memset(&pkt, 0, sizeof(sMsgUnit));
    pkt.source.app            = GET_APP_ID;
    pkt.dest.driver.id_driver = driver_id;
    pkt.type                  = MSG_TYPE_DriverGetInfo;

    if (!m_sendToServMsg.sendMsg(&pkt, 0))
    {
        return false;
    }

    return waitDriverInfo(pkt);

}

bool AppMsgMng::wait_msg(sMsgUnit* recvmsg, uint16_t* msglen, eWaitMsgType mode)
{
    int                   ret;


    if (mode == WAIT_MSG_BLOCK)
    {
        ret = sem_wait(&m_notifySem);
    }
    else
    {
        ret = sem_trywait(&m_notifySem);
    }

    if (ret < 0)
    {
        zprintf1("LibDeviceMng sem_trywait  ret <0!\n");
        return false;
    }

    if (m_notifyList.size() <= 0)
    {
        zprintf1("LibDeviceMng  m_notifyList.size() = %d!\n", m_notifyList.size());
        return false;
    }

    if(recvmsg != NULL && msglen != NULL)
    {

        m_notifyMutex.lock();
        lNotifyList::iterator item = m_notifyList.begin();

        memcpy(recvmsg, &((*item).MsgData), sizeof(sMsgUnit));
        *msglen = (*item).MsgLen;
        m_notifyList.erase(item);
        m_notifyMutex.unlock();
        return true;
    }
    else
    {
        zprintf1("recvmsg NULL or msglen NULL error!\n");
        return false;
    }

}

bool AppMsgMng::sendMail(sMsgUnit& pkt, uint16_t pkt_len, ackfunctype func, uint32_t timeout)
{
    sWaitMsg msg;

    msg.m_waitId     = pkt.dest;
    msg.m_type       = pkt.type;
    msg.m_pack       = 0;
    msg.m_timeoutMs = (timeout == 0) ? MSG_TIMEOUT_VALUE : timeout;
    msg.m_ackFunc    = func;
    if(insertWaitMsg(msg))
    {
        if(m_sendToServMsg.sendMsg(&pkt, pkt_len))
            return true;
        else
        {
            zprintf1("msgmng app send mail error!\n");
            return false;
        }
    }
    else
        return false;
}

bool AppMsgMng::msgSendProcess(Type_MsgAddr& addr, uint16_t msgtype, ackfunctype func, uint8_t* pdata, uint16_t len)
{
    sMsgUnit pkt;
    bool     ret = true;

    memset(&pkt, 0, sizeof(sMsgUnit));
    switch (msgtype)
    {
    case MSG_TYPE_AppLogOut:
        pkt.source.app = GET_APP_ID;
        pkt.dest.app   = m_deviceMngId;
        pkt.type       = MSG_TYPE_AppLogOut;
        sendMail(pkt, 0, func, 0);
        // cancel = true;

        break;

    case MSG_TYPE_AppGetIOParam:
        pkt.source.app = GET_APP_ID;
        pkt.dest.app   = addr.app;
        pkt.type       = MSG_TYPE_AppGetIOParam;
        sendMail(pkt, 0, func, 0);
        break;

    case MSG_TYPE_AppSetIOParam:
        pkt.source.app = GET_APP_ID;
        pkt.dest.app   = addr.app;
        pkt.type       = MSG_TYPE_AppSetIOParam;
        memcpy(pkt.data, pdata, len);
        sendMail(pkt, len, func, 0);
        break;

    case MSG_TYPE_DriverGetInfo:
        pkt.source.app = GET_APP_ID;
        pkt.dest.app   = addr.app;
        pkt.type       = MSG_TYPE_DriverGetInfo;
        sendMail(pkt, 0, func, 0);
        break;

    default:
        break;
    }
    return ret;
}
int AppMsgMng::appStartRecvMsg(void)
{
    return m_appRecvMsg.msgPthreadInit(appRecvMsgBack, this, "AppRecvMsg");
}
