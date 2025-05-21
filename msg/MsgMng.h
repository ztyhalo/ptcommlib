#ifndef MSGMNG_H
#define MSGMNG_H

#include "msg.h"
#include <semaphore.h>
#include <QList>
#include <QMap>
#include <QString>
#include "driver.h"
#include "msgprocess.h"
#include "lsystemsem.h"
#include <QMap>
#include "app.h"
#include "driver.h"

#define WAIT_MSG_MAX   100
#define APP_LOGIN_MAX    11
#define LOGIN_MSG_LEN    1
#define NORMAL_MSG_LEN   9
#define LOGIN_ACK_MSG_LEN    9

#define DEVICEMNG_SHARE_KEY_NUM 4 // 消息占用4个key
#define DRIVER_SHARE_KEY_NUM    5 // 每个驱动占用5个key

typedef enum
{
    KEY_ADD = 0,
    KEY_SUB
} eOperateKeyType;



typedef QMap< uint32_t, app* >    mAppTable;
typedef QMap< int,  driver * > mDriverTable;


class MsgMngBase :public MsgRevClass<sMsgUnit>
{
public:
    Z_Msg<sMsgUnit> m_SendMsg;
public:
    MsgMngBase();
    virtual ~MsgMngBase();
    bool sendMsg(sMsgUnit *pdata,   uint16_t size);
    // bool receiveMsg(sMsgUnit *pdata, uint16_t *psize, int mode);
};


//驱动消息管理类
class MsgMngDriver: public MsgMngBase
{
private:
    MsgMngDriver();
public:
    Type_MsgAddr    soure_id;
    uint32_t        dest_id;
    PtDriverBase *  m_pDriver;


public:
    static MsgMngDriver * GetMsgMngDriver(void);
    static MsgMngDriver  * m_pMsgMngDriver;
    ~MsgMngDriver();
    bool Init(int recvkey,int sendkey, PtDriverBase * pdriver);
    void msgRecvProcess(sMsgUnit val) override;
    void msgmng_send_msg(sMsgUnit *pdata, uint16_t size);
};

//消息管理服务类
class MsgMngServer
{
private:
    // pthread_t     DriverMsg_id;
    // pthread_t     AppMsg_id;
    // msg*          pResMsg;
    // msg*          pDriverMsg;
    // msg*          pAppMsg;
    // msg*          pAppTotalMsg;
    // lWaitList     WaitDriverList;
    // mAppTable       m_appTable;
    // mAppNameTable AppNameTable;
    // SemObject*    pinitsem;
     LSystemSem*    m_pInitSem;


    MsgMngServer();


    // bool CheckWaitMsg(Type_MsgAddr waitid, uint16_t type);
    // bool AckWaitMsg(Type_MsgAddr waitid, uint16_t type);
    // bool isProcessExists(qint64 pid);

public:
    int                                     m_sysResMsgKey;
    int                                     m_sysMinMsgKey;
    int                                     m_sysMaxMsgKey;
    int                                     m_appStMsgKey;
    mDriverTable                            m_driverTable;
    mAppTable                               m_appTable;
    MsgRevBackClass<sMsgUnit, MsgMngServer> m_recvDriMsg;
    MsgRevBackClass<sMsgUnit, MsgMngServer> m_recvAppMsg;
    MsgSendBase                             m_sendToAppMsg;
    QList< int >                            m_appUseKeyList;


    bool                    cancel;
    static MsgMngServer*    m_pMsgMngServ;

    static MsgMngServer* GetMsgMngServer(void);
public:

    ~MsgMngServer();
    bool init(int appkey, int driverkey, int totalkey, int initsemkey, int reskey);
    bool findApp(uint32_t id, app** ppapp);
    bool deleteApp(uint32_t id);
    bool waitDriverInfo(driver * pdirv);
    bool findDriver(uint8_t id, driver** ppdriver);
    void startRecvDrivMsgProcess(void);
    void startRecvAppMsgProcess(void);
    bool isAppMapExist(uint32_t id);
    int  operateAppMsgKey(eOperateKeyType mode, int key);
    bool isProcessExists(qint64 pid);

    // bool InsertWaitMsg(Type_MsgAddr& waitid, uint16_t type, sem_t* pack);
    // void DriverMsgProcess(void);
    // void AppMsgProcess(void);
    // bool BroadcastToApp(uint16_t type, uint8_t* data, uint16_t len);
};

class MsgMngApp
{

  public:
    mDriverTable                            m_driverTable;
    mAppTable                               m_appTable;

    LSystemSem*                             m_pInitSem;
    MsgRevBackClass<sMsgUnit, MsgMngApp>    m_recvServMsg;   //通用注册消息，通过此消息接收m_appRecvMsg的key值
    MsgRevBackClass<sMsgUnit, MsgMngApp>    m_appRecvMsg;     //服务器创建的消息，app接收消息key
    MsgSendBase                             m_sendToServMsg;

    int                                     m_loginOkFlag;
    unsigned int                            m_deviceMngId;
    bool                                    m_isRecv;
    int                                     m_loginKeyId;

    MsgMngApp();
    // bool        CheckWaitMsg(Type_MsgAddr waitid, uint16_t type);
    // bool        AckWaitMsg(Type_MsgAddr waitid, uint16_t type, uint8_t mode);
    // void        timeraddMS(struct timeval* a, uint32_t ms);
    // bool        WaitTimeMsgAck(uint32_t waittime_ms, sWaitMsg* pmsg);
    // bool        InsertWaitMsg(sWaitMsg* waitmsg);
    // ackfunctype GetWaitFunc(Type_MsgAddr waitid, uint16_t type);
    // bool        SendMail(sMsgUnit& pkt, uint16_t pkt_len, ackfunctype func, uint32_t timeout);
    // void        CheckTimeoutMsg(uint16_t intervaltime);


    ~MsgMngApp();
    // void RecvMsgProcess(void);
    // void TotalMsgProcess(void);
    bool initSendMail(int sendkey, int totalkey, int totalmutexkey);
    bool loginRecvMail(void);
    bool waitServerInfo(sMsgUnit & pkt);
    bool initRecvMail(void);
    // bool InitGetInfo(int driver_id, uint32_t timeout_ms);
    // bool MsgSendProcess(Type_MsgAddr& addr, uint16_t msgtype, ackfunctype func, uint8_t* pdata, uint16_t len);
    // void SetIsRecv(bool isRecv);
};

#endif // MSGMNG_H
