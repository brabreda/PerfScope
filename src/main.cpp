#include "PerfInline.h"

int main() {
  volatile uint64_t x = 0;
  {
    auto tmp = PerfInline<
      PERF_COUNT_HW_CACHE_REFERENCES,
      PERF_COUNT_HW_CACHE_MISSES,
      PERF_COUNT_HW_BRANCH_INSTRUCTIONS,
      PERF_COUNT_HW_BRANCH_MISSES>();

    for(int i = 0; i < 100000; i++){
      x+=i;
    }
  }
  
  return 0;
}