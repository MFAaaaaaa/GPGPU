#include "cpu/gpu_sm/gpu_sm_cpu.h"

GpuSmCpu::GpuSmCpu(const std::string& name,
                   const GpuSmParam& param,
                   simcache::CacheInterface* /*l1i*/,
                   simcache::CacheInterface* /*l1d*/)
    : name_(name), p_(param) {
  std::ostringstream oss;
  std::time_t t = std::time(nullptr);
  std::tm tm = *std::localtime(&t);
  oss << "out/gpu_sm_ticklog_" << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S") << ".csv";
  csv_log_path_ = oss.str();

  csv_log_.open(csv_log_path_, std::ios::out);
  csv_log_ << "TICK,P1,P2,P3,P4,P5,STALLS\n";
  LOG(INFO) << name_ << " [gpu_sm] CSV tick log file: " << csv_log_path_;
    }

void GpuSmCpu::boot_once_() {
  booted_ = true;
  p1pc_ = 0;
  instructions_.resize(32);
  for (uint32_t i = 0; i < 32; i++) instructions_[i] = i;
  LOG(INFO) << name_ << " [gpu_sm] boot pipeline: 5-stage demo";
}

// ---------------- IF ----------------
void GpuSmCpu::_cur_p1() {
  if (p1_stall_) return;
  if (p1pc_ / 4 >= instructions_.size()) {
    halted_ = true;
    return;
  }
  p1_output_.valid = true;
  p1_output_.data.pc = p1pc_;
  p1_output_.data.inst = instructions_[p1pc_ / 4];
  p1pc_ += 4;
  LOG(TRACE) << name_ << " [P1] fetched inst=" << p1_output_.data.inst;
}

// ---------------- ID ----------------
void GpuSmCpu::_cur_p2() {
  if (!p2_input_.valid || p2_stall_) return;
  p2_output_.valid = true;
  p2_output_.data.pc = p2_input_.data.pc;
  p2_output_.data.inst = p2_input_.data.inst;
  p2_output_.data.decoded_opcode = p2_input_.data.inst % 4;

  if (p2_output_.data.decoded_opcode == 0) {
    p2_stall_ = true;
    p1_stall_ = true;
    LOG(TRACE) << name_ << " [P2] stall on opcode 0";
  } else {
    LOG(TRACE) << name_ << " [P2] decode op=" << p2_output_.data.decoded_opcode;
  }
}

// ---------------- EX ----------------
void GpuSmCpu::_cur_p3() {
  if (!p3_input_.valid || p3_stall_) return;
  p3_output_.valid = true;
  p3_output_.data.pc = p3_input_.data.pc;
  p3_output_.data.inst = p3_input_.data.inst;
  p3_output_.data.alu_result = p3_input_.data.inst * 2;  // demo ALU
  LOG(TRACE) << name_ << " [P3] execute ALU result=" << p3_output_.data.alu_result;
}

// ---------------- MEM ----------------
void GpuSmCpu::_cur_p4() {
  if (!p4_input_.valid || p4_stall_) return;
  p4_output_.valid = true;
  p4_output_.data.pc = p4_input_.data.pc;
  p4_output_.data.inst = p4_input_.data.inst;
  // 模拟内存访存延迟：opcode==3 时 stall 一拍
  if ((p4_input_.data.inst % 4) == 3) {
    p4_stall_ = true;
    LOG(TRACE) << name_ << " [P4] mem stall on opcode 3";
  } else {
    p4_output_.data.mem_data = p4_input_.data.alu_result + 100;
    LOG(TRACE) << name_ << " [P4] mem access addr=" << p4_output_.data.mem_data;
  }
}

// ---------------- WB ----------------
void GpuSmCpu::_cur_p5() {
  if (!p5_input_.valid) return;
  inst_issued_++;
  LOG(TRACE) << name_ << " [P5] writeback result for inst="
             << p5_input_.data.inst;
  if (p5_input_.data.inst >= instructions_.back()) halted_ = true;
}

// ---------------- APPLY ----------------
void GpuSmCpu::_apply() {
  p1_stall_ = p2_stall_ = p3_stall_ = p4_stall_ = false;

  // 推进寄存器
  p2_input_ = p1_output_;
  p1_output_.valid = false;

  p3_input_ = p2_output_;
  p2_output_.valid = false;

  p4_input_ = p3_output_;
  p3_output_.valid = false;

  p5_input_ = p4_output_;
  p4_output_.valid = false;
}

void GpuSmCpu::on_current_tick() {
  if (!booted_) boot_once_();
  if (halted_) return;

  ticks_++;
  _cur_p1();
  _cur_p2();
  _cur_p3();
  _cur_p4();
  _cur_p5();
  
  log_tick_csv();

}

void GpuSmCpu::apply_next_tick() {
  if (halted_) return;
  _apply();
}

void GpuSmCpu::print_setup_info() {
  LOG(INFO) << name_ << " [gpu_sm] setup: pipeline5";
}

void GpuSmCpu::print_statistic() {
  LOG(INFO) << name_ << " [gpu_sm] stat: ticks=" << ticks_
            << ", inst=" << inst_issued_;
}

void GpuSmCpu::log_tick_csv() {
  if (!csv_log_.is_open()) return;

  csv_log_ << ticks_ << ",";
  csv_log_ << (p1_output_.valid ? std::to_string(p1_output_.data.inst) : "-") << ",";
  csv_log_ << (p2_input_.valid  ? std::to_string(p2_input_.data.inst)  : "-") << ",";
  csv_log_ << (p3_input_.valid  ? std::to_string(p3_input_.data.inst)  : "-") << ",";
  csv_log_ << (p4_input_.valid  ? std::to_string(p4_input_.data.inst)  : "-") << ",";
  csv_log_ << (p5_input_.valid  ? std::to_string(p5_input_.data.inst)  : "-") << ",";

  std::string stalls;
  if (p1_stall_) stalls += "p1 ";
  if (p2_stall_) stalls += "p2 ";
  if (p3_stall_) stalls += "p3 ";
  if (p4_stall_) stalls += "p4 ";
  if (p5_stall_) stalls += "p5 ";
  if (stalls.empty()) stalls = "none";

  csv_log_ << stalls << "\n";
}

GpuSmCpu::~GpuSmCpu() {
  if (csv_log_.is_open()) csv_log_.close();
}



//  // 三级Demo
// #include "cpu/gpu_sm/gpu_sm_cpu.h"

// GpuSmCpu::GpuSmCpu(const std::string& name,
//                    const GpuSmParam& param,
//                    simcache::CacheInterface* /*l1i*/,
//                    simcache::CacheInterface* /*l1d*/)
//     : name_(name), p_(param) {}

// void GpuSmCpu::boot_once_() {
//   booted_ = true;
//   p1pc_ = 0;
//   instructions_.resize(32);
//   for (uint32_t i = 0; i < 32; i++) instructions_[i] = i;  // demo 指令流
//   LOG(INFO) << name_ << " [gpu_sm] boot pipeline: 3-stage demo";
// }

// void GpuSmCpu::_cur_p1() {
//   if (p1_stall_) return;
//   if (p1pc_ / 4 >= instructions_.size()) {
//     halted_ = true;
//     return;
//   }
//   p1_output_.valid = true;
//   p1_output_.data.pc   = p1pc_;
//   p1_output_.data.inst = instructions_[p1pc_ / 4];
//   p1pc_ += 4;
//   LOG(TRACE) << name_ << " [P1] fetched inst=" << p1_output_.data.inst;
// }

// void GpuSmCpu::_cur_p2() {
//   if (!p2_input_.valid || p2_stall_) return;
//   p2_output_.valid = true;
//   p2_output_.data.pc     = p2_input_.data.pc;
//   p2_output_.data.inst   = p2_input_.data.inst;
//   p2_output_.data.decoded = p2_input_.data.inst % 4;

//   if (p2_output_.data.decoded == 0) {
//     p2_stall_ = true;
//     p1_stall_ = true;
//     LOG(TRACE) << name_ << " [P2] stall on opcode 0";
//   } else {
//     LOG(TRACE) << name_ << " [P2] decode op=" << p2_output_.data.decoded;
//   }
// }

// void GpuSmCpu::_cur_p3() {
//   if (!p3_input_.valid) return;
//   LOG(TRACE) << name_ << " [P3] execute op=" << p3_input_.data.decoded;
//   inst_issued_++;
//   if (p3_input_.data.inst >= instructions_.back()) halted_ = true;
// }

// void GpuSmCpu::_apply() {
//   // 清除一次性 stall（demo）
//   p1_stall_ = false;
//   p2_stall_ = false;

//   // 推进流水寄存器
//   p2_input_ = p1_output_;
//   p1_output_.valid = false;

//   p3_input_ = p2_output_;
//   p2_output_.valid = false;
// }

// void GpuSmCpu::on_current_tick() {
//   if (!booted_) boot_once_();
//   if (halted_) return;

//   ticks_++;
//   _cur_p1();
//   _cur_p2();
//   _cur_p3();
// }

// void GpuSmCpu::apply_next_tick() {
//   if (halted_) return;
//   _apply();
// }

// void GpuSmCpu::print_setup_info() {
//   LOG(INFO) << name_ << " [gpu_sm] setup: pipeline3";
// }
// void GpuSmCpu::print_statistic() {
//   LOG(INFO) << name_ << " [gpu_sm] stat: ticks=" << ticks_
//             << ", inst=" << inst_issued_;
// }
