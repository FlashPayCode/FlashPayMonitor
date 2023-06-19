#ifndef	__RESERVE_H__
#define	__RESERVE_H__

#include <libpq-fe.h>
#include "json.h"
#include "json_tokener.h"
#include "json_visit.h"

class TReserve;
class TReserve
{
public:
	TReserve(PGconn *pDbConn);
	~TReserve();

	int		Query();
	int		Insert();
	int		Delete();
	//int		Update();

	//int		Read_By_Id(const char * szMerId, const char *szAccount);
	void	Read_Data_By_Row(PGresult *pResult, int iRow);

	int		Read_Data_From_Json(const char *szJsonStr);
	void	Save_Data_To_Json(char *szJsonStr);

	int 		reserve_id;
	char        mer_id[64];
	char        order_no[64];
	char        reserve_type[4];
	double      reserve_amt;
	char        auth_id_resp[6];
	char		c_time[32];
	char		m_time[32];
	TReserve *	m_pNext;

private:
	int		Read_Data_By_Sql();
	int		Sql_Exec();
	PGconn	*	m_pDbConn;
	char	*	m_szSqlStr;
};

class TReserveList
{
public:
	TReserveList(PGconn *pDbConn);
	~TReserveList();

	void		Clear();
	int			List();
	
	TReserve *	m_pReserveHead;
	TReserve *	m_pReserveTail;
	int		m_iReserveNums;

private:
	void		Init();
	int		Read_Data_By_Sql();

	PGconn	*	m_pDbConn;
	char	*	m_szSqlStr;
};

#endif
