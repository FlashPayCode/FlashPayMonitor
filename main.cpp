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
#include "Log.h"

// #define	RPL_MAX_DATA_BUF_SIZE	200000000
#define	MAX_DATA_BUF_SIZE	100000000
#define	MAX_SQL_BUF_SIZE	8192

//紀錄Log 物件
char		szLogName[128]	= "";
TLog	*	pLog		= NULL;

// Paremeter Definitions 服務器設定變數
//-------------------------------------
int		iServerPort	= 0;         //service port
char		szConfigName[128];   //設定黨參數
char		szDbServerHost[128]; //資料庫IP
char		szServerHost[128];   //service IP
int		iDbServerPort	= 0;
int		iMaxChildNums	= 10; //設定啟動幾個子線程

// Communication Parameters
//-------------------------------------
int		iServerFd	= 0;
int		iClientFd	= 0;

// Child use these 2 file id communication with parent.
//-----------------------------------------------------
int		iChildInFd	= 0;	// pipe from parent
int		iChildOutFd	= 0;	// pipe to parent

// Input Data Buffer 接收資料變數
//-------------------------------------
int		iInputLen 	= 0;
char	*	szInputBuf	= NULL;

// Reply Data Buffer 資料回覆變數
//-------------------------------------
int		iReplyLen	= 0;
byte	*	byReplyBuf	= NULL;
char	*	szReplyBuf	= NULL;

// Database Connection 資料庫物件與設定
//-------------------------------------
char		szDbConnStr[512];
PGconn	*	pDbConn  = NULL;
char	*	szSqlStr = NULL;
int		    iSqlStrLen 	= 0;
char	*	szSqlStrBuf	= NULL;

// Local Variables 內部使用全域變數
//-------------------------------------
static
char		workstr[1024];
int		    iPid		= 0;
char		szRemoteHost[64];

//事件處裡判斷
char        szFunc[128] ;
char        szType[128] ;

//配置 SQL語法  與  接收送出 字串 記憶體大小
int Init_Data_Buffer()
{
    szInputBuf = (char*)malloc(sizeof(char)*MAX_DATA_BUF_SIZE);
    byReplyBuf = (byte*)malloc(sizeof(char)*MAX_DATA_BUF_SIZE);
    szReplyBuf = (char*)byReplyBuf + 8;
    szSqlStrBuf =  (char*)malloc(sizeof(char)*MAX_DATA_BUF_SIZE);
    szSqlStr   = (char*)malloc(sizeof(char)*MAX_SQL_BUF_SIZE);
    return 1;
}

// Send Query Message / Reply Message
// 處理好回傳資料
//----------------------------------------
int Send_Reply(int iFd)
{
    pLog->INFO("PROC","Send_Reply");
    char szTemp[12];
    iReplyLen = strlen(szReplyBuf);
    sprintf(szTemp,"%08d",iReplyLen);
    memcpy(byReplyBuf,szTemp,8);
    sprintf(workstr,"REPLY len=%d",iReplyLen);
    pLog->INFO("REPLY",workstr);
    return write_blk(iFd,byReplyBuf,iReplyLen+8,5);
}

int Send_Reply()
{
    return Send_Reply(iClientFd);
}

// 組成錯誤訊JSON格式
//----------------------------------------
void Json_Return_Message(ReturnCode returnCode)
{
  sprintf(szReplyBuf,"{\"returnCode\":%ld,\"returnDesc\":\"%s\"}",
          static_cast<long>(returnCode),ReturnMsg::getReturnMsg(returnCode) );
    pLog->INFO("szReplyBuf",szReplyBuf);
}

void Json_Return_Message(const char *szMessage)
{
  sprintf(szReplyBuf,"{\"returnCode\":0,\"returnDesc\":\"%s\"}",
           szMessage );
    pLog->INFO("szReplyBuf",szReplyBuf);
}

// Read Query Message
// Return Message Buffer Length (must > 0)
// 接收資料處理
//----------------------------------------
int Read_Input(int iFd, int iTimeOut)
{
    char szLen[10];
    int  iLen;
    iLen = read_blk(iFd, (byte*)szLen,8,iTimeOut);
    //健康檢查Request
    if(strcmp(szLen,"healthch") == 0)
        return -1;

    if (iLen != 8) return 0;
        szLen[8] = '\0';
        
    iInputLen = atoi(szLen);
    pLog->INFO("PROC","Read_Input");
    sprintf(workstr,"INPUT len=%d",iInputLen);
    pLog->INFO("INPUT",workstr);   

    if (iInputLen == 0) 
        return 0;

    iLen = read_blk(iFd, (byte*)szInputBuf, iInputLen, iTimeOut);
    szInputBuf[iLen] = '\0';
    pLog->INFO("InputBuf",szInputBuf);

    if (iLen != iInputLen) 
        return 0;

    szInputBuf[iLen] = '\0';
    return iLen;
}


int Read_Input(int iFd)
{
    return Read_Input(iFd,2);
}

int Read_Input()
{
    return Read_Input(iClientFd,2);
}

// update child status
//--------------------
// read child status and check if child stoped
// 主線程與子線程處理
//--------------------------------------------
int update_child_status()
{
    int i_status = read_child_status(1);
    int i_count  = 0;

    if (i_status > 0)
    {
        child_t * p_child = g_p_child_list;
        pLog->Open();
        while (p_child)
        {
            child_t *p_temp = p_child->p_next;
            if (p_child->p_child_fd->revents)
            {
                sprintf(workstr,"detected child process #%d stoped, free the resource.", p_child->i_child_id);
                pLog->Asert("MAIN", workstr);
                del_child(p_child);
                i_count++;
                if (i_count == i_status) break;
            }
            p_child = p_temp;
        }
        pLog->Stop();
    }
    return i_count;
}

extern int Api_Event_Initial(int argc, char **argv);

int main(int argc, char **argv)
{
    int rc = Api_Event_Initial(argc,argv);
    if (rc <= 0) return 0;

    pLog = new TLog(szLogName);//TLog 物件在子線程使用
    pLog->Open();
    sprintf(workstr,"create internet service on TCP:%d", iServerPort);
    pLog->Trace("MAIN", workstr);
    sprintf(workstr,"szConfigName=%s,iServerPort=%d,iMaxChildNums=%d,szLogName=%s",szConfigName,iServerPort,iMaxChildNums,szLogName);
    pLog->Trace("check", workstr);
    if(iMaxChildNums == 0)
    {
        return 0;
    }
    iServerFd = server_prepare(iServerPort);
    if (iServerFd < 0)
    {
        sprintf(workstr,"service port TCP:%d create failed.",iServerPort);
        pLog->Error("MAIN", workstr);
        pLog->Stop();
        exit(0);
    }
    server_listen(iServerFd,64);
    signal(SIGPIPE,SIG_IGN) ;
    signal(SIGCLD, SIG_IGN) ;

    iPid = getpid();
    init_child_manager(iMaxChildNums);
    pLog->Stop();

    extern void main_process();
    
    main_process();

    return 0;
}
