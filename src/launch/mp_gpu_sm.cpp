#include "common.h"
#include "cpu/gpu_sm/gpu_sm_cpu.h"
#include "easylogging++.h"

namespace launch {

// int mp_gpu_sm(SimWorkload& wl)
int mp_gpu_sm(const std::vector<std::string>& args)
 {

  // ----------- 日志输出配置 ------------
  std::ostringstream oss;
  std::time_t t = std::time(nullptr);
  std::tm tm = *std::localtime(&t);
  oss << "out/gpu_sm_trace_"
      << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S") << ".log";
  std::string log_filename = oss.str();

  el::Configurations conf;
  conf.setToDefault();
  conf.set(el::Level::Global, el::ConfigurationType::Format, "%datetime %level %msg");
  conf.set(el::Level::Global, el::ConfigurationType::Filename, log_filename.c_str());
  conf.set(el::Level::Global, el::ConfigurationType::ToFile, "true");
  conf.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "true");
  el::Loggers::reconfigureLogger("default", conf);

  LOG(INFO) << "Trace log file: " << log_filename;
  // ------------------------------------

  LOG(INFO) << "Execute: launch::mp_gpu_sm(W)";

  GpuSmParam param;
  simcache::CacheInterface* l1i = nullptr;
  simcache::CacheInterface* l1d = nullptr;

  GpuSmCpu sm("GPU_SM0", param, l1i, l1d);
  sm.print_setup_info();

  // 跑几个拍演示
  for (int i = 0; i < 50; ++i) {
    sm.on_current_tick();
    sm.apply_next_tick();
  }

  sm.print_statistic();
  return 1;  // TEST(...) 里非 0 视为通过
}

} // namespace launch
