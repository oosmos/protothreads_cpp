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

