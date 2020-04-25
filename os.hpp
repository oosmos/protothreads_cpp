#ifndef OS_HPP
#define OS_HPP

#include <cstdint>

namespace OS {
  void DelayUS(uint32_t US);
  void DelayMS(uint32_t MS);
  void DelaySeconds(uint32_t Seconds);

  uint32_t GetFreeRunningUS(void);

  void Exit(int);
}

#endif
