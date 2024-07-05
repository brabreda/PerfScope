#include <ranges>
#include <vector>
#include <iostream>

#include <stdlib.h>
#include <perfmon/pfmlib.h>
#include <perfmon/pfmlib_perf_event.h>


std::vector<pfm_event_info_t> GetPmuEventInfo(){
  using namespace std::views;
  std::vector<pfm_event_info_t> EventInfoVector;

  auto PmuRange =  
    iota(0, PFM_PMU_MAX) | 
    transform([](int i) { return static_cast<pfm_pmu_t>(i); }) |
    transform([](pfm_pmu_t type) { pfm_pmu_info_t info; info.size = sizeof(info); return std::make_pair(pfm_get_pmu_info(type, &info), info); }) |
    filter([](auto RetAndInfo){ return RetAndInfo.first == PFM_SUCCESS; });


  for(auto [ret, PmuInfo] : PmuRange){
    for (size_t i = PmuInfo.first_event; i != -1; i = pfm_get_event_next(i)) {

      pfm_event_info_t EventInfo;
      EventInfo.size = sizeof(pfm_event_info_t);
      auto ret = pfm_get_event_info(i, PFM_OS_NONE, &EventInfo);

      if (ret == PFM_SUCCESS) {
        EventInfoVector.push_back(EventInfo);
      }
    }
  }

  return EventInfoVector;
}

std::ostream& operator<<(std::ostream& os, const pfm_event_info_t& EventInfo)
{
  return os  
    << EventInfo.name << " = " << EventInfo.code << "," << std::endl;
}