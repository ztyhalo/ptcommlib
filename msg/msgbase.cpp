#include "msgbase.h"
#include <mutex>


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


MsgKeyClass::MsgKeyClass():m_resMsgKey(0),m_minMsgKey(0),m_maxMsgKey(0),m_appMsgKey(0)
{
    ;
}
MsgKeyClass::~MsgKeyClass()
{
    zprintf3("MsgKeyClass destruct!\n");
}

int  MsgKeyClass::operateAppMsgKey(eOperateKeyType mode, int key)
{
    int remain = m_maxMsgKey - m_appMsgKey + 1 - m_appUseKeyList.size();
    if (mode == KEY_ADD)
    {
        if (remain <= 0)
            return 0;
        for (int i = m_appMsgKey; i < m_maxMsgKey; i++)
        {
            if (!m_appUseKeyList.contains(i))
            {
                m_appUseKeyList.append(i);
                return i;
            }
        }
    }
    else if (mode == KEY_SUB)
    {
        if (m_appUseKeyList.size() == 0)
            return 0;
        for (int i = 0; i < m_appUseKeyList.size(); i++)
        {
            if (m_appUseKeyList.at(i) == key)
            {
                m_appUseKeyList.removeAt(i);
                return 1;
            }
        }
    }
    return 0;
}

bool  MsgKeyClass::keyCheckInit(int size)
{
    if (m_maxMsgKey < m_minMsgKey)
    {
        zprintf1("DeviceMng SysMaxKey < SysMinKey!\n");
        return false;
    }

    if ((m_maxMsgKey - m_minMsgKey + 1) <= (DRIVER_SHARE_KEY_NUM * size + DEVICEMNG_SHARE_KEY_NUM))
    {
        zprintf1("DeviceMng SysKey num fail,SysMaxKey: %d SysMinKey: %d CfgList.size() %d!\n",m_maxMsgKey ,
                 m_minMsgKey , size);
        return false;
    }

    m_appMsgKey = m_minMsgKey + DRIVER_SHARE_KEY_NUM * size + DEVICEMNG_SHARE_KEY_NUM;
    return true;

}

bool MsgWaitBase::checkWaitMsg(Type_MsgAddr waitid, uint16_t type)
{

    // m_waitMutex.lock();
    // lWaitList::iterator item = m_waitList.begin();
    lock_guard<MUTEX_CLASS> lock(m_waitMutex);
    for (auto item = m_waitList.begin(); item != m_waitList.end(); ++item)
    {
        if ((item->m_type == type) && (item->m_waitId.app == waitid.app))
        {
            // m_waitMutex.unlock();
            return true;
        }
    }
    // m_waitMutex.unlock();
    return false;
}

bool MsgWaitBase::ackWaitMsg(Type_MsgAddr waitid, uint16_t type, uint8_t mode)
{

    // m_waitMutex.lock();
    // lWaitList::iterator item = m_waitList.begin();
    lock_guard<MUTEX_CLASS> lock(m_waitMutex);
    for (auto item = m_waitList.begin(); item != m_waitList.end(); ++item)
    {
        if ((item->m_type == type) && (item->m_waitId.app == waitid.app))
        {
            if (!mode)
                sem_post(item->m_pack);
            m_waitList.erase(item);
            // m_waitMutex.unlock();
            return true;
        }
    }
    // m_waitMutex.unlock();
    return false;
}

bool MsgWaitBase::insertWaitMsg(sWaitMsg & waitmsg)
{

    // waitmsg.timeout_ms = (waitmsg.timeout_ms == 0) ? MSG_TIMEOUT_VALUE : waitmsg.timeout_ms;
    // m_waitMutex.lock();
    lock_guard<MUTEX_CLASS> lock(m_waitMutex);
    if (m_waitList.size() < WAIT_MSG_MAX)
    {
        m_waitList.push_back(waitmsg);
        // m_waitMutex.unlock();
        return true;
    }
    else
    {
        // m_waitMutex.unlock();
        return false;
    }
}

void MsgWaitBase::timerAddMS(struct timeval* a, uint32_t ms)
{
    a->tv_usec += ms * 1000;
    if (a->tv_usec >= 1000000)
    {
        a->tv_sec += a->tv_usec / 1000000;
        a->tv_usec %= 1000000;
    }
}

bool MsgWaitBase::waitTimeMsgAck(uint32_t waittime_ms, sWaitMsg* pmsg)
{
    struct timeval  now;
    struct timespec outtime;
    int             ret;

    if (waittime_ms > 0)
    {
        gettimeofday(&now, NULL);

        timerAddMS(&now, waittime_ms);

        outtime.tv_sec  = now.tv_sec;
        outtime.tv_nsec = now.tv_usec * 1000;

        int count       = 0;
        while (count < 5)
        {
            ret = sem_timedwait(pmsg->m_pack, &outtime);
            if (ret == -1)
            {
                zprintf3("LibDeviceMng WaitTimeMsgAck errno: %d coount %d!\n", errno ,count);
            }
            count++;
            if (ret == -1 && errno == EINTR) // if sem_timedwait returns -1 and errno is EINTR, recall sem_timedwait
            {
                continue;
            }
            else
                break;
        }
        if (ret == -1)
        {
            ackWaitMsg(pmsg->m_waitId, pmsg->m_type, 1);
            return false;
        }
    }
    else
    {
        ret = sem_wait(pmsg->m_pack);
        if (ret == -1)
        {
            ackWaitMsg(pmsg->m_waitId, pmsg->m_type, 1);
            return false;
        }
    }

    return true;
}

ackfunctype MsgWaitBase::getWaitFunc(Type_MsgAddr waitid, uint16_t type)
{
    // m_waitMutex.lock();
    // lWaitList::iterator item = m_waitList.begin();
    lock_guard<MUTEX_CLASS> lock(m_waitMutex);
    for(auto item = m_waitList.begin(); item != m_waitList.end(); ++item)
    {
        if ((item->m_type == type) && (item->m_waitId.app == waitid.app))
        {
            // m_waitMutex.unlock();
            return item->m_ackFunc;
        }
    }
    // m_waitMutex.unlock();
    return 0;
}

void MsgWaitBase::checkTimeoutMsg(uint16_t interval)
{
    // m_waitMutex.lock();
    std::lock_guard<MUTEX_CLASS> lock(m_waitMutex); //c++11 支持的新特性

    auto item = m_waitList.begin();            //c++11 支持的新特性   仅尝试使用
    while(item != m_waitList.end())
    {
        if (item->m_timeoutMs <= interval)
        {
            item = m_waitList.erase(item);
        }
        else
        {
            item->m_timeoutMs -= interval;
            ++item;
        }
    }
    // m_waitMutex.unlock();
}

