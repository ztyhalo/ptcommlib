#include "MsgMng.h"
// #include "candata.h"
#include "zprint.h"




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
    uint8_t temp[sizeof(sMsgUnit)];

    memset(temp,0,sizeof(temp));
    temp[0] =pdata->source.driver.id_driver;
    temp[1] =pdata->source.driver.id_parent;
    temp[2] =pdata->source.driver.id_child;
    temp[3] =pdata->source.driver.id_point;
    temp[4] =pdata->dest.driver.id_driver;
    temp[5] =pdata->dest.driver.id_parent;
    temp[6] =pdata->dest.driver.id_child;
    temp[7] =pdata->dest.driver.id_point;
    temp[8] = (uint8_t)((pdata->type&0xff00)>>8);
    temp[9] = (uint8_t)(pdata->type&0x00ff);
    memcpy(&temp[10], pdata->data, size);

    ret = m_SendMsg.send_object(temp, (int)(size+MSG_UNIT_HEAD_LEN), 1);
    return ret;
}

bool MsgMngBase::receiveMsg(sMsgUnit *pdata,uint16_t *psize,int mode)
{
    bool ret;
    uint8_t temp[sizeof(sMsgUnit)];
    int len =0;

    memset(temp,0,sizeof(temp));
    ret = receive_object(temp, &len, mode);
    if(ret)
    {
        *psize = len;
        if(*psize >= MSG_UNIT_HEAD_LEN)
        {
            *psize = *psize-MSG_UNIT_HEAD_LEN;
            pdata->source.driver.id_driver = temp[0];
            pdata->source.driver.id_parent= temp[1];
            pdata->source.driver.id_child = temp[2];
            pdata->source.driver.id_point= temp[3];
            pdata->dest.driver.id_driver= temp[4];
            pdata->dest.driver.id_parent= temp[5];
            pdata->dest.driver.id_child= temp[6];
            pdata->dest.driver.id_point= temp[7];
            pdata->type = ((uint16_t)(temp[8]<<8))|temp[9];
            memcpy(pdata->data,&temp[10],*psize);
            return true;
        }
    }
    return false;
}

// void MsgMngBase::sem_rec_process(sMsgUnit val)
// {
//     ;
// }

MsgMngDriver::MsgMngDriver():testcycle(0),dest_id(0),m_pDriver(NULL)
{
    soure_id.app = 0;
}

MsgMngDriver::~MsgMngDriver()
{
    zprintf3("MsgMngDriver destruct!\n");
}

// void * RecvMsg_task(void * arg)
// {
//     MsgMng *pMsgMng = MsgMng::GetMsgMng();

//     while(1)
//     {
//         //TODO:
//         pMsgMng->RecvMsgProcess(arg);
//     }

//     return NULL;
// }

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
    if((pkt.dest.driver.id_driver != m_pDriver->m_driverId) && (pkt.dest.app != BROADCAST_ID))
        return;

    switch(pkt.type)
    {
        case MSG_TYPE_DriverGetInfo:
            uint8_t midchang ;
            soure_id.driver.id_driver = pkt.dest.driver.id_driver;

            dest_id = pkt.source.app;
            zprintf3("receive dest id is %d\n",dest_id);
            addr = pkt.dest.app;
            pkt.dest.app = pkt.source.app;
            pkt.source.app =addr;
            //            zprintf1("receive driver info id \n\n\n");

            memcpy(&pkt.data[0] ,&(m_pDriver->m_paramInfo), sizeof(sDriverInfoType));
            midchang = pkt.data[0];
            pkt.data[0] = pkt.data[1];
            pkt.data[1] = midchang;
            midchang = pkt.data[2];
            pkt.data[2] = pkt.data[3];
            pkt.data[3] = midchang;
            midchang = pkt.data[4];
            pkt.data[4] = pkt.data[5];
            pkt.data[5] = midchang;
            pkt_len = sizeof(sDriverInfoType);
            // pSendMsg->SendMsg(&pkt,pkt_len);
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
    //    for(int i = 0; i < 7; i++){
    //        printf("send %d is %d\n", i, pdata->data[i]);
    //    }
    sendMsg(pdata, size);
}

// MsgMng::MsgMng():pRecvMsg(NULL),pSendMsg(NULL),testcycle(0),dest_id(0)
// {
//     soure_id.app = 0;
//     pthread_mutex_init(&WaitListMutex, NULL);
// }

// MsgMng::~MsgMng()
// {

// }

// void * RecvMsg_task(void * arg)
// {
//     MsgMng *pMsgMng = MsgMng::GetMsgMng();

//     while(1)
//     {
//         //TODO:
//         pMsgMng->RecvMsgProcess(arg);
//     }

//     return NULL;
// }

// bool MsgMng::Init(int recvkey,int sendkey, void * arg)
// {
//     pRecvMsg = new msg(recvkey);
//     pSendMsg = new msg(sendkey);
//     WaitDriverList.clear();

//     if(!pRecvMsg->create_object())
//         return false;
//     if(!pSendMsg->create_object())
//         return false;

//     pthread_create(&RecvMsg_id,NULL,RecvMsg_task, arg);
//     return true;
// }

// bool MsgMng::InsertWaitMsg( const Type_MsgAddr &waitid,uint16_t type,sem_t * pack)
// {
//     sWaitMsg data;

//     pthread_mutex_lock(&WaitListMutex);
//     data.waitid = waitid;
//     data.type = type;
//     data.pack = pack;
//     (void) data;
//     if(WaitDriverList.size() <WAIT_MSG_MAX)
//     {
//         // WaitDriverList.append(data);
//         // WaitDriverList.
//         pthread_mutex_unlock(&WaitListMutex);
//         return true;
//     }
//     pthread_mutex_unlock(&WaitListMutex);
//     return false;
// }

// bool MsgMng::CheckWaitMsg( Type_MsgAddr waitid,uint16_t type)
// {
//     // lWaitList::iterator item;

//     pthread_mutex_lock(&WaitListMutex);
//     QList <sWaitMsg>::iterator item =  WaitDriverList.begin();
//     while(item != WaitDriverList.end())
//     {
//         if(((*item).type == type)&&((*item).waitid.app == waitid.app))
//         {
//             pthread_mutex_unlock(&WaitListMutex);
//             return true;
//         }
//         ++item;
//     }
//     pthread_mutex_unlock(&WaitListMutex);
//     return false;
// }

// bool MsgMng::AckWaitMsg( Type_MsgAddr waitid,uint16_t type)
// {

//     pthread_mutex_lock(&WaitListMutex);

//     for(lWaitList::iterator item =  WaitDriverList.begin(); item != WaitDriverList.end(); ++item)
//     {
//         if(((*item).type == type)&&((*item).waitid.app == waitid.app))
//         {
//             sem_post((*item).pack);
//             WaitDriverList.erase(item);
//             pthread_mutex_unlock(&WaitListMutex);
//             return true;
//         }
//     }
//     pthread_mutex_unlock(&WaitListMutex);
//     return false;
// }

// void MsgMng::RecvMsgProcess(void * arg)
// {
// //     sMsgUnit   pkt;
// //     uint16_t   pkt_len;
// //     uint32_t   addr;
// //     // Can_Data *pdriver = (Can_Data *) arg;

// //     if(!pRecvMsg->ReceiveMsg(&pkt,&pkt_len,RECV_WAIT))
// //     {
// //         usleep(10000);
// //         return;
// //     }

// //     if((pkt.dest.driver.id_driver != pdriver->devkey.driverid)&&(pkt.dest.app != BROADCAST_ID))
// //         return;

// //     switch(pkt.type)
// //     {
// //         case MSG_TYPE_DriverGetInfo:
// //         uint8_t midchang ;
// //             soure_id.driver.id_driver = pkt.dest.driver.id_driver;

// //             dest_id = pkt.source.app;
// //             zprintf3("receive dest id is %d\n",dest_id);
// //             addr = pkt.dest.app;
// //             pkt.dest.app = pkt.source.app;
// //             pkt.source.app =addr;
// // //            zprintf1("receive driver info id \n\n\n");

// //             memcpy(&pkt.data[0] ,&pdriver->d_info,sizeof(sDriverInfoType));
// //             midchang = pkt.data[0];
// //             pkt.data[0] = pkt.data[1];
// //             pkt.data[1] = midchang;
// //             midchang = pkt.data[2];
// //             pkt.data[2] = pkt.data[3];
// //             pkt.data[3] = midchang;
// //             midchang = pkt.data[4];
// //             pkt.data[4] = pkt.data[5];
// //             pkt.data[5] = midchang;
// //             pkt_len = sizeof(sDriverInfoType);
// //             pSendMsg->SendMsg(&pkt,pkt_len);
// //             break;

// //         case MSG_TYPE_DriverSendHeart:

// //             addr = pkt.dest.app;
// //             pkt.dest.app = pkt.source.app;
// //             pkt.source.app =addr;
// //             pSendMsg->SendMsg(&pkt,0);
// //             break;

// //         case MSG_TYPE_AppGetIOParam:
// //             addr = pkt.dest.app;
// //             pkt.dest.app = pkt.source.app;
// //             pkt.source.app =addr;
// //             can_inode_info ininfo;
// //             pdriver->get_innode_info(pkt.source.driver.id_parent, pkt.source.driver.id_child,
// //                                      pkt.source.driver.id_point, ininfo);
// //             memcpy(pkt.data, &ininfo,sizeof(ininfo));
// //             pSendMsg->SendMsg(&pkt,sizeof(ininfo));
// //             break;


// //         default:
// //             break;
// //     }
//     return;
// }

// void MsgMng::msgmng_send_msg(sMsgUnit *pdata, uint16_t size)
// {
// //    for(int i = 0; i < 7; i++){
// //        printf("send %d is %d\n", i, pdata->data[i]);
// //    }
//     pSendMsg->SendMsg(pdata, size);
// }


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
            // if (!CheckWaitMsg(pkt.source, pkt.type))
            //     break;

            // if (pdevice->FindDriver(pkt.source.driver.id_driver, &pdriver))
            // {
            //     pdriver->DriverInfo.TotalInCnt    = (uint16_t) (((uint16_t) pkt.data[0] << 8) | pkt.data[1]);
            //     pdriver->DriverInfo.TotalOutCnt   = (uint16_t) (((uint16_t) pkt.data[2] << 8) | pkt.data[3]);
            //     pdriver->DriverInfo.TotalStateCnt = (uint16_t) (((uint16_t) pkt.data[4] << 8) | pkt.data[5]);
            //     AckWaitMsg(pkt.source, pkt.type);
            // }
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
        // if (IsAppMapExist(pkt.source.app))
        // {
        //     pkt.dest.app   = pkt.source.app;
        //     pkt.source.app = BROADCAST_ID;
        //     pkt.data[0]    = MSG_ERROR_LogIn_Exist;
        //     sysLogQE() << "DeviceMng app login error:exist!";
        //     if (FindApp(pkt.dest.app, &papp))
        //     {
        //         papp->pmsg->SendMsg(&pkt, ABNORMAL_MSG_LEN);
        //     }
        //     break;
        // }
        if (pro->m_appTable.size() < APP_LOGIN_MAX)
        {
            // DeviceMng* pdevice = DeviceMng::GetDeviceMng();
            // key                = pdevice->OperateAppMsgKey(KEY_ADD, 0);
            // if (key == 0)
            // {
            //     pkt.dest.app   = pkt.source.app;
            //     pkt.source.app = BROADCAST_ID;
            //     pkt.data[0]    = MSG_ERROR_LogIn_NOAPPID;
            //     sysLogQE() << "DeviceMng app login error:no id!";
            //     pAppTotalMsg->SendMsg(&pkt, ABNORMAL_MSG_LEN);
            //     break;
            // }

            bool isRecv = (bool) pkt.data[0];
            zprintf3("isRecv: %d!\n", isRecv);

            app* apphandle = new app(pkt.source.app, key, isRecv);
            if(apphandle == NULL)
            {
                zprintf1("msg mng server app new error!\n");
                return -1;
            }
            // 先清除之前废除的app 感觉不对
            // for (auto iter = pro->m_appTable.begin(); iter != pro->m_appTable.end();)
            // {
            //     if (!isProcessExists(iter.key()))
            //     {
            //         iter = AppTable.erase(iter);
            //     }
            //     else
            //     {
            //         ++iter;
            //     }
            // }

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
            // pAppTotalMsg->SendMsg(&pkt, NORMAL_MSG_LEN);
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
bool MsgMngServer::Init(int appkey, int driverkey, int totalkey, int initsemkey, int reskey)
{
    // pResMsg      = new msg(reskey);
    // pAppMsg      = new msg(appkey);
    // pDriverMsg   = new msg(driverkey);
    // pAppTotalMsg = new msg(totalkey);
    // WaitDriverList.clear();
    // AppTable.clear();

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
    if (!m_pInitSem->createSem(initsemkey, 1))
    {
        zprintf1("MsgMngServer pinitsem create_sem initsemkey: %d!\n",initsemkey) ;
        return false;
    }
    zprintf3("MsgMngServer msgmng init finish!\n");
    m_recvDriMsg.z_pthread_init(msgmng_drirecv_back, this, "msgservdrirecv");
    m_recvAppMsg.z_pthread_init(msgmng_apprecv_back, this, "msgservapprecv");



    return true;
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

bool MsgMngServer::waitDriverInfo(driver *pdirv)
{
    sMsgUnit pkt;
    if(m_recvDriMsg.receive_object(pkt, 0))
    {
        if (pkt.source.driver.id_driver == pdirv->m_driverId)
        {
            pdirv->m_driverInfo.TotalInCnt    = (uint16_t) (((uint16_t) pkt.data[0] << 8) | pkt.data[1]);
            pdirv->m_driverInfo.TotalOutCnt   = (uint16_t) (((uint16_t) pkt.data[2] << 8) | pkt.data[3]);
            pdirv->m_driverInfo.TotalStateCnt = (uint16_t) (((uint16_t) pkt.data[4] << 8) | pkt.data[5]);
        }
        return true;

    }
    else
    {
        zprintf1("wait DriverInfo error!\n");
        return false;
    }
}

