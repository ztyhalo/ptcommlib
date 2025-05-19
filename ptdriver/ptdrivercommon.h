#ifndef PTDRIVERCOMMON_H
#define PTDRIVERCOMMON_H

#include <string.h>


#define CON_OUT_BUF_SIZE 256

typedef struct
{
    int num;
    int parentid;
    int childid;
    int pointid;
    double value;
    int  state;
}sDataUnit;     //主动上报共享内存

class cDataUnit
{
public:
    int num;
    int parentid;
    int childid;
    int pointid;
    double value;
    int  state;
public:
    cDataUnit(int  mark, int devid, int cid, int node, double val, int  st):
        num(mark),parentid(devid), childid(cid),pointid(node),value(val), state(st)
    {
        ;
    }
    cDataUnit()
    {
        memset(this, 0x00, sizeof(cDataUnit));
    }
    void set_all(const cDataUnit & val)
    {
        *this = val;
    }
    void set_val(double val)
    {
        value = val;
    }
    void set_val_state(double val, int st)
    {
        value = val;
        state = st;
    }
};

typedef struct
{
    int num;
    int parentid;
    int childid;
    int pointid;
    double value;
    int  state;
}soutDataUnit;


class PtDriverCommon
{
public:
    PtDriverCommon();
};

#endif // PTDRIVERCOMMON_H
