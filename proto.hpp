#ifndef PROTO_HPP
#define PROTO_HPP

#include "os.hpp"
#include <cstdint>
#include <vector>
#include <queue>

namespace Proto {
  struct Object {
    Object() {
      ObjectList().push_back(this);
    }

    virtual void Run() = 0;

    static std::vector<Object *>& ObjectList() {
      static std::vector<Object *> ObjectList;
      return ObjectList;
    }
  };

  template <class T>
  struct Worker : public Object {
    void Push(T Value) {
      m_Queue.push(Value);
    }

    T Pop() {
      T Value = m_Queue.front();
      m_Queue.pop();
      return Value;
    }

    bool QueueIsEmpty() {
      return m_Queue.empty();
    }

    private:
      std::queue<T> m_Queue;
  };

  struct Timeout {
    Timeout() {
      m_StartUS = 0;
      m_TimeoutUS = 0;
    }

    void TimeoutInUS(uint32_t TimeoutUS) {
      m_StartUS   = OS::GetFreeRunningUS();
      m_TimeoutUS = TimeoutUS;
    }

    void TimeoutInMS(uint32_t TimeoutMS) {
      TimeoutInUS(TimeoutMS * 1000);
    }

    void TimeoutInSeconds(uint32_t TimeoutSeconds) {
      TimeoutInMS(TimeoutSeconds * 1000);
    }

    bool HasExpired() const {
      const uint32_t NowUS = OS::GetFreeRunningUS();
      return (NowUS - m_StartUS) >= m_TimeoutUS;
    }

    uint32_t m_StartUS;
    uint32_t m_TimeoutUS;
  };

  struct Thread {
    #define PROTO_THREAD_CONTEXT_BEGIN   (-1)
    #define PROTO_THREAD_CONTEXT_FINALLY (-2)
    #define PROTO_THREAD_CONTEXT_END     (-3)

    Thread() : m_Timeout() {
      m_Context    = PROTO_THREAD_CONTEXT_BEGIN;
      m_FirstEntry = true;
    }

    bool Join() {
      return m_Context == PROTO_THREAD_CONTEXT_END;
    }

    bool PROTO_ThreadDelayUS(uint32_t US) {
      if (m_FirstEntry) {
        m_Timeout.TimeoutInUS(US);
        m_FirstEntry = false;
        return false;
      }

      if (m_Timeout.HasExpired()) {
        m_FirstEntry = true;
        return true;
      }

      return false;
    }

    bool PROTO_ThreadDelayMS(uint32_t MS) {
      return PROTO_ThreadDelayUS(MS * 1000);
    }

    bool PROTO_ThreadDelaySeconds(uint32_t Seconds) {
      return PROTO_ThreadDelayUS(Seconds * 1000 * 1000);
    }

    bool PROTO_ThreadWaitCond_TimeoutMS(bool Condition, uint32_t TimeoutMS, bool * pTimeoutStatus) {
      if (m_FirstEntry) {
        m_Timeout.TimeoutInMS(TimeoutMS);
        m_FirstEntry = false;
        return false;
      }

      if (Condition) {
        *pTimeoutStatus = false;
        m_FirstEntry = true;
        return true;
      }

      if (m_Timeout.HasExpired()) {
        *pTimeoutStatus = true;
        m_FirstEntry = true;
        return true;
      }

      return false;
    }

    bool PROTO_ThreadYield() {
      if (m_FirstEntry) {
        m_FirstEntry = false;
        return false;
      }

      m_FirstEntry = true;
      return true;
    }

    int32_t  m_Context;
    Timeout  m_Timeout;
    bool     m_FirstEntry;

    // The switch/case/__LINE__ approach is based on protothreads by Adam Dunkels.

    #define ThreadBegin() \
                                        switch (rThread.m_Context) { \
                                          case PROTO_THREAD_CONTEXT_BEGIN:
    #define ThreadDelayUS(US) \
                                          do { case __LINE__: rThread.m_Context = __LINE__; \
                                            if (!rThread.PROTO_ThreadDelayUS(US)) \
                                              return; \
                                          } while (0)

    #define ThreadDelayMS(MS) \
                                          do { case __LINE__: rThread.m_Context = __LINE__; \
                                            if (!rThread.PROTO_ThreadDelayMS(MS)) \
                                              return; \
                                          } while (0)

    #define ThreadDelaySeconds(Seconds) \
                                          do { case __LINE__: rThread.m_Context = __LINE__; \
                                            if (!rThread.PROTO_ThreadDelaySeconds(Seconds)) \
                                              return; \
                                          } while (0)

    #define ThreadYield() \
                                          do { case __LINE__: rThread.m_Context = __LINE__; \
                                            if (!rThread.PROTO_ThreadYield()) \
                                              return; \
                                          } while (0)

    #define ThreadWaitCond(Cond) \
                                          do { case __LINE__: rThread.m_Context = __LINE__; \
                                            if (!(Cond)) \
                                              return; \
                                          } while (0)

    #define ThreadWaitCond_TimeoutMS(Cond, TimeoutMS, pTimeoutStatus) \
                                          do { case __LINE__: rThread.m_Context = __LINE__; \
                                            if (!rThread.PROTO_ThreadWaitCond_TimeoutMS(Cond, TimeoutMS, pTimeoutStatus)) \
                                              return; \
                                          } while(0)

    #define ThreadExit() \
                                          do { rThread.m_Context = PROTO_THREAD_CONTEXT_FINALLY; \
                                            return; \
                                          } while (0)

    #define ThreadFinally() \
                                          do { case PROTO_THREAD_CONTEXT_FINALLY: rThread.m_Context = PROTO_THREAD_CONTEXT_END; \
                                          } while (0)

    #define ThreadEnd() \
                                          default:  \
                                            rThread.m_Context = PROTO_THREAD_CONTEXT_END; \
                                        } return
  };

  inline void Run() {
    for (auto pObject: Object::ObjectList()) {
      pObject->Run();
    }
  }
}

#endif
