/*
* 说明：单件模式实现
*/

#pragma once

#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/if_ether.h>
#include <unistd.h>
#include <string>

template <typename LOCK>
class CGuardLock
{
public:
    explicit CGuardLock(const LOCK* p) : m_pLock(const_cast<LOCK*>(p)) { pthread_mutex_lock( m_pLock ); }
    ~CGuardLock() { pthread_mutex_unlock( m_pLock ); }

private:
    CGuardLock(const CGuardLock&);
    CGuardLock& operator=(const CGuardLock&);
    LOCK* m_pLock;
};

template <typename T, typename LOCK = pthread_mutex_t>
class CSingletonHandler : public T
{
public:
	//在第一次实例化的时候使用DoubleCheckNull方式检测并发性
	static T* Instance()
	{
		if (m_pInstance == NULL)
		{
            pthread_mutex_init( &m_Lock, NULL );
            CGuardLock<LOCK> guard(&m_Lock);
			if (m_pInstance == NULL)
            {
                m_pInstance = new T;
            }
		}
		return m_pInstance;
	}

	static void Release()
	{
		if (m_pInstance != NULL)
		{
            CGuardLock<LOCK> guard(&m_Lock);
			if (m_pInstance != NULL)
			{
                delete m_pInstance;
                m_pInstance = NULL;
			}
		}
	}
private:
	CSingletonHandler();
	~CSingletonHandler();
	
	CSingletonHandler(const CSingletonHandler& singletonhandler);
	CSingletonHandler& operator=(const CSingletonHandler& singletonhandler);

	static T* m_pInstance;
    static LOCK m_Lock;
};

template <typename T, typename LOCK>
T* CSingletonHandler<T, LOCK>::m_pInstance = NULL;

template<typename T, typename LOCK>
LOCK CSingletonHandler<T, LOCK>::m_Lock;

