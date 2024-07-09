#ifndef PerfScope_H
#define PerfScope_H
// C++ includes
#include <initializer_list>
#include <vector>
#include <type_traits>
#include <functional>

// C includes
#include <cstdint>
#include <cstring>

// other includes
#include <linux/perf_event.h>


// Perfscope internal implementation
namespace PerfScope {
  namespace Details {

    const int OpenPerfEvent(perf_event_attr &PerfEventAttr);
    uint64_t ClosePerfEvent(const int FileDescriptor);

    template <perf_type_id EventId>
    class EventBase {
    };

    template <class E>
    concept PerformanceCounter =    
      std::is_base_of_v<EventBase<PERF_TYPE_HARDWARE>, E> ||
      std::is_base_of_v<EventBase<PERF_TYPE_HW_CACHE>, E> ||
      std::is_base_of_v<EventBase<PERF_TYPE_SOFTWARE>, E> ||
      std::is_base_of_v<EventBase<PERF_TYPE_RAW>, E>;

    template<PerformanceCounter T, PerformanceCounter U>
    struct is_same : std::false_type {};

    template<PerformanceCounter T>
    struct is_same<T, T> : std::true_type {};

    template <PerformanceCounter... Types>
    class PerfScopeVisitor;

    template <PerformanceCounter T>
    class PerfScopeVisitor<T> {
      public:
        void OpenPerfEvent() { 
          _PmuFileDescriptors = PerfScope::Details::OpenPerfEvent(_EventAttr); 
        };
        void ClosePerfEvent() { 
          _PmuEventValue = PerfScope::Details::ClosePerfEvent(_PmuFileDescriptors);
        };
        template<PerformanceCounter T1>
        uint64_t GetEventResult() {
          static_assert(is_same<T,T1>::value, "PerfScope is not correctly initialized; missing event.");
          return _PmuEventValue;
        };
        uint64_t GetEventResult() {
          return _PmuEventValue;
        };

      private:
        int _PmuFileDescriptors = -1;
        uint64_t _PmuEventValue = 0;
        perf_event_attr _EventAttr = T::CreatePerfAttribute();
    };

    template <PerformanceCounter T1, PerformanceCounter... Types>
    class PerfScopeVisitor<T1, Types...> : private PerfScopeVisitor<T1>, private PerfScopeVisitor<Types...>{
      public:
        void OpenPerfEvent(){ 
          PerfScopeVisitor<T1>::OpenPerfEvent(); 
          PerfScopeVisitor<Types...>::OpenPerfEvent(); 
        };
        void ClosePerfEvent(){
          PerfScopeVisitor<T1>::ClosePerfEvent(); 
          PerfScopeVisitor<Types...>::ClosePerfEvent(); 
        };
        template<PerformanceCounter T>
        uint64_t GetEventResult() {
          if constexpr(is_same<T1,T>::value){
              return PerfScopeVisitor<T1>::GetEventResult();
          } else{
              return PerfScopeVisitor<Types...>::template GetEventResult<T>();
          }
        }
    }; 
  };

  using namespace Details;



  // Different kind of event that can be catured with PerfScope
  template <perf_hw_id HwEventId>
  class HwEvent : private EventBase<PERF_TYPE_HARDWARE> {
    public:
    static perf_event_attr CreatePerfAttribute() {
      perf_event_attr PerfEvent;
      std::memset(&PerfEvent, 0, sizeof(PerfEvent));
      PerfEvent.type = PERF_TYPE_HARDWARE;
      PerfEvent.disabled = 0;
      PerfEvent.pinned = 1;
      PerfEvent.inherit = 1;
      PerfEvent.config = HwEventId;
      return PerfEvent;
    }
  };

  template <perf_hw_cache_id CacheLevel, perf_hw_cache_op_id CacheOpId, perf_hw_cache_op_result_id CacheOpResult>
  class CacheEvent : private EventBase<PERF_TYPE_HW_CACHE> {
    public:
    static perf_event_attr CreatePerfAttribute() {
      perf_event_attr PerfEvent;
      std::memset(&PerfEvent, 0, sizeof(PerfEvent));
      PerfEvent.type = PERF_TYPE_HW_CACHE;
      PerfEvent.disabled = 0;
      PerfEvent.pinned = 1;
      PerfEvent.inherit = 1;
      PerfEvent.config = CacheLevel;
      return PerfEvent;
    };
  };

  template <perf_sw_ids SwEventId>
  class SwEvent : private EventBase<PERF_TYPE_SOFTWARE> {
    //static perf_event_attr CreatePerfAttribute();
  };

  template <uint64_t RawEventId>
  class RawEvent : private EventBase<PERF_TYPE_RAW> {
    //static perf_event_attr CreatePerfAttribute();
  };

  // Perfscope extern interface
  template <PerformanceCounter... Types>
  class PerfScope : private PerfScopeVisitor<Types...>{
    public:
      template<typename F>
      requires std::is_invocable_v<F,PerfScope<Types...>&>
      PerfScope(F&& DataHandler) : 
        PerfScopeVisitor<Types...>(),
        _DataHandler(std::forward<F>(DataHandler))
      {
        PerfScopeVisitor<Types...>::OpenPerfEvent();
      };
      ~PerfScope(){
        PerfScopeVisitor<Types...>::ClosePerfEvent();
        _DataHandler(*this);
      };
      template<PerformanceCounter T>
      uint64_t GetEventResult() { 
          return PerfScopeVisitor<Types...>::template GetEventResult<T>();
      };
      private:
        std::function<void(PerfScope<Types...>&)> _DataHandler; 
  };


};

#endif // PerfScope_H