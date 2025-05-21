#ifndef MSG_H
#define MSG_H


#include "msgtype.h"
#include "msgprocess.h"




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
