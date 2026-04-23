#pragma once
#include <cstdint>


struct GpuSmParam {
int warps_per_sm = 1; // M0: single warp to simplify bring-up
int threads_per_warp = 16; // align with Nyuzi lanes later
int regs_per_thread = 64; // per-thread scalar registers (future)
int vector_lanes = 16; // = threads_per_warp
int issue_width = 1; // 1 inst / tick in M0
bool enable_icache = false; // M0: fetch from memory directly if desired
};