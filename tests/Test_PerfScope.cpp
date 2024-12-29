#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "PerfScope_Internal.h"
#include "PerfScope.h"

#include <stdlib.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>

#include <ranges>


using ::testing::Gt;
using ::testing::Lt;
using ::testing::Eq;

using namespace PerfScope::Details;
using namespace PerfScope;

/*  unit testing performance counters in not easy. The OS is heavily and this can 
*   may impact the result. This would be considered more like system testing.
*/ 
void ARTIFICAL_LOAD(const int n) {
  for(const auto i : std::views::iota(0,n)) { 
    if (std::time(NULL) == 0) { std::cout << "This should never appear\n"; } 
  }
}

TEST(MasterCounter, CreateAndRead) {
  auto MC = MasterCounter<HwEvent<PERF_COUNT_HW_INSTRUCTIONS>>();
  ioctl(MC.MasterFileDescriptor, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
  ioctl(MC.MasterFileDescriptor, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);

  ARTIFICAL_LOAD(100'000);
  const auto R = ReadEvents_With_FormatGroupEnabled<1>(MC.MasterFileDescriptor);
  
  ASSERT_THAT(R.NumberOfEvents, Eq(1));
  ASSERT_THAT(R.EventValues[0], Gt(100'000));
}

TEST(SlaveCounter, CreateAndRead) {
  auto MC = MasterCounter<HwEvent<PERF_COUNT_HW_CPU_CYCLES>>();
  auto SC = SlaveCounter<HwEvent<PERF_COUNT_HW_INSTRUCTIONS>>(MC.MasterFileDescriptor);
  ioctl(MC.MasterFileDescriptor, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
  ioctl(MC.MasterFileDescriptor, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);

  ARTIFICAL_LOAD(100'000);

  const auto R = ReadEvents_With_FormatGroupEnabled<2>(MC.MasterFileDescriptor);

  ASSERT_THAT(MC.MasterFileDescriptor, Gt(0));
  ASSERT_THAT(SC.FileDescriptor, Gt(0));
  ASSERT_THAT(R.NumberOfEvents, Eq(2));
  ASSERT_THAT(R.EventValues[0], Gt(1));
  ASSERT_THAT(R.EventValues[1], Gt(100'000));
}

TEST(PerfEventGroup, ReadPerfCounters_With_Multiple_Counters){
  auto PEG =  PerfEventGroup<
                HwEvent<PERF_COUNT_HW_INSTRUCTIONS>,
                HwEvent<PERF_COUNT_HW_BRANCH_INSTRUCTIONS>,
                HwEvent<PERF_COUNT_HW_CACHE_MISSES>,
                HwEvent<PERF_COUNT_HW_BRANCH_MISSES>
              >();
  PEG.ResetAndEnablePerfCounters();
  
  ARTIFICAL_LOAD(100'000);

  const auto [Instructions, BranchInstructions,CacheMisses, BranchMisses] = PEG.ReadPerfCounters();

  ASSERT_THAT(Instructions, Gt(100'000));
  ASSERT_THAT(BranchInstructions, Gt(100'000));
  ASSERT_THAT(CacheMisses, Gt(1));
  ASSERT_THAT(BranchMisses, Gt(1));
}

TEST(PerfEventGroup, ReadPerfCounters_With_Single_Counter) {
  auto PEG =  PerfEventGroup<
                HwEvent<PERF_COUNT_HW_INSTRUCTIONS>
              >();
  PEG.ResetAndEnablePerfCounters();
  
  ARTIFICAL_LOAD(100'000);

  const auto [Instructions] = PEG.ReadPerfCounters();

  ASSERT_THAT(Instructions, Gt(100'000));
}

TEST(PerfEventGroup, ResetAndEnablePerfCounters) {
  auto PEG =  PerfEventGroup<
                HwEvent<PERF_COUNT_HW_INSTRUCTIONS>,
                HwEvent<PERF_COUNT_HW_BRANCH_INSTRUCTIONS>
              >();
  PEG.ResetAndEnablePerfCounters();
  ARTIFICAL_LOAD(100'000);
  const auto [Instructions1, BranchInstructions1] = PEG.ReadPerfCounters();
  
  PEG.ResetAndEnablePerfCounters();
  const auto [Instructions2, BranchInstructions2]  = PEG.ReadPerfCounters();
  
  ARTIFICAL_LOAD(200'000);
  const auto [Instructions3, BranchInstructions3]  = PEG.ReadPerfCounters();

  ASSERT_THAT(Instructions1, Gt(Instructions2));
  ASSERT_THAT(BranchInstructions1, Gt(BranchInstructions2));

  ASSERT_THAT(Instructions1, Lt(Instructions3));
  ASSERT_THAT(BranchInstructions1, Lt(BranchInstructions3));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}