/** 
 * @file $Id: ReferenceEx.h 34386 2014-06-24 06:40:56Z skh $
 * @author caoym
 * @brief 支持弱引用的引用计数实现
 */

#ifndef __REFERENCEEX__
#define __REFERENCEEX__

#include "IReference.h"
#include <assert.h>


/**计数器接口*/
class IReferenceCounter:
    public IReference
{
public:
    virtual ~IReferenceCounter(){}
    /** return if 0 return 0,else return  ++1*/
    virtual unsigned long __stdcall Increment() = 0;
    /** return --1      */
    virtual unsigned long __stdcall Decrement() = 0;

};

/**计数器实现*/
class ReferenceCounter:public IReferenceCounter
{
public:
    ReferenceCounter()
        :m_count(0)
        ,m_lock(0)
        ,m_exist(1)
    {
        
    }
    /** return if not exist return 0,else return  ++1 */
    virtual unsigned long __stdcall Increment()
    {
        Lock();
        if( !m_exist ){
            Unlock();
            return 0;
        }
        long count = InterlockedIncrement(&m_count);
        Unlock();
        return count;
    }

    virtual unsigned long __stdcall Decrement()
    {
        Lock();
        if( !m_exist ){
            Unlock();
            return -1;
        }
        long count = InterlockedDecrement(&m_count);
        if(count == 0 )
        {
            m_exist = 0;
        }
        Unlock();
        return count;
    }
    
    
private:
    void Lock()
    {
        while (InterlockedCompareExchange(&m_lock,1,0) != 0)
        {
        }
    }
    void Unlock()
    {
        InterlockedExchange(&m_lock,0);
    }
    volatile long m_lock;
    volatile long m_count;
    volatile bool m_exist;
    
};

//for gcc, compiler option -Werror
template<class T>
class ReferenceEx_T 
    :virtual private CReference
    ,public T
{
public:
    typedef T _TYPE;
    typedef IReference _REF;
    virtual unsigned long __stdcall AddRef()
    {
        return CReference::AddRef();
    }
    virtual unsigned long __stdcall Release()
    {
        return CReference::Release();
    }
protected:
    virtual ~ReferenceEx_T(){}
};

/** 支持弱引用的引用计数接口 */
class IReferenceEx:
    public IReference
{
public:
    virtual ~IReferenceEx(){}
    virtual CRefObj<IReferenceCounter> RefCounter() {return 0;};
};

/** 支持弱引用的引用计数实现 */
class ReferenceEx:
    public IReferenceEx
{
public:
    ReferenceEx()
        :m_exist(1)
        ,m_counter(new ReferenceEx_T<ReferenceCounter>())
    {

    }
    virtual ~ReferenceEx(){

    }
    virtual unsigned long __stdcall AddRef()
    {
        assert(m_exist != 0); // 析构函数内不能调用自身的AddRef
        return m_counter->Increment();
    }
    virtual unsigned long __stdcall Release()
    {
        long count = m_counter->Decrement();
        if( count == 0  )
        {
            if(InterlockedDecrement(&m_exist) == 0)
            {
                delete this;
            }
        }
        return count;
    }
    virtual CRefObj<IReferenceCounter> RefCounter()
    {
        if(!m_exist) return 0;//m_exist == 0 表示正在析构(在this析构时调用this->RefCounter()时出现)
        return m_counter.p;
    }
private:
   volatile long m_exist;
   CRefObj<ReferenceCounter> m_counter;
};

/** 自动引用计数管理 */
template <class T>
class WeakRefObj
{
public:
   WeakRefObj(T*referent=0)
       :m_referent(referent)
   {
        if(m_referent)
            m_counter = m_referent->RefCounter();
   }
   CRefObj<T> Peek()
   {
       if(m_counter == 0) return 0 ;
       if( m_counter->Increment() !=0 )
       {
           CRefObj<T> p;
           p.p = m_referent;
           return p;
       }
       else return 0;
   }
   CRefObj<T> operator->() 
   {
       return Peek();
   }
   operator CRefObj<T> () 
   {
       return  Peek();
   }
   bool operator == (const WeakRefObj &rh)const 
   {
       return m_referent == rh.m_referent;
   }
   bool operator == (const T*rh)const 
   {
       return m_referent == rh;
   }
   bool operator != (const WeakRefObj &rh)const 
   {
       return m_referent != rh.m_referent;
   }
   bool operator != (const T*rh)const 
   {
       return m_referent != rh;
   }
   bool operator!() const
   {
       return (m_referent == 0);
   }
public:
   CRefObj<IReferenceCounter> m_counter;
   T* m_referent;
};

#endif //__REFERENCEEX__
