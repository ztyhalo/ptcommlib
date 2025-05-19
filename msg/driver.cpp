#include "driver.h"
// #include "MsgMng.h"
#include "timers.h"

driver::driver(int id, QString& name, int shminkey, int shmoutkey, int shmoutsem, int msgkey, int shmstatekey)
{
    driver_id   = id;
    driver_name = name;
    ComState    = COMSTATE_NORMAL;

    pshm = new shm(shminkey, shmoutkey, shmoutsem, shmstatekey);
    pmsg = new msg(msgkey);
    m_heartMark = 0;
}

driver::~driver()
{
    zprintf3("DeviceMng driver exit begin:\n");
    DELETE(pshm);
    zprintf3("DeviceMng DELETE(pshm),\n");
    DELETE(pmsg);
    zprintf3("DeviceMng driver exit end.\n");
}

bool driver::InitMsg(void)
{
    if (!pmsg->create_object())
    {
        zprintf1("DeviceMng driver init :create_object error!\n");
        return false;
    }
    return true;
}

bool driver::Init(void)
{

    zprintf3("DeviceMng driver init :shm_create: %d ,shm_state: %d.\n" , DriverInfo.TotalInCnt + DriverInfo.TotalOutCnt,
             DriverInfo.TotalStateCnt);

    if (!pshm->shm_create(DriverInfo.TotalInCnt + DriverInfo.TotalOutCnt, DriverInfo.TotalStateCnt))
    {
        zprintf3("DeviceMng driver init :shm_create: %d ,shm_state: %d fail!\n" , DriverInfo.TotalInCnt + DriverInfo.TotalOutCnt,
                 DriverInfo.TotalStateCnt);
        return false;
    }
    return true;
}



bool driver::Msg_GetInfo(void)
{
    sMsgUnit pkt;


    memset(&pkt, 0, sizeof(sMsgUnit));
    pkt.source.app            = GET_DEVMNG_ID;
    pkt.dest.driver.id_driver = driver_id;
    pkt.type                  = MSG_TYPE_DriverGetInfo;

    if (pmsg->SendMsg(&pkt, 0) == false)
    {
        zprintf1("DeviceMng pmsg->SendMsg(&pkt,0) == false!\n");
        return false;
    }
    return true;
}

bool driver::Msg_SendHeart(void)
{
    sMsgUnit pkt;

    m_heartMark = 1;  //开始发送心跳标志
    memset(&pkt, 0, sizeof(sMsgUnit));
    pkt.source.app            = GET_DEVMNG_ID;
    pkt.dest.driver.id_driver = driver_id;
    pkt.type                  = MSG_TYPE_DriverSendHeart;


    if (pmsg->SendMsg(&pkt, 0) == false)
    {
        zprintf1("driver Id%d msg send heart error!\n", driver_id);
        return false;
    }


    return true;
}
