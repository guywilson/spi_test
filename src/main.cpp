#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <lgpio.h>

#include "cfgmgr.h"
#include "logger.h"
#include "radio.h"
#include "utils.h"


static char szDumpBuffer[1024];

static void printUsage(void) {
	printf("\n Usage: wctl [OPTIONS]\n\n");
	printf("  Options:\n");
	printf("   -h/?             Print this help\n");
	printf("   -version         Print the program version\n");
	printf("   -cfg configfile  Specify the cfg file, default is ./webconfig.cfg\n");
    printf("   --dump-config    Dump the config contents and exit\n");
	printf("   -d               Daemonise this application\n");
	printf("   -log  filename   Write logs to the file\n");
	printf("\n");
}

static nrfcfg::data_rate getDataRate() {
    cfgmgr & cfg = cfgmgr::getInstance();

    string dataRateCfg = cfg.getValue("radio.baud");
    
    nrfcfg::data_rate dataRate;
    if (dataRateCfg.compare("2MHz") == 0) {
        dataRate = nrfcfg::data_rate::data_rate_high;
    }
    else if (dataRateCfg.compare("1MHz") == 0) {
        dataRate = nrfcfg::data_rate::data_rate_medium;
    }
    else if (dataRateCfg.compare("250KHz") == 0) {
        dataRate = nrfcfg::data_rate::data_rate_low;
    }
    else {
        dataRate = nrfcfg::data_rate::data_rate_medium;
    }

    return dataRate;
}

static nrfcfg & getRadioConfig() {
    static nrfcfg radioConfig;
    static char szLocalAddress[32];
    static char szRemoteAddress[32];

    cfgmgr & cfg = cfgmgr::getInstance();

    radioConfig.airDataRate = getDataRate();
    radioConfig.channel = cfg.getValueAsInteger("radio.channel");

    strncpy(szLocalAddress, cfg.getValue("radio.localaddress").c_str(), 31);

    radioConfig.localAddress = szLocalAddress;
    radioConfig.lnaGainOn = false;

    radioConfig.validate();

    return radioConfig;
}

int main(int argc, char ** argv) {
	char *			    pszLogFileName = NULL;
	char *			    pszConfigFileName = NULL;
	int				    i;
	bool			    isDumpConfig = false;
	const char *	    defaultLoggingLevel = "LOG_LEVEL_INFO | LOG_LEVEL_ERROR | LOG_LEVEL_FATAL";

	if (argc > 1) {
		for (i = 1;i < argc;i++) {
			if (argv[i][0] == '-') {
				if (strcmp(&argv[i][1], "log") == 0) {
					pszLogFileName = strdup(&argv[++i][0]);
				}
				else if (strcmp(&argv[i][1], "cfg") == 0) {
					pszConfigFileName = strdup(&argv[++i][0]);
				}
				else if (strcmp(&argv[i][1], "-dump-config") == 0) {
					isDumpConfig = true;
				}
				else if (argv[i][1] == 'h' || argv[i][1] == '?') {
					printUsage();
					return 0;
				}
				else {
					printf("Unknown argument '%s'", &argv[i][0]);
					printUsage();
					return 0;
				}
			}
		}
	}
	else {
		printUsage();
		return -1;
	}

	cfgmgr & cfg = cfgmgr::getInstance();

	try {
		cfg.initialise(pszConfigFileName);
	}
	catch (cfg_error & e) {
		fprintf(stderr, "Could not open configuration file '%s':'%s'\n", pszConfigFileName, e.what());
		exit(-1);
	}

	if (pszConfigFileName != NULL) {
		free(pszConfigFileName);
	}

	if (isDumpConfig) {
        cfg.dumpConfig();
        return 0;
	}

	logger & log = logger::getInstance();

	try {
		if (pszLogFileName != NULL) {
			log.initlogger(pszLogFileName, defaultLoggingLevel);
			free(pszLogFileName);
		}
		else {
			string filename = cfg.getValue("log.filename");
			string level = cfg.getValue("log.level");
	
			if (filename.length() == 0 && level.length() == 0) {
				log.initlogger(defaultLoggingLevel);
			}
			else if (level.length() == 0) {
				log.initlogger(filename, defaultLoggingLevel);
			}
			else {
				log.initlogger(filename.c_str(), level.c_str());
			}
		}
	}
	catch (log_error & e) {
		fprintf(stderr, "Could not initialise logger");
		exit(-1);
	}

	nrf24l01 & radio = nrf24l01::getInstance();

	nrfcfg radioConfig = getRadioConfig();

	radio.configureSPI(NRF_SPI_FREQUENCY, NRF_SPI_CE_PIN);
	radio.open(radioConfig);

	while (1) {
		while (radio.isDataReady()) {
            log.logDebug("NRF24L01 has received data...");
            uint8_t * payload = radio.readPayload();

            if (strHexDump(szDumpBuffer, 1024, payload, NRF24L01_MAXIMUM_PACKET_LEN) > 0) {
                log.logDebug("%s", szDumpBuffer);
            }
		}

        sleep(2);
    }

	return 0;
}
