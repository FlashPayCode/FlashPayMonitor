// TThread.cpp
//----------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "Thread.h"

// class TCriticalSection Implementations:
//----------------------------------------------------------------------
TCriticalSection::TCriticalSection()
{
#ifdef	SOLARIS
  mutex_init(&mutexCriticalSection, USYNC_THREAD, NULL);
#else
  pthread_mutex_init(&mutexCriticalSection, NULL);
#endif
}

TCriticalSection::~TCriticalSection()
{
  mm_mutex_destroy(&mutexCriticalSection);
}

void TCriticalSection::Acquire()
{
  Enter() ;
}

void TCriticalSection::Enter()
{
  mm_mutex_lock(&mutexCriticalSection);
}

int  TCriticalSection::TestEnter()
{
  int iRet = mm_mutex_trylock(&mutexCriticalSection);
  return (iRet?0:1);
}

void TCriticalSection::Leave()
{
  mm_mutex_unlock(&mutexCriticalSection);
}

void TCriticalSection::Release()
{
  Leave() ;
}

// class TThreadSync Implementations:
//----------------------------------------------------------------------
void TThreadSync::Initialize(int iTimeout)
{
#ifdef	SOLARIS
  cond_init(&m_mCond, USYNC_THREAD, NULL);
  mutex_init(&m_mMutex, USYNC_THREAD, NULL);
  mutex_init(&m_mCounterMutex, USYNC_THREAD, NULL);
#else
  pthread_cond_init(&m_mCond, NULL);
  pthread_mutex_init(&m_mMutex, NULL);
  pthread_mutex_init(&m_mCounterMutex, NULL);
#endif

  m_iTimeout = iTimeout;
  m_pData    = NULL;
  m_iCounter = 0;
}

TThreadSync::TThreadSync()
{
  Initialize(DEFAULT_TIMEOUT);
}

TThreadSync::TThreadSync(int iTimeout)
{
  Initialize(iTimeout);
}

TThreadSync::~TThreadSync()
{
  NotifyAll(NULL);
  mm_cond_destroy(&m_mCond);
  mm_mutex_destroy(&m_mMutex);
  mm_mutex_destroy(&m_mCounterMutex);
}

void TThreadSync::CounterInc()
{
  mm_mutex_lock(&m_mCounterMutex);
  m_iCounter++;
  mm_mutex_unlock(&m_mCounterMutex);
}

int TThreadSync::IsWaiting()
{
  int iCounter;
  mm_mutex_lock(&m_mCounterMutex);
  iCounter = m_iCounter;
  mm_mutex_unlock(&m_mCounterMutex);
  return iCounter;
}

void TThreadSync::CounterDec()
{
  mm_mutex_lock(&m_mCounterMutex);
  m_iCounter--;
  mm_mutex_unlock(&m_mCounterMutex);
}

void * TThreadSync::Wait()
{
  CounterInc();  
  mm_cond_wait(&m_mCond,&m_mMutex);
  CounterDec();
  return m_pData;
}

void * TThreadSync::WaitTimeout(int iTimeout)	// 1,000 == 1 sec.
{
  int	sec  = iTimeout / 1000;
  long	nsec = iTimeout % 1000 * 1000000;

#ifdef	SOLARIS
  timestruc_t tm;
  tm.tv_sec = sec;
  tm.tv_nsec = nsec;
  CounterInc();
  cond_timedwait(&m_mCond,&m_mMutex,&tm);

#else
  struct timespec tm;
  tm.tv_sec = sec;
  tm.tv_nsec = nsec;
  CounterInc();
  pthread_cond_timedwait(&m_mCond,&m_mMutex,&tm);

#endif

  CounterDec();
  return m_pData;
}

void TThreadSync::Notify(void *pData)
{
  m_pData = pData;
  mm_cond_signal(&m_mCond);
}

void TThreadSync::NotifyAll(void *pData)
{
  m_pData = pData;
  mm_cond_broadcast(&m_mCond);
}

void TThreadSync::NotifyWait(void *pData)
{
  int iTimeout = 0;
  while (!IsWaiting())
  {
    usleep(1000);
    iTimeout += 1000;
    if (iTimeout >= m_iTimeout) break;
  }
  Notify(pData);
}

void TThreadSync::NotifySafe(void *pData)
{
  if (IsWaiting()) Notify(pData);
}

// class TThread Implemetations:
//----------------------------------------------------------------------

TThread::TThread()
{
  m_bTerminated	  = TRUE ;
  m_bSuspended	  = FALSE ;
  m_iThreadId	  = 0 ;
  m_iReturnValue  = 0 ;
  m_pSuspendData   = NULL ;
}

TThread::~TThread()
{
}

bool TThread::Terminated()
{
  bool bVal ;
  m_csThread.Enter();
  bVal = m_bTerminated ;
  m_csThread.Leave();
  return bVal ;
}

bool TThread::Suspended()
{
  bool bVal ;
  m_csThread.Enter();
  bVal = m_bSuspended ;
  m_csThread.Leave();
  return bVal ;
}

void TThread::Execute()
{
  if (m_mResumeType == TT_WAIT )
  {
    m_syncExecute.NotifyWait(0);
  }
  OnExecute();
  OnTerminate();
}

// this function runs in one thread.
//----------------------------------
void MigoRunTThread(void*arg)
{
  TThread *pThread = (TThread*)arg;
  pThread->Execute();

/* Marked by George.
  mm_thread_exit(NULL);
*/
}

extern "C"
void * __MigoRunTThread(void*arg)
{
  MigoRunTThread(arg) ;
  return NULL;
}

void TThread::Resume(TResumeType mResumeType)
{
  if (!Terminated())		// still run Thread.
  {
    if (Suspended())
    {
      m_mDoSuspend.NotifyWait(0) ;
      //m_bSuspended = FALSE ;
    }
    return ;
  }

  m_csThread.Enter();
  m_bTerminated	 = FALSE ;
  m_bSuspended	 = FALSE ;
  m_iReturnValue = 0 ;
  m_mResumeType	 = mResumeType ;
  m_csThread.Leave();

  mm_thread_t iThreadId;

#ifdef	SOLARIS
  thr_create(NULL,32767,__MigoRunTThread,(void*)this,THR_BOUND,&iThreadId);
#else
  int rc;
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);
  pthread_attr_setstacksize(&thread_attr, 32767);
  rc = pthread_create(&iThreadId, &thread_attr, &__MigoRunTThread, (void*)this);

  if (rc != 0)
       printf("pthread_create() failed, rc = %d\n",rc);
  else printf("pthread_create() ok.\n");
  fflush(stdout);
#endif

  m_iThreadId = iThreadId;

  if (mResumeType == TT_WAIT ) m_syncExecute.Wait() ;
}

void TThread::Resume()
{
  Resume(TT_NOWAIT) ;
}

void TThread::Terminate()
{
  m_csThread.Enter();
  m_bTerminated = TRUE;
  m_csThread.Leave();
}

int TThread::WaitFor()
{
  void *pStatus;

#ifdef	SOLARIS
  mm_thread_t iDeparted;
  int iResult = thr_join(m_iThreadId,&iDeparted,&pStatus);
#else
  int iResult = pthread_join(m_iThreadId,&pStatus);
#endif
  if (iResult == 0)
       return (int)((long)pStatus % 100000000);
  else return 0;
}

mm_thread_t TThread::ThreadId() const
{
  return m_iThreadId ;
}

// Leave This Method For Compitable.
//-------------------------------------
mm_thread_t TThread::getThreadID() const
{
  return m_iThreadId ;
}

// Suspend Operations:
//-------------------------------------------------------------------------
void TThread::Suspend(TResumeType mSuspendType, void *pSuspendData)
{
  m_csThread.Enter() ;
  if (m_bSuspended)
  {
    m_csThread.Leave();
    return;
  }
  m_pSuspendData = pSuspendData;
  m_mSuspendType = mSuspendType;
  m_bSuspended   = TRUE;
  m_csThread.Leave();

  if (mSuspendType == TT_WAIT )
  {
    m_syncSuspend.Wait() ;
    while(!(m_mDoSuspend.IsWaiting())) usleep(1000);
  }
}

void TThread::Suspend(TResumeType mSuspendType)
{
  Suspend(mSuspendType,(void*)0) ;
}

void TThread::Suspend(void *pData)
{
  Suspend(TT_NOWAIT,pData) ;
}

void TThread::Suspend()
{
  Suspend(TT_NOWAIT,(void*)0) ;
}

void* TThread::SuspendData()
{
  void* data ;
  m_csThread.Enter();
  data = m_pSuspendData;
  m_csThread.Leave();
  return data;
}

void TThread::DoSuspend()
{
  TResumeType mSuspendType ;
  m_csThread.Enter();
  mSuspendType = m_mSuspendType ;
  m_csThread.Leave();

  if (mSuspendType == TT_WAIT ) m_syncSuspend.NotifyWait(0);
  m_mDoSuspend.Wait() ;
}
