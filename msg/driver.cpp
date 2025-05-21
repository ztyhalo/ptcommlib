#include "driver.h"
// #include "MsgMng.h"
#include "timers.h"

driver::driver(int id, QString& name, int shminkey, int shmoutkey, int shmoutsem, int msgkey, int shmstatekey)
{
    m_driverId   = id;
    m_drivName = name;
    m_comState    = COMSTATE_NORMAL;

    m_pShm = new shm(shminkey, shmoutkey, shmoutsem, shmstatekey);
    m_pSendmsg = new MsgSendBase(msgkey);
    m_heartMark = 0;
}

driver::~driver()
{
    zprintf3("DeviceMng driver exit begin:\n");
    DELETE(m_pShm);
    zprintf3("DeviceMng DELETE(pshm),\n");
    DELETE(m_pSendmsg);
    zprintf3("DeviceMng driver exit end.\n");
}

bool driver::initMsg(void)
{
    if (!m_pSendmsg->create_object())
    {
        zprintf1("DeviceMng driver init :create_object error!\n");
        return false;
    }
    return true;
}

bool driver::init(void)
{

    zprintf3("DeviceMng driver init :shm_create: %d ,shm_state: %d.\n" , m_driverInfo.TotalInCnt + m_driverInfo.TotalOutCnt,
             m_driverInfo.TotalStateCnt);

    if (!m_pShm->shm_create(m_driverInfo.TotalInCnt + m_driverInfo.TotalOutCnt, m_driverInfo.TotalStateCnt))
    {
        zprintf3("DeviceMng driver init :shm_create: %d ,shm_state: %d fail!\n" , m_driverInfo.TotalInCnt + m_driverInfo.TotalOutCnt,
                 m_driverInfo.TotalStateCnt);
        return false;
    }
    return true;
}



bool driver::msgGetInfo(void)
{
    sMsgUnit pkt;


    memset(&pkt, 0, sizeof(sMsgUnit));
    pkt.source.app            = GET_DEVMNG_ID;
    pkt.dest.driver.id_driver = m_driverId;
    pkt.type                  = MSG_TYPE_DriverGetInfo;
    zprintf1("send get info!\n");
    if (m_pSendmsg->sendMsg(&pkt, 0) == false)
    {
        zprintf1("DeviceMng pmsg->SendMsg(&pkt,0) == false!\n");
        return false;
    }
    return true;
}

bool driver::msgSendHeart(void)
{
    sMsgUnit pkt;

    m_heartMark = 1;            //开始发送心跳标志
    memset(&pkt, 0, sizeof(sMsgUnit));
    pkt.source.app            = GET_DEVMNG_ID;
    pkt.dest.driver.id_driver = m_driverId;
    pkt.type                  = MSG_TYPE_DriverSendHeart;

    zprintf3("send haeart driver %d!\n", m_driverId);
    if (m_pSendmsg->sendMsg(&pkt, 0) == false)
    {
        zprintf1("driver Id%d msg send heart error!\n", m_driverId);
        return false;
    }


    return true;
}
