#include <iostream>
#include <fstream>
#include <string>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "logger.h"
#include "utils.h"

using namespace std;

#define LOG_BUFFER_LENGTH               4096

static pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

static int logLevel_atoi(const char * pszLoggingLevel) {
    int logLevel = 0;

    char * pszLogLevel = strdup(pszLoggingLevel);
    char * reference = pszLogLevel;

    char * pszToken = strtok_r(pszLogLevel, "|", &reference);

    while (pszToken != NULL) {
        string token = pszToken;

        if (token.find("LOG_LEVEL_INFO") != string::npos) {
            logLevel |= LOG_LEVEL_INFO;
        }
        else if (token.find("LOG_LEVEL_STATUS") != string::npos) {
            logLevel |= LOG_LEVEL_STATUS;
        }
        else if (token.find("LOG_LEVEL_DEBUG") != string::npos) {
            logLevel |= LOG_LEVEL_DEBUG;
        }
        else if (token.find("LOG_LEVEL_ERROR") != string::npos) {
            logLevel |= LOG_LEVEL_ERROR;
        }
        else if (token.find("LOG_LEVEL_FATAL") != string::npos) {
            logLevel |= LOG_LEVEL_FATAL;
        }

        pszToken = strtok_r(NULL, "|", &reference);
    }

    free(pszLogLevel);

    return logLevel;
}

static string & getLinePrefix(int logLevel) {
    static string prefix; 
    
    prefix = "[" + getTimestampUs() + "]";

    switch (logLevel) {
        case LOG_LEVEL_DEBUG:
            prefix += "[DBG]";
            break;

        case LOG_LEVEL_STATUS:
            prefix += "[STA]";
            break;

        case LOG_LEVEL_INFO:
            prefix += "[INF]";
            break;

        case LOG_LEVEL_ERROR:
            prefix += "[ERR]";
            break;

        case LOG_LEVEL_FATAL:
            prefix += "[FTL]";
            break;
    }

    return prefix;
}

void logger::logMessage(int logLevel, bool addCR, const char * fmt, va_list args) {
	pthread_mutex_lock(&_mutex);

    if (isLogLevel(logLevel)) {
        if (strlen(fmt) > MAX_LOG_LENGTH) {
            throw log_error(log_error::buildMsg("Log line too long, mudt be less than %d", MAX_LOG_LENGTH));
        }

        string prefix = getLinePrefix(logLevel);

        fwrite(prefix.c_str(), 1, prefix.length(), fptr);
        vfprintf(fptr, fmt, args);

        if (addCR) {
            fputc('\n', fptr);
        }

        fflush(fptr);
    }

	pthread_mutex_unlock(&_mutex);
}

void logger::initlogger(const string & logFileName, const char * logLevel) {
    initlogger(logFileName, logLevel_atoi(logLevel));
}

void logger::initlogger(const string & logFileName, int logLevel) {
    this->loggingLevel = logLevel;

    fptr = fopen(logFileName.c_str(), "wt");

    if (fptr == NULL) {
        throw log_error(log_error::buildMsg("Could not open log file '%s'", logFileName.c_str()));
    }
}

void logger::initlogger(int logLevel) {
    this->loggingLevel = logLevel;
    fptr = stdout;
}

void logger::initlogger(const char * logLevel) {
    initlogger(logLevel_atoi(logLevel));
}

void logger::closelogger() {
    fclose(fptr);
}

int logger::getLogLevel() {
    return this->loggingLevel;
}

void logger::setLogLevel(int logLevel) {
    this->loggingLevel = logLevel;
}

void logger::setLogLevel(const char * logLevel) {
    this->loggingLevel = logLevel_atoi(logLevel);
}

bool logger::isLogLevel(int logLevel) {
    return ((this->loggingLevel & logLevel) == logLevel ? true : false);
}

void logger::newline() {
    fputc('\n', fptr);
}

void logger::logInfo(const char * fmt, ...) {
    va_list args;

    va_start(args, fmt);
    logMessage(LOG_LEVEL_INFO, true, fmt, args);
    va_end(args);
}

void logger::logStatus(const char * fmt, ...) {
    va_list args;

    va_start(args, fmt);
    logMessage(LOG_LEVEL_STATUS, true, fmt, args);
    va_end(args);
}

void logger::logDebug(const char * fmt, ...) {
    va_list args;

    va_start(args, fmt);
    logMessage(LOG_LEVEL_DEBUG, true, fmt, args);
    va_end(args);
}

void logger::logDebugNoCR(const char * fmt, ...) {
    va_list args;

    va_start(args, fmt);
    logMessage(LOG_LEVEL_DEBUG, false, fmt, args);
    va_end(args);
}

void logger::logError(const char * fmt, ...) {
    va_list args;

    va_start(args, fmt);
    logMessage(LOG_LEVEL_ERROR, true, fmt, args);
    va_end(args);
}

void logger::logFatal(const char * fmt, ...) {
    va_list args;

    va_start(args, fmt);
    logMessage(LOG_LEVEL_FATAL, true, fmt, args);
    va_end(args);
}
