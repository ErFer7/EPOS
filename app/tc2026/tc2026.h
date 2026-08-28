#pragma once

#include <architecture/tsc.h>
#include <clerk.h>
#include <process.h>
#include <real-time.h>
#include <utility/convert.h>
#include <utility/random.h>

#include "adpcm_enc/adpcm_enc.h"
#include "ammunition/ammunition.h"
#include "anagram/anagram.h"
#include "audiobeam/audiobeam.h"
#include "bandwidth/bandwidth.h"
#include "bitcount/bitcount.h"
#include "cjpeg_transupp/cjpeg_transupp.h"
#include "cjpeg_wrbmp/cjpeg_wrbmp.h"
#include "cosf/cosf.h"
#include "deg2rad/deg2rad.h"
#include "dijkstra/dijkstra.h"
#include "fac/fac.h"
#include "fft/fft.h"
#include "fmref/fmref.h"
#include "g723_enc/g723_enc.h"
#include "gsm_enc/gsm_enc.h"
#include "h264_dec/h264_dec.h"
#include "huff_enc/huff_enc.h"
#include "iir/iir.h"
#include "lms/lms.h"
#include "ludcmp/ludcmp.h"
#include "matrix1/matrix1.h"
#include "md5/md5.h"
#include "minver/minver.h"
#include "mpeg2/mpeg2.h"
#include "ndes/ndes.h"
#include "petrinet/petrinet.h"
#include "pointer_chase/pointer_chase.h"
#include "prime/prime.h"
#include "quicksort/quicksort.h"
#include "recursion/recursion.h"
#include "rijndael_enc/rijndael_enc.h"
#include "sha/sha.h"
#include "statemate/statemate.h"
#include "susan/susan.h"

using namespace EPOS;

OStream cout;

// TODO: Check all of them
// TODO: Add an "industrial" benchmark (probably SDAV)
// TODO: Add Image Disparity
enum BenchmarkType {
    RIJNDAEL_ENC,  // Single: ~1935us Iteration WCET, Contention: ~3867us -> 2x FIX: Fails when two of them run
    KALMAN,        // TODO: Bring it to standard
    BANDWIDTH_L1,
    BANDWIDTH_L2,
    H264DEC,
    MPEG2,
    SUSAN,
    CJPEG_TRANSUPP,
    CJPEG_WRBMP,  // WARN: There are some problems, but it works
    AUDIOBEAM,    // FIX: Currently hangs after 4 iterations
    ANAGRAM,      // FIX: Currently hangs after 2 iterations
    PETRINET,
    FAC,
    PRIME,
    BITCOUNT,
    COSF,
    DEG2RAD,
    MD5,
    SHA,
    FFT,
    IIR,
    LMS,
    FILTERBANK,
    MINVER,
    LUDCMP,
    MATRIX1,
    QUICKSORT,
    RECURSION,
    DIJKSTRA,
    HUFF_ENC,
    ADPCM_ENC,  // NOTE: Under work
    GSM_ENC,
    G723_ENC,
    STATEMATE,
    NDES,        // TODO: Verify the static variables and see if something went wrong
    AMMUNITION,  // TODO: Check if the size_t is ok
    FMREF,
    POINTER_CHASE_L1,
    POINTER_CHASE_L2
};

struct StressTask {
    const unsigned int period;
    const unsigned int deadline;
    const unsigned int wcet;
    const unsigned int cpu;
    const BenchmarkType task;
    const float duration;  // Iteration duration
};

struct Taskset {
    const StressTask *tasks;
    const unsigned int size;
};

template <int TaskEnum>
struct BenchmarkTraits;

template <>
struct BenchmarkTraits<RIJNDAEL_ENC> {
    using Type = RijndaelEnc;
    static Type *create() { return new Type(); }  // Default constructor
};

template <>
struct BenchmarkTraits<BANDWIDTH_L1> {
    using Type = Bandwidth::Bandwidth;
    static Type *create() { return new Type(Bandwidth::Bandwidth::L1_CACHE_SIZE); }
};

template <>
struct BenchmarkTraits<BANDWIDTH_L2> {
    using Type = Bandwidth::Bandwidth;
    static Type *create() { return new Type(Bandwidth::Bandwidth::L2_CACHE_SIZE); }
};

template <>
struct BenchmarkTraits<H264DEC> {
    using Type = H264Dec;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<MPEG2> {
    using Type = Mpeg2;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<SUSAN> {
    using Type = Susan;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<CJPEG_TRANSUPP> {
    using Type = CJpegTransupp;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<CJPEG_WRBMP> {
    using Type = CJpegWRBMP;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<AUDIOBEAM> {
    using Type = Audiobeam::Audiobeam;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<ANAGRAM> {
    using Type = Anagram;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<PETRINET> {
    using Type = Petrinet;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<FAC> {
    using Type = Fac;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<PRIME> {
    using Type = Prime;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<BITCOUNT> {
    using Type = BitCount;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<COSF> {
    using Type = Cosf;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<DEG2RAD> {
    using Type = Deg2Rad;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<MD5> {
    using Type = Md5;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<SHA> {
    using Type = Sha;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<FFT> {
    using Type = Fft;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<IIR> {
    using Type = Iir;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<LMS> {
    using Type = Lms;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<FILTERBANK> {
    using Type = Lms;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<MINVER> {
    using Type = Minver;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<LUDCMP> {
    using Type = LudCmp;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<MATRIX1> {
    using Type = Matrix1;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<QUICKSORT> {
    using Type = Quicksort;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<RECURSION> {
    using Type = Recursion;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<DIJKSTRA> {
    using Type = Dijkstra;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<HUFF_ENC> {
    using Type = HuffEnc;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<ADPCM_ENC> {
    using Type = AdpcmEnc::AdpcmEnc;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<GSM_ENC> {
    using Type = GsmEnc;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<G723_ENC> {
    using Type = G723Enc;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<STATEMATE> {
    using Type = Statemate;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<NDES> {
    using Type = Ndes;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<AMMUNITION> {
    using Type = Ammunition;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<FMREF> {
    using Type = Fmref::Fmref;
    static Type *create() { return new Type(); }
};

template <>
struct BenchmarkTraits<POINTER_CHASE_L1> {
    using Type = PointerChase::PointerChase;
    static Type *create() { return new Type(PointerChase::PointerChase::L1_CACHE_SIZE); }
};

template <>
struct BenchmarkTraits<POINTER_CHASE_L2> {
    using Type = PointerChase::PointerChase;
    static Type *create() { return new Type(PointerChase::PointerChase::L2_CACHE_SIZE); }
};

class BenchmarkRunner {
    typedef TSC::Time_Stamp Time_Stamp;

   private:
    static const unsigned int TEST_DURATION = Traits<Build>::EXPECTED_SIMULATION_TIME - 10;  // in seconds
    static const unsigned int SELECTED_TASKSET = 1;
    static const unsigned int SEED = 20260610;

    static constexpr float SINGLE = 200000.0f;
    static constexpr float AES_IT_DURATION = 2800.0f;  // in microseconds
    static constexpr float KALMAN_IT_DURATION = 500.0f;
    static constexpr float BANDWIDTH_IT_DURATION = 2000.0f;

    static constexpr StressTask taskset_1[] = {
        {1000000, 1000000, 200000, 1, RIJNDAEL_ENC, SINGLE},
        {1000000, 1000000, 200000, 1, H264DEC, SINGLE},
        {1000000, 1000000, 200000, 1, FILTERBANK, SINGLE},
        {1000000, 1000000, 200000, 1, FILTERBANK, SINGLE},
        {1000000, 1000000, 200000, 1, MATRIX1, SINGLE},
        {1000000, 1000000, 200000, 1, POINTER_CHASE_L1, SINGLE},
        {1000000, 1000000, 200000, 2, BANDWIDTH_L2, BANDWIDTH_IT_DURATION},
        {1000000, 1000000, 200000, 2, MINVER, BANDWIDTH_IT_DURATION},
        {1000000, 1000000, 200000, 2, MINVER, BANDWIDTH_IT_DURATION},
        {1000000, 1000000, 200000, 2, QUICKSORT, BANDWIDTH_IT_DURATION},
        {1000000, 1000000, 200000, 2, QUICKSORT, BANDWIDTH_IT_DURATION},
        {1000000, 1000000, 200000, 3, BANDWIDTH_L1, BANDWIDTH_IT_DURATION},
        {1000000, 1000000, 200000, 3, BANDWIDTH_L1, BANDWIDTH_IT_DURATION},
        {1000000, 1000000, 200000, 3, LUDCMP, BANDWIDTH_IT_DURATION},
        {1000000, 1000000, 200000, 3, RECURSION, BANDWIDTH_IT_DURATION},
        {1000000, 1000000, 200000, 3, POINTER_CHASE_L2, BANDWIDTH_IT_DURATION},
    };  // HP = 1

    static constexpr Taskset tasksets[] = {{taskset_1, sizeof(taskset_1) / sizeof(StressTask)}};

    inline static constexpr Taskset taskset = tasksets[SELECTED_TASKSET - 1];
    inline static constexpr unsigned int task_count = taskset.size;

   public:
    inline static void init() {
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

        _rand->seed(SEED);
        _init_taskset<0>();
    }

    inline static void free() {
        cout << "Taskset deallocation..." << endl;
        _free_taskset<0>();
        cout << "Done." << endl;
    }

    // TODO: Remember the resume on RT!
    inline static void run() {
        Time_Stamp tsc0 = _get_time() + Convert::us2count<Time_Stamp, Time_Base>(TSC::frequency(), 10000);

        // TODO: Check this
        // Monitor::enable_captures();

        cout << "Creating threads..." << endl;
        _init_thread<0>(10000);
        cout << "Done" << endl;

        cout << "Current time: " << _us(tsc0) << endl;

        _logger = new Thread(Thread::Configuration(Thread::READY, Thread::Criterion(1000000, 1000000, 10000, 0)),
                             &_log_status);

        for (unsigned int i = 0; i < task_count; i++) {
            _threads[i]->join();
            // Monitor::disable_captures();
            cout << "Joined task [" << i << ']' << endl;
        }

        _logger->join();
        cout << "Joined logger" << endl;

        // Monitor::disable_captures();

        Time_Stamp times = _get_time() - tsc0;

        cout << "Returned to application main!" << endl;
        cout << "Elapsed: " << _us(times) << endl;
        cout << "Threads: " << task_count << endl;
    }

    inline static void print_thread_timing() {
        cout << "-----------------------------------------------------" << endl;
        cout << "...............Threads Timing Behavior..............." << endl;
        cout << "-----------------------------------------------------" << endl;

        for (unsigned int i = 0; i < task_count; i++) {
            cout << "Task [" << i << "]: " << _threads[i] << '\n'
                 << ">   Counted iterations: " << _current_iteration[i] << endl;

            const int total_jobs = _calc_jobs(i);
            const int total_iterations = total_jobs * _calc_iter_per_job(i);

            cout << ">   Execution time: " << _us(_task_runtime[i]) << " us\n"
                 << ">   Job WCET: " << _us(_job_wcet[i]) << " us\n"
                 << ">   Iteration WCET: " << _us(_iteration_wcet[i]) << " us\n"
                 << ">   Average job runtime: " << _us(_task_runtime[i] / total_jobs) << " us\n"
                 << ">   Average iteration runtime: " << _us(_task_runtime[i] / total_iterations) << " us\n"
                 << ">   Iterations per job: " << _calc_iter_per_job(i) << endl;
        }
    }

   private:
    static constexpr int _calc_iter_per_job(int i) {
        int iterations = static_cast<int>(static_cast<float>(taskset.tasks[i].wcet) / taskset.tasks[i].duration);
        if (iterations < 1) iterations = 1;

        return iterations;
    }

    static constexpr int _calc_jobs(int i) { return (TEST_DURATION * 1000000) / taskset.tasks[i].period; }

    inline static Time_Stamp _get_time() { return TSC::time_stamp(); }

    inline static Microsecond _us(Time_Stamp ts) {
        return Convert::count2us<Hertz, Time_Stamp, Time_Base>(TSC::frequency(), ts);
    }

    template <int ID>
    inline static void _init_taskset() {
        constexpr BenchmarkType benchmark_type = taskset.tasks[ID].task;

        auto *benchmark = BenchmarkTraits<benchmark_type>::create();
        _benchmarks[ID] = benchmark;

        _task_runtime[ID] = 0;
        _job_wcet[ID] = 0;
        _iteration_wcet[ID] = 0;
        _current_iteration[ID] = 0;
        _return[ID] = 0;

        _init_taskset<ID + 1>();
    }

    template <int ID>
    inline static void _free_taskset() {
        constexpr auto task = taskset.tasks[ID].task;
        using Benchmark = typename BenchmarkTraits<task>::Type;
        Benchmark *benchmark = static_cast<Benchmark *>(_benchmarks[ID]);

        delete benchmark;

        cout << ">  Deleting thread " << _threads[ID] << endl;

        delete _threads[ID];

        _free_taskset<ID + 1>();
    }

    template <unsigned int ID>
    inline static void _init_thread(Microsecond activation) {
        constexpr int job = _calc_jobs(ID);

        _threads[ID] = new RT_Thread(&_run_func<ID>,
                                     taskset.tasks[ID].period,
                                     taskset.tasks[ID].deadline,
                                     taskset.tasks[ID].wcet,
                                     activation,
                                     job,
                                     taskset.tasks[ID].cpu);

        cout << ">  Thread[" << ID << "]<" << _threads[ID] << ">: period = " << taskset.tasks[ID].period
             << ", deadline = " << taskset.tasks[ID].deadline << ", wcet = " << taskset.tasks[ID].wcet
             << ", activation = " << activation << ", times = " << job << ", cpu = " << taskset.tasks[ID].cpu << endl;

        _init_thread<ID + 1>(activation);
    }

    static int _log_status() {
        for (unsigned int i = 0; i < TEST_DURATION; i++) {
            // cout << ">  Iteration [" << i << "], Clock: " << HardwareClock::get_cpu_clock() << "Hz"
            //     << ", Temperature: " << static_cast<float>(Temperature_Sensor::get_temperature()) / 1000.0f << 'C' <<
            //     endl;
            cout << ">  Iteration [" << i << ']' << endl;

            for (unsigned int i = 0; i < task_count; i++) {
                cout << "   > Task [" << i << "], IWCET: " << _iteration_wcet[i] << "us; return -> " << _return[i]
                     << endl;
            }

            Delay(1000000);
        }

        return 0;
    }

    template <unsigned int ID>
    static void _run_func() {
        constexpr auto task = taskset.tasks[ID].task;
        using Benchmark = typename BenchmarkTraits<task>::Type;
        Benchmark *benchmark = static_cast<Benchmark *>(_benchmarks[ID]);

        unsigned int my_iter_per_job = _calc_iter_per_job(ID);

        Time_Stamp init = _get_time();

        for (unsigned int iterations = 0; iterations < my_iter_per_job; iterations++) {
            _return[ID] = benchmark->run();
        }

        Time_Stamp job_runtime = _get_time() - init;
        _task_runtime[ID] += job_runtime;

        _current_iteration[ID]++;

        Time_Stamp average_iteration_runtime = job_runtime / my_iter_per_job;

        if (average_iteration_runtime > _iteration_wcet[ID]) {
            _iteration_wcet[ID] = average_iteration_runtime;
        }

        if (job_runtime > _job_wcet[ID]) {
            _job_wcet[ID] = job_runtime;
        }
    }

   private:
    inline static Time_Stamp _task_runtime[task_count];
    inline static Time_Stamp _job_wcet[task_count];
    inline static Time_Stamp _iteration_wcet[task_count];

    inline static Random *_rand;
    inline static unsigned int _current_iteration[task_count];
    inline static Thread *_threads[task_count];
    inline static Thread *_logger;
    inline static void *_benchmarks[task_count];
    inline static int _return[task_count];
};

template <>
inline void BenchmarkRunner::_init_taskset<BenchmarkRunner::task_count>() {}

template <>
inline void BenchmarkRunner::_init_thread<BenchmarkRunner::task_count>(Microsecond activation) {}

template <>
inline void BenchmarkRunner::_free_taskset<BenchmarkRunner::task_count>() {}
