#ifndef PerfScopeInternal_H
#define PerfScopeInternal_H
// C++ includes

// C includes
#include <cstdint>
#include <cstring>
#include <cassert>
#include <array>
#include <tuple>
#include <type_traits>

// other includes
#include <linux/perf_event.h>

// Predefines
namespace PerfScope::Details { 
perf_event_attr CreateDefaultPerfAttribute();
};

// Internal implementation
namespace PerfScope {


template <perf_type_id _EventType>
struct EventBase { static constexpr perf_type_id EventType = _EventType; };

// should we make CreatePerfAttribute to keep the ext. interface simpler. 
template <perf_hw_id HwEventId>
struct HwEvent : public EventBase<PERF_TYPE_HARDWARE> {
  static perf_event_attr GetEvent() {
    perf_event_attr PerfEventAttr = Details::CreateDefaultPerfAttribute();
    PerfEventAttr.type = EventType;
    PerfEventAttr.config = HwEventId;
    return PerfEventAttr; 
  }
};


template <perf_hw_cache_id CacheLevel, 
          perf_hw_cache_op_id CacheOpId, 
          perf_hw_cache_op_result_id CacheOpResult>
struct CacheEvent : private EventBase<PERF_TYPE_HW_CACHE> {
  static perf_event_attr GetEvent() {
    perf_event_attr PerfEventAttr = Details::CreateDefaultPerfAttribute();
    PerfEventAttr.type = EventType;
    PerfEventAttr.config = CacheLevel;
    return PerfEventAttr; 
  }
};


}; // namespace PerfScope


namespace PerfScope::Details {

template<class E>
concept ConcreteEvent = std::is_base_of_v<EventBase<PERF_TYPE_HARDWARE>, E> ||
                        std::is_base_of_v<EventBase<PERF_TYPE_HW_CACHE>, E> ||
                        std::is_base_of_v<EventBase<PERF_TYPE_SOFTWARE>, E> ||
                        std::is_base_of_v<EventBase<PERF_TYPE_RAW>, E>;


/*  Returns perf_event_attr object with common settings
*   that are used by all PerfEvents
*/
auto CreateDefaultPerfAttribute() -> perf_event_attr {
  perf_event_attr PerfEventAttr;
  std::memset(&PerfEventAttr, 0, sizeof(PerfEventAttr));
  PerfEventAttr.disabled = 0;
  PerfEventAttr.pinned = 1;
  PerfEventAttr.inherit = 0;
  // Read all counter values in an event group with one read.
  PerfEventAttr.read_format = PERF_FORMAT_GROUP;
  return PerfEventAttr;
};

/* PerfReadFormat
*   - Structure for reading the values of the events
*   - This Structure is applicable if PERF_FORMAT_TOTAL_TIME_ENABLED, 
*     PERF_FORMAT_TOTAL_TIME_RUNNING, PERF_FORMAT_ID and PERF_FORMAT_LOST 
*     are disabled, and if PERF_FORMAT_GROUP is enabled. 
*/
template <uint64_t GroupSize>
struct PerfReadFormat {
  uint64_t NumberOfEvents = 0;
  std::array<uint64_t,GroupSize> EventValues = {0};    
};

/* ReadPerformanceCounter
*   - Read counter value from performance counter
*   - Is applicable for the same conditions as PerfReadFormat 
*/
template <uint64_t GroupSize>
auto ReadEvents_With_FormatGroupEnabled(const int FileDescriptor) -> PerfReadFormat<GroupSize> {
  PerfReadFormat<GroupSize> Result; 
  const int32_t rc = read(FileDescriptor, &Result, sizeof(Result));
  assert(Result.NumberOfEvents == GroupSize);
  return Result;
};

/* Master & slave PerfEvent:
*   - The group_fd argument allows event groups to be created. In such an event group
*     the master is created first and all other event in the event group use the file 
*     discriptor of the master to be created. Event groups ensure that the Events
*     are sheduled togheter, so that they are measuring the same instructions.
*/
template<ConcreteEvent T>
struct MasterCounter {

  int MasterFileDescriptor;

  MasterCounter(){
    perf_event_attr PerfEventAttr = T::GetEvent();
    MasterFileDescriptor = syscall(__NR_perf_event_open, &PerfEventAttr, 0, -1, -1, 0);
  }

  ~MasterCounter() { close(MasterFileDescriptor); }
};

template<ConcreteEvent T>
struct SlaveCounter {
  
  int FileDescriptor;
  
  SlaveCounter(int MasterFileDescriptor){
    perf_event_attr PerfEventAttr = T::GetEvent();
    PerfEventAttr.pinned = 0; // Pinned only aplies to group leaders
    FileDescriptor = syscall(__NR_perf_event_open, &PerfEventAttr, 0, -1, MasterFileDescriptor, 0);
  }

  ~SlaveCounter() { close(FileDescriptor); }
};

/* PerfEventGroup
*   - Provides functionality to enable and reset the counters of the events
*   - Provides functionality to read all counters simultaneously
*/
template<ConcreteEvent Event_1, ConcreteEvent... Events> 
class PerfEventGroup : MasterCounter<Event_1>, SlaveCounter<Events>... { 
  
  using MasterCounter<Event_1>::MasterFileDescriptor;

  template <typename T, ConcreteEvent> using Expand = T; // helper

  public: 
    PerfEventGroup(const PerfEventGroup&) = delete;
    PerfEventGroup(const PerfEventGroup&&) = delete;
    
    explicit PerfEventGroup() :
      MasterCounter<Event_1>(),
      SlaveCounter<Events>(MasterFileDescriptor)... { };
    
    auto ResetAndEnablePerfCounters() -> void { 
      ioctl(MasterCounter<Event_1>::MasterFileDescriptor, 
            PERF_EVENT_IOC_RESET | PERF_EVENT_IOC_ENABLE, 
            PERF_IOC_FLAG_GROUP);
    };
    
    auto ReadPerfCounters() const -> std::tuple<int, Expand<int, Events>...> {
      return  std::tuple_cat(
                ReadEvents_With_FormatGroupEnabled<sizeof...(Events)+1>(MasterFileDescriptor).EventValues
              );
    };
};

}; // namespace PerfScope::Details

#endif // PerfScopeInternal_H