#include <windows.h>
#include <cstdint>
#include <iostream>

using std::cout;

namespace OS {
  void DelayUS(uint32_t US)
  {
    cout << "DelayUS is not implemented on Windows\n";
    US = US;
  }

  void DelayMS(uint32_t MS)
  {
    Sleep(MS);
  }

  void DelaySeconds(uint32_t Seconds)
  {
    Sleep(Seconds * 1000);
  }

  uint32_t GetFreeRunningUS(void)
  {
    SYSTEMTIME st;
    GetSystemTime(&st);

    uint64_t MS = 0;
    MS += st.wMilliseconds;
    MS += st.wSecond * 1000ULL;
    MS += st.wMinute * 60000ULL;
    MS += st.wHour   * 3600000ULL;

    const uint64_t US = MS * 1000;

    return (uint32_t) US;
  }

  void Exit(int Code)
  {
    exit(Code);
  }
}
