#ifndef _TThread_h_
#define _TThread_h_

#ifdef	SOLARIS

#include <thread.h>
#define	mm_thread_t		thread_t
#define	mm_thread_id()		thr_self()
#define	mm_thread_exit(v)	thr_exit(v)
#define	mm_mutex_t		mutex_t
#define	mm_mutex_lock(m)	mutex_lock(m)
#define	mm_mutex_unlock(m)	mutex_unlock(m)
#define	mm_mutex_trylock(m)	mutex_trylock(m)
#define	mm_mutex_destroy(m)	mutex_destroy(m)

#define	mm_cond_t		cond_t
#define	mm_cond_wait(c,m)	cond_wait((c),(m))
#define	mm_cond_signal(c)	cond_signal(c)
#define	mm_cond_broadcast(c)	cond_broadcast(c)
#define	mm_cond_destroy(c)	cond_destroy(c)

#else

#include <pthread.h>
#define	mm_thread_t		pthread_t
#define	mm_thread_id()		pthread_self()
#define	mm_thread_exit(v)	pthread_exit(v)
#define	mm_mutex_t		pthread_mutex_t
#define	mm_mutex_lock(m)	pthread_mutex_lock(m)
#define	mm_mutex_unlock(m)	pthread_mutex_unlock(m)
#define	mm_mutex_trylock(m)	pthread_mutex_trylock(m)
#define	mm_mutex_destroy(m)	pthread_mutex_destroy(m)

#define	mm_cond_t		pthread_cond_t
#define	mm_cond_wait(c,m)	pthread_cond_wait((c),(m))
#define	mm_cond_signal(c)	pthread_cond_signal(c)
#define	mm_cond_broadcast(c)	pthread_cond_broadcast(c)
#define	mm_cond_destroy(c)	pthread_cond_destroy(c)

#endif

#ifndef	TRUE
#define	FALSE	0
#define	TRUE	(!FALSE)
#endif

typedef enum
{
  TT_WAIT = 0, TT_NOWAIT = 1
} TResumeType;

class TCriticalSection
{
public:
	TCriticalSection() ;
	~TCriticalSection() ;
	void	Acquire() ;
	void	Enter() ;
	void	Leave() ;
	void	Release() ;
	int	TestEnter();	// return 0: not enter, 1: enter.

private:
	mm_mutex_t	mutexCriticalSection;
};  // TCriticalSection

#define	DEFAULT_TIMEOUT	1000

class TThreadSync
{
public:
	TThreadSync() ;
	TThreadSync(int iTimeout);
	~TThreadSync() ;

	void	*	Wait();
	void	*	WaitTimeout(int iTimeout);
	int		IsWaiting();
	
	void		Notify(void *pData);
	void		NotifyAll(void *pData);
	void		NotifyWait(void* pData);
	void		NotifySafe(void* pData);

private:
	void		Initialize(int iTimeout);
	void		CounterInc();
	void		CounterDec();

	int		m_iTimeout;
	int		m_iCounter;
	mm_mutex_t	m_mCounterMutex;

	void	*	m_pData;
	mm_cond_t	m_mCond;
	mm_mutex_t	m_mMutex;

}; // TThreadSync

class TThread
{
friend void MigoRunTThread(void*);
public:
	TThread() ;
	virtual ~TThread();

	void		Resume();
	void		Resume(TResumeType mResumeType);

	// Request Thread To Suspend.
	void		Suspend();
	void		Suspend(void *pData);
	void		Suspend(TResumeType mSuspendType);
	void		Suspend(TResumeType mSuspendType, void *pData) ;
	
	// Request Thread To Terminate.
	void		Terminate() ;
	int		WaitFor() ;
	mm_thread_t	ThreadId() const;
	
	// Leave This Method For Compitable.
	mm_thread_t	getThreadID() const;

protected:
	virtual void	OnTerminate() = 0;
	virtual void	OnExecute() = 0;

	bool		Terminated();	// check if terminated
	bool		Suspended();	// check if suspended

	void		Execute();

	void	*	SuspendData();
	void		DoSuspend();

	int		m_iReturnValue;

	// Leave This Atribute For Compitable.
	mm_thread_t	ThreadID;

private:
	void	*	Wait();
	void	*	WaitTimeout(int timeout);
	void		Notify(void* pData) ;
	int		IsWaiting() ;

	mm_thread_t		m_iThreadId;
	bool			m_bTerminated ;
	bool			m_bSuspended ;
	TResumeType		m_mResumeType ;
	TResumeType		m_mSuspendType ;
	void		 *	m_pSuspendData ;

	TCriticalSection 	m_csThread;
	TThreadSync	 	m_mDoSuspend ;
	TThreadSync	 	m_mThreadSync;
	TThreadSync	 	m_syncExecute;
	TThreadSync	 	m_syncSuspend;


}; // TThread

#endif // _TThread_h_
