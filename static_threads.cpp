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

