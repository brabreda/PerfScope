#include <vector>
#include <iostream>
#include <fstream>
#include "PerfInline.h"

#define EV_BRANCH_MISS     (0xC5 | (0x1 << 8))
#define EV_BRANCH          (0xC4 | (0x1 << 8))
#define EV_CYCLES          (0x3C | (0x0 << 8))
#define EV_INSTR           (0xC0 | (0x0 << 8))


int main() {
  volatile uint64_t x = 0;
  {
    PerfInline perf = {
      EV_INSTR,
      EV_CYCLES,
      EV_BRANCH,
      EV_BRANCH_MISS
    };

    for(int i = 0; i < 100; i++){
      x+=i;
    }
  }
  std::cout << x << std::endl;

  return 0;
}