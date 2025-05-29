#include "shm.h"
uint32_t gSemNum;

shm::shm(int inkey, int outkey, int outsem, int statekey):data_key(inkey),ctrl_key(outkey),
      ctrl_semkey(outsem),state_key(statekey),data_cnt(0),m_pDataShm(NULL),m_pCtrlShm(NULL),
      m_pStateShm(NULL)
{
}

shm::~shm()
{
    zprintf3("devicemng shm exit start!\n");

    DELETE(m_pDataShm);
    DELETE(m_pCtrlShm);
    DELETE(m_pStateShm);
    IndexMap.clear();
    zprintf3 ("devicemng shm exit finish!\n");
}

bool shm::shm_create(int incnt, int statecnt)
{
    bool ret = true;

    data_cnt = (incnt > DATA_AREA_POINT_MAX) ? DATA_AREA_POINT_MAX : incnt;

    zprintf3("DeviceMng shm_create sDataUnit!\n");
    if(m_pDataShm != NULL)
    {
        DELETE(m_pDataShm);
    }
    m_pDataShm = new QTShareDataT<sDataUnit>();
    if(m_pDataShm == NULL)
    {
        zprintf1("DeviceMng new m_pDataShm fail!\n");
        return false;
    }
    if(m_pDataShm->creat_data((sizeof(sDataUnit)) * data_cnt, QString("%1").arg(data_key), ZSysShmBase::Open) == NULL)
    {
        zprintf1("DeviceMng m_pDataShm create data_cnt fail!\n");
        return false;
    }

    zprintf3("DeviceMng shm_create sCtrlArea!\n");
    m_pCtrlShm = new Sem_Qt_Data<soutDataUnit>();
    if(m_pCtrlShm == NULL)
    {
        zprintf1("DeviceMng new m_pCtrlShm fail!\n");
        return false;
    }
    if(m_pCtrlShm->creat_sem_data(CON_OUT_BUF_SIZE * sizeof(soutDataUnit), (key_t)ctrl_semkey,
          QString("%1").arg(ctrl_key),  ZSysShmBase::Open) != 0)
    {
        zprintf1("DeviceMng m_pCtrlShm create data_cnt fail!\n");
        return false;
    }

    zprintf3("DeviceMng shm_create sDataUnit.\n");
    if(m_pStateShm != NULL)
    {
        DELETE(m_pStateShm);
    }
    m_pStateShm = new QTShareDataT<char>();
    if(m_pStateShm == NULL)
    {
        zprintf1("DeviceMng new m_pStateShm fail!\n");
        return false;
    }
    if(m_pStateShm->creat_data(statecnt, QString("%1").arg(state_key), ZSysShmBase::Open) == NULL)
    {
        zprintf1("DeviceMng m_pStateShm create data_cnt fail!\n");
        return false;
    }


    zprintf3("DeviceMng shm_create shm_init.\n");
    ret = shm_init();
    zprintf3("DeviceMng shm_init ret= %d.\n" , ret);
    return ret;
}

bool shm::shm_delete(void)
{
    // bool ret = true;
    // ret &= pMsgSem->del_sem();
    // ret &= pDataShm->delete_object();
    // ret &= pCtrlShm->delete_object();
    // ret &= pStateShm->delete_object();
    return true;
}


bool shm::shm_init(void)
{
    sDataUnit temp;

    IndexMap.clear();
    zprintf3("DeviceMng shm_init data_cnt: %d.\n" , data_cnt);
    for (int i = 0; i < data_cnt; i++)
    {
        if (m_pDataShm->get_data(i, temp) == 0)
            IndexMap.insert(SERIALIZE_FUNC(temp.parentid, temp.childid, temp.pointid, temp.num), i);
        else
        {
            zprintf1("DeviceMng shm_init fail!\n");
            return false;
        }
    }
    zprintf3("DeviceMng shm_init sucess.\n");
    return true;
}


bool shm::shm_read(int parentid, int childid, int pointid, int type, double* pvalue)
{
    mShmIndex::Iterator item;
    sDataUnit           temp;

    if (IndexMap.isEmpty())
    {
        zprintf1("DeviceMng IndexMap.isEmpty!\n");
        return false;
    }
    item = IndexMap.find(SERIALIZE_FUNC(parentid, childid, pointid, type));
    if ((item != IndexMap.end()) && (item.key() == SERIALIZE_FUNC(parentid, childid, pointid, type)))
    {
        if (m_pDataShm->get_data(item.value(), temp) == 0)
        {
            *pvalue = temp.value;
            return true;
        }
        else
        {
            zprintf1("DeviceMng read_object.%d.%d.%d type %d error!\n", parentid, childid, pointid, type);
            return false;
        }
    }
    else
    {
        zprintf4("DeviceMng find %d.%d.%d type %d error!\n", parentid, childid, pointid, type);
        zprintf4("DeviceMng read_object.nofind %d parentid: %d func %d!\n", item.key(), parentid,
                   SERIALIZE_FUNC(parentid, childid, pointid,type));
        return false;
    }
}

bool shm::shm_ctrl(int parentid, int childid, int pointid, double value)
{
    soutDataUnit data;
    int      ret;
    data.num      = 0;
    data.parentid = parentid;
    data.childid  = childid;
    data.pointid  = pointid;
    data.value    = value;
    data.state = 0;
    ret = m_pCtrlShm->write_send_data(data);
    if(ret == 0)
    {
        return true;
    }
    else
    {
        zprintf1("shm ctrl %d.%d.%d value %d error!\n", parentid, childid, pointid,value);
        return false;
    }
}

bool shm::shm_ctrl(uint8_t driid, int parentid, int childid, int pointid, double value)
{

    bool      ret;
    soutDataUnit data;


    data.num      = 2;
    data.parentid = parentid;
    data.childid  = childid;
    data.pointid  = pointid;
    data.value    = value;

    ret = m_pCtrlShm->write_send_data(data);
    if(ret == false)
    {
        zprintf1("shm ctrl %d.%d.%d value %d error!\n", parentid, childid, pointid,value);
        return false;
    }

    if (driid == 3)
    {
        gSemNum++;
    }
    return true;
}

bool shm::shm_write(int parentid, int childid, int pointid, int type, double value)
{
    mShmIndex::Iterator item;
    sDataUnit           temp;

    if (IndexMap.isEmpty())
    {
        zprintf1("shm write DeviceMng IndexMap.isEmpty!\n");
        return false;
    }
    item = IndexMap.find(SERIALIZE_FUNC(parentid, childid, pointid, type));
    if ((item != IndexMap.end()) && (item.key() == SERIALIZE_FUNC(parentid, childid, pointid, type)))
    {
        if (m_pDataShm->get_data(item.value(), temp) != 0)
        {
            zprintf1("DeviceMng shm_write get %d.%d.%d type %d error!\n", parentid, childid, pointid, type);
            return false;
        }
        temp.value = value;
        if(m_pDataShm->set_data(item.value(), temp) == 0)
            return true;
        else
        {
            zprintf1("shm_write pDataShm %d.%d.%d type %d error!\n", parentid, childid, pointid, type);
            return false;
        }
    }
    else
    {
        zprintf1("DeviceMng IndexMap not found parentid:%d childid:%d pointid: %d!\n",parentid,childid,pointid);
        return false;
    }
}

double* shm::shm_get_datapoint(int parentid, int childid, int pointid, int type)
{
    mShmIndex::Iterator item;
    sDataUnit     *     ptemp;

    if (IndexMap.isEmpty())
        return 0;

    item = IndexMap.find(SERIALIZE_FUNC(parentid, childid, pointid, type));
    if ((item != IndexMap.end()) && (item.key() == SERIALIZE_FUNC(parentid, childid, pointid, type)))
    {
        ptemp = m_pDataShm->getDataAddr(item.value());
        if (ptemp != 0)
            return &(ptemp->value);
        else
        {
            zprintf1("shm get datapoint  %d.%d.%d type %d error!\n", parentid, childid, pointid, type);
            return 0;
        }
    }
    else
        return 0;
}

bool shm::shm_readstate(char* pvalue, int len)
{
    int ret;
    ret = m_pStateShm->get_data(0, pvalue, len);
    return (ret == 0) ? true : false;

}

bool shm::shm_readstate(int childid, char* pvalue, int len)
{
    int ret;
    ret = m_pStateShm->get_data(childid, pvalue, len);
    if(ret == 0)
        return true;
    else
    {
        zprintf1("shm readstate childid %d error!\n", childid);
        return false;
    }
}

bool shm::shm_read_used(int parentid, int childid, int pointid, int type, int* pvalue)
{
    mShmIndex::Iterator item;
    sDataUnit           temp;

    if (IndexMap.isEmpty())
        return false;

    item = IndexMap.find(SERIALIZE_FUNC(parentid, childid, pointid, type));

    if ((item != IndexMap.end()) && (item.key() == SERIALIZE_FUNC(parentid, childid, pointid, type)))
    {
        if (m_pDataShm->get_data(item.value(), temp) == 0)
        {
            *pvalue = temp.state;
            return true;
        }
        else
        {
            zprintf1("shm get datapoint  %d.%d.%d type %d error!\n", parentid, childid, pointid, type);
            return false;
        }
    }
    else
        return false;
}

bool shm::shm_write_used(int parentid, int childid, int pointid, int type, int value)
{
    mShmIndex::Iterator item;
    sDataUnit           temp;

    if (IndexMap.isEmpty())
        return false;

    m_pDataShm->lock();
    item = IndexMap.find(SERIALIZE_FUNC(parentid, childid, pointid, type));
    if ((item != IndexMap.end()) && (item.key() == SERIALIZE_FUNC(parentid, childid, pointid, type)))
    {
        if (m_pDataShm->noblock_get_data(item.value(), temp) == 0)
        {
            temp.state = value;
            if(m_pDataShm->noblock_set_data(item.value(), temp) == 0)
            {
                m_pDataShm->unlock();
                return true;
            }
            else
            {
                m_pDataShm->unlock();
                zprintf1("shm_write_used set %d.%d.%d type %d error!\n", parentid, childid, pointid, type);
                return false;
            }
        }
        else
        {
            m_pDataShm->unlock();
            zprintf1("shm_write_used get %d.%d.%d type %d error!\n", parentid, childid, pointid, type);
            return false;
        }
    }
    else
    {
        m_pDataShm->unlock();
        return false;
    }
}
