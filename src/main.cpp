#include <iostream>

#include "PerfScope.h"

int main() {
  volatile uint64_t x = 0;
  { 

    using PerfScope::HwEvent;

    auto PrintPerfData = [](auto& PerfData){
      std::cout << (PerfData.template GetEventResult<HwEvent<PERF_COUNT_HW_CACHE_REFERENCES>>()) << "\n";
      std::cout << (PerfData.template GetEventResult<HwEvent<PERF_COUNT_HW_CACHE_MISSES>>()) << "\n";
    };
    auto tmp = PerfScope::PerfScope<
        HwEvent<PERF_COUNT_HW_CACHE_REFERENCES>,
        HwEvent<PERF_COUNT_HW_CACHE_MISSES>
      >(PrintPerfData);

    for(int i = 0; i < 100000; i++){
      x+=i;
    }
  }
  
  return 0;
}