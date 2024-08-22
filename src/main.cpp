#include <iostream>

#include "PerfScope.h"
#include "Tools/Capabilities.h"
// #include <sys/ioctl.h>

// int is_event_enabled(int fd) {
//     int enabled = 0;
//     int ret = ioctl(fd, PERF_EVENT_IOC_ENABLE, &enabled);
//     if (ret == -1) {
//         perror("ioctl");
//         return -1;
//     }
//     return enabled;
// }

int main() {
  using PerfScope::Tools::IsCapabilitySet;

  std::cout << "CAP_SYS_ADMIN: " << (IsCapabilitySet(CAP_SYS_ADMIN) ? "true" : "false") << "\n";
  std::cout << "CAP_PERFMON: " << (IsCapabilitySet(CAP_PERFMON) ? "true" : "false") << "\n";
  std::cout << "CAP_BPF: " << (IsCapabilitySet(CAP_BPF) ? "true" : "false") << "\n";
  volatile uint64_t x = 0;
  { 

    using PerfScope::HwEvent;

  auto DataHandler = [&](auto& PerfData){ 
    std::cout << (PerfData.template GetEventResult<HwEvent<PERF_COUNT_HW_CPU_CYCLES>>()) << "\n";
    std::cout << (PerfData.template GetEventResult<HwEvent<PERF_COUNT_HW_INSTRUCTIONS>>()) << "\n";
    std::cout << (PerfData.template GetEventResult<HwEvent<PERF_COUNT_HW_CACHE_REFERENCES>>()) << "\n";
    std::cout << (PerfData.template GetEventResult<HwEvent<PERF_COUNT_HW_CACHE_MISSES>>()) << "\n";
    std::cout << (PerfData.template GetEventResult<HwEvent<PERF_COUNT_HW_BRANCH_INSTRUCTIONS>>()) << "\n";
  };

  auto perf = PerfScope::PerfScope<
                HwEvent<PERF_COUNT_HW_CPU_CYCLES>,
                HwEvent<PERF_COUNT_HW_INSTRUCTIONS>,
                HwEvent<PERF_COUNT_HW_CACHE_REFERENCES>,
                HwEvent<PERF_COUNT_HW_CACHE_MISSES>,
                HwEvent<PERF_COUNT_HW_BRANCH_INSTRUCTIONS>
              >(DataHandler);

    for(int i = 0; i < 100000; i++){
      x+=i;
    }
  }
  x = 0;
  { 

    using PerfScope::HwEvent;

  auto DataHandler = [&](auto& PerfData){ 
    std::cout << (PerfData.template GetEventResult<HwEvent<PERF_COUNT_HW_CPU_CYCLES>>()) << "\n";
    std::cout << (PerfData.template GetEventResult<HwEvent<PERF_COUNT_HW_INSTRUCTIONS>>()) << "\n";
    std::cout << (PerfData.template GetEventResult<HwEvent<PERF_COUNT_HW_CACHE_REFERENCES>>()) << "\n";
    std::cout << (PerfData.template GetEventResult<HwEvent<PERF_COUNT_HW_CACHE_MISSES>>()) << "\n";
    std::cout << (PerfData.template GetEventResult<HwEvent<PERF_COUNT_HW_BRANCH_INSTRUCTIONS>>()) << "\n";
  };

  auto perf = PerfScope::PerfScope<
                HwEvent<PERF_COUNT_HW_CPU_CYCLES>,
                HwEvent<PERF_COUNT_HW_INSTRUCTIONS>,
                HwEvent<PERF_COUNT_HW_CACHE_REFERENCES>,
                HwEvent<PERF_COUNT_HW_CACHE_MISSES>,
                HwEvent<PERF_COUNT_HW_BRANCH_INSTRUCTIONS>
              >(DataHandler);

    for(int i = 0; i < 100000; i++){
      x+=i;
    }
  }
  
  return 0;
}