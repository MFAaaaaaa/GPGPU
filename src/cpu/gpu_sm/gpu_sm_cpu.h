#pragma once
#include <string>
#include <vector>
#include <fstream>    // 
#include <sstream>    // 
#include <iomanip>    // 
#include "common.h"
#include "tickqueue.h"             // ValidData<T>
#include "cache/cacheinterface.h"  // simcache::CacheInterface
#include "simroot.h"               // SimObject
#include "gpu_sm_param.h"
#include "warp.h"
#include "cpu/gpu_sm/gpu_sm_param.h"


class GpuSmCpu : public SimObject {
 public:
  GpuSmCpu(const std::string& name,
           const GpuSmParam& param,
           simcache::CacheInterface* l1i,
           simcache::CacheInterface* l1d);

  ~GpuSmCpu();

  void on_current_tick() override;
  void apply_next_tick() override;

  void print_setup_info();
  void print_statistic();

 private:
  void boot_once_();

  // 各流水阶段
  void _cur_p1();
  void _cur_p2();
  void _cur_p3();
  void _cur_p4();
  void _cur_p5();
  void _apply();

// ===== 日志辅助函数 =====
  void log_tick_csv();  // ✅ 每 tick 记录 CSV 一行

  // ---------------- pipeline structs ----------------
  struct P1TOP2 {
    uint64_t pc;
    uint32_t inst;
  };

  struct P2TOP3 {
    uint64_t pc;
    uint32_t inst;
    uint32_t decoded_opcode;
  };

  struct P3TOP4 {
    uint64_t pc;
    uint32_t inst;
    uint32_t alu_result;
  };

  struct P4TOP5 {
    uint64_t pc;
    uint32_t inst;
    uint32_t mem_data;
  };

  // ---------------- pipeline registers ----------------
  ValidData<P1TOP2> p1_output_;
  ValidData<P1TOP2> p2_input_;
  ValidData<P2TOP3> p2_output_;
  ValidData<P2TOP3> p3_input_;
  ValidData<P3TOP4> p3_output_;
  ValidData<P3TOP4> p4_input_;
  ValidData<P4TOP5> p4_output_;
  ValidData<P4TOP5> p5_input_;

  // ---------------- simulation states ----------------
  const std::string name_;
  GpuSmParam p_;
  bool booted_ = false;
  bool halted_ = false;
  bool p1_stall_ = false;
  bool p2_stall_ = false;
  bool p3_stall_ = false;
  bool p4_stall_ = false;
  bool p5_stall_ = false;

  uint64_t ticks_ = 0;
  uint64_t inst_issued_ = 0;
  uint64_t p1pc_ = 0;

  std::vector<uint32_t> instructions_;

  // ---------------- CSV logging ----------------
  std::ofstream csv_log_;       
  std::string csv_log_path_;    
};


//  //SM三级流水线demo 
// struct P1TOP2 {
//   uint64_t pc;
//   uint32_t inst;
// };
// struct P2TOP3 {
//   uint64_t pc;
//   uint32_t inst;
//   uint32_t decoded;
// };

// class GpuSmCpu : public SimObject {
//  public:
//   GpuSmCpu(const std::string& name,
//            const GpuSmParam& param,
//            simcache::CacheInterface* l1i,
//            simcache::CacheInterface* l1d);

//   void on_current_tick() override;
//   void apply_next_tick() override;

//   // 按工程里常见风格，不加 const/override
//   void print_setup_info();
//   void print_statistic();
//   const char* cpu_type_name() const { return "gpu_sm"; }

//  private:
//   void boot_once_();
//   void _cur_p1();
//   void _cur_p2();
//   void _cur_p3();
//   void _apply();

//   // 状态
//   std::string name_;
//   GpuSmParam  p_;
//   bool booted_ = false;
//   bool halted_ = false;

//   uint64_t ticks_ = 0;
//   uint64_t inst_issued_ = 0;

//   // Pipeline
//   bool p1_stall_ = false;
//   bool p2_stall_ = false;

//   uint64_t p1pc_ = 0;
//   std::vector<uint32_t> instructions_;

//   ValidData<P1TOP2> p1_output_;
//   ValidData<P1TOP2> p2_input_;
//   ValidData<P2TOP3> p2_output_;
//   ValidData<P2TOP3> p3_input_;
// };
