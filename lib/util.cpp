#include "header.h"
#include "json.h"
#include "json_tokener.h"
#include "json_visit.h"
#include <openssl/rand.h>
#include <ctype.h> 
#include "iconv.h"
#include "string.h"
#include "stdlib.h"
#include "errno.h"

char szLogmsg[4096];
int get_date_int()
{
  struct timeval sys_ts;
  struct tm *sys_tm ;
  gettimeofday(&sys_ts, (struct timezone*)0) ;
  sys_tm = localtime(&sys_ts.tv_sec);
  return (sys_tm->tm_year+1900) * 10000 + (sys_tm->tm_mon+1) * 100 + sys_tm->tm_mday;
}

char *get_date_str(const char *sep, char *ostr)
{
  struct timeval sys_ts;
  struct tm *sys_tm ;
  gettimeofday(&sys_ts, (struct timezone*)0) ;
  sys_tm = localtime(&sys_ts.tv_sec);
  sprintf(ostr,"%04d%s%02d%s%02d",
  	sys_tm->tm_year+1900,sep,sys_tm->tm_mon+1,sep,sys_tm->tm_mday) ;
  return ostr ;
}

char *get_cdate_str(const char *sep, char *ostr)
{
  struct timeval sys_ts;
  struct tm *sys_tm ;
  gettimeofday(&sys_ts, (struct timezone*)0) ;
  sys_tm = localtime(&sys_ts.tv_sec);
  sprintf(ostr,"%d%s%02d%s%02d",
  	sys_tm->tm_year+1900,sep,sys_tm->tm_mon+1,sep,sys_tm->tm_mday) ;
  return ostr ;
}


char *get_time_str(const char *sep, char *ostr)
{
  struct timeval sys_ts;
  struct tm *sys_tm ;
  gettimeofday(&sys_ts, (struct timezone*)0) ;
  sys_tm = localtime(&sys_ts.tv_sec);
  sprintf(ostr,"%02d%s%02d%s%02d%s%02d",
	  	sys_tm->tm_hour,sep,
  		sys_tm->tm_min,sep,
  		sys_tm->tm_sec,sep,
  		(int)(sys_ts.tv_usec / 10000)) ;
  return ostr ;
}

void time_begin(struct timeval *sys_ts)
{
  gettimeofday(sys_ts, (struct timezone*)0) ;
}

int time_second(struct timeval *sys_ts)
{
  struct tm *sys_tm ;
  sys_tm = localtime(&sys_ts->tv_sec);
  return sys_tm->tm_hour * 1000000 +
         sys_tm->tm_min  *   10000 +
         sys_tm->tm_sec  *     100 +
         (int)(sys_ts->tv_usec / 10000);
}

int time_stop(struct timeval *old_ts)
{
  struct timeval sys_ts;
  gettimeofday(&sys_ts, (struct timezone*)0) ;
  return (sys_ts.tv_sec - old_ts->tv_sec) * 1000000 + sys_ts.tv_usec - old_ts->tv_usec;
}

char *format_price(char *str)
{
  int i = 0 ;
  static char buf[16] ;
  strcpy(buf,str) ;
  strcpy(buf+6,".") ;
  strcpy(buf+7,str+6) ;
  while ( buf[i] == '0' ) buf[i++] = ' ' ;
  return buf ;
}

char *trim(char *str)
{
  int   len = strlen(str) - 1 ;
  if ( len < 0 ) return str ;
  while (len >= 0)
  {
    //if ( str[len] >= '\0' && str[len] <= ' ' )
    if ( str[len] == ' ' || str[len] == '\t' )
         str[len] = '\0' ;
    else break ;
    len-- ;
  }  
  return str ;
}

char *trim_left(char *str)
{
  while (*str)
  {
    //if ( *str >= '\0' && *str <= ' ' )
    if ( *str == ' ' || *str == '\t' )
         *str = '\0' ;
    else break ;
    str++ ;
  }  
  return str ;
}

char *trim_all(char *str)
{
  return(trim(trim_left(str)));
}

#ifndef	MIGO_SDK

char* strupper( char* dest, char* source )
{
  char* res = dest ;
  while (*source)
  {
    if ( *source >= 'a' && *source <= 'z' )
         *dest = *source - 'a' + 'A' ;
    else *dest = *source ;
    dest++ ;
    source++ ;
  }
  *dest = '\0' ;
  return res ;
}

char* strlower( char* dest, char* source )
{
  char* res = dest ;
  while (*source)
  {
    if ( *source >= 'A' && *source <= 'Z' )
         *dest = *source - 'A' + 'a' ;
    else *dest = *source ;
    dest++ ;
    source++ ;
  }
  *dest = '\0' ;
  return res ;
}

long uctol( unsigned char *p, int num )  
{  
  int	sign = 0, i = 1;  
  long	val ;
  if ( *p >= 128 )
  {
       val  = (long)(*p-128) ;
       sign = 1 ;
  }
  else val  = (long)(*p) ;
  while ( i < num )  
  {  
    val = val * 256 + p[i];  
    i++;  
  }  
  if ( sign ) val *= (-1) ;
  return( val );  
}  

void ltouc( long l, unsigned char *p, int num )  
{  
  int	sign = 0 ;
  unsigned char c;
  if ( l == 0L ) { memset(p,0,num); return ; }  
  if ( l <  0L ) { l = l * (-1); sign =  1 ; }
  num--;  
  while ( num >= 0 )  
  {  
    c = (unsigned char)(l%256L);  
    p[num] = c;  
    l = l / 256L;  
    num--;  
  }
  if ( sign ) p[0] += 128 ;  
}  

unsigned long uctoul( unsigned char *p, int num )  
{  
  int	i   = 0;  
  long	val = 0L;
  while ( i < num )  
  {  
    val = val * 256 + p[i];  
    i++;  
  }  
  return( val );  
}  

void ultouc( unsigned long l, unsigned char *p, int num )  
{  
  unsigned char c;
  if ( l == 0L ) { memset(p,0,num); return ; }  
  num--;  
  while ( num >= 0 )  
  {  
    c = (unsigned char)(l%256L);  
    p[num] = c;  
    l = l / 256L;  
    num--;  
  }
}  

/* deflate == compress 'uncompr' --> 'compr'
-----------------------------------------------------------------------*/
/*
char* mgz_compress1(char *uncompr,int len,char *compr,int *comprLen)
{
  long llen = (long)(*comprLen);
  int err = compress((Byte*)compr,(uLong*)&llen,(const Bytef*)uncompr,(uLong)len);
  *comprLen = (int)llen ;
  if ( err != Z_OK ) return NULL ;
  return compr ;
}

char* mgz_compress( char *data, int len, int *new_len )
{
  char *dout = (char*)calloc((uInt)(*new_len)+1024,1);
  return mgz_compress1(data,len,dout,new_len) ;
}
*/
/* inflate = decompress
-----------------------------------------------------------------------*/
/*
char* mgz_decompress1(char*compr,int comprLen,char *uncompr,int *uncomprLen )
{
  int err = uncompress((Byte*)uncompr,(uLong*)uncomprLen,(Byte*)compr,(uLong)comprLen);
  if ( err != Z_OK ) return NULL ;
  return uncompr ;
}

char* mgz_decompress( char*data, int len, int *new_len )
{
  char *dout = (char*)calloc((uInt)(*new_len)+1024, 1);
  return mgz_decompress1(data,len,dout,new_len) ;
}
*/

//-----------------------------------------------------------------------
#endif

// Debug Functions:
//-----------------------------------------------------------------------
static struct timeval __sys_ts;
static struct tm *__sys_tm ;

void ef_localtime()
{
  gettimeofday(&__sys_ts, (struct timezone*)0) ;
  __sys_tm = localtime(&__sys_ts.tv_sec);
}
    
int _tm_year()       { return(__sys_tm->tm_year+1900) ; }
int _tm_mon()        { return(__sys_tm->tm_mon+1) ; }
int _tm_mday()       { return(__sys_tm->tm_mday) ; }
int _tm_hour()       { return(__sys_tm->tm_hour) ; }
int _tm_min()        { return(__sys_tm->tm_min) ; }
int _tm_sec()        { return(__sys_tm->tm_sec) ; }
int _tm_wday()       { return(__sys_tm->tm_wday) ; }
int _tm_yday()       { return(__sys_tm->tm_yday) ; }
int _tm_isdst()      { return(__sys_tm->tm_isdst) ; }
int _tm_usec()       { return( __sys_ts.tv_usec ) ; }
int _tm_msec()       { return( __sys_ts.tv_usec / 1000 ) ; }
int _tm_dsec()       { return( __sys_ts.tv_usec / 10000 ) ; }

char *datestr()
{
  static char __datestr[64] ;
  int pid = getpid() ;
  
  ef_localtime() ;
  sprintf(__datestr,"%08d-%04d/%02d/%02d_%02d:%02d:%02d.%02d",
  		pid, _tm_year(), _tm_mon(), _tm_mday(),
  		_tm_hour(), _tm_min(), _tm_sec(), _tm_dsec()) ;
  return __datestr ;
}

char *current_datetime_nossec()
{
  static char __datestr[64] ;
  //int pid = getpid() ;
  
  ef_localtime() ;
  sprintf(__datestr,"%04d-%02d-%02d %02d:%02d:%02d",
  		_tm_year(), _tm_mon(), _tm_mday(),
  		_tm_hour(), _tm_min(), _tm_sec()) ;
  return __datestr ;
}

char *current_datetime()
{
  static char __datestr[64] ;
  //int pid = getpid() ;
  
  ef_localtime() ;
  sprintf(__datestr,"%04d-%02d-%02d %02d:%02d:%02d.%06d",
  		_tm_year(), _tm_mon(), _tm_mday(),
  		_tm_hour(), _tm_min(), _tm_sec(), _tm_dsec()) ;
  return __datestr ;
}

char *current_date()
{
  static char __datestr[64] ;
  //int pid = getpid() ;
  
  ef_localtime() ;
  sprintf(__datestr,"%04d-%02d-%02d",
  		_tm_year(), _tm_mon(), _tm_mday() );
  return __datestr ;
}

char *current_month()
{
  static char __datestr[64] ;
  //int pid = getpid() ;
  
  ef_localtime() ;
  sprintf(__datestr,"%04d-%02d",
  		_tm_year(), _tm_mon() );
  return __datestr ;
}

int check_log(char * msg)
{

    char *pan = strstr(msg,"\"pan\":\"");
    if (pan)
    {  
        char *pancheck = strstr(msg,"\"pan\":\"\"");
        if(pancheck)
        {
            pan = pan+13;
            memcpy(pan,"******",6);
        }      
    }
    char *card_pan = strstr(msg,"\"card_pan\":\"");
    if (card_pan)
    {
        char *cardpancheck = strstr(msg,"\"card_pan\":\"\"");
        if(!cardpancheck)
        {
            card_pan = card_pan+18;
            memcpy(card_pan,"******",6);
            
        } 
    }
    char *exp_date = strstr(msg,"\"exp_date\":\"");
    if (exp_date)
    {
        char *exp_datecheck = strstr(msg,"\"exp_date\":\"\"");
        if(!exp_datecheck)
        {
            exp_date = exp_date+12;
            memcpy(exp_date,"****",4);
            
        } 
    }
    char *cvv2 = strstr(msg,"\"cvv2\":\"");
    if (cvv2)
    {
        char *cvv2check = strstr(msg,"\"cvv2\":\"\"");
        if(!cvv2check)
        {
            cvv2 = cvv2+8;
            memcpy(cvv2,"***",3);
            
        } 
    }
    char *password = strstr(msg,"\"password\":");
    if (password)
    {
        password =  password +12;
        char *end_password = strstr(password,"\""); 
        // if(!password)
        // {
            
        int len = end_password-password;
        // char workstr[128];
        char star[48]="";
        for(int i=0;i<len;i++)
        {
            strcat(star,"*");
        }
        strcat(star,"\"");
        memcpy(password,star,len+1);
    }

    char *password2 = strstr(msg,"password='");
    if(password2)
    {
        password2 =  password2 +10;
        char *end_password = strstr(password2,"'");
        int len = end_password - password2;
        char star[48]="";
        for(int i=0;i<len;i++)
        {
            strcat(star,"*");
        }
        strcat(star,"'");
        memcpy(password2,star,len+1);
    }

    char *sztoken = strstr(msg,"\"token\":");
    if (sztoken)
    {
        sztoken =  sztoken +9;
        char *end_token = strstr(sztoken,"\""); 
        // if(!password)
        // {
            
        int len = end_token-sztoken;
        // char workstr[128];
        char star[48]="";
        for(int i=0;i<len;i++)
        {
            strcat(star,"*");
        }
        strcat(star,"\"");
        memcpy(sztoken,star,len+1);
    }

    char *szInput = strstr(msg,"[InputBuf]");
    
    if (szInput)
    {
        
        printf("szInput:%s\n",szInput);
        fflush(stdout) ;
        szInput =  szInput +10;
        if(szInput[1] == '1')
        {
            printf("key check");
            fflush(stdout) ;
            char *midcheck = strstr(szInput,":"); 
            // *encodecheck = '\0'; 
            midcheck++; 
            printf("midcheck:%s\n",midcheck);
            fflush(stdout) ;
            char *datcheck = strstr(midcheck,":");
            datcheck++; 
            printf("datcheck:%s\n",datcheck);
            fflush(stdout) ;
            char *keycheck = strstr(datcheck,":"); 
            keycheck++; 
            printf("keycheck:%s\n",keycheck);
            fflush(stdout) ;

            int keylen = strlen(keycheck);
            printf("datlencheck:%d\n",keylen);
            fflush(stdout) ;
            char star[80]="";
            for(int i=0;i<keylen;i++)
            {
                strcat(star,"*");
            }
            memcpy(keycheck,star,keylen);
        }
    }

    char *szkey = strstr(msg,"[key]");
    if (szkey)
    {
        szkey =  szkey +5;
        // char *end_token = strstr(sztoken,"\""); 
        // if(!password)
        // {
            
        int len = strlen(szkey);
        // char workstr[128];
        char star[48]="";
        for(int i=0;i<len;i++)
        {
            strcat(star,"*");
        }
        memcpy(szkey,star,len+1);
    }

    return 1;
}

bool is_chinese(const char* str)
{
  unsigned char utf[4] = {0};
  unsigned char unicode[3] = {0};
  bool res = false;
  int len = strlen(str);
  for (int i = 0; i < len; i++) {
    if ((str[i] & 0x80) == 0) {   //ascii begin with 0
      res = false;
    }
    else /*if ((str[i] & 0x80) == 1) */{
      utf[0] = str[i];
      utf[1] = str[i + 1];
      utf[2] = str[i + 2];
      i++;
      i++;
      unicode[0] = ((utf[0] & 0x0F) << 4) | ((utf[1] & 0x3C) >>2);
      unicode[1] = ((utf[1] & 0x03) << 6) | (utf[2] & 0x3F);
//      printf("%x,%x\n",unicode[0],unicode[1]);
//      printf("aaaa %x,%x,%x\n\n",utf[0],utf[1],utf[2]);
      if(unicode[0] >= 0x4e && unicode[0] <= 0x9f){
         if (unicode[0] == 0x9f && unicode[1] >0xa5)
                res = false;
         else         
               res = true;
      }else
         res = false;
    }
  }
  return res;
}

void ABEND(const char *msg)
{
  printf("[%s-ERROR]: %s\n",current_datetime(),msg) ;
  printf("[%s-ABEND]: Service Stoped.\n",current_datetime()) ;
  fflush(stdout) ;
  exit(0) ;
}

void ERROR(const char *msg)
{
  sprintf(szLogmsg,"[%s-ERROR]: %s\n",current_datetime(),msg) ;
  if(check_log(szLogmsg)!=0)
  {
    printf("%s",szLogmsg);
    fflush(stdout) ;
  }
}

void WARNING(const char *msg)
{

  printf("[%s-WARN]:  %s\n",current_datetime(),msg) ;
  fflush(stdout) ;
}

void ASSERT(const char *msg)
{

  printf("[%s-ASERT]: %s\n",current_datetime(),msg) ;
  fflush(stdout) ;
}


void INFO(const char *msg)
{
  printf("[%s-INFO]: %s\n",current_datetime(),msg) ;
  if(check_log(szLogmsg)!=0)
  {
    printf("%s",szLogmsg);
    fflush(stdout) ;
  }
}


void TRACE(const char *msg)
{
  printf("[%s-TRACE]: %s\n",current_datetime(),msg) ;
  fflush(stdout) ;
}

void DEBUG(const char *msg)
{  

  sprintf(szLogmsg,"[%s-DEBUG]: %s\n",current_datetime(),msg) ;
  if(check_log(szLogmsg)!=0)
  {
    printf("%s",szLogmsg);
    fflush(stdout) ;
  }
}

void csv2txt(char *ptr)
{
  int all_space = 1;
  while (*ptr)
  {
    if (*ptr == ',')
    {
      if (all_space)
      {
printf("\nALL-SPACE, Set Default To '-'\n");
fflush(stdout);
        *(ptr-1) = '-';
      }
      *ptr = ' ';
      all_space = 1;
    }
    else
    {
      if (!(*ptr == ' ' || *ptr == '\t')) all_space = 0;
    }
    ptr++;
  }
}

/* Get Data From XML document by <tag>data</tag>
---------------------------------------------------------------------------
return -1: <tag>  not found.
return -2: </tag> not found.
return >0: the data length, where the data store in saData
---------------------------------------------------------------------------*/
int XmlGetData(const char *szXml, const char *szTagName, char *szData)
{
  int datlen, taglen;
  char szTagBuf[64], *szTag, *szStart, *szEnd = NULL;
  szData[0] = '\0';

  szTag = szTagBuf + 1;
  sprintf(szTag,"<%s>",  szTagName);
  szStart = strstr((char*)szXml,szTag);
  if (szStart == NULL) return -1;
  taglen = strlen(szTag);

  szTag = szTagBuf;
  szTag[0] = '<';
  szTag[1] = '/';
  szEnd = strstr(szStart,szTag);
  if (szEnd == NULL) return -2;

  datlen = szEnd - szStart - taglen;
  memcpy(szData,szStart+taglen,datlen);
  szData[datlen] = '\0';
  
  return datlen;
}

static
char *XmlScramble1(char *szXml, const char *szTagName, char cData)
{
  int datlen, taglen;
  char szTagBuf[64], *szTag, *szStart, *szEnd = NULL;

  szTag = szTagBuf + 1;
  sprintf(szTag,"<%s>",  szTagName);
  szStart = strstr((char*)szXml,szTag);
  if (szStart == NULL) return NULL;
  taglen = strlen(szTag);

  szTag = szTagBuf;
  szTag[0] = '<';
  szTag[1] = '/';
  szEnd = strstr(szStart,szTag);
  if (szEnd == NULL) return NULL;

  datlen = szEnd - szStart - taglen;
  memset(szStart+taglen,cData,datlen);
  
  return szStart + taglen * 2 + datlen + 1;
}

void XmlScramble(char *szXml, const char *szTagName, char cData)
{
  while (szXml)
  {
    szXml = XmlScramble1(szXml, szTagName, cData);
  }
}

int hcs_strcmp(const void *p1, const void *p2)
{
  return (strcmp((char*)p1,(char*)p2));
}

unsigned char hex2chr(unsigned char *p)
{
  int order1 = 0, order2 = 0;

       if (*p >= '0' && *p <= '9') order1 = *p - '0';
  else if (*p >= 'A' && *p <= 'F') order1 = *p - 'A' + 10;
  else if (*p >= 'a' && *p <= 'f') order1 = *p - 'a' + 10;
              
  order1 *= 16;
  p++;
                  
       if (*p >= '0' && *p <= '9') order2 = *p - '0';
  else if (*p >= 'A' && *p <= 'F') order2 = *p - 'A' + 10;
  else if (*p >= 'a' && *p <= 'f') order2 = *p - 'a' + 10;
                             
  return (order1+order2);
}

int hex2bin(char *hex, unsigned char *bin)
{
    int i, len = strlen(hex) / 2;
    for (i = 0; i < len; i++) bin[i] = hex2chr((unsigned char*)(hex+i*2));
    bin[len] = '\0';
    return len;
}

int bin2hex(unsigned char *bin, int len, char *hex)
{
    int i;
    for (i = 0; i < len; i++) sprintf(hex+i*2,"%02x",bin[i]);
    hex[len*2] = '\0';
    return len*2;
}

char * XmlSetEaiHead(char *ostr,
       const char *szTxnId, const char *szResC, const char *szDesc, const char *szActC)
{
  char tmp[256];

  strcpy(ostr,"<Header>\n");
  strcat(ostr,"  <ClientId></ClientId>\n");
  strcat(ostr,"  <CServiceType></CServiceType>\n");
  strcat(ostr,"  <CMsgType></CMsgType>\n");
  strcat(ostr,"  <TxnRecDteTme>00</TxnRecDteTme>\n");

  sprintf(tmp,"  <TxnId>%s</TxnId>\n", szTxnId);
  strcat(ostr,tmp);

  sprintf(tmp,"  <ComResCod>%s</ComResCod>\n", szResC);
  strcat(ostr,tmp);
  
  sprintf(tmp,"  <ComResDes>%s</ComResDes>\n", szDesc);
  strcat(ostr,tmp);

  strcat(ostr,"  <EAIHCSTimeOut></EAIHCSTimeOut>\n");
  
  sprintf(tmp,"  <EAIActCode>%s</EAIActCode>\n", szActC);
  strcat(ostr,tmp);
  
  strcat(ostr,"</Header>");
  return ostr;
}

static char szWork[2048];
void XmlGenErrorMsg(char *szOutput, const char *szTxnId,
                    const char *szDesc, const char *szActC)
{
  int len;
  char tmp[20], *ptr = szOutput + 9;
  XmlSetEaiHead(szWork,szTxnId,"0000",szDesc,szActC);
  sprintf(ptr,"<eai>\n%s\n<Body></Body>\n</eai>",szWork);
  len = strlen(ptr);
  sprintf(tmp,"%06dVR3",len+9);
  memcpy(szOutput,tmp,9);
}

/* copy xml data and transfer special char */
int XmlCopyData(byte *ostr, const char *istr)
{
  int len = 0;
  while (*istr)
  {
    if (*istr == '&')
    {
      *ostr++ = '&'; *ostr++ = 'a'; *ostr++ = 'm'; *ostr++ = 'p'; *ostr++ = ';';
      len += 5;
    }
    else if (*istr == '"')
    {
      *ostr++ = '&'; *ostr++ = 'q'; *ostr++ = 'u'; *ostr++ = 'o'; *ostr++ = 't'; *ostr++ = ';';
      len += 6;
    }
    else if (*istr == '\'')
    {
      *ostr++ = '&'; *ostr++ = 'a'; *ostr++ = 'p'; *ostr++ = 'o'; *ostr++ = 's'; *ostr++ = ';';
      len += 6;
    }
    else if (*istr == '>')
    {
      *ostr++ = '&'; *ostr++ = 'g'; *ostr++ = 't'; *ostr++ = ';';
      len += 4;
    }
    else if (*istr == '<')
    {
      *ostr++ = '&'; *ostr++ = 'l'; *ostr++ = 't'; *ostr++ = ';';
      len += 4;
    }
    else
    {
      *ostr++ = *istr;
      len++;
    }
    istr++;
  }
  *ostr = '\0';
  return len;
}

/**
 ***********************************************************************************************************************
 * @brief get the seconds from 00:00 of this date
 ***********************************************************************************************************************
 */
int get_seconds_of_day()
{
  time_t sys_timer = time(NULL);
  struct tm *sys_tm;
  gettimeofday(&__sys_ts,NULL);
  sys_tm = localtime(&sys_timer);
  return (int)(sys_tm->tm_hour) * 3600 + (int)(sys_tm->tm_min) * 60 + (int)(sys_tm->tm_sec);
}

int get_elapsed_second(int last_sec)
{
  int curr_sec = get_seconds_of_day();
  if (curr_sec < last_sec)
  {
    return 86400 - last_sec + curr_sec;
  }
  return curr_sec - last_sec;
}

//

int sql_injection_check(const char* str)
{
  int len = strlen(str);
  char szupper[len];

  strcpy(szupper,str);
  strlower(szupper,szupper);
  // DEBUG(szupper);

  char strcheck[len];
  strcpy(strcheck,szupper);

  const char *const InjectionCheck[] = {
    "--"    ,"::"     ,"/*"     ,"*/"    , "@@"        , "%09"       , "'"     , "\""        ,"nvarchar" ,
    "alter" ,"begin"  ,"cast"   ,"create" ,"cursor"     ,"declare"    ,"delete" , "drop"      , "exec"    ,"execute"  ,
    "fetch" ,"insert" ,"kill"   ,"select" ,"sysobjects" ,"syscolumns" ,"update" , "nchar"     , "varchar" ,
    "<"     ,"<"      ,"script" ,"iframe" ,"<for<"      ,">for>"      ,"&quot"  ,"onclick="   , "onfocus"
    };


  for(int i=0;i<37;i++)
  {
    // DEBUG(InjectionCheck[i]);
    if( strstr(strcheck,InjectionCheck[i]) != NULL)
    {
        char failstr[516];
        sprintf(failstr,"sql injection check FAIL: %s",str);
        ERROR(failstr);
        return 0;
    }
  }
  
  return 1;
}


static void xxx(char *istr)
{
  static char szTemp[32767];
  if (*istr == '\"')
       strcpy(szTemp, istr+1);
  else strcpy(szTemp, istr);
  int len = strlen(szTemp);
  if (szTemp[len-1] == '\"') szTemp[len-1] = '\0';
  strcpy(istr,szTemp);
}

static char szJsonData[65536];

char *Get_Json_Data(json_object *obj, const char *szKeyName, const char *szDefault)
{
  char szKey[128];
  szJsonData[0] = '\0';
  json_object_object_foreach(obj, key, val)
  {
    strcpy(szKey,key);
    if (strcmp(szKey,szKeyName) == 0)
    {
      strcpy(szJsonData, (char*)json_object_to_json_string(val));
      xxx(szJsonData);
      break;
    }
  }
  if (szJsonData[0] == '\0')
  {
    strcpy(szJsonData,szDefault);
  }

  return szJsonData;
}

char *	Get_Json_Data_L(json_object *obj, const char *szKeyName, const char *szDefault,int istrlen)
{
    char szKey[128];
    
    szJsonData[0] = '\0';
    json_object_object_foreach(obj, key, val)
    {
        strcpy(szKey,key);
        if (strcmp(szKey,szKeyName) == 0)
        {
          strcpy(szJsonData, (char*)json_object_to_json_string(val));
          xxx(szJsonData);
          break;
        }
    }
    if (szJsonData[0] == '\0')
    {
      strcpy(szJsonData,szDefault);
    }
   
    int len =strlen(szJsonData);
    if( len > istrlen )
    {
      strcpy(szJsonData,"");
      DEBUG(szKeyName);
      DEBUG("check length error");
    }
    else
    {
      
      if(strcmp(szKeyName,"functype"))
      {
        if(sql_injection_check(szJsonData) == 0)    return 0;
      }
      
    }
    return szJsonData;
}

int  Get_Json_List(json_object *obj, json_object *par, const char *szListName)
{
    char workstring[128];
    szJsonData[0] = '\0';
    // lst = NULL;

    // json_object *tmp1_obj = NULL;
    par = json_object_object_get(obj,szListName);
    if (par == NULL) { json_object_put(obj); return 0; }
    if (!par)
    {
      ERROR("Cannot get object\n");
    }

    int arrsize = json_object_array_length(par);
    sprintf(workstring,"arrsize=%d",arrsize);
    // lst = json_object_array_get_idx(par, 0);
    return arrsize;
}

json_object *Get_Json_List(json_object *obj, const char *szListName,int *arrsize)
{
    szJsonData[0] = '\0';
    // lst = NULL;
    char szMid[16];
    char workstr[256];
    strcpy(szMid,	Get_Json_Data_L(obj,"Mid",szMid,16));
    sprintf(workstr,"Mid=%s szListName=%s",szMid,szListName);
    DEBUG(workstr);
    // json_object *tmp1_obj = NULL;
    json_object *par = json_object_object_get(obj,szListName);
    if (par == NULL) { json_object_put(obj); return 0; }
    if (!par)
    {
      ERROR("Cannot get object\n");
    }

    *arrsize = json_object_array_length(par);
    sprintf(workstr,"arrsize=%d",*arrsize);
    DEBUG(workstr);
    // lst = json_object_array_get_idx(par, 0);
    return par;
}

json_object *Get_Json_Object(json_object *obj, const char *szKeyName)
{
  json_object *pObject = NULL;
  char szKey[128];
  szJsonData[0] = '\0';
  json_object_object_foreach(obj, key, val)
  {
    strcpy(szKey,key);
    if (strcasecmp(szKey,szKeyName) == 0)
    {
      pObject = val;
      break;
    }
  }
  return pObject;
}


int load_file(const char *fname, char *buf)
{
  long size ;
  buf[0] = '\0';
  FILE *fd = fopen(fname,"rb") ;
  if ( fd == NULL ) return 0 ;
  fseek(fd, 0L, SEEK_END) ;
  size = ftell(fd) ;
  if ( size > 0 )
  {
    fseek(fd, 0L, SEEK_SET) ;
    fread(buf,(int)size,1,fd) ;
  }
  fclose (fd) ;
  buf[(int)size] = '\0';
  return (int)size ;
}

int ParseUrl(const char *szUrl, char *szHost, char *szPage)
{
  char *ptr = strstr((char*)szUrl,"//");
  if (ptr)
  {
    strcpy(szHost, ptr+2);
    ptr = strstr(szHost, "/");
    if (ptr)
    {
      strcpy(szPage, ptr);
      *ptr = '\0';
      return 1;
    }
  }
  return 0;
}
int ParseDate(const char *szUrl)
{
  int len = strlen(szUrl);
  int count=0;
  for(int i=0;i<=len;i++)
  {
    if(szUrl[i]=='-')count++;
  }

  for(int i=0;i<=count;i++)
  {
      // DEBUG(szUrl) ;
      // DEBUG("SEARCH");
      char *ptr = strstr((char*)szUrl,"-");
      if (ptr)
      {
          memcpy(ptr,"/",1);
      }
  }
  return 1;
}


int Gen_Random_Uuid(char *szUuid)
{
    union
    {
	struct
	{
	    uint32_t time_low;
	    uint16_t time_mid;
	    uint16_t time_hi_and_version;
	    uint8_t  clk_seq_hi_res;
	    uint8_t  clk_seq_low;
	    uint8_t  node[6];
	};
	uint8_t __rnd[16];
    } uuid;

    int rc = RAND_bytes(uuid.__rnd, sizeof(uuid));

    uuid.clk_seq_hi_res = (uint8_t) ((uuid.clk_seq_hi_res & 0x3F) | 0x80);
    uuid.time_hi_and_version = (uint16_t) ((uuid.time_hi_and_version & 0x0FFF) | 0x4000);

    snprintf(szUuid, 38, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			uuid.time_low, uuid.time_mid, uuid.time_hi_and_version,
			uuid.clk_seq_hi_res, uuid.clk_seq_low,
			uuid.node[0], uuid.node[1], uuid.node[2],
			uuid.node[3], uuid.node[4], uuid.node[5]);
    return rc;    
}


void Pan_Salt(const char *szPan, char *szOut)
{
    int i;
    char temp[20];

    strcpy(szOut, szPan);
    if (strlen(szPan) != 16) return;
    
    for (i = 0; i < 8; i++)
    {
        temp[i*2+1] = szPan[i*2];
        temp[i*2] = szPan[i*2+1];
    }
    
    memcpy(szOut+ 0, temp+ 4, 4);
    memcpy(szOut+ 4, temp+ 0, 4);
    memcpy(szOut+ 8, temp+12, 4);
    memcpy(szOut+12, temp+ 8, 4);

    szOut[16] = '\0';
}


void Update_ErrorLog(const char* section, char *ret_code, char *ret_msg)
{
    static	char	workstr[4096];
    char org_ret_msg[512];
    int err_no,err_section;
    err_no=atof(ret_code);
    sprintf(workstr,"err_no = %d",err_no);
    DEBUG(workstr);
    strcpy(org_ret_msg,ret_msg);
    if(strcmp(section,"Motion")==0)//00開頭
    {
        //摩迅
        if(strcmp(ret_code, "00") == 0) strcpy(ret_code,"00");
    }
    else if(strcmp(section,"TS")==0)//10開頭
    {
        err_section = 1000;
        //台新
        if(err_no == 0)    err_no = err_no; 
        else if(err_no > 0) err_no = err_no + err_section; 
        else if(err_no < 0) err_no = err_no - err_section;
    }
    
    sprintf(ret_code,"%d",err_no);
    sprintf(ret_msg,"%s:%s",section,org_ret_msg);
    sprintf(workstr,"ret_code = %s, ret_msg = %s",ret_code,ret_msg);
    DEBUG(workstr);


}

char * strim(char *str)//去除首尾的空格
{
	char *end,*sp,*ep;
	int len;
	sp = str;
	end = str + strlen(str) - 1;
	ep = end;
 
	while(sp<=end && isspace(*sp))// *sp == ' '也可以
		sp++;
	while(ep>=sp && isspace(*ep))
		ep--;
	len = (ep < sp) ? 0:(ep-sp)+1;//(ep < sp)判断是否整行都是空格
	sp[len] = '\0';
	return sp;
}

// int UTF82BIG5(char * instr, char * outstr)
// {
//   DEBUG(instr);
//   DEBUG(outstr);
//   iconv_t cd;
//   size_t in_s,out_s;
//   cd = iconv_open("BIG-5", "UTF8");
//   char *in_ptr,*out_ptr;
//   outstr = NULL;

//   in_s = strlen(instr);
//   in_ptr = instr;

//   outstr = (char*)malloc(in_s*3);
//   out_s = in_s*3;
//   out_ptr = outstr;

//   if( cd == (iconv_t )-1 )
//   {
//     fprintf( stderr, "error opening iconv \n" );
//     exit(1);
//   }

//   if(!iconv( cd, &in_ptr, &in_s, &out_ptr, &out_s) )
//   {
//     printf("errno: %s\n", strerror(errno));
//   }

//   *out_ptr = '\0';
//   char workstr[1024];
//   sprintf(workstr,"%s, %s\n",instr, outstr);
//   DEBUG(workstr);

//   iconv_close( cd );
//   free( outstr );
  
//   return 1;
// }

