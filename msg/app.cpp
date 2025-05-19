#include "app.h"


app::app(uint32_t id, int msgkey, bool isRecvIn)
{
    app_id = id;
    isRecv = isRecvIn;
    pmsg   = new msg(msgkey);
    Init();
}

app::~app()
{
    zprintf3("app destruct!\n");
    // pmsg->delete_object();
}

bool app::Init(void)
{
    if (!pmsg->create_object())
        return false;
    return true;
}
