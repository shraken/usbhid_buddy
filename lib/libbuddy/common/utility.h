#ifndef  _UTILITY_H_
#define  _UTILITY_H_

#include <stdio.h>

// enable to write debug messages to output file
// #define DEBUG_FILE

// things we want to log to file
#if defined(DEBUG_FILE)
#define debugf(message) debug_write(message)
#else
#define debugf(message)
#endif

// things we might want to know
#if defined(NDEBUG)
#define debug
#else
#define debug(params) printf params;
#endif

// things we need to know
#if defined(NCRITICAL)
#define critical
#else
#define critical(params) printf params;
#endif

#ifdef __cplusplus
extern "C" {
#endif

int debug_init(char *output_file);
void debug_write(char *message);
void debug_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif