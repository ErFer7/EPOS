#include "tc2026.h"

#include <clerk.h>
#include <machine/pmic.h>
#include <process.h>
#include <real-time.h>
#include <utility/ostream.h>
#include <utility/random.h>

#include "kalman.h"
#include "tc2026_traits.h"

using namespace EPOS;

OStream cout;

typedef TSC::Time_Stamp Time_Stamp;

// Configuration
const unsigned int TEST_DURATION = Traits<Build>::EXPECTED_SIMULATION_TIME - 10;  // in seconds
const unsigned int SELECTED_TASKSET = 1;
const unsigned int SEED = 20260610;

constexpr float AES_IT_DURATION = 2800.0f;  // in microseconds
constexpr float KALMAN_IT_DURATION = 500.0f;
constexpr float BANDWIDTH_IT_DURATION = 2000.0f;

constexpr static StressTask taskset_1[] = {
    {1000000, 1000000, 200000, 1, RIJNDAEL, AES_IT_DURATION},  // 20 - band
    {1000000, 1000000, 200000, 3, KALMAN, KALMAN_IT_DURATION},
    {1000000, 1000000, 200000, 3, BANDWIDTH_L2, BANDWIDTH_IT_DURATION},
};  // HP = 1

constexpr static Taskset tasksets[] = {{taskset_1, sizeof(taskset_1) / sizeof(StressTask)}};

constexpr static Taskset taskset = tasksets[SELECTED_TASKSET - 1];
constexpr static unsigned int task_count = taskset.size;

// Definitions
Time_Stamp task_runtime[task_count];
Time_Stamp job_wcet[task_count];
Time_Stamp iteration_wcet[task_count];

Random *rand;
unsigned int current_iteration[task_count];
Thread *threads[task_count];
void *benchmarks[task_count];

// Functions
inline Time_Stamp get_time();
inline Microsecond us(Time_Stamp ts);

constexpr int calc_iter_per_job(int i);
constexpr int calc_jobs(int i);

int log_status();

void init_taskset();
void free_taskset();

template <int ID>
inline void init_thread(Microsecond activation);

template <>
inline void init_thread<task_count>(Microsecond activation);

template <>
inline void init_thread<task_count>(Microsecond activation) {}

template <unsigned int ID>
void run_func();

void rijndael_enc_init();
void rijndael_enc_main();
int rijndael_enc_return();

void kalman_init();
void kalman_main();

int main() {
    cout << "TC 2026 Research Experiment Benchmark\n"
         << "Running experiments with the following configurations:\n"
         << ">  Test duration: " << TEST_DURATION << '\n'
         << ">  RNG Seed: " << SEED << '\n'
         << ">  Selected taskset: " << SELECTED_TASKSET << '\n'
         << ">  CPU Clock: " << CPU::clock() / 1000000 << "MHz" << '\n'
         << endl;
    // << ">  CPU voltage: " << PMIC::get_cpu_voltage() << "mV" << '\n'
    // << ">  DDR Clock: " << Clock_Tree::get_ddr_clock() / 1000000 << "MHz" << endl;

    Monitor::print_monitor_info();

    rand->seed(SEED);

    Time_Stamp tsc0 = get_time() + Convert::us2count<Time_Stamp, Time_Base>(TSC::frequency(), 10000);

    cout << "Creating threads..." << endl;
    init_thread<0>(10000);
    cout << "Done" << endl;

    // TODO: Remeber the resume on RT!

    // Monitor::enable_captures();

    cout << "Current time: " << us(tsc0) << endl;

    Thread *logger =
        new Thread(Thread::Configuration(Thread::READY, Thread::Criterion(1000000, 1000000, 10000, 0)), &log_status);

    for (unsigned int i = 0; i < task_count; i++) {
        threads[i]->join();
        // Monitor::disable_captures();
        cout << "Joined task [" << i << ']' << endl;
    }

    logger->join();
    cout << "Joined logger" << endl;

    Time_Stamp times = get_time() - tsc0;

    // Monitor::disable_captures();

    cout << "Returned to application main!" << endl;
    cout << "Elapsed: " << us(times) << endl;
    cout << "Threads: " << task_count << endl;

    cout << "-----------------------------------------------------" << endl;
    cout << "...............Threads Timing Behavior..............." << endl;
    cout << "-----------------------------------------------------" << endl;

    for (unsigned int i = 0; i < task_count; i++) {
        cout << "Task [" << i << "]: " << threads[i] << '\n'
             << ">   Counted iterations: " << current_iteration[i] << endl;

        const int total_jobs = calc_jobs(i);
        const int total_iterations = total_jobs * calc_iter_per_job(i);

        cout << ">   Execution time: " << us(task_runtime[i]) << " us\n"
             << ">   Job WCET: " << us(job_wcet[i]) << " us\n"
             << ">   Iteration WCET: " << us(iteration_wcet[i]) << " us\n"
             << ">   Average job runtime: " << us(task_runtime[i] / total_jobs) << " us\n"
             << ">   Average iteration runtime: " << us(task_runtime[i] / total_iterations) << " us\n"
             << ">   Iterations per job: " << calc_iter_per_job(i) << endl;
    }

    free_taskset();

    return 0;
}

inline Time_Stamp get_time() { return TSC::time_stamp(); }

inline Microsecond us(Time_Stamp ts) { return Convert::count2us<Hertz, Time_Stamp, Time_Base>(TSC::frequency(), ts); }

constexpr int calc_iter_per_job(int i) {
    int iterations = static_cast<int>(static_cast<float>(taskset.tasks[i].wcet) / taskset.tasks[i].duration);
    if (iterations < 1) iterations = 1;

    return iterations;
}

constexpr int calc_jobs(int i) { return (TEST_DURATION * 1000000) / taskset.tasks[i].period; }

void free_taskset() {
    cout << "Taskset deallocation..." << endl;

    for (unsigned int i = 0; i < task_count; i++) {
        switch (taskset.tasks[i].task) {
            default:
                break;
        }

        cout << ">  Deleting thread " << threads[i] << endl;

        delete threads[i];
    }

    cout << "Done." << endl;
}

template <unsigned int ID>
inline void init_thread(Microsecond activation) {
    constexpr auto task = taskset.tasks[ID].task;
    using Traits = Benchmark_Traits<task>;
    using Benchmark = typename Traits::Type;

    Benchmark *benchmark = Traits::create();
    benchmark->init();
    benchmarks[ID] = benchmark;

    constexpr int job = calc_jobs(ID);

    cout << ">  Thread[" << ID << "]: period = " << taskset.tasks[ID].period
         << ", deadline = " << taskset.tasks[ID].deadline << ", wcet = " << taskset.tasks[ID].wcet
         << ", activation = " << activation << ", times = " << job << ", cpu = " << taskset.tasks[ID].cpu << endl;

    threads[ID] = new RT_Thread(&run_func<ID>,
                                taskset.tasks[ID].period,
                                taskset.tasks[ID].deadline,
                                taskset.tasks[ID].wcet,
                                activation,
                                job,
                                taskset.tasks[ID].cpu);

    init_thread<ID + 1>(activation);
}

template <unsigned int ID>
void run_func() {
    constexpr auto task = taskset.tasks[ID].task;
    using Benchmark = typename Benchmark_Traits<task>::Type;
    Benchmark *benchmark = static_cast<Benchmark *>(benchmarks[ID]);

    unsigned int my_iter_per_job = calc_iter_per_job(ID);

    Time_Stamp init = get_time();

    for (unsigned int iterations = 0; iterations < my_iter_per_job; iterations++) {
        benchmark->run();
    }

    Time_Stamp job_runtime = get_time() - init;
    task_runtime[ID] += job_runtime;

    current_iteration[ID]++;

    Time_Stamp average_iteration_runtime = job_runtime / my_iter_per_job;

    if (average_iteration_runtime > iteration_wcet[ID]) {
        iteration_wcet[ID] = average_iteration_runtime;
    }

    if (job_runtime > job_wcet[ID]) {
        job_wcet[ID] = job_runtime;
    }
}

int log_status() {
    for (unsigned int i = 0; i < TEST_DURATION; i++) {
        // cout << ">  Iteration [" << i << "], Clock: " << HardwareClock::get_cpu_clock() << "Hz"
        //     << ", Temperature: " << static_cast<float>(Temperature_Sensor::get_temperature()) / 1000.0f << 'C' <<
        //     endl;
        cout << ">  Iteration [" << i << ']' << endl;

        Delay(1000000);
    }

    return 0;
}
