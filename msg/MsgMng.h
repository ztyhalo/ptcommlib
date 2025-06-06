#ifndef MSGMNG_H
#define MSGMNG_H

#include "msgbase.h"
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
#include "msgmngbase.h"


#define APP_LOGIN_MAX       11
#define NORMAL_MSG_LEN      9
#define ABNORMAL_MSG_LEN    1


#define MSG_CHECK_TIME       100



//消息管理服务类
class MsgMngServer:public MsgKeyClass,public DrivAppMap
{
private:

    LSystemSem*    m_pInitSem;
    MsgMngServer();

public:

    MsgRevBackClass<sMsgUnit, MsgMngServer> m_recvDriMsg;   //key:31
    MsgRevBackClass<sMsgUnit, MsgMngServer> m_recvAppMsg;   //key:30
    MsgSendBase                             m_sendToAppMsg; //key:32

    bool                    m_cancel;
    static MsgMngServer*    m_pMsgMngServ;

    static MsgMngServer*    GetMsgMngServer(void);
public:

    ~MsgMngServer();
    bool init(int appkey, int driverkey, int totalkey, int initsemkey, int reskey);

    bool waitDriverInfo(driver * pdirv);
    void startRecvDrivMsgProcess(void);
    void startRecvAppMsgProcess(void);
    bool isProcessExists(qint64 pid);
    void sendHeartToDriver(void);
    bool broadcastToApp(uint16_t type, uint8_t* data, uint16_t len);

};




#endif // MSGMNG_H
