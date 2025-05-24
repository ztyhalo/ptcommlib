#include "drivmsgmng.h"


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
    ret = m_SendMsg.sendMsg(pdata, size);
    return ret;
}

/*************************************************************************************
 * MsgMngDriver 驱动消息管理类
 * *************************************************************************************/


DrivMsgMng* DrivMsgMng::m_pDrivMsgMng = NULL;
DrivMsgMng* DrivMsgMng::getDrivMsgMng()
{
    if (m_pDrivMsgMng == NULL)
    {
        m_pDrivMsgMng = new DrivMsgMng();
    }
    return m_pDrivMsgMng;
}

DrivMsgMng::DrivMsgMng():dest_id(0),m_pDriver(NULL)
{
    soure_id.app = 0;
}

DrivMsgMng::~DrivMsgMng()
{
    zprintf3("MsgMngDriver destruct!\n");
}

bool DrivMsgMng::init(int recvkey,int sendkey, PtDriverBase * pdriver)
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


void DrivMsgMng::msgRecvProcess(sMsgUnit pkt, int len)
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

void DrivMsgMng::driverSendMsg(sMsgUnit *pdata, uint16_t size)
{
    sendMsg(pdata, size);
}
