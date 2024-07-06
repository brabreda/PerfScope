# PerfInline
Harness the power of performance counters, akin to tools like perf, but scoped within your C++ applications. This project provides a seamless way to measure and analyze performance metrics within defined scopes.
- Only limited to the [`perf_hw_id`](https://sites.uclouvain.be/SystInfo/usr/include/linux/perf_event.h.html) Generalized performance event (More coming soon).

## Example
```cpp
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
```
### Output
```
3771
2433
110931
657
```
