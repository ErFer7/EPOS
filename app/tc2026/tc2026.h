#pragma once

#include "bandwidth.h"

enum Benchmark { RIJNDAEL, KALMAN, BANDWIDTH_L1, BANDWIDTH_L2 };

struct StressTask {
    const unsigned int period;
    const unsigned int deadline;
    const unsigned int wcet;
    const unsigned int cpu;
    const Benchmark task;
    const float duration;  // Iteration duration
};

struct Taskset {
    const StressTask *tasks;
    const unsigned int size;
};

template <int TaskEnum> struct Benchmark_Traits;

template <> struct Benchmark_Traits<BANDWIDTH_L1> {
    using Type = Bandwidth;
    static Type* create() { return new Type(Bandwidth::L1_CACHE_SIZE); }
};

template <> struct Benchmark_Traits<BANDWIDTH_L2> {
    using Type = Bandwidth;
    static Type* create() { return new Type(Bandwidth::L2_CACHE_SIZE); }
};

// template <> struct Benchmark_Traits<RIJNDAEL> {
//     using Type = Rijndael;
//     static Type* create() { return new Type(); } // Default constructor
// };

