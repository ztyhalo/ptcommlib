#ifndef MSGMNG_H
#define MSGMNG_H

#include "msg.h"
#include <semaphore.h>
#include <QList>
#include <QMap>
#include <QString>
#include "driver.h"
#include "semprocess.h"
#include "zprint.h"
#include "lsystemsem.h"
#include <QMap>
#include "app.h"
#include "driver.h"
#define WAIT_MSG_MAX   100
#define APP_LOGIN_MAX    11

#define NORMAL_MSG_LEN   9

typedef struct
{
    Type_MsgAddr waitid;
    uint16_t            type;
    sem_t *            pack;
}sWaitMsg;
typedef QList<sWaitMsg> lWaitList;


typedef QMap< uint32_t, app* >    mAppTable;
typedef QMap< int,  driver * > mDriverTable;


class MsgSendBase :public MsgRevClass<sMsgUnit>
{
public:
    MsgSendBase();
    virtual ~MsgSendBase();
    bool sendMsg(sMsgUnit *pdata,   uint16_t size);
    // bool receiveMsg(sMsgUnit *pdata, uint16_t *psize, int mode);
};

class MsgMngBase :public MsgRevClass<sMsgUnit>
{
public:
    Z_Msg<sMsgUnit> m_SendMsg;
public:
    MsgMngBase();
    virtual ~MsgMngBase();
    bool sendMsg(sMsgUnit *pdata,   uint16_t size);
    bool receiveMsg(sMsgUnit *pdata, uint16_t *psize, int mode);
};

class PtDriverBase
{
public:
    int             m_driverId;
    sDriverInfoType  m_paramInfo;
public:
    PtDriverBase():m_driverId(0)
    {
        memset(&m_paramInfo, 0x00, sizeof(m_paramInfo));
    }
    virtual ~PtDriverBase()
    {
        zprintf3("PtDriverBase destruct!\n");
    }
    virtual int  get_innode_info(uint devnum, uint innode, pt_inode_info& val) =0;

};

class MsgMngDriver: public MsgMngBase
{
private:

    int testcycle ;

    MsgMngDriver();
public:
    Type_MsgAddr    soure_id;
    uint32_t        dest_id;
    // int             m_driverId;
    // sParamInfoType  m_paramInfo;
    PtDriverBase *  m_pDriver;


public:
    static MsgMngDriver * GetMsgMngDriver(void)
    {
        static MsgMngDriver gMsgMngDriver;
        return &gMsgMngDriver;
    }
    ~MsgMngDriver();
    bool Init(int recvkey,int sendkey, PtDriverBase * pdriver);
    void sem_rec_process(sMsgUnit val) override;
    void msgmng_send_msg(sMsgUnit *pdata, uint16_t size);
};


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
    // mAppTable     AppTable;
    // mAppNameTable AppNameTable;
    // SemObject*    pinitsem;
     LSystemSem*    m_pInitSem;


    MsgMngServer();
    // bool IsAppMapExist(uint32_t id);
    // bool FindApp(uint32_t id, app** ppapp);
    // bool DeleteApp(uint32_t id);
    // bool CheckWaitMsg(Type_MsgAddr waitid, uint16_t type);
    // bool AckWaitMsg(Type_MsgAddr waitid, uint16_t type);
    // bool isProcessExists(qint64 pid);

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
    bool Init(int appkey, int driverkey, int totalkey, int initsemkey, int reskey);
    bool findApp(uint32_t id, app** ppapp);
    bool deleteApp(uint32_t id);
    bool waitDriverInfo(driver * pdirv);
    bool findDriver(uint8_t id, driver** ppdriver);
    // bool InsertWaitMsg(Type_MsgAddr& waitid, uint16_t type, sem_t* pack);
    // void DriverMsgProcess(void);
    // void AppMsgProcess(void);
    // bool BroadcastToApp(uint16_t type, uint8_t* data, uint16_t len);
};


class MsgMng
{
private:
    pthread_t RecvMsg_id;
    msg  *pRecvMsg;
    msg  *pSendMsg;
    lWaitList  WaitDriverList;
    pthread_mutex_t WaitListMutex;
    int testcycle ;

    MsgMng();
    bool CheckWaitMsg( Type_MsgAddr waitid,uint16_t type);
    bool AckWaitMsg( Type_MsgAddr waitid,uint16_t type);

public:
    Type_MsgAddr soure_id;
    uint32_t     dest_id;
public:
    static MsgMng * GetMsgMng(void)
    {
        static MsgMng gMsgMng;
        return &gMsgMng;
    }
    ~MsgMng();
    bool Init(int recvkey,int sendkey, void * arg=NULL);
    bool InsertWaitMsg(const Type_MsgAddr &waitid,uint16_t type,sem_t * pack);
    void RecvMsgProcess(void * arg);
    void msgmng_send_msg(sMsgUnit *pdata, uint16_t size);
};

#endif // MSGMNG_H
