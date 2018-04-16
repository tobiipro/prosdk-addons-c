#ifndef STOPWATCH_H_
#define STOPWATCH_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Stopwatch Stopwatch;

extern Stopwatch* stopwatch_init();
extern void stopwatch_start(Stopwatch* instance);
extern long stopwatch_stop(Stopwatch* instance);
extern long stopwatch_elapsed(Stopwatch* instance);
extern void stopwatch_reset(Stopwatch* instance);

#ifdef __cplusplus
}
#endif

#endif  /* STOPWATCH_H_ */
