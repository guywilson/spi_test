#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "utils.h"

using namespace std;

#define TIME_STAMP_BUFFER_LEN             32
#define DATE_STAMP_BUFFER_LEN             16

static pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

static string & _getTimestamp(bool includeMicroseconds) {
    static string ts;
    struct timeval tv;

	pthread_mutex_lock(&_mutex);

    gettimeofday(&tv, NULL);
    time_t t = tv.tv_sec;
    struct tm * localTime = localtime(&t);

    char timestamp[TIME_STAMP_BUFFER_LEN];

    snprintf(
        timestamp, 
        TIME_STAMP_BUFFER_LEN, 
        "%d-%02d-%02d %02d:%02d:%02d.%06d", 
        localTime->tm_year + 1900, 
        localTime->tm_mon + 1, 
        localTime->tm_mday,
        localTime->tm_hour,
        localTime->tm_min,
        localTime->tm_sec,
        tv.tv_usec);

    ts.assign(timestamp);

    pthread_mutex_unlock(&_mutex);

    return ts;
}

struct tm * getLocalTime() {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    time_t t = tv.tv_sec;
    return localtime(&t);
}

string & getTodaysDate() {
    static string date;

    struct tm * localtime = getLocalTime();

    char szDate[DATE_STAMP_BUFFER_LEN];

    snprintf(
        szDate, 
        DATE_STAMP_BUFFER_LEN, 
        "%d-%02d-%02d", 
        localtime->tm_year + 1900, 
        localtime->tm_mon + 1, 
        localtime->tm_mday);

    date.assign(szDate);

    return date;
}

string & getTimestamp() {
    return _getTimestamp(false);
}

string & getTimestampUs() {
    return _getTimestamp(true);
}

int strHexDump(char * pszBuffer, int strBufferLen, void * buffer, uint32_t bufferLen) {
    int         i;
    int         j = 0;
    int         counter = 0;
    uint8_t *   buf;
    static char szASCIIBuf[17];

    buf = (uint8_t *)buffer;

    if (strBufferLen < (((bufferLen / 16) * 53) + (bufferLen % 16) > 0 ? 53 : 0)) {
        return -1;
    }

    pszBuffer[0] = 0;

    for (i = 0;i < bufferLen;i++) {
        if ((i % 16) == 0) {
            if (i != 0) {
                szASCIIBuf[j] = 0;
                j = 0;

                counter += snprintf(&pszBuffer[counter], strBufferLen, "  |%s|", szASCIIBuf);
            }
                
            counter += snprintf(&pszBuffer[counter], strBufferLen, "\n%08X\t", i);
        }

        if ((i % 2) == 0 && (i % 16) > 0) {
            counter += snprintf(&pszBuffer[counter], strBufferLen, " ");
        }

        counter += snprintf(&pszBuffer[counter], strBufferLen, "%02X", buf[i]);
        szASCIIBuf[j++] = isprint(buf[i]) ? buf[i] : '.';
    }

    /*
    ** Print final ASCII block...
    */
    szASCIIBuf[j] = 0;
    counter += snprintf(&pszBuffer[counter], strBufferLen, "  |%s|\n", szASCIIBuf);

    return counter;
}

void hexDump(void * buffer, uint32_t bufferLen) {
    int         i;
    int         j = 0;
    uint8_t *   buf;
    static char szASCIIBuf[17];

    buf = (uint8_t *)buffer;

    for (i = 0;i < bufferLen;i++) {
        if ((i % 16) == 0) {
            if (i != 0) {
                szASCIIBuf[j] = 0;
                j = 0;

                printf("  |%s|", szASCIIBuf);
            }
                
            printf("\n%08X\t", i);
        }

        if ((i % 2) == 0 && (i % 16) > 0) {
            printf(" ");
        }

        printf("%02X", buf[i]);
        szASCIIBuf[j++] = isprint(buf[i]) ? buf[i] : '.';
    }

    /*
    ** Print final ASCII block...
    */
    szASCIIBuf[j] = 0;
    printf("  |%s|\n", szASCIIBuf);
}

void daemonise(void) {
	pid_t			pid;
	pid_t			sid;

	fprintf(stdout, "Starting daemon...\n");
	fflush(stdout);

	do {
		pid = fork();
	}
	while ((pid == -1) && (errno == EAGAIN));

	if (pid < 0) {
		fprintf(stderr, "Forking daemon failed...\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}
	if (pid > 0) {
		fprintf(stdout, "Exiting child process...\n");
		fflush(stdout);
		exit(EXIT_SUCCESS);
	}

	sid = setsid();
	
	if(sid < 0) {
		fprintf(stderr, "Failed calling setsid()...\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);    
	
	umask(0);

	if((chdir("/") == -1)) {
		fprintf(stderr, "Failed changing directory\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}
	
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
}
