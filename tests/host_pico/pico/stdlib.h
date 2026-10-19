#pragma once
#include <cstdint>
// Single-threaded test shims; no peripheral or real-time emulation.
using uint = unsigned;
#define __not_in_flash_func(x) x
#define __time_critical_func(x) x
inline uint64_t host_device_us = 0;
inline uint64_t get_absolute_time() { return host_device_us; }
inline uint32_t to_ms_since_boot(uint64_t t) { return uint32_t(t / 1000); }
