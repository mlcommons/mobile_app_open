#include "./dart_cpuinfo.h"

#include <cstring>

#include "cpuinfo.h"

extern "C" dart_ffi_cpuinfo_result *dart_ffi_cpuinfo() {
  cpuinfo_initialize();
  const struct cpuinfo_core *coresInfo = cpuinfo_get_cores();

  auto data = new dart_ffi_cpuinfo_result;
  data->soc_name = coresInfo[0].package->name;

  return data;
}

extern "C" void dart_ffi_cpuinfo_free(dart_ffi_cpuinfo_result *result) {
  delete result;
}
