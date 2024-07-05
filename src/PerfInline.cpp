// C++ includes
#include <ranges>
#include <iostream>

// C includes
#include <cassert>

// other includes
#include <sys/syscall.h>
#include <unistd.h>

#include "PerfInline.h"

#define PERFMON_EVENTSEL_OS     (1 << 17)
#define PERFMON_EVENTSEL_USR    (1 << 16)

PerfInline::PerfInline(std::initializer_list<int> PmuEvents) : 
  _PmuEventCodes(PmuEvents), 
  _PmuPerfEvents(PmuEvents.size(), InitPerfEvent()), 
  _PmuFileDescriptors(PmuEvents.size()) 
{
  assert(_PmuPerfEvents.size() == _PmuEventCodes.size());
  assert(_PmuPerfEvents.size() == _PmuFileDescriptors.size());

  for(const int32_t idx : std::views::iota(0, static_cast<int32_t>(_PmuEventCodes.size()))){

    _PmuPerfEvents[idx].config = _PmuEventCodes[idx] & 0xFFFF;
    _PmuFileDescriptors[idx] = syscall(__NR_perf_event_open, &_PmuPerfEvents[idx], getpid(), -1, -1, 0);
    uint64_t conf = _PmuEventCodes[idx] & 0xFFFFFF;
    if ((conf & PERFMON_EVENTSEL_OS) == 0){
      _PmuPerfEvents[idx].exclude_kernel = 1; 
    }
		if ((conf & PERFMON_EVENTSEL_USR) == 0){
      _PmuPerfEvents[idx].exclude_user = 1; 
    }
  }
}

PerfInline::~PerfInline(){
  for(const int32_t idx : std::views::iota(0, static_cast<int32_t>(_PmuEventCodes.size()))){
    if(_PmuFileDescriptors[idx] >= 0 ){
      uint64_t val;
      const int32_t rc = read(_PmuFileDescriptors[idx], &val, sizeof(val));
	    assert(rc == sizeof(val));
      close(_PmuFileDescriptors[idx]);
      std::cout << val << std::endl;
    }
  }
}

constexpr perf_event_attr PerfInline::InitPerfEvent(){
  perf_event_attr PerfEvent;
  std::memset(&PerfEvent, 0, sizeof(PerfEvent));
  PerfEvent.type = PERF_TYPE_RAW;
  PerfEvent.disabled = 0;
  PerfEvent.pinned = 1;
  PerfEvent.inherit = 1;
  return PerfEvent;
}
