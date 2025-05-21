#ifndef DRIVER_H
#define DRIVER_H

#include <QObject>
#include <semaphore.h>
#include "shm.h"
#include "msg.h"
// #include <time.h>

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

class PtDriverBase:public Key_Info
{
  public:
    int              m_driverId;
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
    virtual int  get_innode_info(int dev, int child, int innode, void* val) =0;

};

typedef enum
{
    COMSTATE_NORMAL = 0,
    COMSTATE_ABNORMAL
} eComStateType;

class driver
{
  private:
    QString         m_drivName;

  public:
    int             m_driverId;
    shm*            m_pShm;
    MsgSendBase *   m_pSendmsg;
    int             m_heartMark;
    sDriverInfoType m_driverInfo;
    eComStateType   m_comState;

    driver(int id, QString& name, int shminkey, int shmoutkey, int shmoutsem, int msgkey, int shmstatekey);
    ~driver();
    bool init(void);
    bool initMsg(void);
    bool msgSendHeart(void);
    bool msgGetInfo(void);
};

#endif // DRIVER_H
