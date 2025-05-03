#include <string>

#include <stdint.h>
#include <time.h>
#include <sys/time.h>

using namespace std;

#ifndef __INCL_UTILS
#define __INCL_UTILS

struct tm * getLocalTime();
string & getTodaysDate();
string & getTimestamp();
string & getTimestampUs();

int         strHexDump(char * pszBuffer, int strBufferLen, void * buffer, uint32_t bufferLen);
void        hexDump(void * buffer, uint32_t bufferLen);
void        daemonise(void);

#endif
