#include "header.h"
#include "StopWatch.h"

TStopWatch::TStopWatch()
{
}

TStopWatch::~TStopWatch()
{
}

void TStopWatch::Start()
{
    gettimeofday(&ts_all, (struct timezone*)0);
}

int TStopWatch::Stop()
{
    struct timeval sys_ts;
    gettimeofday(&sys_ts, (struct timezone*)0) ;
    m_iTime = (sys_ts.tv_sec - ts_all.tv_sec) * 1000000 + sys_ts.tv_usec - ts_all.tv_usec;
    return m_iTime;
}

char * TStopWatch::TimeStr()
{
    sprintf(m_szTimeStr,"%d.%06d", m_iTime/1000000, m_iTime%1000000);
    return m_szTimeStr;
}
