#ifndef DRIVERMSGMNG_H
#define DRIVERMSGMNG_H

#include "msg.h"
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
class DriverMsgMng: public MsgMngBase
{
  private:
    MsgMngDriver();
  public:
    Type_MsgAddr    soure_id;
    uint32_t        dest_id;
    PtDriverBase *  m_pDriver;


  public:
    static DriverMsgMng * getDrivMsgMng(void);
    static DriverMsgMng  * m_pDrivMsgMng;
    ~DriverMsgMng();
    bool init(int recvkey,int sendkey, PtDriverBase * pdriver);
    void msgRecvProcess(sMsgUnit val, int len) override;
    void msgmng_send_msg(sMsgUnit *pdata, uint16_t size);
};

#endif // DRIVERMSGMNG_H
