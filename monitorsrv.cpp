#include "public.h"
#include "commu.h"
#include "CreditTrade.h"
#include "Reserve.h"

static char workstr[4096]; // 資料暫存區，通常用於 log


TCreditTrade *pCreditTrade = NULL;
TReserve *pReserve = NULL;

//Data 物件 初始化(DB)
void Data_Init()
{
    pCreditTrade = new TCreditTrade(pDbConn);
    pReserve = new TReserve(pDbConn);

}
//Free Data 物件(DB)
void Data_Free()
{
    if(!pCreditTrade)
        delete pCreditTrade;
    if(!pReserve)
        delete pReserve;
}



int ProceCreditTade()
{
    pLog->INFO("Event","ProceCreditTade()");
    int result = 0;
    ReturnCode ret;
    if (strcmp(szType,"add") == 0)	
        {
            
            result =pCreditTrade->Read_Data_From_Json(szInputBuf);
            if(result!=1)
            {
                ret = ReturnCode::ERROR_JSON;
                Json_Return_Message(ret);
                return result;
            }
            result = pCreditTrade->Insert();
            if(result!=1)
                 ret = ReturnCode::ERROR_ADD_DATA;
            else
                 ret = ReturnCode::COMPLETE;
            delete pCreditTrade;
        }
    else
        ret = ReturnCode::ERROR_API_NONEXISTENCE;
    Json_Return_Message(ret);
    return result;
}

int ProceReserve()
{
    pLog->INFO("Event","ProceReserve()");
    int result = 0;
    ReturnCode ret;
    if (strcmp(szType,"add") == 0)	
        {
            
            result =pReserve->Read_Data_From_Json(szInputBuf);
            if(result!=1)
            {
                ret = ReturnCode::ERROR_JSON;
                Json_Return_Message(ret);
                return result;
            }
            result = pReserve->Insert();
            if(result!=1)
                 ret = ReturnCode::ERROR_ADD_DATA;
            else
                 ret = ReturnCode::COMPLETE;
            delete pReserve;
        }
    else
        ret = ReturnCode::ERROR_API_NONEXISTENCE;
    Json_Return_Message(ret);
    return result;
}






int Api_Event_Handler()
{
    pLog->Open();
    pLog->INFO("Event","Api_Event_Handler()");
    int result =0;
    json_object *jsonObj = json_tokener_parse(szInputBuf);
    if (jsonObj == NULL) 
    {
        ReturnCode ret = ReturnCode::ERROR_JSON;
        Json_Return_Message(ret);
        return 0;
    }

    Data_Init();

    strcpy(szFunc,  Get_Json_Data_L(jsonObj, "function", "",128));    
    strcpy(szType,  Get_Json_Data_L(jsonObj, "functype", "",128));
    if(strcmp("credit-trade",szFunc) == 0)
    {
      result = ProceCreditTade();
    }else if(strcmp("reserve",szFunc) == 0)
    {
        result = ProceReserve();
    }else
    {
        ReturnCode ret = ReturnCode::ERROR_API_NONEXISTENCE;
        Json_Return_Message(ret);
        return 0;
    }  
    Data_Free();
    return result;
}


//程式初始化讀取設定檔
int Api_Event_Initial(int argc, char **argv)
{
    sprintf(workstr,"Monitor AP (%s) Version 1.0", argv[0]);
    INFO(workstr) ;
    if ( argc != 5 )
    {
        ERROR("usage: Monitor <db_cfg> <server_port> <max_server> <log_name> [enter]" ) ;
        return 0 ;
    }

    strcpy(szConfigName, argv[1]);
    iServerPort = atoi(argv[2]);
    iMaxChildNums = atoi(argv[3]);
    strcpy(szLogName, argv[4]);
    //Loading DB config
    if(LoadConfig(szConfigName)!=1) 
    {
        sprintf(workstr,"Load Config fail : %s",szConfigName);
        ABEND(workstr);
        return 0;
    }
    return Check_Database();
}