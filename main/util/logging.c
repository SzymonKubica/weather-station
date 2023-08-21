#include "logging.h"

unsigned char log_run_level = LOG_LVL_INFO;

const char *log_level_strings[] = {
    "NONE",    // 0
    "ERROR",   // 1
    "INFO",    // 2
    "DEBUG",   // 3
    "VERBOSE", // 4
};
