#ifndef _LOG_H_
#define _LOG_H_

#ifdef NDEBUG
#define log_info(str)
#define log_err(str)
#else
#include <stdio.h>
#define log_info(str) printf("[%s:%d]: %s\n", __FILE__, __LINE__, str)
#define log_err(str) fprintf(stderr, "[%s:%d]: %s\n", __FILE__, __LINE__, str)
#endif

#endif
