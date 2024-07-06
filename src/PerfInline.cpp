// C++ includes
#include <ranges>
#include <iostream>

// C includes
#include <cassert>

// other includes
#include <sys/syscall.h>
#include <unistd.h>

#include "PerfInline.h"

perf_event_attr InitPerfEvent(){
  perf_event_attr PerfEvent;
  std::memset(&PerfEvent, 0, sizeof(PerfEvent));
  PerfEvent.type = PERF_TYPE_HARDWARE;
  PerfEvent.disabled = 0;
  PerfEvent.pinned = 1;
  PerfEvent.inherit = 1;
  return PerfEvent;
}

namespace detail {

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