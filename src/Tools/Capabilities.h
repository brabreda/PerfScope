#include <linux/capability.h>
#include <linux/prctl.h>
#include <sys/prctl.h>

#include <iostream>

namespace PerfScope::Tools {


  const bool IsCapabilitySet(int Capability){
    if(!cap_valid(Capability)){
      std::cout << "ERROR: Capability is not valid\n";
      return false;
    }

    return (prctl(PR_CAPBSET_READ, Capability) == 1);
  }
};
