/*
Copyright 2018 Tobii AB

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "stopwatch.h"

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

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
    return (long)elapsed.QuadPart;
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
        return -1;
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

#else

#include <time.h>

Stopwatch* stopwatch_init() {
    // TODO: Implement
    return NULL;
}

void stopwatch_start(Stopwatch* instance) {
    // TODO: Implement
}

long stopwatch_stop(Stopwatch* instance) {
    // TODO: Implement
    return 0;
}

long stopwatch_elapsed(Stopwatch* instance) {
    // TODO: Implement
    return 0;
}

void stopwatch_reset(Stopwatch* instance) {
    // TODO: Implement
}

#endif
