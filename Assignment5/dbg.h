/*
 * The following is based on Zed's awesome debug macros
 */
#ifndef DBG_H_INCLUDED
#define DBG_H_INCLUDED

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef DBG
// debug() - maskable debug message, expanded only if symbol DEBUG is defined
#define debug(M, ...) fprintf(stderr, "\n[DEBUG] %s:%d: " M, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define debug(M, ...)
#endif

// helper macro, not to be used on its own
#define clean_errno() (errno == 0 ? "None" : strerror(errno))

// logging macros - unlike debug() above, cannot be masked
#define log_err(M, ...)  fprintf(stderr, "\n[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define log_warn(M, ...) fprintf(stderr, "\n[WARN] (%s:%d: errno: %s) "  M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define log_info(M, ...) fprintf(stderr, "\n[INFO] (%s:%d) "             M "\n", __FILE__, __LINE__,                ##__VA_ARGS__)

// probe() checks an assertion A. if it fails, logs an error and goes to label 'error'
#define probe(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }
// sentinel() put into a part of a program that should not be accessed by program flow. if still accessed, logs error and goes to 'error'
#define sentinel(M, ...)          { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#endif
