#include <string>
#include <unordered_map>
#include <vector>
#include <exception>

#include <limits.h>
#include <stdint.h>
#include <stdarg.h>

using namespace std;

#ifndef _INCL_CONFIGMGR
#define _INCL_CONFIGMGR

class cfg_error : public exception {
    private:
        string message;
        static const int MESSAGE_BUFFER_LEN = 4096;

    public:
        const char * getTitle() {
            return "CFG Error: ";
        }

        cfg_error() {
            this->message.assign(getTitle());
        }

        cfg_error(const char * msg) : cfg_error() {
            this->message.append(msg);
        }

        cfg_error(const char * msg, const char * file, int line) : cfg_error() {
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

class cfgmgr {
    public:
        static cfgmgr & getInstance() {
            static cfgmgr instance;
            return instance;
        }

    private:
        unordered_map<string, string> values;
        bool isConfigured = false;

        cfgmgr() {}

        bool isValuePropertyFile(string & value);
        char * readPropertyValue(string & propertyFileName, string & configFilePath);

    public:
        ~cfgmgr() {}

        void initialise(const string & configFileName);
        void readConfig();

        string getValue(const string & key);
        bool getValueAsBoolean(const string & key);
        int getValueAsInteger(const string & key);
        int32_t getValueAsLongInteger(const string & key);
        uint32_t getValueAsLongUnsignedInteger(const string & key);
        double getValueAsDouble(const string & key);

        void dumpConfig();
        
        static void test();
};

#endif
