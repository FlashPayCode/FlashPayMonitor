#ifndef	_UTIL_H_
#define	_UTIL_H_

#include "header.h"

#include "json.h"
#include "json_tokener.h"
#include "json_visit.h"

typedef	unsigned long	ulong;

char *	current_datetime();
char *  current_date();
char *  current_month();
char *  current_datetime_nossec();
int	    get_date_int();
char *	get_date_str( const char *sep, char *ostr);
char *	get_cdate_str(const char *sep, char *ostr);
char *	get_time_str( const char *sep, char *ostr);

char *	format_price(char *str);
char *	trim(char *str);
char *	trim_left(char *str);
char *	trim_all(char *str);

long	uctol( unsigned char *p, int num );
void	ltouc( long l, unsigned char *p, int num );
ulong	uctoul( unsigned char *p, int num );
void	ultouc( unsigned long l, unsigned char *p, int num );
char *	strupper( char* dest, char* source );
char *	strlower( char* dest, char* source );

int     sql_injection_check(const char* str);

int	load_file(const char *fname, char *buf);

void	csv2txt(char *ptr);

int     check_log(char * msg);
void	ASSERT (const char *msg);
void	TRACE  (const char *msg);
void	DEBUG  (const char *msg);
void	INFO   (const char *msg);
void	WARNING(const char *msg);
void	ERROR  (const char *msg);
void	ABEND  (const char *msg);

/* xml operations */
int	XmlGetData(const char *szXml, const char *szTagName, char *szData);

int	hcs_strcmp(const void *p1, const void *p2);

unsigned char hex2chr(unsigned char *p);
int	hex2bin(char *hex, unsigned char *bin);
int	bin2hex(unsigned char *bin, int len, char *hex);

char *	XmlSetEaiHead(char *ostr, const char *szTxnId,
                     const char *szResC, const char *szDesc, const char *szActC);
void	XmlGenErrorMsg(char *szOutput, const char *szTxnId, const char *szDesc, const char *szActC);
int	XmlCopyData(byte *ostr, const char *istr);
void	XmlScramble(char *szXml, const char *szTagName, char cData);

/* time elapsed */
void	time_begin(struct timeval *sys_ts);
int	time_stop(struct timeval *old_ts);
int	time_second(struct timeval *sys_ts);

int	get_seconds_of_day();
int	get_elapsed_second(int last_sec);

int  Get_Json_List(json_object *obj, json_object *par, const char *szListName);
json_object *Get_Json_List(json_object *obj, const char *szListName,int *arrsize);
char *	Get_Json_Data(json_object *obj, const char *szKeyName, const char *szDefault);
char *	Get_Json_Data_L(json_object *obj, const char *szKeyName, const char *szDefault,int istrlen);
json_object *Get_Json_Object(json_object *obj, const char *szKeyName);

int	ParseUrl(const char *szUrl, char *szHost, char *szPage);
int ParseDate(const char *szUrl);

char *	Html_Post(const char *url, char *poststr);
void	Html_Port(int iPort);

int	Gen_Random_Uuid(char *szUuid);

void	Pan_Salt(const char *szPan, char *szOut);
void    Update_ErrorLog(const char* section,char *ret_code,char *ret_msg);
char * strim(char *str);
// int UTF82BIG5(char * instr, char * outstr);
#endif
