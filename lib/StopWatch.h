#ifndef	__STOP_WATCH_H__
#define	__STOP_WATCH_H__

class TStopWatch
{
public:
        TStopWatch();
        ~TStopWatch();
        
        void		Start();
        int		Stop();
        char	*	TimeStr();

private:
        struct timeval	ts_all;
        int		m_iTime;
        char		m_szTimeStr[32];
};

#endif
