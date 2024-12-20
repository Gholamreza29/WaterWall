#include "ww_logger.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct logger_s;
static logger_t *logger = NULL;

static void destroyWWLogger(void)
{
    if (logger)
    {
        logger_fsync(logger);
        logger_destroy(logger);
        logger = NULL;
    }
}

static void wwLoggerHandleWithStdStream(int loglevel, const char *buf, int len)
{
    switch (loglevel)
    {
    case LOG_LEVEL_WARN:
    case LOG_LEVEL_ERROR:
    case LOG_LEVEL_FATAL:
        stderr_logger(loglevel, buf, len);
        break;
    default:
        stdout_logger(loglevel, buf, len);
        break;
    }
    logfile_write(logger, buf, len);
}

static void wwLoggerHandle(int loglevel, const char *buf, int len)
{
    (void) loglevel;
    logfile_write(logger, buf, len);
}

logger_t *getWWLogger(void)
{
    return logger;
}
void setWWLogger(logger_t *newlogger)
{
    assert(logger == NULL);
    logger = newlogger;
}

logger_t *createWWLogger(const char *log_file, bool console)
{
    assert(logger == NULL);
    logger = logger_create();
    logger_set_file(logger, log_file);
    if (console)
    {
        logger_set_handler(logger, wwLoggerHandleWithStdStream);
    }
    else
    {
        logger_set_handler(logger, wwLoggerHandle);
    }

    atexit(destroyWWLogger);
    return logger;
}

logger_handler getWWLoggerHandle(void)
{
    return logger_handle(logger);
}
