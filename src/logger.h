#include <iostream>
#include <fstream>
#include <string>

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

using namespace std;

#ifndef _INCL_logger
#define _INCL_logger

#define MAX_LOG_LENGTH          256

/*
** Supported log levels...
*/
#define LOG_LEVEL_INFO          0x01
#define LOG_LEVEL_STATUS        0x02
#define LOG_LEVEL_DEBUG         0x04
#define LOG_LEVEL_ERROR         0x08
#define LOG_LEVEL_FATAL         0x10

#define LOG_LEVEL_ALL           (LOG_LEVEL_INFO | LOG_LEVEL_STATUS | LOG_LEVEL_DEBUG | LOG_LEVEL_ERROR | LOG_LEVEL_FATAL)

class log_error : public exception {
    private:
        string message;
        static const int MESSAGE_BUFFER_LEN = 4096;

    public:
        const char * getTitle() {
            return "LOG Error: ";
        }

        log_error() {
            this->message.assign(getTitle());
        }

        log_error(const char * msg) : log_error() {
            this->message.append(msg);
        }

        log_error(const char * msg, const char * file, int line) : log_error() {
            char lineNumBuf[8];

            snprintf(lineNumBuf, 8, ":%d", line);

            this->message.append(msg);
            this->message.append(" at ");
            this->message.append(file);
            this->message.append(lineNumBuf);
        }

        virtual const char * what() const noexcept {
            return this->message.c_str();
        }

        static char * buildMsg(const char * fmt, ...) {
            va_list     args;
            char *      buffer;

            buffer = (char *)malloc(MESSAGE_BUFFER_LEN);
            
            va_start(args, fmt);
            vsnprintf(buffer, MESSAGE_BUFFER_LEN, fmt, args);
            va_end(args);

            return buffer;
        }
};

class logger {
    public:
        static logger & getInstance() {
            static logger instance;
            return instance;
        }

    private:
        logger() {}

        FILE * fptr;

        int loggingLevel;

        void logMessage(int logLevel, bool addCR, const char * fmt, va_list args);

    public:
        ~logger() {}

        void initlogger(int logLevel);
        void initlogger(const char * logLevel);
        void initlogger(const string & logFileName, int logLevel);
        void initlogger(const string & logFileName, const char * logLevel);
        
        void closelogger();

        int getLogLevel();
        void setLogLevel(int logLevel);
        void setLogLevel(const char * pszLogLevel);
        
        bool isLogLevel(int logLevel);

        void newline();

        void logInfo(const char * fmt, ...);
        void logStatus(const char * fmt, ...);
        void logDebug(const char * fmt, ...);
        void logDebugNoCR(const char * fmt, ...);
        void logError(const char * fmt, ...);
        void logFatal(const char * fmt, ...);
};

#endif