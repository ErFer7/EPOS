// EPOS Scheduler Component Declarations

#ifndef __scheduler_h
#define __scheduler_h

#include <architecture/cpu.h>
#include <architecture/pmu.h>
#include <architecture/tsc.h>
#include <utility/scheduling.h>
#include <utility/math.h>
#include <utility/convert.h>
#include <utility/fann.h>

__BEGIN_SYS

// All scheduling criteria, or disciplines, must define operator int() with
// the semantics of returning the desired order of a given object within the
// scheduling list
class Scheduling_Criterion_Common
{
    friend class Thread;                // for handle()
    friend class Periodic_Thread;       // for handle()
    friend class RT_Thread;             // for handle()
    friend class Clerk<System>;         // for _statistics

protected:
    typedef IF<Traits<TSC>::enabled, TSC_Common::Time_Stamp, Timer_Common::Tick>::Result Tick;

public:
    // Priorities
    enum : int {
        CEILING = -1000,
        MAIN    = -1,
        HIGH    = 0,
        NORMAL  = (unsigned(1) << (sizeof(int) * 8 - 3)) - 1,
        LOW     = (unsigned(1) << (sizeof(int) * 8 - 2)) - 1,
        IDLE    = (unsigned(1) << (sizeof(int) * 8 - 1)) - 1
    };

    // Constructor helpers
    enum : unsigned int {
        SAME        = 0,
        NOW         = 0,
        UNKNOWN     = 0,
        ANY         = -1U
    };

    // Policy types
    enum : int {
        PERIODIC    = HIGH,
        SPORADIC    = NORMAL,
        APERIODIC   = LOW
    };

    // Policy events
    typedef int Event;
    enum {
        CREATION        = 1 << 0,
        FINISH          = 1 << 1,
        DISPATCH        = 1 << 2,
        RETREAT         = 1 << 3,
        JOB_RELEASE     = 1 << 4,
        JOB_FINISH      = 1 << 5
    };

    // Policy operations
    typedef int Operation;
    enum {
        COLLECT         = 1 << 16,
        CHARGE          = 1 << 17,
        AWARD           = 1 << 18,
        UPDATE          = 1 << 19,
        REBALANCE       = 1 << 20,
        HALT            = 1 << 21
    };

    // Policy traits
    static const bool timed = false;
    static const bool dynamic = false;
    static const bool preemptive = true;

    // Multicore algorithms
    static const unsigned int QUEUES = 1;
    static const unsigned int HEADS = 1;

    // Runtime Statistics (for policies that don't use any; that's why its a union)
    union Dummy_Statistics {  // for Traits<System>::monitored = false
        // Thread related statistics
        Tick thread_creation;                   // tick in which the thread was created
        Tick thread_destruction;                // tick in which the thread was destroyed
        Tick thread_execution_time;             // accumulated execution time (in ticks)
        Tick thread_last_dispatch;              // tick in which the thread was last dispatched to the CPU
        Tick thread_last_preemption;            // tick in which the thread left the CPU by the last time

        // Job related statistics
        Tick job_release;                       // tick in which the last job of a periodic thread was made ready for execution
        Tick job_start;                         // tick in which the last job of a periodic thread started (different from "thread_last_dispatch" since jobs can be preempted)
        Tick job_finish;                        // tick in which the last job of a periodic thread finished (i.e. called _alarm->p() at wait_netxt(); different from "thread_last_preemption" since jobs can be preempted)
        Tick job_utilization;                   // accumulated execution time (in ticks)
        unsigned int jobs_released;             // number of jobs of a thread that were released so far (i.e. the number of times _alarm->v() was called by the Alarm::handler())
        unsigned int jobs_finished;             // number of jobs of a thread that finished execution so far (i.e. the number of times alarm->p() was called at wait_next())

        static Tick cpu_time[Traits<Build>::CPUS];
    };

    struct Real_Statistics {  // for Traits<System>::monitored = true
        // Thread related statistics
        Tick thread_creation;                   // tick in which the thread was created
        Tick thread_destruction;                // tick in which the thread was destroyed
        Tick thread_execution_time;             // accumulated execution time (in ticks)
        Tick thread_last_dispatch;              // tick in which the thread was last dispatched to the CPU
        Tick thread_last_preemption;            // tick in which the thread left the CPU by the last time

        // Job related statistics
        Tick job_release;                       // tick in which the last job of a periodic thread was made ready for execution
        Tick job_start;                         // tick in which the last job of a periodic thread started (different from "thread_last_dispatch" since jobs can be preempted)
        Tick job_finish;                        // tick in which the last job of a periodic thread finished (i.e. called _alarm->p() at wait_netxt(); different from "thread_last_preemption" since jobs can be preempted)
        Tick job_utilization;                   // accumulated execution time (in ticks)
        unsigned int jobs_released;             // number of jobs of a thread that were released so far (i.e. the number of times _alarm->v() was called by the Alarm::handler())
        unsigned int jobs_finished;             // number of jobs of a thread that finished execution so far (i.e. the number of times alarm->p() was called at wait_next())

        static Tick cpu_time[Traits<Build>::CPUS];
    };

    typedef IF<Traits<System>::monitored, Real_Statistics, Dummy_Statistics>::Result Statistics;

protected:
    Scheduling_Criterion_Common() {}

public:
    Microsecond period() { return 0;}
    Microsecond deadline() { return 0; }
    Microsecond capacity() { return 0; }

    bool periodic() { return false; }

    volatile Statistics & statistics() { return _statistics; }

protected:
    void handle(Event event);

    unsigned int queue() const { return 0; }
    void queue(unsigned int q) {}

    static Tick now();

    static void init() {}

protected:
    Statistics _statistics;
};

// Priority (static and dynamic)
class Priority: public Scheduling_Criterion_Common
{
public:
    template <typename ... Tn>
    Priority(int i = NORMAL, Tn & ... an): _priority(i) {}

    operator const volatile int() const volatile { return _priority; }

protected:
    volatile int _priority;
};

// Round-Robin
class RR: public Priority
{
public:
    static const bool timed = true;
    static const bool dynamic = false;
    static const bool preemptive = true;

public:
    template <typename ... Tn>
    RR(int i = NORMAL, Tn & ... an): Priority(i) {}
};

// First-Come, First-Served (FIFO)
class FCFS: public Priority
{
public:
    static const bool timed = false;
    static const bool dynamic = false;
    static const bool preemptive = false;

public:
    template <typename ... Tn>
    FCFS(int i = NORMAL, Tn & ... an);
};


// Real-time Algorithms
class RT_Common: public Priority
{
    friend class FCFS;
    friend class Thread;                // for handle() and queue()
    friend class Periodic_Thread;       // for handle() and queue()
    friend class RT_Thread;             // for handle() and queue()

public:
    static const bool timed = true;
    static const bool preemptive = true;

protected:
    RT_Common(int i): Priority(i), _period(0), _deadline(0), _capacity(0) {} // aperiodic
    RT_Common(int i, Microsecond p, Microsecond d, Microsecond c): Priority(i), _period(ticks(p)), _deadline(ticks(d ? d : p)), _capacity(ticks(c)) {}

public:
    Microsecond period() { return time(_period); }
    Microsecond deadline() { return time(_deadline); }
    Microsecond capacity() { return time(_capacity); }

    bool periodic() { return (_priority >= PERIODIC) && (_priority <= SPORADIC); }

    volatile Statistics & statistics() { return _statistics; }

protected:
    void handle(Event event);

    Tick ticks(Microsecond time);
    Microsecond time(Tick ticks);

protected:
    Tick _period;
    Tick _deadline;
    Tick _capacity;
    Statistics _statistics;
};

// Rate Monotonic
class RM: public RT_Common
{
public:
    static const bool dynamic = false;

public:
    RM(int p = APERIODIC): RT_Common(p) {}
    RM(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN): RT_Common(int(ticks(p)), p, d, c) {}
};

// Deadline Monotonic
class DM: public RT_Common
{
public:
    static const bool dynamic = false;

public:
    DM(int p = APERIODIC): RT_Common(p) {}
    DM(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN): RT_Common(int(ticks(d ? d : p)), p, d, c) {}
};

// Laxity Monotonic
class LM: public RT_Common
{
public:
    static const bool dynamic = false;

public:
    LM(int p = APERIODIC): RT_Common(p) {}
    LM(Microsecond p, Microsecond d, Microsecond c): RT_Common(int(ticks((d ? d : p) - c)), p, d, c) {}
};

// Earliest Deadline First
class EDF: public RT_Common
{
public:
    static const bool dynamic = true;

public:
    EDF(int p = APERIODIC): RT_Common(p) {}
    EDF(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN);

    void handle(Event event);
};

// Least Laxity First
class LLF: public RT_Common
{
public:
    static const bool dynamic = true;

public:
    LLF(int p = APERIODIC): RT_Common(p) {}
    LLF(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN);

    void handle(Event event);
};


// Multicore Algorithms
class Variable_Queue_Scheduler
{
protected:
    Variable_Queue_Scheduler(unsigned int queue): _queue(queue) {};

    unsigned int queue() const volatile { return _queue; }
    void queue(unsigned int q) { _queue = q; }

protected:
    volatile unsigned int _queue;
    static volatile unsigned int _next_queue;
};


// Global Round-Robin
class GRR: public RR
{
public:
    static const unsigned int HEADS = Traits<Machine>::CPUS;

public:
    template <typename ... Tn>
    GRR(int p = NORMAL, Tn & ... an): RR(p) {}

    static unsigned int current_head() { return CPU::id(); }
};

// Fixed CPU (fully partitioned)
class Fixed_CPU: public Priority, public Variable_Queue_Scheduler
{
public:
    static const bool dynamic = false;

    static const unsigned int QUEUES = Traits<Machine>::CPUS;

public:
    template <typename ... Tn>
    Fixed_CPU(int p = NORMAL, unsigned int cpu = ANY, Tn & ... an)
    : Priority(p), Variable_Queue_Scheduler(((_priority == IDLE) || (_priority == MAIN)) ? CPU::id() : (cpu != ANY) ? cpu : ++_next_queue %= CPU::cores()) {}

    using Variable_Queue_Scheduler::queue;
    static unsigned int current_queue() { return CPU::id(); }
};

// CPU Affinity
class CPU_Affinity: public Priority, public Variable_Queue_Scheduler
{
public:
    static const bool dynamic = false;
    static const unsigned int QUEUES = Traits<Machine>::CPUS;

public:
    template <typename ... Tn>
    CPU_Affinity(int p = NORMAL, unsigned int cpu = ANY, Tn & ... an)
    : Priority(p), Variable_Queue_Scheduler(((_priority == IDLE) || (_priority == MAIN)) ? CPU::id() : (cpu != ANY) ? cpu : ++_next_queue %= CPU::cores()) {}

    void handle(Event event);

    using Variable_Queue_Scheduler::queue;
    static unsigned int current_queue() { return CPU::id(); }

private:
    Statistics _statistics;
};


/// Partitioned Rate Monotonic (multicore)
class PRM: public RM, public Variable_Queue_Scheduler
{
public:
    static const unsigned int QUEUES = Traits<Machine>::CPUS;

public:
    PRM(int p = APERIODIC)
: RM(p), Variable_Queue_Scheduler(((_priority == IDLE) || (_priority == MAIN)) ? CPU::id() : 0) {}

    PRM(const Microsecond & d, const Microsecond & p = SAME, const Microsecond & c = UNKNOWN, unsigned int cpu = ANY)
    : RM(d, p, c), Variable_Queue_Scheduler((cpu != ANY) ? cpu : ++_next_queue %= CPU::cores()) {}

    using Variable_Queue_Scheduler::queue;
    static unsigned int current_queue() { return CPU::id(); }
};

// Global Earliest Deadline First (multicore)
class GEDF: public EDF
{
public:
    static const unsigned int HEADS = Traits<Machine>::CPUS;

public:
    GEDF(int p = APERIODIC): EDF(p) {}
    GEDF(const Microsecond & d, const Microsecond & p = SAME, const Microsecond & c = UNKNOWN, unsigned int cpu = ANY)
    : EDF(d, p, c) {}

    unsigned int queue() const { return current_head(); }
    void queue(unsigned int q) {}
    static unsigned int current_head() { return CPU::id(); }
};

// Partitioned Earliest Deadline First (multicore)
class PEDF: public EDF, public Variable_Queue_Scheduler
{
public:
    static const unsigned int QUEUES = Traits<Machine>::CPUS;

public:
    PEDF(int p = APERIODIC)
    : EDF(p), Variable_Queue_Scheduler(((_priority == IDLE) || (_priority == MAIN)) ? CPU::id() : 0) {}

    PEDF(const Microsecond & d, const Microsecond & p = SAME, const Microsecond & c = UNKNOWN, unsigned int cpu = ANY)
    : EDF(d, p, c), Variable_Queue_Scheduler((cpu != ANY) ? cpu : ++_next_queue %= CPU::cores()) {}

    using Variable_Queue_Scheduler::queue;
    static unsigned int current_queue() { return CPU::id(); }
};

#ifdef __PMU_H
// Energy-Aware ANN - Partitioned Earliest Deadline First (multicore)
class EA_PEDF: public PEDF
{
    friend class _SYS::Thread;
    friend class _SYS::Periodic_Thread;
    friend class _SYS::RT_Thread;
    friend class _SYS::Clerk<System>;         // for _statistics

public:
    static const bool collecting = true;
    static const bool charging = true;
    static const bool awarding = true;
    static const bool migrating = true;
    static const bool system_wide = true;

    // PMU Monitoring
    static const unsigned int PMU_EVENTS = 6;

    // ANN
    static const unsigned int INPUTS = PMU_EVENTS + 2; // Thread Execution Time and Current CPU Frequency
    static const unsigned int MAX_TRAINS = 8;
    static const unsigned int SAMPLES_PER_TRAIN_LIMIT = 16;
    static constexpr float ANN_DEVIATION_THRESHOLD = 0.02;

    // Voting and DVFS
    static constexpr float SAFETY_MARGIN = 0.05;
    static const unsigned int FREQUENCY_LEVEL = 100 * 1000 * 1000; // 100 MHz

    // Activity Weights
    static const bool TRAIN_WEIGHTS = false;
    static constexpr float LEARNING_RATE = 0.7;

    struct Activity {
        Activity() { reset(); }

        Activity(const Activity &a) {
            for(unsigned int i = 0; i < PMU_EVENTS; ++i)
                vector[i] = a.vector[i];
            utilization = a.utilization;
        }

        void reset() {
            for(unsigned int i = 0; i < PMU_EVENTS; ++i)
                vector[i] = 0;
            utilization = 0;
        }

        void operator+=(const Activity & a) {
            for(unsigned int i = 0; i < PMU_EVENTS; ++i)
                vector[i] += a.vector[i];
            utilization += a.utilization;
        }

        void operator-=(const Activity & a) {
            for(unsigned int i = 0; i < PMU_EVENTS; ++i)
                vector[i] -= a.vector[i];
            utilization -= a.utilization;
        }

        float sum() {
            float result = 0;
            for(unsigned int i = 0; i < PMU_EVENTS; ++i)
                result += vector[i];
            return result;
        }

        bool fits(float* _vector) {
            bool result = true;
            for(unsigned int i = 0; i < PMU_EVENTS; ++i)
                result &= vector[i]+ _vector[i] < (1 * _activity_weights[i]);
            return result;
        }

        bool fits(const Activity & a) {
            bool result = true;
            for(unsigned int i = 0; i < PMU_EVENTS; ++i)
                result &= vector[i]+ a.vector[i] <= (1 * _activity_weights[i]);
            return result;// && (a.utilization + utilization < 1 - Traits<Criterion>::VARIANCE_THRESHOLDS[0]);
        }

        float vector[PMU_EVENTS];
        float utilization;
    };

    struct Statistics {
        Statistics(): thread_execution_time(0), last_thread_dispatch(0), destination_cpu(ANY), output(0) {
            for(unsigned int i = 1; i < Traits<Build>::CPUS; ++i)
                migration_locked[i] = false;

            for(unsigned int i = 0; i < PMU_EVENTS; ++i) {
                input[i] = 0;
                last_pmu_read[i] = 0;
                pmu_accumulated[i] = 0;
            }
            input[6] = 0; // Thread Execution Time
            input[7] = 0; // CPU Frequency
        }

        // Thread Execution Time
        TSC::Time_Stamp thread_execution_time;  // Sum of jobs execution time
        TSC::Time_Stamp last_thread_dispatch;   // The timestamp of the last dispatch
        // On context-switch: execution time += TSC::timestamp() - last_dispatch
        // Migration
        unsigned int destination_cpu; // to help migration
        bool migration_locked[Traits<Build>::CPUS];

        // ANN
        float input[INPUTS];
        float output;

        Activity activity;
        Activity last_activity;

        // Per-Task Monitoring PMU
        PMU::Count pmu_accumulated[PMU_EVENTS];   // accumulated PMU counters during thread execution
        PMU::Count last_pmu_read[PMU_EVENTS];     // the pmu read() when thread entered CPU

        // Dealine Miss count - Used By Clerk<System>
        Alarm * alarm_times;            // Reference to RT_Thread private alarm (for monitoring pourposes)
        unsigned int finished_jobs;     // Number of finished jobs  <=> times alarm->p() has been called for this task
        unsigned int missed_deadlines;  // Number of finished jobs (finished_jobs) - number of dispatched jobs (alarm_times->times)

        // CPU Execution Time
        static TSC::Time_Stamp _cpu_time[Traits<Build>::CPUS];              // accumulated cpu time at each hyperperiod
        static TSC::Time_Stamp _last_hyperperiod[Traits<Build>::CPUS];      // Time Stamp of last hyperperiod
        static TSC::Time_Stamp _hyperperiod;                                // Global Hyperperiod
        static TSC::Time_Stamp _hyperperiod_cpu_time[Traits<Build>::CPUS];  // cpu time in last hyperperiod

        static Activity _activity_cpu[Traits<Build>::CPUS];
        static Activity _last_activity_cpu[Traits<Build>::CPUS];
    };

public:
    EA_PEDF(int p = APERIODIC): PEDF(p) {}

    EA_PEDF(const Microsecond & d, const Microsecond & p = SAME, const Microsecond & c = UNKNOWN, unsigned int cpu = ANY)
    : PEDF(d, p, c, cpu) {
        if(Statistics::_hyperperiod == 0)
            Statistics::_hyperperiod = Convert::us2count<TSC::Time_Stamp, Time_Base>(TSC::frequency(), p);
        else
            Statistics::_hyperperiod = Math::lcm(Statistics::_hyperperiod, Convert::us2count<TSC::Time_Stamp, Time_Base>(TSC::frequency(), p));
    }

    bool collect(bool end = false);
    bool charge(bool end = false);
    bool award(bool end = false);

    static void init();

    volatile Statistics & statistics() { return _statistics; }

    using PEDF::queue;

private:
    static Hertz _max_clock;
    static Hertz _min_clock;
    static Hertz _last_freq;

    static bool _imbalanced;
    static unsigned int _last_migration_from;
    static unsigned int _last_migration_to;
    static EA_PEDF *_last_migration;
    static EA_PEDF *_last_swap;
    static float _migration_optimization_threshold;
    static unsigned long long _imbalance_threshold;

    static EA_PEDF *_first_criterion[Traits<Build>::CPUS];
    static EA_PEDF *_last_criterion[Traits<Build>::CPUS];

    // Actuation Control
    static bool _cooldown[Traits<Build>::CPUS];                              // not actuate on cooldown
    static bool _decrease_frequency_vote[Traits<Build>::CPUS];               // CPU votes
    static bool _vote_ready[Traits<Build>::CPUS];                            // CPU vote ready
    static bool _to_learn[Traits<Build>::CPUS];                              // online learning
    static float _activity_weights[PMU_EVENTS];    // Migration feature weights

    // ANN Control
    static unsigned int _train_count[Traits<Build>::CPUS];
    static FANN::fann_type *_train_input[Traits<Build>::CPUS][SAMPLES_PER_TRAIN_LIMIT];
    static FANN::fann_type _desired_output[Traits<Build>::CPUS][SAMPLES_PER_TRAIN_LIMIT];
    static struct FANN::fann *_ann[Traits<Build>::CPUS];
    static FANN::fann_type _utilization_prediction[Traits<Build>::CPUS];

    // Collect control
    static unsigned int _collected[Traits<Build>::CPUS];

    static Clerk<PMU>* _bus_access_st_ca53_v8[Traits<Build>::CPUS];
    static Clerk<PMU>* _data_write_stall_st_buffer_full_ca53_v8[Traits<Build>::CPUS];
    static Clerk<PMU>* _immediate_branches_ca[Traits<Build>::CPUS];
    static Clerk<PMU>* _l2d_writeback[Traits<Build>::CPUS];
    static Clerk<PMU>* _cpu_cycles[Traits<Build>::CPUS];
    static Clerk<PMU>* _l1_cache_hits[Traits<Build>::CPUS];

private:
    Statistics _statistics;
    EA_PEDF * _next;
};

#endif

// Clustered Earliest Deadline First (multicore)
class CEDF: public EDF, public Variable_Queue_Scheduler
{
public:
    // QUEUES x HEADS must be equal to Traits<Machine>::CPUS
    static const unsigned int HEADS = 2;
    static const unsigned int QUEUES = Traits<Machine>::CPUS / HEADS;

public:
    CEDF(int p = APERIODIC)
    : EDF(p), Variable_Queue_Scheduler(((_priority == IDLE) || (_priority == MAIN)) ? current_queue() : 0) {} // Aperiodic

    CEDF(const Microsecond & d, const Microsecond & p = SAME, const Microsecond & c = UNKNOWN, unsigned int cpu = ANY)
    : EDF(d, p, c), Variable_Queue_Scheduler((cpu != ANY) ? cpu / HEADS : ++_next_queue %= CPU::cores() / HEADS) {}

    using Variable_Queue_Scheduler::queue;

    static unsigned int current_queue() { return CPU::id() / HEADS; }
    static unsigned int current_head() { return CPU::id() % HEADS; }
};

__END_SYS

__BEGIN_UTIL

// Scheduling Queues
template<typename T>
class Scheduling_Queue<T, GRR>:
public Multihead_Scheduling_List<T> {};

template<typename T>
class Scheduling_Queue<T, Fixed_CPU>:
public Scheduling_Multilist<T> {};

template<typename T>
class Scheduling_Queue<T, CPU_Affinity>:
public Scheduling_Multilist<T> {};

template<typename T>
class Scheduling_Queue<T, PRM>:
public Scheduling_Multilist<T> {};

template<typename T>
class Scheduling_Queue<T, GEDF>:
public Multihead_Scheduling_List<T> {};

template<typename T>
class Scheduling_Queue<T, PEDF>:
public Scheduling_Multilist<T> {};

template<typename T>
class Scheduling_Queue<T, EA_PEDF>:
public Scheduling_Multilist<T> {};

template<typename T>
class Scheduling_Queue<T, CEDF>:
public Multihead_Scheduling_Multilist<T> {};

__END_UTIL

#endif
