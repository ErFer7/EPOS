// EPOS Scheduler Test Program

#include <process.h>
#include <synchronizer.h>
#include <time.h>

using namespace EPOS;

const int iterations = 1000;
const int thread_count = 2;

Mutex mutex;

Thread *threads[thread_count];

OStream cout;

int test(int n);

int main() {
    mutex.lock();
    cout << "Basic test" << endl;

    for (int i = 0; i < thread_count; i++) {
        threads[i] = new Thread(&test, i);
    }

    cout << "The threads are ready" << endl;
    mutex.unlock();

    for (int i = 0; i < thread_count; i++) {
        int ret = threads[i]->join();
        mutex.lock();
        cout << "Thread [" << ret << "] finished" << endl;
        mutex.unlock();
    }

    for (int i = 0; i < thread_count; i++) {
        delete threads[i];
    }

    cout << "Test result: OK" << endl;

    return 0;
}

int test(int n) {
    for (int i = 0; i < iterations; i++) {
        mutex.lock();
        cout << "[" << n << "B" << n << "]" << endl;
        mutex.unlock();
    }

    return n;
}
