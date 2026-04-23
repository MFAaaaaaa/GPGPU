#pragma once
#include "common.h"
#include "simroot.h"   // 提供 SimObject

class GpuDevice : public SimObject {
 public:
  explicit GpuDevice(const std::string& name) : name_(name) {}

  void on_current_tick() override {}
  void apply_next_tick() override {}

  // 父类里这两个函数签名与你现在的 override 不匹配；直接提供普通方法即可
  void print_setup_info() {}
  void print_statistic() {}

 private:
  std::string name_;
};
