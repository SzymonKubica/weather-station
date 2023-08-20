#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdlib.h>

enum {
  LOG_LVL_NONE,  // 0
  LOG_LVL_ERROR, // 1
  LOG_LVL_INFO,  // 2
  LOG_LVL_DEBUG, // 3
};

#ifndef LOG_BUILD_LEVEL
#ifdef NDEBUG
#define LOG_BUILD_LEVEL LVL_LOG
#else
#define LOG_BUILD_LEVEL LOG_LVL_DEBUG
#endif
#endif

extern unsigned char log_run_level;

extern const char *log_level_strings[];

#define LEVEL_ENABLED(level)                                                   \
  (level <= LOG_BUILD_LEVEL && level <= log_run_level)

#define LOG(level, fmt, ...)                                                   \
  do {                                                                         \
    if (LEVEL_ENABLED(level)) {                                                \
      printf("[%s] %s:%d: " fmt "\n", log_level_strings[level], __FUNCTION__,  \
             __LINE__, ##__VA_ARGS__);                                          \
    }                                                                          \
  } while (0)

#define LOG_INFO(fmt, ...) LOG(LOG_LVL_INFO, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG(LOG_LVL_ERROR, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) LOG(LOG_LVL_DEBUG, fmt, ##__VA_ARGS__)

#endif
