/**
 * @file logger.h
 * @author pierre@wisear.io
 * @brief
 * @version 0.1
 * @date 2024-03-18
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef LOGGER_H
#define LOGGER_H

#define EMPTY_OPERATION \
    do                  \
    {                   \
    } while (0)

#ifdef DEBUG

    #include <stdio.h>

    // printf-like function
    #ifndef NAQILOG_PRINTF
        #define NAQILOG_PRINTF printf
    #endif

typedef enum
{
    NAQILOG_LEVEL_DEBUG,
    NAQILOG_LEVEL_INFO,
    NAQILOG_LEVEL_WARN,
    NAQILOG_LEVEL_ERROR,
    NAQILOG_LEVEL_NONE,
} NaqiLogLevel_t;

    #define NAQILOG_RESET   "\x1b[0m"
    #define NAQILOG_BOLD    "\x1b[1m"
    #define NAQILOG_BLACK   "\x1b[30m"
    #define NAQILOG_RED     "\x1b[31m"
    #define NAQILOG_GREEN   "\x1b[32m"
    #define NAQILOG_YELLOW  "\x1b[33m"
    #define NAQILOG_BLUE    "\x1b[34m"
    #define NAQILOG_MAGENTA "\x1b[35m"
    #define NAQILOG_CYAN    "\x1b[36m"
    #define NAQILOG_WHITE   "\x1b[37m"

extern NaqiLogLevel_t    eCurrentLogLevel;
extern const char *const apcLogLevelStringMap[];

    #ifdef LOG_COLOR_ENABLED
extern const char *const apcLogLevelColorMap[];
    #endif

    #ifndef ACTIVE_LOG_LEVEL
        #define ACTIVE_LOG_LEVEL NAQILOG_LEVEL_INFO
    #endif

    // Set current trace level: `LOG_LEVEL_(DEBUG, INFO, WARN, ERROR, NONE)`
    #define NAQILOG_SET_LEVEL(LOG_LEVEL)  \
        do                                \
        {                                 \
            eCurrentLogLevel = LOG_LEVEL; \
        } while (0)

    // Prints filename and line number in header
    #ifdef LOG_FILE_LINE_ENABLED
        #define NAQILOG_FILE_LINE(file, line) NAQILOG_PRINTF("%s (%d): ", file, line)
    #else
        #define NAQILOG_FILE_LINE(file, line) EMPTY_OPERATION
    #endif

    // Prints trace level with colors (if activated)
    #ifdef LOG_COLOR_ENABLED
        #define NAQILOG_PRINT_HEADER(level) \
            NAQILOG_PRINTF("[%s%s" NAQILOG_RESET "] ", apcLogLevelColorMap[level], apcLogLevelStringMap[level])
    #else
        #define NAQILOG_PRINT_HEADER(level) NAQILOG_PRINTF("[%s] ", apcLogLevelStringMap[level])
    #endif

    #define NAQILOG_LOG(level, file, line, ...)                              \
        do                                                                   \
        {                                                                    \
            if ((level >= eCurrentLogLevel) && (level < NAQILOG_LEVEL_NONE)) \
            {                                                                \
                NAQILOG_PRINT_HEADER(level);                                 \
                NAQILOG_FILE_LINE(file, line);                               \
                NAQILOG_PRINTF(__VA_ARGS__);                                 \
                NAQILOG_PRINTF("\n");                                        \
            }                                                                \
        } while (0)

    #define NAQILOG_DEBUG(...)          NAQILOG_LOG(NAQILOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
    #define NAQILOG_INFO(...)           NAQILOG_LOG(NAQILOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
    #define NAQILOG_WARN(...)           NAQILOG_LOG(NAQILOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
    #define NAQILOG_ERROR(...)          NAQILOG_LOG(NAQILOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

    #define NAQILOG_HEX_ARRAY_MAX_BYTES 128

    #define NAQILOG_HEX_ARRAY(buf, size)                                                                              \
        do                                                                                                            \
        {                                                                                                             \
            static const char hexDigits[] = "0123456789abcdef";                                                       \
            char              hexStr[NAQILOG_HEX_ARRAY_MAX_BYTES * 2 + 1];                                            \
            unsigned int      hexLen = ((size) < NAQILOG_HEX_ARRAY_MAX_BYTES) ? (size) : NAQILOG_HEX_ARRAY_MAX_BYTES; \
            for (unsigned int i = 0; i < hexLen; i++)                                                                 \
            {                                                                                                         \
                uint8_t byte = ((const uint8_t *)(buf))[i];                                                           \
                hexStr[i * 2] = hexDigits[byte >> 4];                                                                 \
                hexStr[i * 2 + 1] = hexDigits[byte & 0x0F];                                                           \
            }                                                                                                         \
            hexStr[hexLen * 2] = '\0';                                                                                \
            NAQILOG_PRINTF("0x%s%s\n", hexStr, (hexLen < (size)) ? "..." : "");                                       \
        } while (0)

#else

    #define NAQILOG_DEBUG(...)     EMPTY_OPERATION
    #define NAQILOG_INFO(...)      EMPTY_OPERATION
    #define NAQILOG_WARN(...)      EMPTY_OPERATION
    #define NAQILOG_ERROR(...)     EMPTY_OPERATION
    #define NAQILOG_HEX_ARRAY(...) EMPTY_OPERATION

#endif  // DEBUG

#endif  // LOGGER_H
