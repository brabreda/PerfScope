#ifndef PERFINLINE_H
#define PERFINLINE_H
// C++ includes
#include <initializer_list>
#include <vector>
#include <type_traits>

// C includes
#include <cstdint>
#include <cstring>

// other includes
#include <linux/perf_event.h>

perf_event_attr InitPerfEvent();

namespace detail {

  const int OpenPerfEvent(perf_event_attr &PerfEventAttr);
  uint64_t ClosePerfEvent(const int FileDescriptor);

  // template <typename T>
  using PerformanceCounters = perf_hw_id;

  template <PerformanceCounters... Types>
  class PerfInlineVisitor;

  template <PerformanceCounters T>
  class PerfInlineVisitor<T> {
    public:
      void OpenPerfEvent() { 
        _EventAttr.config = T; 
        _PmuFileDescriptors = detail::OpenPerfEvent(_EventAttr); 
      };
      void ClosePerfEvent() { 
        _PmuEventValue = detail::ClosePerfEvent(_PmuFileDescriptors);
        std::cout << _PmuEventValue << std::endl; 
      }; 

    private:
      int _PmuFileDescriptors = -1;
      uint64_t _PmuEventValue = 0;
      perf_event_attr _EventAttr = InitPerfEvent();
  };

  template <PerformanceCounters T1, PerformanceCounters... Types>
  class PerfInlineVisitor<T1, Types...> : private PerfInlineVisitor<T1>, private PerfInlineVisitor<Types...>{
    public:
      void OpenPerfEvent(){ 
        PerfInlineVisitor<T1>::OpenPerfEvent(); 
        PerfInlineVisitor<Types...>::OpenPerfEvent(); 
      };
      void ClosePerfEvent(){
        PerfInlineVisitor<T1>::ClosePerfEvent(); 
        PerfInlineVisitor<Types...>::ClosePerfEvent(); 
      };
  };
};


template <detail::PerformanceCounters... Types>
class PerfInline : private detail::PerfInlineVisitor<Types...>{
  public:
    PerfInline() : detail::PerfInlineVisitor<Types...>(){
      detail::PerfInlineVisitor<Types...>::OpenPerfEvent();
    };

    ~PerfInline(){
      detail::PerfInlineVisitor<Types...>::ClosePerfEvent();
    };
};

#endif // PERFINLINE_H