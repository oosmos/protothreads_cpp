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

using std::cout;

int main()
{
  Proto::Thread HeartbeatThread;

  for (;;) {
    [](Proto::Thread& rThread) {
      ThreadBegin();
        for (;;) {
          cout << "HeartbeatThread: Heartbeat On\n";
          ThreadDelayMS(50);
          cout << "HeartbeatThread: Heartbeat Off\n";
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

using std::cout;

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
          cout << "BlinkingThread: LED On\n";
          ThreadDelayMS(250);
          cout << "BlinkingThread: LED Off\n";
          ThreadDelayMS(750);
        }
      ThreadEnd();
    }(BlinkingThread_Instance);

    [&](Proto::Thread& rThread) {
      ThreadBegin();
        for (;;) {
          m_BeepCount += 1;
          cout << "BeepingThread: Beep " << m_BeepCount << '\n';
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

  struct sTestThread_Data {
    int   i;
    bool  TimedOut;
    int   Value;
  } TestThread_Data;

  Proto::Thread  TestThread_Instance;

  Proto::Thread  ControllerThread_Instance;

  void Run()
  {
    [&](Proto::Thread& rThread, sTestThread_Data& rData) {
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
    }(TestThread_Instance, TestThread_Data);

    [&](Proto::Thread& rThread) {
      ThreadBegin();
        Push(6);
        ThreadWaitCond(TestThread_Instance.Join());
        cout << "COMPLETE.  Exiting.\n";
        OS::Exit(1);
      ThreadEnd();
    }(ControllerThread_Instance);
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

One of the severe limitations of the original C implementation of ProtoThreads is that any values stored in variables on the runtime stack are not preserved from one invocation of the thread function to the next. This implementation addresses this limitation by optionally passing an instance of a data structure to the thread where you can preserve values from one invocation to the next in a clean, reliable, and readable way.  In the example above, see how the `TestThread` function uses the members of the `TestThread_Data` structure.

## How ProtoThreads Work

For a detailed walk-through of how ProtoThreads work, visit [HOW-PROTOTHREADS-WORK.md](HOW-PROTOTHREADS-WORK.md).

## Rules

1. You must call each thread function periodically.
   * If you use object threads, you must override virtual function `Run()` in each object that you create and then call each thread function in the object, in turn.
   * On Windows and Linux, you'll want to throttle the calls to thread functions with a hard delay in order to be polite to others on the system. See `OS::DelayMS(1)` in the example.  Vary the delay time depending on how responsive you need your application to be.
   * On 'bare metal' (Arduino, PIC32, STM32, etc.), you'll want to run without throttling.
2. You must pass the corresponding thread instance variable to the thread function and it must be called `rThread`.
3. If you have variables that you'd like to retain from one invocation to the next, you must allocate a data structure for each thread and pass it to the thread function.  There is no restriction on its name.
4. For new platforms, implement a new `os_<name>.cpp` file that conforms to the modest interface specified in `os.hpp`.
