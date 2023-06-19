#include "header.h"
#include "commu.h"
#include "util.h"
#include "CsvInput.h"
#include "Reserve.h"
#include "sha256.h"
static char workstr[2048];

//TReserve物件初始化
TReserve::TReserve(PGconn *pDbConn)
{
	reserve_id       = 0;
	mer_id[0]        = '\0';
	order_no[0]      = '\0';
    reserve_type[0]  = '\0';
    reserve_amt      = 0;
	auth_id_resp[0]  = '\0';
	c_time[0]        = '\0';
	m_time[0]        = '\0';

    m_pNext	= NULL;
    m_pDbConn	= pDbConn;
    m_szSqlStr	= (char*)malloc(8192);
}

//TReserve生命週期結束時釋放記憶體空間
TReserve::~TReserve()
{
    free(m_szSqlStr);
}

//Read json TO ObJect
int TReserve::Read_Data_From_Json(const char *szJsonStr)
{
    INFO("TReserve::Read_Data_From_Json");
    json_object *obj = json_tokener_parse(szJsonStr);
    if (obj == NULL) return 0;
    char temp[16];
    strcpy(mer_id,      Get_Json_Data_L(obj,"mer_id", mer_id, 32));
    strcpy(order_no,	Get_Json_Data_L(obj,"order_no",order_no,64));
    strcpy(reserve_type,	Get_Json_Data_L(obj,"reserve_type",reserve_type,4));
    strcpy(temp,	    Get_Json_Data_L(obj,"reserve_amt",temp,16));
    strcpy(auth_id_resp,	Get_Json_Data_L(obj,"auth_id_resp",auth_id_resp,6));
    reserve_amt = atoi(temp);
    if (reserve_amt==0) 
    {
        ERROR("TReserve::Read_Data_From_Json  reserve_amt can't be 0");
        return 0;
    }
    json_object_put(obj);
    return 1;
}

//Object to json
void TReserve::Save_Data_To_Json(char *szJsonStr)
{
    sprintf(szJsonStr,"{\"mer_id\":\"%s\",\"order_no\":\"%s\","
                      "\"reserve_type\":\"%s\",\"reserve_amt\":%.3f,"
                      "\"auth_id_resp\":\"%s\",\"c_time\":\"%s\",\"m_time\":\"%s\"}",
                      mer_id,order_no,reserve_type,reserve_amt,auth_id_resp,c_time,m_time);
}

//read DB data to object
void TReserve::Read_Data_By_Row(PGresult *pResult, int iRow)
{
    char temp[16];
    int icolumn=0;
    strcpy(mer_id,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(order_no,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(reserve_type,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(temp,PQgetvalue(pResult,iRow,icolumn++));
    reserve_amt = atoi(temp);
    strcpy(auth_id_resp,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(c_time,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(m_time,	PQgetvalue(pResult,iRow,icolumn++));
}

// Read CreditTrade Data From Database, by m_szSqlStr[]
// On Success, return 1, else return 0
//------------------------------------------------
int TReserve::Read_Data_By_Sql()
{
    PGresult * res = PQexec(m_pDbConn,m_szSqlStr);
    sprintf(workstr,"TReserve Read_Data_By_Sql: %s", m_szSqlStr);
    INFO(workstr);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        ERROR(m_szSqlStr);
        sprintf(workstr, "PQexec(), msg = [%s]", PQerrorMessage(m_pDbConn));
        ERROR(workstr);
        PQclear(res);
        return 0;
    }
    int ntuples = PQntuples(res);;  
    if (ntuples != 1)
    {
        PQclear(res);
        return 0;
    }        
  Read_Data_By_Row(res,0);
  PQclear(res);
  return ntuples;
}

int TReserve::Sql_Exec()
{
    PGresult * res = PQexec(m_pDbConn,m_szSqlStr);
    sprintf(workstr,"TReserve Sql_Exec: %s", m_szSqlStr);
    INFO(workstr);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        ERROR(m_szSqlStr);
        sprintf(workstr, "PQexec(), msg = [%s]", PQerrorMessage(m_pDbConn));
        ERROR(workstr);
        PQclear(res);
        return 0;
    }
    PQclear(res);
    return 1;
}


int TReserve::Query()
{
    sprintf(m_szSqlStr, "SELECT "
                           " mer_id, "
                           " order_no, "
                           " reserve_type, "
                           " reserve_amt, "
                           " auth_id_resp, "
                           " c_time, "
                           " m_time "
	                    " FROM public.reserve "
                        " WHERE mer_id ='%s' AND order_no ='%s';",
                        mer_id,order_no);
    return Read_Data_By_Sql();
}

int TReserve::Insert()
{
    sprintf(m_szSqlStr, "INSERT INTO public.reserve("
	                    " mer_id, order_no, reserve_type, reserve_amt, auth_id_resp)"
	                    " VALUES ( '%s', '%s', '%s', %.3f, '%s');",
                        mer_id,order_no,reserve_type,reserve_amt,auth_id_resp);
    return Sql_Exec();
}

int TReserve::Delete()
{
    sprintf(m_szSqlStr, "DELETE FROM  public.reserve WHERE mer_id ='%s' AND order_no ='%s'; ", mer_id,order_no);
    return Sql_Exec();
}


// CreditTrade List Implementations:
//=============================
TReserveList::TReserveList(PGconn *pDbConn)
{
    Init();
    m_szSqlStr	= (char*)malloc(8192);
    m_pDbConn	= pDbConn;
}

TReserveList::~TReserveList()
{
    free(m_szSqlStr);
}

void TReserveList::Init()
{
    m_pReserveHead	= NULL;
    m_pReserveTail	= NULL;
    m_iReserveNums	= 0;
}

void TReserveList::Clear()
{
    TReserve * pReserve = m_pReserveHead;
    while (pReserve)
    {
        TReserve *pReserveTmp = pReserve->m_pNext;
        delete pReserve;
        pReserve = pReserveTmp;
    }
    Init();
}

int TReserveList::Read_Data_By_Sql()
{
    PGresult * res = PQexec(m_pDbConn,m_szSqlStr);
    sprintf(workstr,"TReserveList Read_Data_By_Sql: %s", m_szSqlStr);
    INFO(workstr);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        sprintf(workstr, "PQexec(), msg = [%s]", PQerrorMessage(m_pDbConn));
        ERROR(workstr);
        PQclear(res);
        return 0;
    }
    int i, ntuples = PQntuples(res);;  
    if (ntuples < 1)
    {
        INFO("Empty Data Set.\n");
        PQclear(res);
        return 0;
    }

    for (i = 0; i < ntuples; i++)
    {
        TReserve *pReserve = new TReserve(m_pDbConn);
        pReserve->Read_Data_By_Row(res,i);
        if (m_pReserveHead == NULL)
             m_pReserveHead = pReserve;
        else m_pReserveTail->m_pNext = pReserve;
        m_pReserveTail = pReserve;
        m_iReserveNums++;
    }
    PQclear(res);
    return m_iReserveNums;
}

int TReserveList::List()
{
    Clear();
    sprintf(m_szSqlStr, 
    "SELECT mer_id, "
          "  order_no, "
          "  reserve_type, "
          "  reserve_amt, "
          "  auth_id_resp, "
          "  c_time, "
          "  m_time "
	    " FROM public.reserve;");
    return Read_Data_By_Sql();
}