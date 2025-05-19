#ifndef MSGOBJECT_H
#define MSGOBJECT_H

#include <sys/msg.h>
// #include <string.h>
#include <QDebug>
#include "zmsg.h"
#define MSG_OBJECT_LENGTH  1024

enum
{
    RECV_WAIT =0,
    RECV_NOWAIT
};

typedef struct
{
    long msgtype;
    char msgtext[MSG_OBJECT_LENGTH];
}sMsgType;


// class MsgObject
// {
// private:
//     int msg_key;
//     int msg_id;
//     sMsgType msg_data;

// public:
//     explicit MsgObject(int key);
//     ~MsgObject();
//     bool create_object(void);
//     bool delete_object(void);
//     bool send_object(void *pdata,int size);
//     bool receive_object(void *pdata,int *psize,int mode);
//     int GetMsgKey(void);
// };
class MsgObject:public Z_Msg<char [MSG_OBJECT_LENGTH]>
{
public:
    explicit MsgObject(int key):Z_Msg(key)
    {
        ;
    }
    ~MsgObject()
    {
        ;
    }
};

#endif // MSGOBJECT_H
