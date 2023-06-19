#include "Log.h"
#include "util.h"

#define	MAX_DATA_BUF_SIZE	100000000

TLog::TLog(const char *szFileName)
{
    m_szLogBuff = (char*)malloc(sizeof(char)*MAX_DATA_BUF_SIZE);
	strcpy(m_szLogName,szFileName);
    m_pLogFd = NULL;
    m_iDate  = get_date_int();
}

TLog::~TLog()
{
    free(m_szLogBuff);
    Stop();
}

int TLog::Open()
{
    if (m_pLogFd) return 1;
    m_iPid   = getpid();

    // int iDate = get_date_int();
    char 	szNewName[168];
    sprintf(szNewName,"%s", m_szLogName);
    //DEBUG(szNewName);
    m_pLogFd = fopen(szNewName,"a");

    if(!m_pLogFd){
        char workstr[512];
        sprintf(workstr,"open file: %s",szNewName);
        Error("TLOG",workstr);
    }
    return 1;
}

int TLog::Stop()
{
    
    if (m_pLogFd) fclose(m_pLogFd);
    m_pLogFd = NULL;
    return 1;
}

int TLog::Check()
{
    //int iDate = get_date_int();
    //sprintf(m_szLogName,"%s_%d", m_szOrginName, iDate);
    //if (iDate != m_iDate)
    //{
    //char 	szNewName[168];
    //sprintf(szNewName,"%s_%d", m_szLogName, iDate);
    //rename(m_szLogName,szNewName);
    //}
    //m_iDate = iDate;
    return 1;
}
int TLog::mark(const char * szNeedMark,const char * szEndMark)
{

    // DEBUG(szNeedMark);
    char *mark = strstr(m_szLogBuff,szNeedMark);
    if(mark)
    {
    int inlen = strlen(szNeedMark);
    mark = mark + inlen;
    char test[32];
    sprintf(test,"len = %d",inlen);
    DEBUG(test);
    // DEBUG(end_check);        
    // mark = mark+len;
    DEBUG(mark);
    char * end_mark = strstr(mark,szEndMark) ;
    int len = end_mark-mark;
    // DEBUG(mark);
    char star[48]="";
    for(int i=0;i<len;i++)
    {
        strcat(star,"*");
    }
    strcat(star,szEndMark);
    memcpy(mark,star,len+1);


    }
    return 1;
}
// int TLog::check_log()
// {

//     char *pan = strstr(m_szLogBuff,"\"pan\":\"");
//     if (pan)
//     {  
//         char *pancheck = strstr(m_szLogBuff,"\"pan\":\"\"");
//         if(!pancheck)
//         {
//             pan = pan+13;
//             memcpy(pan,"******",6);
//         }      
//     }
//     char *card_pan = strstr(m_szLogBuff,"\"card_pan\":\"");
//     if (card_pan)
//     {
//         char *cardpancheck = strstr(m_szLogBuff,"\"card_pan\":\"\"");
//         if(!cardpancheck)
//         {
//             card_pan = card_pan+18;
//             memcpy(card_pan,"******",6);
            
//         } 
//     }
//     char *exp_date = strstr(m_szLogBuff,"\"exp_date\":\"");
//     if (exp_date)
//     {
//         char *exp_datecheck = strstr(m_szLogBuff,"\"exp_date\":\"\"");
//         if(!exp_datecheck)
//         {
//             exp_date = exp_date+12;
//             memcpy(exp_date,"****",4);
            
//         } 
//     }
//     char *cvv2 = strstr(m_szLogBuff,"\"cvv2\":\"");
//     if (cvv2)
//     {
//         char *cvv2check = strstr(m_szLogBuff,"\"cvv2\":\"\"");
//         if(!cvv2check)
//         {
//             cvv2 = cvv2+8;
//             memcpy(cvv2,"***",3);
            
//         } 
//     }
//     char *password = strstr(m_szLogBuff,"\"password\":");
//     if (password)
//     {
//         password =  password +12;
//         char *end_password = strstr(password,"\""); 
//         // if(!password)
//         // {
            
//         int len = end_password-password;
//         // char workstr[128];
//         char star[48]="";
//         for(int i=0;i<len;i++)
//         {
//             strcat(star,"*");
//         }
//         strcat(star,"\"");
//         memcpy(password,star,len+1);
//     }

//     char *password2 = strstr(m_szLogBuff,"password='");
//     if(password2)
//     {
//         password2 =  password2 +10;
//         char *end_password = strstr(password2,"'");
//         int len = end_password - password2;
//         char star[48]="";
//         for(int i=0;i<len;i++)
//         {
//             strcat(star,"*");
//         }
//         strcat(star,"'");
//         memcpy(password2,star,len+1);
//     }

//     char *sztoken = strstr(m_szLogBuff,"\"token\":");
//     if (sztoken)
//     {
//         sztoken =  sztoken +9;
//         char *end_token = strstr(sztoken,"\""); 
//         // if(!password)
//         // {
            
//         int len = end_token-sztoken;
//         // char workstr[128];
//         char star[48]="";
//         for(int i=0;i<len;i++)
//         {
//             strcat(star,"*");
//         }
//         strcat(star,"\"");
//         memcpy(sztoken,star,len+1);
//     }
    
//     return 1;
// }

int TLog::Write(const char *szMark, const char *szKey, const char *szMessage)
{
    char szDate[12],szTime[16];
    get_date_str("/",szDate);
    get_time_str(":",szTime);
    
    sprintf(m_szLogBuff,"[%08d-%s_%s-%s] [%s] %s",
            m_iPid,szDate,szTime,szMark,szKey,szMessage);
    // Scramble The PAN, Leave 6******4
    //-------------------------------------
    
    check_log(m_szLogBuff);
    
    //-------------------------------------
    fprintf(m_pLogFd,"%s\n", m_szLogBuff);
    fflush(m_pLogFd);
    return 1;
}

int TLog::Asert(const char *szKey, const char *szMessage)
{
    if (m_pLogFd == NULL) return 0;
    return this->Write("ASERT",szKey,szMessage);
}

int TLog::Trace(const char *szKey, const char *szMessage)
{
    if (m_pLogFd == NULL) return 0;
    return this->Write("TRACE",szKey,szMessage);
}

int TLog::Debug(const char *szKey, const char *szMessage)
{
    if (m_pLogFd == NULL) return 0;
    return this->Write("DEBUG",szKey,szMessage);
}

int TLog::INFO(const char *szKey, const char *szMessage)
{
    if (m_pLogFd == NULL) return 0;
    return this->Write("INFO",szKey,szMessage);
}


int TLog::Alert(const char *szKey, const char *szMessage)
{
    if (m_pLogFd == NULL) return 0;
    return this->Write("ALERT",szKey,szMessage);
}

int TLog::Error(const char *szKey, const char *szMessage)
{
    if (m_pLogFd == NULL) return 0;
    return this->Write("ERROR",szKey,szMessage);
}
