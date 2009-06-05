/* 
 * =============================================================================
 * $Header$
 * $Revision$
 * $Date$
 * =============================================================================
 */

#ifndef __THREAD_H__
#define __THREAD_H__

#include <errno.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

namespace clusterlib {

class Thread
{
  public:
    typedef void* (*ThreadFunc) (void*);
    Thread() 
        : _ctx(NULL), 
          _func(NULL) 
    {
        memset(&mThread, 0, sizeof(mThread));
    }
    ~Thread() { }

    void Create(void* ctx, ThreadFunc func);
    void Join();
  private:
    pthread_t mThread;  
    void *_ctx;
    ThreadFunc _func;
};

template<typename T>
struct ThreadContext
{
    typedef void (T::*FuncPtr) (void *);
    ThreadContext(T& ctx, FuncPtr func, void *funcParam) 
        : _ctx(ctx), _func(func), _funcParam(funcParam) {}
    void run(void) 
    {
        (_ctx.*_func)(_funcParam);
    }
    T& _ctx;
    FuncPtr _func;
    void *_funcParam;
};

template<typename T>
void* ThreadExec(void *obj)
{
    ThreadContext<T>* tc = (ThreadContext<T>*)(obj);
    assert(tc != 0);
    tc->run();
    return 0;
}

template <typename T>
class CXXThread
    : public Thread
{
  public:
    typedef void (T::*FuncPtr) (void *);
    CXXThread() : ctx(0) {}
    ~CXXThread() { if (ctx) delete ctx; }

    void Create(T& obj, FuncPtr func, void *funcParam = NULL) 
    {
        assert(ctx == 0);
        ctx = new ThreadContext<T>(obj, func, funcParam);
        Thread::Create(ctx, ThreadExec<T>);
    }

  private:
    ThreadContext<T>* ctx;
};

};	/* End of 'namespace clusterlib' */

#endif /* __THREAD_H__ */

