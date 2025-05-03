#include <iostream>
#include <fstream>
#include <string>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "cfgmgr.h"

//#define UNIT_TEST_MODE

using namespace std;

static bool inline isStringHexadecimal(string & value) {
    if (value[0] == '0' && (value[1] == 'x' || value[1] == 'X')) {
        return true;
    }

    return false;
}

static const char * getHexadecimalValue(string & value) {
    static string hexValue = value.substr(2);
    return hexValue.c_str();
}

static const char * getDecimalValue(string & value) {
    return value.c_str();
}

static inline bool isPropertyFileNamePath(string & propertyFileName) {
    return (propertyFileName.find_first_of('/') != string::npos ? true : false);
}

static string & getPropertyFileFullPath(string & value, string & configFilePath) {
    static string propertyFilePath;

    string propertyFileName = value.substr(1, value.length() - 2);

    if (isPropertyFileNamePath(propertyFileName)) {
        propertyFilePath = propertyFileName;
    }
    else {
        propertyFilePath = configFilePath + propertyFileName;
    }

    return propertyFilePath;
}

static string & getConfigFilePath(const string & configFileName) {
    static string path;
    
    path = configFileName.substr(0, (configFileName.find_last_of('/') + 1));

    return path;
}

bool cfgmgr::isValuePropertyFile(string & value) {
    if (value[0] == '<' && value[value.length() - 1] == '>') {
        return true;
    }

    return false;
}

char * cfgmgr::readPropertyValue(string & value, string & configFilePath) {
    string propertyFileName = getPropertyFileFullPath(value, configFilePath);

    FILE * fprop = fopen(propertyFileName.c_str(), "rt");

    if (fprop == NULL) {
        throw cfg_error(cfg_error::buildMsg("Failed to open property file %s", propertyFileName.c_str()));
    }

    fseek(fprop, 0L, SEEK_END);
    int propLength = (int)ftell(fprop);
    fseek(fprop, 0L, SEEK_SET);

    char * property = (char *)malloc(propLength + 1);

    if (property == NULL) {
        throw cfg_error(cfg_error::buildMsg("Failed to allocate %d bytes for property file %s", propLength, propertyFileName.c_str()));
    }

    fread(property, 1, propLength, fprop);
    fclose(fprop);

    property[propLength] = 0;
    
    return property;
}

void cfgmgr::initialise(const string & configFileName) {
    ifstream ifs;

    try {
        ifs.open(configFileName.c_str());
    }
    catch (exception & e) {
        throw cfg_error(cfg_error::buildMsg("Failed to open config file '%s'", configFileName.c_str()));
    }

    string path = getConfigFilePath(configFileName);

    string line;

    while (getline(ifs, line)) {
        if (line.front() == '#' || line.length() == 0) {
            continue;
        }

        uint32_t equalPos = line.find_first_of('=');

        string key = line.substr(0, equalPos);
        string value = line.substr(equalPos + 1);

        if (isValuePropertyFile(value)) {
            char * property = readPropertyValue(value, path);
            value.assign(property);
            free(property);
        }

        this->values[key] = value;
    }

    isConfigured = true;

    ifs.close();
}

string cfgmgr::getValue(const string & key) {
    if (values.count(key) == 0) {
        return "";
    }

    return values[key];
}

bool cfgmgr::getValueAsBoolean(const string & key) {
    string value = getValue(key);

    return ((value.compare("yes") == 0  || value.compare("true") == 0 || value.compare("on") == 0) ? true : false);
}

int cfgmgr::getValueAsInteger(const string & key) {
    string value = getValue(key);

    return atoi(getDecimalValue(value));
}

int32_t cfgmgr::getValueAsLongInteger(const string & key) {
    string value = getValue(key);

    if (isStringHexadecimal(value)) {
        return strtol(getHexadecimalValue(value), NULL, 16);
    }
    else {
        return strtol(getDecimalValue(value), NULL, 10);
    }
}

uint32_t cfgmgr::getValueAsLongUnsignedInteger(const string & key) {
    string value = getValue(key);

    if (isStringHexadecimal(value)) {
        return strtoul(getHexadecimalValue(value), NULL, 16);
    }
    else {
        return strtoul(getDecimalValue(value), NULL, 10);
    }
}

double cfgmgr::getValueAsDouble(const string & key) {
    string value = getValue(key);

    return strtod(getDecimalValue(value), NULL);
}

void cfgmgr::test() {
    string value = "<dbname.prop>";
    string configFile = "/usr/local/bin/wctl/wctl.cfg";
    string propertyFileName;

    string configPath = getConfigFilePath(configFile);

    if (configPath.compare("/usr/local/bin/wctl/") == 0) {
        cout << "Test 1 passed!: " << configPath << endl;
    }
    else {
        cout << "Test 1 failed!: " << configPath << endl;
    }

    propertyFileName = getPropertyFileFullPath(value, configPath);

    if (propertyFileName.compare("/usr/local/bin/wctl/dbname.prop") == 0) {
        cout << "Test 2 passed!: " << propertyFileName << endl;
    }
    else {
        cout << "Test 2 failed!: " << propertyFileName << endl;
    }

    value = "<dbpasswd.prop>";
    propertyFileName = getPropertyFileFullPath(value, configPath);

    if (propertyFileName.compare("/usr/local/bin/wctl/dbpasswd.prop") == 0) {
        cout << "Test 3 passed!: " << propertyFileName << endl;
    }
    else {
        cout << "Test 3 failed!: " << propertyFileName << endl;
    }

    value = "</var/wctl/dbpasswd.prop>";
    propertyFileName = getPropertyFileFullPath(value, configPath);

    if (propertyFileName.compare("/var/wctl/dbpasswd.prop") == 0) {
        cout << "Test 4 passed!: " << propertyFileName << endl;
    }
    else {
        cout << "Test 4 failed!: " << propertyFileName << endl;
    }

    value = "<dbpasswd.prop>";
    configFile = "wctl.cfg";
    configPath = getConfigFilePath(configFile);
    propertyFileName = getPropertyFileFullPath(value, configPath);

    if (propertyFileName.compare("dbpasswd.prop") == 0) {
        cout << "Test 5 passed!: " << propertyFileName << endl;
    }
    else {
        cout << "Test 5 failed!: " << propertyFileName << endl;
    }
}

void cfgmgr::dumpConfig() {
    if (isConfigured) {
        for (auto& i : values) {
            string key = i.first;
            string value = i.second;

            cout << "'" << key << "' = '" << value << "'" << endl;
        }
    }
}

#ifdef UNIT_TEST_MODE
int main(void) {
    cfgmgr::test();
}
#endif
