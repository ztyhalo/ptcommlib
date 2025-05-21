#include "msg.h"



MsgSendBase::MsgSendBase()
{
    ;
}

MsgSendBase::MsgSendBase(int key):MsgSendClass(key)
{
    ;
}

MsgSendBase::~MsgSendBase()
{
    zprintf3("MsgSendBase destruct!\n");
}

bool MsgSendBase::sendMsg(sMsgUnit *pdata, uint16_t size)
{
    bool ret;

    ret = send_object(pdata, (int)(size+MSG_UNIT_HEAD_LEN), 1);
    return ret;
}

