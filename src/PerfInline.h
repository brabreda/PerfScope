#ifndef PERFINLINE_H
#define PERFINLINE_H
// C++ includes
#include <initializer_list>
#include <vector>

// C includes
#include <cstdint>
#include <cstring>

// other includes
#include <linux/perf_event.h>

class PerfInline {
  public:
    PerfInline(std::initializer_list<int> PmuEventsCodes);
    ~PerfInline();

  private:
    constexpr perf_event_attr InitPerfEvent();
      
    std::vector<int> _PmuEventCodes = {};
    std::vector<perf_event_attr> _PmuPerfEvents = {};
    std::vector<int> _PmuFileDescriptors = {};
};

#endif // PERFINLINE_H