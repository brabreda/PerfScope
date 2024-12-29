#ifndef PerfScope_H
#define PerfScope_H
// C++ includes
#include <type_traits>
#include <functional>

// C includes
#include <cstdint>
#include <cstring>

// other includes
#include <linux/perf_event.h>
#include "PerfScope_Internal.h"

namespace PerfScope {


/* Supported Event Types:
*   Thier important parameters are passed as compile-time constants.
*/
template <perf_hw_id HwEventId>
class HwEvent;

template <perf_hw_cache_id CacheLevel, perf_hw_cache_op_id CacheOpId, perf_hw_cache_op_result_id CacheOpResult>
class CacheEvent;

/* PerfScope:
*   Expernal API.
*/
template <Details::ConcreteEvent... Events>
class PerfScope : private Details::PerfEventGroup<Events...> {
  public:

    template<typename F> requires std::is_invocable_v<F,PerfScope<Events...>&>
    explicit PerfScope(F&& DataHandler) : 
      _DataHandler(std::move(DataHandler)),
      Details::PerfEventGroup<Events...>() { };
    
    template<typename F> requires std::is_invocable_v<F,PerfScope<Events...>&>
    explicit PerfScope(F& DataHandler) : 
      _DataHandler(DataHandler),
      Details::PerfEventGroup<Events...>() { };

    ~PerfScope(){ _DataHandler(*this); };
    
    auto GetEventResults() -> auto {  return Details::PerfEventGroup<Events...>::ReadPerfCounters(); };

  private:
    std::function<void(PerfScope<Events...>&)> _DataHandler; 
};


}; // namespace PerfScope

#endif // PerfScope_H