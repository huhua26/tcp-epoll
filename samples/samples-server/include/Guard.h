/******************************************************************
一些通用类的实现
******************************************************************/

#pragma once

#include "singletonHandler.h"

/*
通过局部返回类的方式，对每一个->操作进行加锁和解锁操作
*/
template<typename T>
class CLockingProxy
{
public:
    explicit CLockingProxy( T *p ) : m_pObj(p) { m_pObj->Lock(); }
    ~CLockingProxy(){ m_pObj->unLock(); }

    T* operator->() const { return m_pObj; }

private:
    CLockingProxy& operator=(const CLockingProxy& );
    T *m_pObj;
};


/*
线程安全指针
*/
template<typename T>
class CThreadSafePtr
{
public:
    explicit CThreadSafePtr( T *p ) : m_pObj( p ){}
    ~CThreadSafePtr(){}

    CLockingProxy<T> operator->() const
    {
        return CLockingProxy<T>( m_pObj );
    }
private:
    T *m_pObj;
};

//from Loki Compile Time Check
template<bool> struct CompileTimeError;
template<> struct CompileTimeError<false>{};
#define STATIC_CHECK(expr)  (CompileTimeError<(expr) != 0>())

template <typename T, typename U>
class CConversion
{
    typedef char Small;
    class Big { char dummy[2]; };
    static Small Test(U);
    static Big Test(...);
    static T MakeT();
public:
    enum { exists = (sizeof(Test(MakeT())) == sizeof(Small)) };
    enum { exists2Way = (exists && CConversion<U, T>::exists) };
    enum { sameType = false };
};

template< typename T>
class CConversion<T, T>
{
public:
    enum { exists = 1, exists2Way = 1, sameType = 1, };
};

#define SUPERSUBCLASS(T, U)\
    (CConversion<const U*, const T*>::exists &&\
    !CConversion<const T*, void*>::sameType )

#define INHERITSCLASS(T, U)\
    (CConversion<const U*, const T*>::exists ||\
    CConversion<const T*, const U*>::exists )


/*
引用计数方式智能指针
*/
template <typename T>
class CRefCountPtr
{
public:
    CRefCountPtr() : m_pObj(0), m_pRef(0) {}
    explicit CRefCountPtr(T* p) : m_pObj(p)
    {
        m_pRef = new long;
        *m_pRef = 1;
    }

    CRefCountPtr(const CRefCountPtr<T>& rcf) : m_pObj(rcf.m_pObj), m_pRef(rcf.m_pRef)
    {
        if (m_pRef){
            __sync_fetch_and_add( m_pRef, 1 );
        }
    }

    template<typename U>
    CRefCountPtr(const CRefCountPtr<U>& rcf) : m_pObj(reinterpret_cast<T*>(rcf.Obj())), m_pRef(rcf.Ref())
    {
        STATIC_CHECK( !INHERITSCLASS(T, U) );
        if( m_pRef ){
            __sync_fetch_and_add( m_pRef, 1 );
        }
    }

    template<typename U>
    operator CRefCountPtr<U>(){ return CRefCountPtr<U>(*this); }


    ~CRefCountPtr()
    {
        Release();
    }

    CRefCountPtr<T>& operator=(const CRefCountPtr<T>& rcf)
    {
        Release();
        m_pObj = rcf.m_pObj;
        if (m_pObj){
            m_pRef = rcf.m_pRef;
            __sync_fetch_and_add( m_pRef, 1 );
        }
        return *this;
    }

    bool operator==(const CRefCountPtr<T>& rcf) const
    {
        if (m_pObj && rcf.m_pObj && m_pRef && rcf.m_pRef &&
            m_pObj == rcf.m_pObj && m_pRef == rcf.m_pRef){
            return true;
        }
        return false;
    }

    template<typename U>
    CRefCountPtr<T>& operator=(const CRefCountPtr<U>& rcf)
    {
        STATIC_CHECK( !INHERITSCLASS(T, U) );
        Release();
        m_pObj = dynamic_cast<T*>(rcf.m_pObj);
        if(m_pObj){
            m_pRef = rcf.m_pRef;
            __sync_fetch_and_add( m_pRef, 1 );
        }
        return *this;
    }

    T* operator->() const { return m_pObj; }
    T& operator*() const { return *m_pObj; }
    T* Obj() const { return m_pObj; }
    long* Ref() const { return m_pRef; }

private:
    void Release()
    {
        if (m_pRef && *m_pRef != 0)
        {
            if ( __sync_sub_and_fetch(m_pRef, 1) == 0 )
            {
                delete m_pObj;
                m_pObj = NULL;
                delete m_pRef;
                m_pRef = NULL;
            }
        }
    }

private:
    T* m_pObj;
    long* m_pRef;
};

template<class T>
bool operator==( const CRefCountPtr<T> &left, const CRefCountPtr<T> &right )
{
    return left.operator ==(right);
}

/*
普通智能指针
*/
template<typename T>
class CSmartlPtr
{
public:
    CSmartlPtr( T *pPtr, bool bArray, unsigned int unBuffSize = -1)
    {
        m_pPtr = pPtr;
        m_bArray = bArray;
        m_unBuffSize = unBuffSize;
    }

    ~CSmartlPtr()
    {
        if( m_bArray ){
            delete []m_pPtr;
        }else{
            delete m_pPtr;
        }

        m_pPtr = NULL;
    }

    T* data()
    {
        return m_pPtr;
    }

    unsigned int getBufferSize()
    {
        return m_unBuffSize;
    }

private:
    CSmartlPtr(const CSmartlPtr<T>& rcf);
    CSmartlPtr<T>& operator=(const CSmartlPtr<T>& rcf);

private:
    T *m_pPtr;
    bool m_bArray;
    unsigned int m_unBuffSize;
};


//用双向链表实现内存池
template<typename Data>
class CCacheDataUnit
{
public:
    explicit CCacheDataUnit( const UINT32 ui_len )
        : mp_data( new Data[ui_len]), mui_len(ui_len), mui_usedlen(0), mp_front(NULL), mp_next(NULL){

    }

    ~CCacheDataUnit()
    {
        delete []mp_data;
        mp_data = NULL;
        mp_front = NULL;
        mp_next = NULL;
    }

    Data *get_data()
    {
        return mp_data;
    }

    UINT32 get_len()
    {
        return mui_len;
    }

    Data *mp_data;
    UINT32 mui_len;
    UINT32 mui_usedlen;
    CCacheDataUnit<Data> *mp_front;
    CCacheDataUnit<Data> *mp_next;
};


template<typename Data>
class CCachePool
{
public:
    CCachePool()
        : mp_head(NULL), mp_tail(NULL), mui_size(0), mui_cursize(0), mui_limitsize(0){
        pthread_mutex_init( &mo_lock, NULL );
    }

    virtual ~CCachePool()
    {
        cache_pool_clear();
        pthread_mutex_destroy( &mo_lock );
    }

    bool cache_pool_init( const UINT32 ui_limitsize, const UINT32 ui_initsize, const UINT32 ui_buffsize )
    {
        CGuardLock<pthread_mutex_t> guard( &mo_lock );
        mui_limitsize = ui_limitsize;
        for( UINT32 ui_index = 0; ui_index < ui_initsize; ++ui_index ){
            cache_pool_push_back( new CCacheDataUnit<Data>(ui_buffsize) );
        }

        mui_size = ui_initsize;
        mui_cursize = ui_initsize;

        return true;
    }

    CCacheDataUnit<Data> *cache_pool_get_data( const UINT32 ui_needlen )
    {
        CGuardLock<pthread_mutex_t> guard( &mo_lock );
        CCacheDataUnit<Data> *p_tmp = mp_head;
        while( p_tmp ){
            if( ui_needlen <= p_tmp->get_len() ){
                if( p_tmp == mp_head ){
                    mp_head = p_tmp->mp_next;
                    if( mp_head ){
                        mp_head->mp_front = NULL;
                    } else {
                        mp_tail = NULL;
                    }
                } else {
                    p_tmp->mp_front->mp_next = p_tmp->mp_next;
                    if( p_tmp->mp_next ){
                        p_tmp->mp_next->mp_front = p_tmp->mp_front;
                    }
                    if( p_tmp == mp_tail ){
                        mp_tail = p_tmp->mp_front;
                    }
                }

                p_tmp->mp_front = NULL;
                p_tmp->mp_next = NULL;
                mui_cursize--;
                break;
            }

            p_tmp = p_tmp->mp_next;
        }

        if( !p_tmp ){
            if( mui_size < mui_limitsize ){
                CCacheDataUnit<Data> *p_new = new CCacheDataUnit<Data>( ui_needlen );
                mui_size++;
                p_tmp = p_new;
            } else {
                return NULL;
            }
        }
        p_tmp->mui_usedlen = ui_needlen;
        return p_tmp;
    }

    void cache_pool_return( CCacheDataUnit<Data> *punit )
    {
        if( punit != NULL ){
            CGuardLock<pthread_mutex_t> guard( &mo_lock );
            cache_pool_push_back( punit );
        }
    }

private:
    void cache_pool_push_back( CCacheDataUnit<Data> *punit )
    {
        if( mp_tail ){
            mp_tail->mp_next = punit;
            punit->mp_front = mp_tail;
        } else {
            mp_head = punit;
        }

        mp_tail = punit;
        mui_cursize++;
    }

    void cache_pool_clear()
    {
        CGuardLock<pthread_mutex_t> guard( &mo_lock );
        CCacheDataUnit<Data> *p_tmp = mp_head;
        while( p_tmp ){
            CCacheDataUnit<Data> *p_next = p_tmp->mp_next;
            delete p_tmp;
            p_tmp = p_next;
        }

        mp_head = NULL;
        mp_tail = NULL;
        mui_size = 0;
        mui_cursize = 0;
        mui_limitsize = 0;
    }

private:
    pthread_mutex_t mo_lock;
    CCacheDataUnit<Data> *mp_head; //内存头块
    CCacheDataUnit<Data> *mp_tail; //内存尾块
    UINT32  mui_size;        //个数
    UINT32  mui_cursize;     //当前个数
    UINT32  mui_limitsize;   //限制个数
};
