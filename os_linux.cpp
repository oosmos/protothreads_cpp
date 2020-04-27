#include <unistd.h>
#include <cstdint>

#include <sys/time.h>
#include <time.h>

#if _POSIX_C_SOURCE >= 199309L
  #include <time.h>   // for nanosleep
#else
  #include <unistd.h> // for usleep
#endif

namespace OS {
  extern void DelayUS(uint32_t US)
  {
    #if _POSIX_C_SOURCE >= 199309L
      struct timespec ts;
      const uint32_t MS = US / 1000UL;
      ts.tv_sec  = MS / 1000UL;
      ts.tv_nsec = (MS % 1000UL) * 1000000UL;

      nanosleep(&ts, NULL);
    #else
      usleep(US);
    #endif
  }

  extern void DelayMS(uint32_t MS)
  {
    DelayUS(MS * 1000);
  }

  extern void DelaySeconds(uint32_t Seconds)
  {
    sleep(Seconds);
  }

  extern uint32_t GetFreeRunningUS()
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    const uint64_t US = tv.tv_sec * 1000000ULL + tv.tv_usec;
    return (uint32_t) US;
  }

  void Exit(int Code)
  {
    exit(Code);
  }
}

