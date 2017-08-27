#ifndef  _UTILITY_H_
#define  _UTILITY_H_

#include <stdio.h>

// enable to write debug messages to output file
#define DEBUG_FILE

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

/** @brief opens a debug file that will be written to by subsequent
 *			debug_write calls.
 *  @param output_file null terminated string for file to open
 *  @return 0 on sucess, -1 on failure.
*/
int debug_init(char *output_file);

/** @brief writes a debug message to the file previously open with debug_init
 *			function.
 *  @param message null terminated string with message to be written to the
 *			debug file.
 *  @return Void.
*/
void debug_write(char *message);

/** @brief closes the debug file handle.
 *  @return Void.
*/
void debug_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif