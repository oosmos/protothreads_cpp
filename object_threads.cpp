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
