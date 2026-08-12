#pragma once

enum Benchmark { RIJNDAEL, KALMAN, BANDWIDTH };

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
