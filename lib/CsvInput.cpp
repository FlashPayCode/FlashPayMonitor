#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CsvInput.h"

TCsvInput::TCsvInput()
{
  Init(MIN_DATA_SIZE,MIN_ITEM_NUMS);
}

TCsvInput::TCsvInput(int iMaxDataSize, int iMaxItemNums)
{
  Init(iMaxDataSize,iMaxItemNums);
}

void TCsvInput::Init(int iMaxDataSize, int iMaxItemNums)
{
  if (iMaxDataSize < MIN_DATA_SIZE) iMaxDataSize = MIN_DATA_SIZE;
  if (iMaxItemNums < MIN_ITEM_NUMS) iMaxItemNums = MIN_ITEM_NUMS;

  m_iMaxDataSize	= iMaxDataSize;
  m_iMaxItemNums	= iMaxItemNums;
  m_pCsvFd		= NULL;

  m_iHeadNums		= 0;
  m_pHeadBuff		= (char*)malloc(iMaxDataSize);
  m_pHeadValu		= (char**)malloc(sizeof(char*)*iMaxItemNums);

  m_iItemNums		= 0;
  m_pItemBuff		= (char*)malloc(iMaxDataSize);
  m_pItemValu		= (char**)malloc(sizeof(char*)*iMaxItemNums);
}

TCsvInput::~TCsvInput()
{
  if (m_pHeadBuff) free(m_pHeadBuff);
  if (m_pHeadValu) free(m_pHeadValu);
  if (m_pItemBuff) free(m_pItemBuff);
  if (m_pItemValu) free(m_pItemValu);
}

int TCsvInput::Open(char *szFileName)
{
  if (m_pCsvFd) fclose(m_pCsvFd);
  m_pCsvFd = fopen(szFileName,"r");
  if (m_pCsvFd)
       return 1;
  else return 0;
}

void TCsvInput::DeFormat(char *szValue)
{
//printf("Input: [%s]\n",szValue);
  if (*szValue != '\"') return;
  int i, iLen = strlen(szValue);
  if (szValue[iLen-1] != '\"') return;
  szValue[iLen-1] = '\0';
  for (i = 0; i < iLen; i++) szValue[i] = szValue[i+1];
  
  char *szOldPtr = szValue;
  char *szNewPtr = szValue;
  while (*szOldPtr)
  {
    if (*szOldPtr == '\"' && *(szOldPtr+1) == '\"') szOldPtr++;
    *szNewPtr = *szOldPtr;

//printf("%c\n",*szOldPtr);

    szNewPtr++;
    szOldPtr++;
  }
  *szNewPtr = '\0';
}

int TCsvInput::Parse(char *pItemBuff, char **pItemValu)
{
  int i, bTermFirst = 1, bInQuotation = 0, iItemNums = 0;

  while (*pItemBuff)
  {
    if (bTermFirst)
    {
      pItemValu[iItemNums] = pItemBuff;
      iItemNums++;
      bTermFirst = 0;
      if (*pItemBuff == '\"')
      {
        bInQuotation = 1;
        pItemBuff++;
      }
    }

    if (*pItemBuff == ',' && !bInQuotation)
    {
      *pItemBuff = '\0';
      bTermFirst = 1;
      pItemBuff++;
      continue;
    }

    if (*pItemBuff == '\"' && bInQuotation)
    {
      bInQuotation = 0;
      pItemBuff++;
      continue;
    }
    
    if (*pItemBuff == 0x0d || *pItemBuff == 0x0a)
    {
      *pItemBuff = '\0';
      pItemBuff++;
      continue;
    }

    pItemBuff++;
  }
  pItemValu[iItemNums] = NULL;
  
  for (i = 0; i < iItemNums; i++) DeFormat(pItemValu[i]);
  return iItemNums;
}

int TCsvInput::Read()
{
  if (!m_pCsvFd) return 0;
  char *szResult = fgets(m_pItemBuff,m_iMaxDataSize,m_pCsvFd);
  if (!szResult) return -1;
  m_iItemNums = Parse(m_pItemBuff,m_pItemValu);
  return m_iItemNums;
}

int TCsvInput::ReadFromBuffer(char *szBuffer)
{
  strcpy(m_pItemBuff,szBuffer);
  m_iItemNums = Parse(m_pItemBuff,m_pItemValu);
  return m_iItemNums;
}

int TCsvInput::ReadAsHead()
{
  this->Read();
  m_iHeadNums = m_iItemNums;
  char *pBuff = m_pItemBuff, **pValu = m_pItemValu;
  m_pItemBuff = m_pHeadBuff;
  m_pItemValu = m_pHeadValu;
  m_pHeadBuff = pBuff;
  m_pHeadValu = pValu;
  return 0;
}

int TCsvInput::GetHeadNums()
{
  return m_iHeadNums;
}

char * TCsvInput::GetHeadValu(int iIndex)
{
  if (iIndex < 0 || iIndex >= m_iHeadNums) return NULL;
  return m_pHeadValu[iIndex];
}

int TCsvInput::GetIndex(char *szHeadName)
{
  int iIndex = 0;
  while (iIndex < m_iHeadNums)
  {
    if (strcmp(szHeadName,m_pHeadValu[iIndex]) == 0)
      return iIndex;
  }
  return -1;
}


int TCsvInput::GetItemNums()
{
  return m_iItemNums;
}

char * TCsvInput::GetItemValu(int iIndex)
{
  if (iIndex < 0 || iIndex >= m_iItemNums) return NULL;
  return m_pItemValu[iIndex];
}

void TCsvInput::Close()
{
  if (m_pCsvFd) fclose(m_pCsvFd);
  m_pCsvFd = NULL;
}

void TCsvInput::DisplayHead()
{
  int i ;
  printf ("--Begin--------------------------------------------\n");
  for ( i = 0; i < m_iHeadNums; i++ )
  {
    printf( "%2d: [%s], %d\n", i+1, m_pHeadValu[i], (int)strlen(m_pHeadValu[i]) ) ;
  }
  printf ("--End----------------------------------------------\n" ) ;
}

void TCsvInput::DisplayItem()
{
  int i ;
  printf ("--Begin--------------------------------------------\n");
  for ( i = 0; i < m_iItemNums; i++ )
  {
    printf( "%2d: [%s], %d\n", i+1, m_pItemValu[i], (int)strlen(m_pItemValu[i]) ) ;
  }
  printf ("--End----------------------------------------------\n" ) ;
}
