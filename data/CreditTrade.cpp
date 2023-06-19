#include "header.h"
#include "commu.h"
#include "util.h"
#include "CsvInput.h"
#include "CreditTrade.h"
#include "sha256.h"
static char workstr[2048];

//TCreditTrade物件初始化
TCreditTrade::TCreditTrade(PGconn *pDbConn)
{
	index           = 0;
	mer_id[0]       = '\0';
	order_no[0]    = '\0';
	cur[0]          = '\0';
	order_item[0]   = '\0';
	pan[0]          = '\0';
    amt             = 0.00;
	install_period  = 0;
	cellphone[0]    = '\0';
	mail[0]         = '\0';
	send_bank[0]    = '\0';
	pan_bank[0]     = '\0';
	trade_date[0]   = '\0';
	remark[0]       = '\0';
	c_time[0]       = '\0';
	m_time[0]       = '\0';

    m_pNext	= NULL;
    m_pDbConn	= pDbConn;
    m_szSqlStr	= (char*)malloc(8192);
}

//TCreditTrade生命週期結束時釋放記憶體空間
TCreditTrade::~TCreditTrade()
{
    free(m_szSqlStr);
}

//Read json TO ObJect
int TCreditTrade::Read_Data_From_Json(const char *szJsonStr)
{
    INFO("TCreditTrade::Read_Data_From_Json");
    json_object *obj = json_tokener_parse(szJsonStr);
    if (obj == NULL) return 0;
    char temp[16];
    strcpy(mer_id,      Get_Json_Data_L(obj,"mer_id", mer_id, 32));
    strcpy(order_no,	Get_Json_Data_L(obj,"order_no",order_no,64));
    strcpy(cur,	        Get_Json_Data_L(obj,"cur",cur,8));
    strcpy(order_item,	Get_Json_Data_L(obj,"order_item",order_item,1024));
    strcpy(pan,	        Get_Json_Data_L(obj,"pan",pan,32));
    strcpy(temp,	    Get_Json_Data_L(obj,"amt",temp,16));
    amt = atoi(temp);
    if (amt==0) return 0;
    strcpy(temp,	    Get_Json_Data_L(obj,"install_period",temp,8));
    strcpy(cellphone,	Get_Json_Data_L(obj,"cellphone",cellphone,32));
    strcpy(mail,	    Get_Json_Data_L(obj,"mail",mail,256));
    strcpy(send_bank,	Get_Json_Data_L(obj,"send_bank",send_bank,8));
    strcpy(pan_bank,	Get_Json_Data_L(obj,"pan_bank",pan_bank,8));
    strcpy(trade_date,	Get_Json_Data_L(obj,"trade_date",trade_date,32));
    strcpy(remark,	    Get_Json_Data_L(obj,"remark",remark,512));
    json_object_put(obj);
    return 1;
}

//Object to json
void TCreditTrade::Save_Data_To_Json(char *szJsonStr)
{
    sprintf(szJsonStr,"{\"mer_id\":\"%s\",\"order_no\":\"%s\","
                      "\"cur\":\"%s\",\"order_item\":\"%s\","
                      "\"pan\":\"%s\",\"install_period\":%d,"
                      "\"cellphone\":\"%s\",\"mail\":\"%s\","
                      "\"send_bank\":\"%s\",\"pan_bank\":\"%s\","
                      "\"trade_date\":\"%s\",\"remark\":\"%s\",\"amt\":%.3f,"
                      "\"c_time\":\"%s\",\"m_time\":\"%s\"}",
                      mer_id,order_no,cur,order_item,pan,install_period,cellphone
                      ,mail,send_bank,pan_bank,trade_date,remark,amt,c_time,m_time);
}

//read DB data to object
void TCreditTrade::Read_Data_By_Row(PGresult *pResult, int iRow)
{
    char temp[16];
    int icolumn=0;
    strcpy(mer_id,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(order_no,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(cur,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(order_item,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(pan,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(temp,PQgetvalue(pResult,iRow,icolumn++));
    install_period = atoi(temp);
    strcpy(cellphone,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(mail,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(send_bank,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(pan_bank,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(trade_date,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(remark,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(c_time,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(m_time,	PQgetvalue(pResult,iRow,icolumn++));
    strcpy(temp,PQgetvalue(pResult,iRow,icolumn++));
    amt = atoi(temp);
}

// Read CreditTrade Data From Database, by m_szSqlStr[]
// On Success, return 1, else return 0
//------------------------------------------------
int TCreditTrade::Read_Data_By_Sql()
{
    PGresult * res = PQexec(m_pDbConn,m_szSqlStr);
    sprintf(workstr,"TCreditTrade Read_Data_By_Sql: %s", m_szSqlStr);
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

int TCreditTrade::Sql_Exec()
{
    PGresult * res = PQexec(m_pDbConn,m_szSqlStr);
    sprintf(workstr,"TCreditTrade Sql_Exec: %s", m_szSqlStr);
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


int TCreditTrade::Query()
{
    sprintf(m_szSqlStr, "SELECT mer_id, order_no, cur, order_item, pan, "
                            "install_period, cellphone, mail, send_bank, "
                            "pan_bank, trade_date, remark, c_time, m_time , amt "
                        "FROM public.credit_trade WHERE mer_id ='%s' AND order_no ='%s';",
                        mer_id,order_no);
    return Read_Data_By_Sql();
}

int TCreditTrade::Insert()
{
    sprintf(m_szSqlStr, "INSERT INTO public.credit_trade( "
                            "mer_id, order_no, cur, order_item, "
                            "pan, install_period, cellphone, mail, send_bank, pan_bank,  "
                            "trade_date, remark, m_time , amt )"
                        "VALUES ('%s', '%s', '%s', '%s', '%s', %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s' ,%.3f);",
                        mer_id,order_no,cur,order_item,pan,install_period,
                        cellphone,mail,send_bank,pan_bank,trade_date,remark,current_datetime_nossec(),amt);
    return Sql_Exec();
}

int TCreditTrade::Delete()
{
    sprintf(m_szSqlStr, "DELETE FROM  public.credit_trade WHERE mer_id ='%s' AND order_no ='%s'; ", mer_id,order_no);
    return Sql_Exec();
}


// CreditTrade List Implementations:
//=============================
TCreditTradeList::TCreditTradeList(PGconn *pDbConn)
{
    Init();
    m_szSqlStr	= (char*)malloc(8192);
    m_pDbConn	= pDbConn;
}

TCreditTradeList::~TCreditTradeList()
{
    free(m_szSqlStr);
}

void TCreditTradeList::Init()
{
    m_pCreditTradeHead	= NULL;
    m_pCreditTradeTail	= NULL;
    m_iCreditTradeNums	= 0;
}

void TCreditTradeList::Clear()
{
    TCreditTrade * pCreditTrade = m_pCreditTradeHead;
    while (pCreditTrade)
    {
        TCreditTrade *pCreditTradeTmp = pCreditTrade->m_pNext;
        delete pCreditTrade;
        pCreditTrade = pCreditTradeTmp;
    }
    Init();
}

int TCreditTradeList::Read_Data_By_Sql()
{
    PGresult * res = PQexec(m_pDbConn,m_szSqlStr);
    sprintf(workstr,"TCreditTradeList Read_Data_By_Sql: %s", m_szSqlStr);
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
        TCreditTrade *pCreditTrade = new TCreditTrade(m_pDbConn);
        pCreditTrade->Read_Data_By_Row(res,i);
        if (m_pCreditTradeHead == NULL)
             m_pCreditTradeHead = pCreditTrade;
        else m_pCreditTradeTail->m_pNext = pCreditTrade;
        m_pCreditTradeTail = pCreditTrade;
        m_iCreditTradeNums++;
    }
    PQclear(res);
    return m_iCreditTradeNums;
}

int TCreditTradeList::List()
{
    Clear();
    sprintf(m_szSqlStr, 
    "SELECT mer_id, order_no, cur, order_item, pan, "
                            "install_period, cellphone, mail, send_bank, "
                            "pan_bank, trade_date, remark, c_time, m_time , amt "
                        "FROM public.credit_trade;");
    return Read_Data_By_Sql();
}