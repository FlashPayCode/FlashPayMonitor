#ifndef	__LOG_H__
#define	__LOG_H__

#include "header.h"
#include "util.h"

class TLog
{
public:
        TLog(const char *szFileName);
        ~TLog();

        int	Open();
        int	Stop();

        int	Asert(const char *szKey, const char *szMessage);
        int	Trace(const char *szKey, const char *szMessage);
        int	Debug(const char *szKey, const char *szMessage);
        int	INFO(const char *szKey, const char *szMessage);
        int	Alert(const char *szKey, const char *szMessage);
        int	Error(const char *szKey, const char *szMessage);

        int	Check();
        // int     check_log();
        int     mark(const char * szNeedMark,const char * szEndMark);

private:
        int	Write(const char *szMark, const char *szKey, const char *szMessage);

        char	m_szLogName[128];
        char *	m_szLogBuff;
        FILE *	m_pLogFd;
        int	m_iPid, m_iDate;
};

#endif
