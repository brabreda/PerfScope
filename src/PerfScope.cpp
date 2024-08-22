// C++ includes
#include <ranges>
#include <iostream>

// C includes
#include <cassert>

// other includes
#include <sys/syscall.h>
#include <unistd.h>
#include <iostream>

#include "PerfScope.h"


namespace PerfScope {
  namespace Details {

    const int OpenPerfEvent(perf_event_attr &PerfEventAttr){
      return syscall(__NR_perf_event_open, &PerfEventAttr, 0, -1, -1, 0);
    }

    uint64_t ClosePerfEvent(const int FileDescriptor){
      if( FileDescriptor < 0 ){
        return -1;
      }

      uint64_t RetVal[3];
      const int32_t rc = read(FileDescriptor, &RetVal, sizeof(RetVal));
      assert(rc == sizeof(RetVal));

      std::cout << "RETVAL[1] " << RetVal[1] << " RETVAL[2] " << RetVal[2] << "\n";

      close(FileDescriptor);
      return RetVal[0];
    }

  };
};