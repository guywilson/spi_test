#include <exception>

#include <stdint.h>

using namespace std;

#ifndef __INCL_RADIO
#define __INCL_RADIO

#define NRF24L01_DEFAULT_PACKET_LEN                 32
#define NRF24L01_MINIMUM_PACKET_LEN                  1
#define NRF24L01_MAXIMUM_PACKET_LEN                 NRF24L01_DEFAULT_PACKET_LEN

/*
** nRF24L01 commands
*/
#define NRF24L01_CMD_R_REGISTER                     0x00
#define NRF24L01_CMD_W_REGISTER                     0x20
#define NRF24L01_CMD_R_RX_PAYLOAD                   0x61
#define NRF24L01_CMD_W_TX_PAYLOAD                   0xA0
#define NRF24L01_CMD_FLUSH_TX                       0xE1
#define NRF24L01_CMD_FLUSH_RX                       0xE2
#define NRF24L01_CMD_REUSE_TX_PL                    0xE3
#define NRF24L01_CMD_ACTIVATE                       0x50
#define NRF24L01_CMD_R_RX_PL_WID                    0x60
#define NRF24L01_CMD_W_ACK_PAYLOAD                  0xA8
#define NRF24L01_CMD_W_TX_PAYLOAD_NOACK             0xB0
#define NRF24L01_CMD_NOP                            0xFF

#define NRF24L01_ACTIVATE_ADDITIONAL                0x73

/*
** nRF24L01 register map
*/
#define NRF24L01_REG_CONFIG                         0x00
#define NRF24L01_REG_EN_AA                          0x01
#define NRF24L01_REG_EN_RXADDR                      0x02
#define NRF24L01_REG_SETUP_AW                       0x03
#define NRF24L01_REG_SETUP_RETR                     0x04
#define NRF24L01_REG_RF_CH                          0x05
#define NRF24L01_REG_RF_SETUP                       0x06
#define NRF24L01_REG_STATUS                         0x07
#define NRF24L01_REG_OBSERVE_TX                     0x08
#define NRF24L01_REG_CD                             0x09
#define NRF24L01_REG_RX_ADDR_PO                     0x0A
#define NRF24L01_REG_RX_ADDR_P1                     0x0B
#define NRF24L01_REG_RX_ADDR_P2                     0x0C
#define NRF24L01_REG_RX_ADDR_P3                     0x0D
#define NRF24L01_REG_RX_ADDR_P4                     0x0E
#define NRF24L01_REG_RX_ADDR_P5                     0x0F
#define NRF24L01_REG_TX_ADDR                        0x10
#define NRF24L01_REG_RX_PW_P0                       0x11
#define NRF24L01_REG_RX_PW_P1                       0x12
#define NRF24L01_REG_RX_PW_P2                       0x13
#define NRF24L01_REG_RX_PW_P3                       0x14
#define NRF24L01_REG_RX_PW_P4                       0x15
#define NRF24L01_REG_RX_PW_P5                       0x16
#define NRF24L01_REG_FIFO_STATUS                    0x17
#define NRF24L01_REG_DYNPD                          0x1C
#define NRF24L01_REG_FEATURE                        0x1D

/*
** RF_SETUP register flags...
*/
#define NRF24L01_RF_SETUP_DATA_RATE_250KBPS         0x20        // NRF24L01+ only
#define NRF24L01_RF_SETUP_DATA_RATE_1MBPS           0x00
#define NRF24L01_RF_SETUP_DATA_RATE_2MBPS           0x08

#define NRF24L01_RF_SETUP_RF_POWER_LOWEST           0x00
#define NRF24L01_RF_SETUP_RF_POWER_LOW              0x02
#define NRF24L01_RF_SETUP_RF_POWER_MEDIUM           0x04
#define NRF24L01_RF_SETUP_RF_POWER_HIGH             0x06

#define NRF24L01_RF_SETUP_RF_LNA_GAIN_ON            0x01
#define NRF24L01_RF_SETUP_RF_LNA_GAIN_OFF           0x00

/*
** CONFIG register flags...
*/
#define NRF24L01_CFG_MASK_RX_DR                     0x40
#define NRF24L01_CFG_MASK_TX_DS                     0x20
#define NRF24L01_CFG_MASK_MAX_RT                    0x10
#define NRF24L01_CFG_ENABLE_CRC                     0x08
#define NRF24L01_CFG_DISABLE_CRC                    0x08
#define NRF24L01_CFG_CRC_1_BYTE                     0x00
#define NRF24L01_CFG_CRC_2_BYTE                     0x04
#define NRF24L01_CFG_POWER_UP                       0x02
#define NRF24L01_CFG_POWER_DOWN                     0x00
#define NRF24L01_CFG_MODE_RX                        0x01
#define NRF24L01_CFG_MODE_TX                        0x00

/*
** STATUS register flags...
*/
#define NRF24L01_STATUS_CLEAR_RX_DR                 0x40
#define NRF24L01_STATUS_CLEAR_TX_DS                 0x20
#define NRF24L01_STATUS_CLEAR_MAX_RT                0x10

#define NRF24L01_STATUS_R_RX_PIPE_0                 0x00
#define NRF24L01_STATUS_R_RX_PIPE_1                 0x02
#define NRF24L01_STATUS_R_RX_PIPE_2                 0x04
#define NRF24L01_STATUS_R_RX_PIPE_3                 0x06
#define NRF24L01_STATUS_R_RX_PIPE_4                 0x08
#define NRF24L01_STATUS_R_RX_PIPE_5                 0x0A

#define NRF24L01_STATUS_R_TX_FIFO_FULL              0x01
#define NRF24L01_STATUS_R_RX_FIFO_EMPTY             0x0E
#define NRF24L01_STATUS_R_RX_DR                     0x40

/*
** FEATURE register flags...
*/
#define NRF24L01_FEATURE_EN_DYN_PAYLOAD_LEN         0x04
#define NRF24L01_FEATURE_EN_PAYLOAD_WITH_ACK        0x02
#define NRF24L01_FEATURE_EN_TX_NO_ACK               0x01

/*
** DYNPD register flags...
*/
#define NRF24L01_DYNPD_DPL_P5                       0x20
#define NRF24L01_DYNPD_DPL_P4                       0x10
#define NRF24L01_DYNPD_DPL_P3                       0x08
#define NRF24L01_DYNPD_DPL_P2                       0x04
#define NRF24L01_DYNPD_DPL_P1                       0x02
#define NRF24L01_DYNPD_DPL_P0                       0x01

#define NRF_SPI_DEVICE                              0
#define NRF_SPI_CHANNEL                             0
#define NRF_SPI_FREQUENCY                           4000000U
#define NRF_SPI_CE_PIN                              25

class nrf24_error : public exception {
    private:
        string message;
        static const int MESSAGE_BUFFER_LEN = 4096;

    public:
        const char * getTitle() {
            return "CFG Error: ";
        }

        nrf24_error() {
            this->message.assign(getTitle());
        }

        nrf24_error(const char * msg) : nrf24_error() {
            this->message.append(msg);
        }

        nrf24_error(const char * msg, const char * file, int line) : nrf24_error() {
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

class nrfcfg {
    friend class nrf24l01;

    private:
        bool isValidated = false;

    public:
        enum primary_mode {
            mode_rx,
            mode_tx
        };

        enum data_rate {
            data_rate_low               = NRF24L01_RF_SETUP_DATA_RATE_250KBPS,
            data_rate_medium            = NRF24L01_RF_SETUP_DATA_RATE_1MBPS,
            data_rate_high              = NRF24L01_RF_SETUP_DATA_RATE_2MBPS
        };

        enum rf_power {
            rf_power_lowest             = NRF24L01_RF_SETUP_RF_POWER_LOWEST,
            rf_power_low                = NRF24L01_RF_SETUP_RF_POWER_LOW,
            rf_power_medium             = NRF24L01_RF_SETUP_RF_POWER_MEDIUM,
            rf_power_high               = NRF24L01_RF_SETUP_RF_POWER_HIGH
        };

        primary_mode    primaryMode = mode_rx;
        int             channel = 11;
        int             payloadLength = 32;
        int             addressLength = 5;
        int             numCRCBytes = 2;
        data_rate       airDataRate = data_rate_high;
        rf_power        rfPower = rf_power_high;
        bool            lnaGainOn = true;
        uint8_t         paddingByte = 0x20;

        const char *    localAddress;
        const char *    remoteAddress;

        void validate();
};

class nrf24l01 {
    public:
        static nrf24l01 & getInstance() {
            static nrf24l01 nrf;
            return nrf;
        }

    private:
        int             ceGPIOChipID;

        int             spiHandle;
        int             spiChannel;
        int             spiDevice;

        int             cePinID;

        nrfcfg          nrfConfig;

        nrf24l01() {}

        int readRegister(int registerID, uint8_t * buffer, uint16_t numBytes);
        int writeRegister(int registerID, uint8_t * buffer, uint16_t numBytes);

        void chipEnable();
        void chipDisable();

        void rfPowerDown();
        void rfPowerUpRx();
        void rfPowerUpTx();

        void rfFlushRx();
        void rfFlushTx();

        void setRFChannel(int channel);
        void setRFPayloadLength(int payloadLength, bool isACK);
        void setRFAddressLength(int addressLength);

        void rfSetup(
                nrfcfg::data_rate & dataRate, 
                nrfcfg::rf_power & rfPower, 
                bool isLNAGainOn);

        int padAddress(uint8_t * buffer, int actualAddressLength);

        void setLocalAddress(const char * address);
        void setRemoteAddress(const char * address);

    public:
        ~nrf24l01() {}

        void configureSPI(uint32_t spiFrequency, int CEPin);
        void open(nrfcfg & cfg);

        void close();

        bool isDataReady();
        uint8_t * readPayload();

        int getStatus();
};

#endif
