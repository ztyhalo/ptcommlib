#include "MsgMng.h"
#include "zprint.h"
#include <signal.h>
#include <mutex>


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
    DELETE(m_pInitSem);
    m_pMsgMngServ = NULL;
}

//服务端接收驱动msg回掉函数  key值：31
int  serverRecvDrivMsgBack(MsgMngServer * pro, sMsgUnit pkt, int len)
{
    driver * pdriver;

    lock_guard<MUTEX_CLASS> lock(pro->m_mapMutex);
    if (pkt.dest.app == GET_DEVMNG_ID)
    {
        switch (pkt.type)
        {
            case MSG_TYPE_DriverGetInfo:
                // if (!CheckWaitMsg(pkt.source, pkt.type))
                //     break;

                // if (pro->findDriver(pkt.source.driver.id_driver, &pdriver))
                // {
                //     pdriver->m_driverInfo.TotalInCnt    = (uint16_t) (((uint16_t) pkt.data[0] << 8) | pkt.data[1]);
                //     pdriver->m_driverInfo.TotalOutCnt   = (uint16_t) (((uint16_t) pkt.data[2] << 8) | pkt.data[3]);
                //     pdriver->m_driverInfo.TotalStateCnt = (uint16_t) (((uint16_t) pkt.data[4] << 8) | pkt.data[5]);
                //     // AckWaitMsg(pkt.source, pkt.type);
                // }
                break;

        case MSG_TYPE_DriverSendHeart:

            // pro->m_recvDriMsg.lock();
            if (pro->findDriver(pkt.source.driver.id_driver, &pdriver))
            {
                pdriver->m_heartMark = 0;
            }
            // pro->m_recvDriMsg.unlock();
            break;
        }
    }
    else if (pkt.dest.app == BROADCAST_ID)
    {
        mAppMap::iterator item;

        for (item = pro->m_appMap.begin(); item != pro->m_appMap.end(); ++item)
        {
            pkt.dest.app = item.key();
            item.value()->m_pMsg->sendMsg(&pkt, len);
        }
    }
    else
    {

        mAppMap::iterator item;
        for (item = pro->m_appMap.begin(); item != pro->m_appMap.end(); ++item)
        {
            if (item.value()->m_isRecv)
            {
                pkt.dest.app = item.key();
                item.value()->m_pMsg->sendMsg(&pkt, len);
            }
        }
    }
    return 0;
}
//服务端接收app msg回调函数 key值：30
int  servRecvAppBack(MsgMngServer * pro,  sMsgUnit pkt, int len)
{
    app*     papp;
    driver * pdriver;
    int      key;

    switch (pkt.type)
    {
        case MSG_TYPE_AppLogIn:
        zprintf3("DeviceMng MSG_TYPE_AppLogIn enter!\n");
        if (pkt.source.app != GET_DEVMNG_ID)
        {
            lock_guard<MUTEX_CLASS> lock(pro->m_mapMutex);
            if (pro->isAppMapExist(pkt.source.app))
            {
                pkt.dest.app   = pkt.source.app;
                pkt.source.app = BROADCAST_ID;
                pkt.data[0]    = MSG_ERROR_LogIn_Exist;
                zprintf1("DeviceMng app login error:exist!");

                if(pro->findApp(pkt.dest.app, &papp))
                {
                    papp->m_pMsg->sendMsg(&pkt, ABNORMAL_MSG_LEN);
                }
                break;
            }
            if (pro->m_appMap.size() < APP_LOGIN_MAX)
            {
                key = pro->operateAppMsgKey(KEY_ADD, 0);
                if (key == 0)
                {
                    pkt.dest.app   = pkt.source.app;
                    pkt.source.app = BROADCAST_ID;
                    pkt.data[0]    = MSG_ERROR_LogIn_NOAPPID;
                    zprintf1("DeviceMng app login error:no id!\n");
                    pro->m_sendToAppMsg.sendMsg(&pkt, ABNORMAL_MSG_LEN);
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
                // for (auto iter = pro->m_appMap.begin(); iter != pro->m_appMap.end();)
                // {
                //     if (!pro->isProcessExists(iter.key()))
                //     {
                //         iter = pro->m_appMap.erase(iter);
                //     }
                //     else
                //     {
                //         ++iter;
                //     }
                // }

                pro->m_appMap.insert(pkt.source.app, apphandle);

                for (auto iter : pro->m_appMap.keys())
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
        }
        else
        {
            zprintf3("DeviceMng pkt.source.app == GET_DEVMNG_ID!\n");
            return 0;
        }
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
        lock_guard<MUTEX_CLASS> lock(pro->m_mapMutex);
        if (!pro->findApp(pkt.source.app, &papp))
            break;

        if (!pro->findDriver(pkt.dest.driver.id_driver, &pdriver))
        {
            pkt.dest.app   = pkt.source.app;
            pkt.source.app = GET_DEVMNG_ID;
            pkt.data[0]    = MSG_ERROR_Driver_NotExist;
            papp->m_pMsg->sendMsg(&pkt, ABNORMAL_MSG_LEN);
        }
        else
        {
            pdriver->m_pSendmsg->sendMsg(&pkt, len);
        }
        break;
    }
    return 0;
}

/*********************************************************************************************************************
 * appkey: receive app msg key 30
 * driverkey: driver msg key 31
 * totalkey: send to app msg key 32
 * initsemkey:               33
 * reskey:                   29
 *
 *    app端                              app发送key 30                                      app map表 接收    app接收该key初始化创建appmap
 *                                          |                                                   ↑                  ↑
 *                                          |                                                   |                  |
 *                                          ↓                                                   |                  |
 *    服务端       driver map发送 ←------- 接收app key 30           接收driver key 31 --------→app map表 发送   发送到app的通用key 32
 *                       |                                              ↑
 *                       |                                              |
 *                       ↓                                              |
 *                driver map 接收Key --------------------------→  驱动发送key 31
 *
 * **************************************************************************************************************************/

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
    m_recvDriMsg.set_z_callback(serverRecvDrivMsgBack, this);
    m_recvDriMsg.start("msgservdrirecv");
}

void MsgMngServer::startRecvAppMsgProcess(void)
{
    m_recvAppMsg.set_z_callback(servRecvAppBack, this);
    m_recvAppMsg.start("msgServAppRecv");
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

void MsgMngServer::sendHeartToDriver(void)
{
    mDrivMap::iterator item;

    lock_guard<MUTEX_CLASS> lock(m_mapMutex);

    for (item = m_drivMap.begin(); item != m_drivMap.end(); ++item)
    {
        item.value()->msgSendHeart();
    }

}


