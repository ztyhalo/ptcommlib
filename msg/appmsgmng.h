#ifndef APPMSGMNG_H
#define APPMSGMNG_H

#include "msgbase.h"
#include <semaphore.h>
// #include "driver.h"
#include "msgprocess.h"
#include "lsystemsem.h"
#include "msgmngbase.h"

#define MSG_TIMEOUT_VALUE    2000
#define LOGIN_MSG_LEN        1
#define LOGIN_ACK_MSG_LEN    9

typedef enum
{
    WAIT_MSG_BLOCK = 0,
    WAIT_MSG_UNBLOCK
} eWaitMsgType;

//消息应用app类
class AppMsgMng:public MsgKeyClass,public DrivAppMap,public MsgWaitBase
{
private:
    AppMsgMng();
public:


    LSystemSem*                             m_pInitSem;
    Z_Msg<sMsgUnit>                         m_recvServMsg;   //key:32 通用注册消息，通过此消息接收m_appRecvMsg的key值
    MsgRevBackClass<sMsgUnit, AppMsgMng>    m_appRecvMsg;    //服务器创建的消息，app接收消息key
    MsgSendBase                             m_sendToServMsg;  //key 30

    int                                     m_loginOkFlag;
    unsigned int                            m_deviceMngId;
    bool                                    m_isRecv;
    int                                     m_loginKeyId;

    sem_t                                   m_notifySem;
    lNotifyList                             m_notifyList;
    MUTEX_CLASS                             m_notifyMutex;

    static AppMsgMng*   m_pMsgMngApp;

    static AppMsgMng*   getMsgMngApp(void);

    ~AppMsgMng();

    bool initSendMail(int sendkey, int totalkey, int totalmutexkey);
    bool loginRecvMail(void);
    // bool findDriver(uint8_t id, driver** ppdriver);
    bool waitServerInfo(sMsgUnit & pkt);
    bool waitDriverInfo(sMsgUnit & pkt);
    bool initRecvMail(void);
    bool initGetInfo(int driver_id, uint32_t timeout_ms);
    bool wait_msg(sMsgUnit* recvmsg, uint16_t* msglen, eWaitMsgType mode);
    bool sendMail(sMsgUnit& pkt, uint16_t pkt_len, ackfunctype func, uint32_t timeout);
    bool msgSendProcess(Type_MsgAddr& addr, uint16_t msgtype, ackfunctype func, uint8_t* pdata, uint16_t len);
    void setIsRecv(bool isRecv)
    {
        m_isRecv = isRecv;
    }
    int appStartRecvMsg(void);

};

#endif // APPMSGMNG_H
