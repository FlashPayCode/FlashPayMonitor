#ifndef	__CREDITTRADE_H__
#define	__CREDITTRADE_H__

#include <libpq-fe.h>
#include "json.h"
#include "json_tokener.h"
#include "json_visit.h"

class TCreditTrade;
class TCreditTrade
{
public:
	TCreditTrade(PGconn *pDbConn);
	~TCreditTrade();

	int		Query();
	int		Insert();
	int		Delete();
	//int		Update();

	//int		Read_By_Id(const char * szMerId, const char *szAccount);
	void	Read_Data_By_Row(PGresult *pResult, int iRow);

	int		Read_Data_From_Json(const char *szJsonStr);
	void	Save_Data_To_Json(char *szJsonStr);

	int 		index;
	char        mer_id[64];
	char        order_no[64];
	char        cur[8];
	char        order_item[1024];
	char        pan[32];
	double      amt;
	int         install_period;
	char        cellphone[32];
	char        mail[256];
	char        send_bank[8];
	char		pan_bank[8];
	char		trade_date[32];
	char		remark[512];
	char		c_time[32];
	char		m_time[32];

	TCreditTrade *	m_pNext;

private:
	int		Read_Data_By_Sql();
	int		Sql_Exec();
	PGconn	*	m_pDbConn;
	char	*	m_szSqlStr;
};

class TCreditTradeList
{
public:
	TCreditTradeList(PGconn *pDbConn);
	~TCreditTradeList();

	void		Clear();
	int			List();
	
	TCreditTrade *	m_pCreditTradeHead;
	TCreditTrade *	m_pCreditTradeTail;
	int		m_iCreditTradeNums;

private:
	void		Init();
	int		Read_Data_By_Sql();

	PGconn	*	m_pDbConn;
	char	*	m_szSqlStr;
};

#endif
