#include "header.h"
#include "public.h"
#include <libpq-fe.h>
#include "json.h"
#include "json_tokener.h"
#include "json_visit.h"
#include "util.h"
#include "commu.h"
#include "StopWatch.h"
#include "child.h"
#include "aes256.h"


// main, child process
//---------------------------------------------------

extern int Api_Event_Handler();

static char workstr[1024];

//利用AES256解密
int pw_decode(char * pw)
{
    char key[64];
    char iv[32];
    strcpy(key,"X3XRX0P9MBFGEC5KMPPOLO1DHXKECH4G");
    strcpy(iv, "H37QOB9JFZOSKX7J");
    char szTempBuf[64];
    int pwBufLen = hex2bin(pw,(unsigned char*)szTempBuf);

    decrypt((unsigned char *)szTempBuf, pwBufLen, (unsigned char *)key,
            (unsigned char *)iv, (unsigned char *)pw);

    pw[pwBufLen] = '\0';
    int actuallen=pwBufLen;
    int i=0,count=0;
    for(i=0;i<actuallen;i++)
    {
        if(pw[i] == ' ') count++;
        if(pw[i] <32 || pw[i]>126) break;
    }
    actuallen = i;
    return actuallen;
}

// Load Dbsrv Config File
// Store Connection String To szDbConnStr
// 抓取DB連線設定檔
//---------------------------------------
int LoadConfig(char *szFileName)
{
  FILE *fd = fopen(szFileName,"r");
  char buf[256];
  if (fgets(buf,256,fd))
  {
    int iLen = strlen(buf);
    if (buf[iLen-1] <= 0x20) buf[iLen-1] = '\0';
    if (buf[iLen-2] <= 0x20) buf[iLen-2] = '\0';
    char * srart_pswd;
    srart_pswd = strstr(buf,"password='");
    srart_pswd = srart_pswd + 10;
    char * end_pswd;
    end_pswd = strstr(srart_pswd,"'");
    *end_pswd='\0';
    if(srart_pswd[0] != '\0')
    {
        char pw[256];
        strcpy(pw,srart_pswd);
        int pwlen = pw_decode(pw);
        memcpy(srart_pswd,"",pwlen+1);
        memcpy(srart_pswd,pw,pwlen);
        memcpy(srart_pswd+pwlen,"'",1);
        memcpy(srart_pswd+pwlen+1,"",1);
    }
    else
    {
        pLog->Error("DB_CFG","db config needs password");
    }
    strcpy(szDbConnStr,buf);
  }
  fclose(fd);
  return 1;
}


//測試DB連線是否正常
int Check_Database()
{
    pDbConn = PQconnectdb(szDbConnStr);
    if (PQstatus(pDbConn) != CONNECTION_OK)
    {
        ERROR("Database Connect Failed");
        sprintf(workstr, "pgsql-message = %s ", PQerrorMessage(pDbConn));
        ABEND(workstr);
        return 0;
    }
    INFO("DB successfully connected");
    return 1;
}

//設定Client端DB資料庫字元編碼
void Set_Db_Client_Encode(const char *szCodeType)
{
    
sprintf(workstr,"set client_encoding to '%s';",szCodeType);
INFO(workstr);
    strcpy(szSqlStr,workstr);
    PGresult * res = PQexec(pDbConn,szSqlStr);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        sprintf(workstr, "PQexec(), msg = [%s]", PQerrorMessage(pDbConn));
        ERROR(workstr);
    }
    PQclear(res);
}
//復原DB Client 字元編碼
void Reset_Db_Client_Encode()
{
    INFO("reset client_encoding;");
    strcpy(szSqlStr,"reset client_encoding;");
    PGresult * res = PQexec(pDbConn,szSqlStr);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        sprintf(workstr, "PQexec(), msg = [%s]", PQerrorMessage(pDbConn));
        ERROR(workstr);
    }
    PQclear(res);
}

void child_process()
{
    struct sockaddr_in sin;
    int pos;
    int	len = sizeof(sin);

    pLog->Open();
    //不在子線程去載入DB設定檔，改成主程式啟動時
    /*if (LoadConfig(szConfigName) <= 0)
    {
        pLog->Error("CONN", "database config is empty.");
        exit(0);
    }
    int slen = strlen(szDbConnStr);*/
    pDbConn = PQconnectdb(szDbConnStr);
    if (PQstatus(pDbConn) != CONNECTION_OK)
    {
        pLog->Error("CONN", "Database Connect Failed");
        sprintf(workstr, "pgsql-message = %s\n", PQerrorMessage(pDbConn));
        pLog->Error("CONN", workstr);
        exit(0);
    }
    Init_Data_Buffer();
    while (true)
    {
        pLog->Stop();
        iClientFd = accept(iServerFd, (struct sockaddr *)&sin, (socklen_t*)&len);
        if (iClientFd < 0) continue;
        TStopWatch StopWatch;
        StopWatch.Start();

        /* check ip addr */
        // 確認client 連線
        get_remote_host(iClientFd,szRemoteHost);
        if (szRemoteHost[0] == '\0')
        {
            INFO("Tcp/Ip accepted, but remote ip addr is Empty.");
            close(iClientFd);
            continue;
        }    
        pLog->Open();
        /* read eai client message */
        // 讀取client 送來的訊息
        pos = Read_Input();
        if (pos <= 0)
        {
            if(pos == -1)
            {
                close(iClientFd);
                continue;
            }
            sprintf(workstr, "read client failed, client ip is '%s'", szRemoteHost);
            pLog->Error("CONN", workstr);
        }
        else
        {
            pLog->Trace("CONN","Connect Accepted, Service Begin");
            sprintf(workstr, "Tcp/Ip accepted, remote ip = '%s'.", szRemoteHost);
            INFO(workstr);         
            Api_Event_Handler();
        }
        //回覆訊息
        Send_Reply();   
        close(iClientFd);
        StopWatch.Stop();
        sprintf(workstr,"transaction done, Elapsed Time = %s seconds", StopWatch.TimeStr());
        pLog->Trace("CONN", workstr);
        pLog->Asert("CONN", "Clinet Request Finished");
    }
}

void child_execute()
{
    int i_child_id;
    int	i_child_fi[2];
    int	i_child_fo[2];

    pipe(i_child_fi);
    pipe(i_child_fo);

    i_child_id = fork();
    if (i_child_id) 
    {
        close(i_child_fi[1]);
        close(i_child_fo[0]);
        add_child(i_child_id,i_child_fi[0],i_child_fo[1]);
        return;
    }

    pLog->Open();
    sprintf(workstr,"child fork successful, pid = %d.", getpid());
    pLog->Trace("CONN", workstr);
    pLog->Stop();

    close(i_child_fi[0]);
    close(i_child_fo[1]);
    iChildInFd  = i_child_fo[0];
    iChildOutFd = i_child_fi[1];
    free_child_manager();
    child_process();
    close(iServerFd);
    close(iChildInFd);
    close(iChildOutFd);

    pLog->Open();
    pLog->Asert("CONN", "service stoped.");
    pLog->Stop();
    exit(0);
}

/* main program.
-------------------------------------------------*/
void main_process()
{
    while (g_p_child_free) child_execute();
    while (1)
    {
        int i_count = update_child_status();
        if (i_count == 0)
        {
            /* Check Log For Date Change */
            pLog->Check();
            usleep(1000);
            continue;
        }

        while (g_p_child_free)
        {
            child_execute();
        }
    }
    pLog->Open();
    pLog->Asert("CONN", "service stoped.");
    pLog->Stop();

}
