// C++ includes
#include <ranges>
#include <iostream>

// C includes
#include <cassert>

// other includes
#include <sys/syscall.h>
#include <unistd.h>

#include "PerfScope.h"




namespace PerfScope {
  namespace Details {

    const int OpenPerfEvent(perf_event_attr &PerfEventAttr){
      return syscall(__NR_perf_event_open, &PerfEventAttr, getpid(), -1, -1, 0);
    }

    uint64_t ClosePerfEvent(const int FileDescriptor){
      if( FileDescriptor < 0 ){
        return -1;
      }

      uint64_t RetVal;
      const int32_t rc = read(FileDescriptor, &RetVal, sizeof(RetVal));
      assert(rc == sizeof(RetVal));
      close(FileDescriptor);
      return RetVal;
    }

  };
};