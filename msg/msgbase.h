#ifndef MSGBASE_H
#define MSGBASE_H


#include "msgtype.h"
#include "msgprocess.h"
#include <semaphore.h>
#include "mutex_class.h"

#define WAIT_MSG_MAX   100

typedef enum
{
    KEY_ADD = 0,
    KEY_SUB
} eOperateKeyType;

#define DEVICEMNG_SHARE_KEY_NUM 4 // 消息占用4个key
#define DRIVER_SHARE_KEY_NUM    5 // 每个驱动占用5个key

class MsgSendBase :public MsgSendClass<sMsgUnit>
{
public:
    MsgSendBase();
    explicit MsgSendBase(int key);
    virtual ~MsgSendBase();
    bool sendMsg(sMsgUnit *pdata,   uint16_t size);
    // bool receiveMsg(sMsgUnit *pdata, uint16_t *psize, int mode);
};


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


typedef int (*ackfunctype)(void* pdata, unsigned int len);

typedef struct
{
    Type_MsgAddr m_waitId;
    uint16_t     m_type;
    sem_t*       m_pack;
    uint32_t     m_timeoutMs;
    ackfunctype  m_ackFunc;
} sWaitMsg;

typedef list< sWaitMsg >         lWaitList;
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



#endif // MSGBASE_H
