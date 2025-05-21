#ifndef MSG_H
#define MSG_H

#include "MsgObject.h"
#include "msgtype.h"
#include "msgprocess.h"

// class msg:public MsgObject
// {
// public:
//     explicit msg(int key):MsgObject(key)
//     {

//     }
//     ~msg();

//     bool SendMsg(sMsgUnit *pdata,uint16_t size);
//     bool ReceiveMsg(sMsgUnit *pdata,uint16_t *psize,int mode);
// };


class MsgSendBase :public MsgSendClass<sMsgUnit>
{
public:
    MsgSendBase();
    explicit MsgSendBase(int key);
    virtual ~MsgSendBase();
    bool sendMsg(sMsgUnit *pdata,   uint16_t size);
    // bool receiveMsg(sMsgUnit *pdata, uint16_t *psize, int mode);
};

#endif // MSG_H
