#ifndef  _UTILITY_H_
#define  _UTILITY_H_

// things we might want to know
#if defined(NDEBUG)
#define debug
#else
#define debug(params) printf params;
#endif

// things we need to know
#if defined (NCRITICAL)
#define critical
#else
#define critical(params) printf params;
#endif
#endif