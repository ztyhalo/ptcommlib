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
#define ABNORMAL_MSG_LEN 1

#define MSG_TIMEOUT_VALUE    2000
#define MSG_CHECK_TIME       100

#define DEVICEMNG_SHARE_KEY_NUM 4 // 消息占用4个key
#define DRIVER_SHARE_KEY_NUM    5 // 每个驱动占用5个key

typedef enum
{
    KEY_ADD = 0,
    KEY_SUB
} eOperateKeyType;



typedef QMap< uint32_t, app* >    mAppTable;
typedef QMap< int,  driver * > mDriverTable;

//后期尝试删除
typedef struct
{
    sMsgUnit MsgData;
    uint16_t MsgLen;
} sNotifyMsg;
typedef QList< sNotifyMsg > lNotifyList;


class MsgKeyClass
{
  public:
    int             m_resMsgKey;
    int             m_minMsgKey;
    int             m_maxMsgKey;
    int             m_appMsgKey; //app 开始消息key值
    QList< int >    m_appUseKeyList;

  public:
    MsgKeyClass();
    ~MsgKeyClass();
    int  operateAppMsgKey(eOperateKeyType mode, int key);
    bool  keyCheckInit(int size);
};


// class MsgMngBase :public MsgRevClass<sMsgUnit>
// {
// public:
//     Z_Msg<sMsgUnit> m_SendMsg;
// public:
//     MsgMngBase();
//     virtual ~MsgMngBase();
//     bool sendMsg(sMsgUnit *pdata,   uint16_t size);
//     // bool receiveMsg(sMsgUnit *pdata, uint16_t *psize, int mode);
// };


// //驱动消息管理类
// class MsgMngDriver: public MsgMngBase
// {
// private:
//     MsgMngDriver();
// public:
//     Type_MsgAddr    soure_id;
//     uint32_t        dest_id;
//     PtDriverBase *  m_pDriver;


// public:
//     static MsgMngDriver * GetMsgMngDriver(void);
//     static MsgMngDriver  * m_pMsgMngDriver;
//     ~MsgMngDriver();
//     bool Init(int recvkey,int sendkey, PtDriverBase * pdriver);
//     void msgRecvProcess(sMsgUnit val, int len) override;
//     void msgmng_send_msg(sMsgUnit *pdata, uint16_t size);
// };

typedef int (*ackfunctype)(void* pdata, unsigned int len);

typedef struct
{
    Type_MsgAddr waitid;
    uint16_t     type;
    sem_t*       pack;
    uint32_t     timeout_ms;
    ackfunctype  ackfunc;
} sWaitMsg;

typedef QList< sWaitMsg >         lWaitList;
class MsgWaitBase
{
public:
    lWaitList       m_waitList;
    MUTEX_CLASS     m_waitMutex;
  public:
    MsgWaitBase();
    ~MsgWaitBase();

    bool        checkWaitMsg(Type_MsgAddr waitid, uint16_t type);
    bool        ackWaitMsg(Type_MsgAddr waitid, uint16_t type, uint8_t mode);
    bool        insertWaitMsg(sWaitMsg & waitmsg);
    void        timerAddMS(struct timeval* a, uint32_t ms);
    bool        waitTimeMsgAck(uint32_t waittime_ms, sWaitMsg* pmsg);
    ackfunctype getWaitFunc(Type_MsgAddr waitid, uint16_t type);
    void        checkTimeoutMsg(uint16_t intervaltime);

};

//消息管理服务类
class MsgMngServer:public MsgKeyClass
{
private:

    LSystemSem*    m_pInitSem;
    MsgMngServer();

public:

    mDriverTable                            m_driverTable;
    mAppTable                               m_appTable;
    MsgRevBackClass<sMsgUnit, MsgMngServer> m_recvDriMsg;
    MsgRevBackClass<sMsgUnit, MsgMngServer> m_recvAppMsg;
    MsgSendBase                             m_sendToAppMsg;



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
    bool isProcessExists(qint64 pid);

};


typedef enum
{
    WAIT_MSG_BLOCK = 0,
    WAIT_MSG_UNBLOCK
} eWaitMsgType;

//消息应用app类
class MsgMngApp:public MsgKeyClass,public MsgWaitBase
{
private:
        MsgMngApp();
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

    sem_t                                   m_notifySem;
    lNotifyList                             m_notifyList;
    MUTEX_CLASS                             m_notifyMutex;

    static MsgMngApp*    m_pMsgMngApp;

    static MsgMngApp* getMsgMngApp(void);

    ~MsgMngApp();

    bool initSendMail(int sendkey, int totalkey, int totalmutexkey);
    bool loginRecvMail(void);
    bool findDriver(uint8_t id, driver** ppdriver);
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

};

#endif // MSGMNG_H
