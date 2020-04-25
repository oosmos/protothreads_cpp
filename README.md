# ProtoThreads for C++

Proto is a C++ implementation of protothreads, invented by Adam Dunkels, originally written in C.

Features:

* Header only implementation.
* Cleanly addresses the local variable shortcoming of the C implementation.
* Timeout capability.
* Thread functions are all one-liners.
* Supports both static threads (file scope) and object threads.
* Very portable C++ core.
* Link time dependency injection of operating system specific code.
* Each thread can optionally specify a "Finally" section for common exit code.
* Ability to determine whether a thread has completed.

## Building the Examples

(Assumes that Python is installed.)

On Windows, open a command line window whose environment variables are set up for any version of Visual Studio C++, VS 2013 through 2019. On Linux, simply open any terminal window.

Then run:

```text
python build.py
```

This will build three example applications: `static_threads`, `object_threads`, and `test_threads`.

## Static Threads Example

This is an example of a thread at file scope.

```cpp
#include "proto.hpp"
#include "os.hpp"
#include <iostream>

int main()
{
  Proto::Thread HeartbeatThread;

  for (;;) {
    [](Proto::Thread& rThread) {
      ThreadBegin();
        for (;;) {
          std::cout << "HeartbeatThread: Heartbeat On\n";
          ThreadDelayMS(50);
          std::cout << "HeartbeatThread: Heartbeat Off\n";
          ThreadDelaySeconds(2);
        }
      ThreadEnd();
    } (HeartbeatThread);

    OS::DelayMS(1);
  }
}
```

### Static Threads Output

```text
HeartbeatThread: Heartbeat On
HeartbeatThread: Heartbeat Off
HeartbeatThread: Heartbeat On
HeartbeatThread: Heartbeat Off
...
```

## Object Threads Example

Here is an example of two object threads, where the thread functions are inside the scope of an object and that can access all the members of the containing object:

```cpp
#include "proto.hpp"
#include "os.hpp"
#include <iostream>
#include <cstdint>

struct MyObject : public Proto::Object
{
  Proto::Thread BlinkingThread_Instance;
  Proto::Thread BeepingThread_Instance;
  uint32_t m_BeepCount = 0;

  void Run()
  {
    [](Proto::Thread& rThread) {
      ThreadBegin();
        for (;;) {
          std::cout << "BlinkingThread: LED On\n";
          ThreadDelayMS(250);
          std::cout << "BlinkingThread: LED Off\n";
          ThreadDelayMS(750);
        }
      ThreadEnd();
    }(BlinkingThread_Instance);

    [&](Proto::Thread& rThread) {
      ThreadBegin();
        for (;;) {
          m_BeepCount += 1;
          std::cout << "BeepingThread: Beep " << m_BeepCount << '\n';
          ThreadDelaySeconds(2);
        }
      ThreadEnd();
    }(BeepingThread_Instance);
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
```

### Object Threads Output

```text
BlinkingThread: LED On
BeepingThread: Beep 1
BlinkingThread: LED Off
BlinkingThread: LED On
BlinkingThread: LED Off
BeepingThread: Beep 2
BlinkingThread: LED On
BlinkingThread: LED Off
BlinkingThread: LED On
BlinkingThread: LED Off
BeepingThread: Beep 3
BlinkingThread: LED On
BlinkingThread: LED Off
BlinkingThread: LED On
BlinkingThread: LED Off
...
```

## Test Threads Example

This example tests all the Proto Thread functions.

```cpp
#include "proto.hpp"
#include "os.hpp"
#include <iostream>
#include <cstdint>

static void AssertWarn(bool MustBeTrue, const char * pMessage)
{
  if (!MustBeTrue) {
    std::cout << pMessage << '\n';
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
        std::cout << "TestThread: Iteration " << rData.i << '\n';
        ThreadDelayUS(300);
      }

      ThreadWaitCond(!QueueIsEmpty());
      rData.Value = Pop();
      std::cout << "Popped value: " << rData.Value << '\n';

      std::cout << "TestThread: DelaySeconds\n";
      ThreadDelaySeconds(1);

      std::cout << "TestThread: Yield\n";
      ThreadYield();

      m_ObjectMember += 1;

      std::cout << "TestThread: WaitCond\n";
      ThreadWaitCond(true);

      std::cout << "TestThread: WaitCond_Timeout 1\n";
      ThreadWaitCond_TimeoutMS(true, 100, &rData.TimedOut);
      AssertWarn(!rData.TimedOut, "Should not have timed out.");

      std::cout << "TestThread: WaitCond_Timeout 2\n";
      ThreadWaitCond_TimeoutMS(false, 100, &rData.TimedOut);
      AssertWarn(rData.TimedOut, "Should have timed out.");

      std::cout << "TestThread: Exit (to ThreadFinally)\n";
      ThreadExit();
      std::cout << "TestThread: Should not get here\n";
    ThreadFinally();
      std::cout << "TestThread: Exiting\n";
    ThreadEnd();
  }

  Proto::Thread ControllerThread_Instance;

  void ControllerThread(Proto::Thread& rThread)
  {
    ThreadBegin();
      Push(6);
      ThreadWaitCond(TestThread_Instance.Join());
      std::cout << "COMPLETE.  Exiting.\n";
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
```

### Test Threads Output

```text
TestThread: Iteration 1
TestThread: Iteration 2
TestThread: Iteration 3
TestThread: Iteration 4
TestThread: Iteration 5
TestThread: DelaySeconds
TestThread: Yield
TestThread: WaitCond
TestThread: WaitCond_Timeout 1
TestThread: WaitCond_Timeout 2
TestThread: Exit (to ThreadFinally)
TestThread: Exiting
...
```

The program does not terminate.  You must press `CNTL-C` to exit.

## Notable

### Stack Variables

One of the severe limitations of the original C implementation of ProtoThreads is that any values stored in variables on the runtime stack are not preserved from one invocation of the thread function to the next. This implementation addresses this limitation by optionally passing the thread a data structure where the programmer can preserve values from one invocation to the next in a clean, reliable, and readable way.  Call it a ProtoStack.  In the example above, see how the `TestThread` function uses the members of the `TestThreadStack` class.

## How ProtoThreads Work

For a detailed walk-through of how ProtoThreads work, visit [HOW-PROTOTHREADS-WORK.md](HOW-PROTOTHREADS-WORK.md).

## Rules

1. You must call each thread function periodically.
   * If you use object threads, you must override virtual function `Run()` in each object that you create and then call each thread function in the object, in turn.
   * On Windows and Linux, you'll want to throttle the calls to thread functions with a hard delay in order to be polite to others on the system. See `OS::DelayMS(1)` in the example.  Vary the delay time depending on how responsive you need your application to be.
   * On 'bare metal' (Arduino, PIC32, STM32, etc.), you'll want to run without throttling.
2. You must allocate a stack for each thread.
   * If you have iterators or need other variables local to the function, you must specialize `Proto::Stack` and allocate them there (see `TestThreadStack` in the `TestThreads` example).
   * If you don't need local variables, then simply allocate a stack of type `Proto::Stack`. See `BeepingThread_Stack` in the example.
3. You must pass at least one argument to each thread function that is a reference to the thread's stack that you created.  The name of the argument _must_ be `rStack`.
4. For new platforms, implement a new `os_<name>.cpp` file that conforms to the modest interface specified in `os.hpp`.
