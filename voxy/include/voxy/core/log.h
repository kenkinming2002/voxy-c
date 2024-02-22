#ifndef VOXY_CORE_LOG_H
#define VOXY_CORE_LOG_H

#include <stdio.h>

#define LOG_INFO(fmt, ...)  do { fprintf(stderr, "[" "\033[32m" "INFO"  "\033[37m" "] " fmt "\n", ##__VA_ARGS__); } while(0)
#define LOG_WARN(fmt, ...)  do { fprintf(stderr, "[" "\033[33m" "WARN"  "\033[37m" "] " fmt "\n", ##__VA_ARGS__); } while(0)
#define LOG_ERROR(fmt, ...) do { fprintf(stderr, "[" "\033[31m" "ERROR" "\033[37m" "] " fmt "\n", ##__VA_ARGS__); } while(0)

#endif // VOXY_CORE_LOG_H
