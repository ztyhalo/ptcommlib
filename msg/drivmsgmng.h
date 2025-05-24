#ifndef DRIVMSGMNG_H
#define DRIVMSGMNG_H

#include "msgbase.h"
#include "driver.h"



class MsgMngBase :public MsgRevClass<sMsgUnit>
{
  public:
    MsgSendBase     m_SendMsg;
  public:
    MsgMngBase();
    virtual ~MsgMngBase();
    bool sendMsg(sMsgUnit *pdata,   uint16_t size);
};


//驱动消息管理类
class DrivMsgMng: public MsgMngBase
{
  private:
    DrivMsgMng();
  public:
    Type_MsgAddr    soure_id;
    uint32_t        dest_id;
    PtDriverBase *  m_pDriver;


  public:
    static DrivMsgMng * getDrivMsgMng(void);
    static DrivMsgMng  * m_pDrivMsgMng;
    ~DrivMsgMng();
    bool init(int recvkey,int sendkey, PtDriverBase * pdriver);
    void msgRecvProcess(sMsgUnit val, int len) override;
    void driverSendMsg(sMsgUnit *pdata, uint16_t size);
};

#endif // DRIVMSGMNG_H
