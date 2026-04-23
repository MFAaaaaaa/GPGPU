#pragma once
#include <cstdint>
#include <vector>


struct Warp {
uint64_t pc = 0; // program counter (guest address space)
uint32_t active_mask = 0xFFFF; // 16 lanes active by default
bool done = false; // set true when EXIT observed (M0: immediate)
// Future: per-thread registers laid out as [thread][reg]
};