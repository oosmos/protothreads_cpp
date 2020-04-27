#include "proto.hpp"
#include "os.hpp"
#include <iostream>
#include <cstdint>

using std::cout;

static void AssertWarn(bool MustBeTrue, const char * pMessage)
{
  if (!MustBeTrue) {
    cout << pMessage << '\n';
  }
}

struct MyObject : public Proto::Worker<int>
{
  uint32_t m_ObjectMember = 0;

  Proto::Thread TestThread_Instance;

  struct sTestThread_Data
  {
    int   i;
    bool  TimedOut;
    int   Value;
  } TestThread_Data;

  void TestThread(Proto::Thread& rThread, sTestThread_Data& rData)
  {
    ThreadBegin();
      for (rData.i = 1; rData.i <= 5; rData.i++) {
        cout << "TestThread: Iteration " << rData.i << '\n';
        ThreadDelayUS(300);
      }

      ThreadWaitCond(!QueueIsEmpty());
      rData.Value = Pop();
      cout << "Popped value: " << rData.Value << '\n';

      cout << "TestThread: DelaySeconds\n";
      ThreadDelaySeconds(1);

      cout << "TestThread: Yield\n";
      ThreadYield();

      m_ObjectMember += 1;

      cout << "TestThread: WaitCond\n";
      ThreadWaitCond(true);

      cout << "TestThread: WaitCond_Timeout 1\n";
      ThreadWaitCond_TimeoutMS(true, 100, &rData.TimedOut);
      AssertWarn(!rData.TimedOut, "Should not have timed out.");

      cout << "TestThread: WaitCond_Timeout 2\n";
      ThreadWaitCond_TimeoutMS(false, 100, &rData.TimedOut);
      AssertWarn(rData.TimedOut, "Should have timed out.");

      cout << "TestThread: Exit (to ThreadFinally)\n";
      ThreadExit();
      cout << "TestThread: Should not get here\n";
    ThreadFinally();
      cout << "TestThread: Exiting\n";
    ThreadEnd();
  }

  Proto::Thread ControllerThread_Instance;

  void ControllerThread(Proto::Thread& rThread)
  {
    ThreadBegin();
      Push(6);
      ThreadWaitCond(TestThread_Instance.Join());
      cout << "COMPLETE.  Exiting.\n";
      OS::Exit(1);
    ThreadEnd();
  }

  void Run()
  {
    ControllerThread(ControllerThread_Instance);
    TestThread(TestThread_Instance, TestThread_Data);
  }
};

int main()
{
  MyObject MyObject;

  for (;;) {
    Proto::Run();
    OS::DelayMS(1);
  }
}
