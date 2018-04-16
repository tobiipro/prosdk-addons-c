#ifdef _WIN32 || _WIN64
#include <windows.h>
#else
// TODO: Implement also for POSIX
#include <time.h>
#endif

#include "stopwatch.h"

struct Stopwatch {
    LARGE_INTEGER freq;
    LARGE_INTEGER last_start_time;
    LARGE_INTEGER total_time;
    int running;
};

static long diff_milliseconds(LARGE_INTEGER* timespan, LARGE_INTEGER* freq) {
    LARGE_INTEGER elapsed;
    elapsed.QuadPart = timespan->QuadPart;
    elapsed.QuadPart *= 1000;
    elapsed.QuadPart /= freq->QuadPart;
    return elapsed.QuadPart;
}

Stopwatch* stopwatch_init() {
    Stopwatch *instance = malloc(sizeof(*instance));
    QueryPerformanceFrequency(&instance->freq);
    stopwatch_reset(instance);
    return instance;
}

void stopwatch_start(Stopwatch* instance) {
    if (instance->running)
        return;
    QueryPerformanceCounter(&instance->last_start_time);
    instance->running = 1;
}

long stopwatch_stop(Stopwatch* instance) {
    if (!instance->running)
        return;
    instance->running = 0;
    LARGE_INTEGER stop_time;
    QueryPerformanceCounter(&stop_time);
    instance->total_time.QuadPart += stop_time.QuadPart - instance->last_start_time.QuadPart;
    return diff_milliseconds(&instance->total_time, &instance->freq);
}

long stopwatch_elapsed(Stopwatch* instance) {
    if (!instance->running) {
        return 0;
    }
    LARGE_INTEGER accumulated_time;
    LARGE_INTEGER current_time;
    QueryPerformanceCounter(&current_time);
    accumulated_time.QuadPart = instance->total_time.QuadPart;
    accumulated_time.QuadPart += current_time.QuadPart - instance->last_start_time.QuadPart;
    return diff_milliseconds(&accumulated_time, &instance->freq);
}

void stopwatch_reset(Stopwatch* instance) {
    instance->total_time.QuadPart = 0;
    instance->running = 0;
}
