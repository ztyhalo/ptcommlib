#ifndef APP_H
#define APP_H

#include <QObject>
#include "msgbase.h"

class app
{
  private:
    uint32_t m_appId;

  public:
    int             m_key;
    MsgSendBase *   m_pMsg;
    bool            m_isRecv;

    app(uint32_t id, int msgkey, bool isRecvIn);
    ~app();
    bool init(void);
};

#endif // APP_H
