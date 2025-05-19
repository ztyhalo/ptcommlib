#ifndef MSG_H
#define MSG_H

#include "MsgObject.h"
#include "msgtype.h"

class msg:public MsgObject
{
public:
    explicit msg(int key):MsgObject(key)
    {

    }
    ~msg();

    bool SendMsg(sMsgUnit *pdata,uint16_t size);
    bool ReceiveMsg(sMsgUnit *pdata,uint16_t *psize,int mode);
};

#endif // MSG_H
