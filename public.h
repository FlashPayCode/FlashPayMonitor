#ifndef	__API_PUBLIC_H__
#define	__API_PUBLIC_H__

#include "header.h"
#include <libpq-fe.h>
#include "json.h"
#include "json_tokener.h"
#include "json_visit.h"
#include "util.h"
#include "CsvInput.h"
#include "sha256.h"
#include "Log.h"
#include <map>
//#include "Errorlog.h"

// main_cc is dynamic child processes
// main_db is fixed number child processes.
//-----------------------------------------
#define	MAIN_CC	1
#define	MAIN_DB	2

extern	char		szLogName[];

// Paremeter Definitions
//-------------------------------------
extern	int		iServerPort;
extern	char		szConfigName[];
extern	char		szDbServerHost[];
extern	char		szServerHost[];
extern	int		iDbServerPort;
extern	int		iMaxChildNums;

// Input A Json Document (Object)
//-------------------------------------
extern	json_object *	my_object;

// Communication Parameters
//-------------------------------------
extern	int		iServerFd;
extern	int		iClientFd;
extern	char		szRemoteHost[];

// Child use these 2 file id communication with parent.
//-----------------------------------------------------
extern	int		iChildInFd;	// pipe from parent
extern	int		iChildOutFd;	// pipe to parent

// Input Data Buffer
//-------------------------------------
extern	int		iInputLen;
extern	char	*	szInputBuf;

// Output A Json Document (String)
//-------------------------------------
extern	char	*	szReplyBuf;

// Database Connection
//-------------------------------------
extern	char		szDbConnStr[];
extern	PGconn	*	pDbConn;
extern	char	*	szSqlStr;

// Input Function & Function Type
//-------------------------------------
extern	char		szFunc[];
extern	char		szType[];

typedef enum class ReturnCodeType : int {    
    COMPLETE   = 0,                    
    ERROR_JSON = -100,   
    ERROR_ADD_DATA =-101,  
    ERROR_API_NONEXISTENCE = -99,         
} ReturnCode;


// Utilities & Functions
//-------------------------------------
extern	void		Json_Return_Message(ReturnCode returnCode);
extern	void		Json_Return_Message(const char *szMessage);

extern	int 		SQL_Execute(int bErrorReply);
extern	int 		SQL_Execute();

extern	PGresult *	SQL_Query();
extern	PGresult *	SQL_Query(int bErrorReply);

// Product Pre-Load Functions
//-------------------------------------
/*typedef struct product_rec TProd;
struct product_rec
{
  char	szProductNo[36];
  char	szProductCode[36];
  char	szProductName[256];
  char	szProductPrice[32];
  char	szProductType[32];
  char	szProductType1[32];
  char	szProductId[64];
  int	iProductSerial;
  char	unit[32];
  char	per_carton_size[32];
  char	per_pallet_size[32];
};*/

extern	int		Read_Input(int iFd,int iTimeOut);	// Read Data and Set szInputBuf
extern	int		Read_Input(int iFd);	// Read Data and Set szInputBuf
extern	int		Read_Input();		// Read Data and Set szInputBuf
extern	int		Send_Reply(int iFd);	// Send szReplyBuf To or Reply To
extern	int		Send_Reply();		// Send szReplyBuf To or Reply To

extern	int		LoadConfig(char *szFileName);
extern	int		update_child_status();
extern	int		Init_Data_Buffer();

//extern	TProd	*	Get_Product(int iSerial);
extern	int		Load_Product();
extern	int		Gen_Random_Uuid(char *szUuid);

extern	int		Check_Database();
extern	void		Set_Db_Client_Encode(const char *szCodeType);
extern	void		Reset_Db_Client_Encode();
extern	TLog	*	pLog;

extern  void  Data_Init();
extern  void  Data_Free();

class ReturnMsg {
public:
    static const char * getReturnMsg(ReturnCode returnCode);

private:
    static std::map<ReturnCode, const char *>    RETURN_MSG;
};


#endif
