// EPOS Scheduler Test Program

#include <process.h>
#include <synchronizer.h>
#include <time.h>
#include "architecture.h"

using namespace EPOS;

const int iterations = 100;
const int thread_count = 4;

Mutex mutex;

Thread *threads[thread_count];

OStream cout;

int test(int n);

int main() {
    cout << '[' << CPU::id() << "]: Basic test" << endl;

    for (int i = 0; i < thread_count; i++) {
        threads[i] = new Thread(&test, i);
    }

    for (int i = 0; i < thread_count; i++) {
        int ret = threads[i]->join();
        mutex.lock();
        cout << '[' << CPU::id() << "]: Thread [" << ret << "] finished" << endl;
        mutex.unlock();
    }

    for (int i = 0; i < thread_count; i++) {
        delete threads[i];
    }

    cout << '[' << CPU::id() << "]: Test result: OK" << endl;

    return 0;
}

int test(int n) {
    for (int i = 0; i < iterations; i++) {
        mutex.lock();
        cout << '[' << CPU::id() << "]: "<< n << endl;
        mutex.unlock();
    }

    return n;
}
