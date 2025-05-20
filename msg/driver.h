#ifndef DRIVER_H
#define DRIVER_H

#include <QObject>
#include <semaphore.h>
#include "shm.h"
#include "msg.h"
#include <time.h>

typedef struct
{
    uint16_t TotalInCnt;
    uint16_t TotalOutCnt;
    uint16_t TotalStateCnt;
} sDriverInfoType;

typedef struct
{
    int data_key;
    int ctrl_key;
    int sem_key;
    int recvmsg_key;
    int sendmsg_key;

    int driverid;
    int state_key;

}sDriverInfo;

class Key_Info
{
public:
    sDriverInfo devkey;
    Key_Info(){
        memset(&devkey, 0x00, sizeof(sDriverInfo));
    }

};

struct pt_inode_info
{
    int      nodeid;        //点的序列号
    int      datatype;      //点的数据类型(开关量常闭/开关量常开/频率量)
    uint16_t node_en;       //使能标志
    uint16_t shake_time;    //去抖时间
    uint16_t threshold_min;
    uint16_t threshold_max;
    uint8_t  notify_time_interval;
    uint8_t  notify_en;
    uint8_t  notify_range;
};

typedef enum
{
    COMSTATE_NORMAL = 0,
    COMSTATE_ABNORMAL
} eComStateType;

class driver
{
  private:
    QString driver_name;

  public:
    int             driver_id;
    shm*            pshm;
    MsgSendBase *   pmsg;
    int             m_heartMark;
    sDriverInfoType DriverInfo;
    eComStateType   ComState;

    driver(int id, QString& name, int shminkey, int shmoutkey, int shmoutsem, int msgkey, int shmstatekey);
    ~driver();
    bool Init(void);
    bool InitMsg(void);
    bool Msg_SendHeart(void);
    bool Msg_GetInfo(void);
};

#endif // DRIVER_H
