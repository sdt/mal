#ifndef INCLUDE_DEBUG_H
#define INCLUDE_DEBUG_H

#define DEBUG_TRACE                         1
#define DEBUG_SHOW_OBJECT_LIFETIMES         1

#define DEBUG_TRACE_FILE    stderr

#define NOOP    do { } while (false)
#define NOTRACE(...)    NOOP

#if DEBUG_TRACE
    #define TRACE(format, ...) fprintf(DEBUG_TRACE_FILE, format, __VA_ARGS__)
#else
    #define TRACE NOTRACE
#endif

#if DEBUG_SHOW_OBJECT_LIFETIMES
    #define TRACE_OBJECT TRACE
#else
    #define TRACE_OBJECT NOTRACE
#endif

#endif // INCLUDE_DEBUG_H
