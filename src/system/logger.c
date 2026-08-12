/**
 * @file logger.c
 * @author pierre@wisear.io
 * @brief
 * @version 0.1
 * @date 2024-03-18
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "logger.h"

#ifdef DEBUG

NaqiLogLevel_t eCurrentLogLevel = ACTIVE_LOG_LEVEL;
const char *const apcLogLevelStringMap[] = {"DBG", "INF", "WAR", "ERR"};

#ifdef LOG_COLOR_ENABLED
const char *const apcLogLevelColorMap[] = {NAQILOG_CYAN, NAQILOG_WHITE, NAQILOG_YELLOW, NAQILOG_RED};
#endif

#endif // DEBUG
