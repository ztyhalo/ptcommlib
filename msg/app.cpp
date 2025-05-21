#include "app.h"
#include "define.h"

app::app(uint32_t id, int msgkey, bool isRecvIn):m_appId(id),m_key(msgkey),m_pMsg(NULL),m_isRecv(isRecvIn)
{
    // m_pMsg   = new MsgSendBase(msgkey);
    // msg_init(msgkey);
    init();
}

app::~app()
{
    zprintf3("app destruct!\n");
    // pmsg->delete_object();
    DELETE(m_pMsg);
}

bool app::init(void)
{
    m_pMsg = new MsgSendBase(m_key);
    if (!m_pMsg->create_object())
        return false;
    return true;
}
