#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lgpio.h>

#include "cfgmgr.h"
#include "logger.h"
#include "radio.h"

using namespace std;

void nrfcfg::validate() {
    if (channel < 1 || channel > 39) {
        throw nrf24_error(nrf24_error::buildMsg("Invalid RF channel specified: %d", channel));
    }

    if (payloadLength < 0 || payloadLength > 32) {
        throw nrf24_error(nrf24_error::buildMsg("Invalid payload length specified: %d", payloadLength));
    }

    if (addressLength < 2 || addressLength > 5) {
        throw nrf24_error(nrf24_error::buildMsg("Invalid address length specified: %d", addressLength));
    }

    isValidated = true;
}

void inline nrf24l01::chipEnable() {
    lgGpioWrite(ceGPIOChipID, cePinID, 1);
}

void inline nrf24l01::chipDisable() {
    lgGpioWrite(ceGPIOChipID, cePinID, 0);
}

int nrf24l01::readRegister(int registerID, uint8_t * buffer, uint16_t numBytes) {
    char txBuf[64];
    char rxBuf[64];
 
    txBuf[0] = (char)((NRF24L01_CMD_R_REGISTER | registerID) & 0xFF);
 
    for (int i = 1;i <= numBytes;i++) {
       txBuf[i] = 0;
    }
 
    int numBytesTransferred = lgSpiXfer(spiHandle, txBuf, rxBuf, numBytes + 1);
 
    if (numBytesTransferred >= 0) {
       for (int i = 0;i < numBytes;i++) {
          buffer[i] = rxBuf[i + 1];
       }
 
       return numBytesTransferred;
    }
 
    return numBytesTransferred;
}

int nrf24l01::writeRegister(int registerID, uint8_t * buffer, uint16_t numBytes) {
    char buf[64];
    char rxBuf[64];
 
    buf[0] = (char)((NRF24L01_CMD_W_REGISTER | registerID) & 0xFF);
 
    for (int i = 0;i < numBytes;i++) {
       buf[i + 1] = buffer[i];
    }
 
    int numBytesTransferred = lgSpiXfer(spiHandle, buf, rxBuf, numBytes + 1);
 
    return numBytesTransferred;
}

void nrf24l01::rfPowerDown() {
    chipDisable();
 
    uint8_t registerConfig = NRF24L01_CFG_ENABLE_CRC | nrfConfig.numCRCBytes;
    writeRegister(NRF24L01_REG_CONFIG, &registerConfig, 1);
}

void nrf24l01::rfPowerUpRx() {
    chipDisable();
 
    uint8_t registerConfig = 
                NRF24L01_CFG_ENABLE_CRC | 
                (nrfConfig.numCRCBytes == 1 ? NRF24L01_CFG_CRC_1_BYTE : NRF24L01_CFG_CRC_2_BYTE) | 
                NRF24L01_CFG_POWER_UP | 
                NRF24L01_CFG_MODE_RX;

    writeRegister(NRF24L01_REG_CONFIG, &registerConfig, 1);
 
    uint8_t registerStatus = NRF24L01_STATUS_CLEAR_RX_DR | NRF24L01_STATUS_CLEAR_TX_DS | NRF24L01_STATUS_CLEAR_MAX_RT;
    writeRegister(NRF24L01_REG_STATUS, &registerStatus, 1);
 
    chipEnable();
}

void nrf24l01::rfPowerUpTx() {
    chipDisable();
 
    uint8_t registerConfig = 
                NRF24L01_CFG_ENABLE_CRC | 
                nrfConfig.numCRCBytes | 
                NRF24L01_CFG_POWER_UP;

    writeRegister(NRF24L01_REG_CONFIG, &registerConfig, 1);
    writeRegister(NRF24L01_REG_CONFIG, &registerConfig, 1);
 
    uint8_t registerStatus = NRF24L01_STATUS_CLEAR_RX_DR | NRF24L01_STATUS_CLEAR_TX_DS | NRF24L01_STATUS_CLEAR_MAX_RT;
    writeRegister(NRF24L01_REG_STATUS, &registerStatus, 1);
 
    chipEnable();
}

void nrf24l01::rfFlushRx() {
    char rxBuf;
    char command = NRF24L01_CMD_FLUSH_RX;
    lgSpiXfer(spiHandle, &command, &rxBuf, 1);
 }

void nrf24l01::rfFlushTx() {
    char rxBuf;
    char command = NRF24L01_CMD_FLUSH_TX;
    lgSpiXfer(spiHandle, &command, &rxBuf, 1);
}

void nrf24l01::setRFChannel(int channel) {
    uint8_t registerRFChannel = (uint8_t)(channel & 0xFF);
    writeRegister(NRF24L01_REG_RF_CH, &registerRFChannel, 1);
}

void nrf24l01::setRFPayloadLength(int payloadLength, bool isACK) {
    if (payloadLength >= NRF24L01_MINIMUM_PACKET_LEN) {
        uint8_t registerPayloadN = payloadLength;
        writeRegister(NRF24L01_REG_RX_PW_P0, &registerPayloadN, 1);
        writeRegister(NRF24L01_REG_RX_PW_P1, &registerPayloadN, 1);

        uint8_t registerDynPD = 0x00;
        writeRegister(NRF24L01_REG_DYNPD, &registerDynPD, 1);
        writeRegister(NRF24L01_REG_FEATURE, &registerDynPD, 1);
    }
    else {
        uint8_t registerDynPD = NRF24L01_DYNPD_DPL_P0 | NRF24L01_DYNPD_DPL_P1;
        writeRegister(NRF24L01_REG_DYNPD, &registerDynPD, 1);

        uint8_t registerFeature;

        if (isACK) {
            registerFeature = NRF24L01_FEATURE_EN_DYN_PAYLOAD_LEN | NRF24L01_FEATURE_EN_PAYLOAD_WITH_ACK;
        }
        else {
            registerFeature = NRF24L01_FEATURE_EN_DYN_PAYLOAD_LEN;
        }

        writeRegister(NRF24L01_REG_FEATURE, &registerFeature, 1);
    }
}

void nrf24l01::setRFAddressLength(int addressLength) {
    uint8_t registerSetupAW = (uint8_t)((addressLength - 2) & 0xFF);
    writeRegister(NRF24L01_REG_SETUP_AW, &registerSetupAW, 1);
}

void nrf24l01::rfSetup(nrfcfg::data_rate & dataRate, nrfcfg::rf_power & rfPower, bool isLNAGainOn) {
    uint8_t registerRFSetup = 
                        dataRate | 
                        rfPower | 
                        (isLNAGainOn ? NRF24L01_RF_SETUP_RF_LNA_GAIN_ON : NRF24L01_RF_SETUP_RF_LNA_GAIN_OFF);

    writeRegister(NRF24L01_REG_RF_SETUP, &registerRFSetup, 1);
}

int nrf24l01::padAddress(uint8_t * buffer, int actualAddressLength) {
    if (actualAddressLength < nrfConfig.addressLength) {
        for (int i = actualAddressLength;i < nrfConfig.addressLength;i++) {
            buffer[i] = nrfConfig.paddingByte;
        }
    }

    return nrfConfig.addressLength;
}

void nrf24l01::setLocalAddress(const char * address) {
    uint8_t txBuf[64];

    int actualAddressLength = strlen(address);

    for (int i = 0;i < actualAddressLength;i++) {
       txBuf[i] = address[i];
    }

    actualAddressLength = padAddress(txBuf, actualAddressLength);
 
    chipDisable();
    writeRegister(NRF24L01_REG_RX_ADDR_PO, txBuf, actualAddressLength);
    chipEnable();
}

void nrf24l01::setRemoteAddress(const char * address) {
    uint8_t txBuf[64];
 
    int actualAddressLength = strlen(address);

    for (int i = 0;i < actualAddressLength;i++) {
       txBuf[i] = address[i];
    }

    actualAddressLength = padAddress(txBuf, actualAddressLength);
 
    chipDisable();
    writeRegister(NRF24L01_REG_TX_ADDR, txBuf, actualAddressLength);
    writeRegister(NRF24L01_REG_RX_ADDR_PO, txBuf, actualAddressLength);
    chipEnable();
}

bool nrf24l01::isDataReady() {
    int status = getStatus();

    if (status & NRF24L01_STATUS_R_RX_DR) {
        return true;
    }

    uint8_t fifoStatus;
    readRegister(NRF24L01_REG_FIFO_STATUS, &fifoStatus, 1);

    bool dataReady = (int)(fifoStatus & NRF24L01_STATUS_R_TX_FIFO_FULL ? false : true);

    return dataReady;
}

uint8_t * nrf24l01::readPayload() {
    static uint8_t  rxBuffer[NRF24L01_MAXIMUM_PACKET_LEN];
    int count;
 
    if (nrfConfig.payloadLength == 0) {
        char plBuffer[NRF24L01_MAXIMUM_PACKET_LEN];
        char txBuffer[64];
 
        txBuffer[0] = NRF24L01_CMD_R_RX_PL_WID;
        txBuffer[1] = 0;
    
        lgSpiXfer(spiHandle, txBuffer, plBuffer, 2);
        count = plBuffer[1];
    }
    else {
       count = nrfConfig.payloadLength;
    }

    readRegister(NRF24L01_REG_CONFIG | NRF24L01_CMD_R_RX_PAYLOAD, rxBuffer, count);

    chipDisable();
 
    uint8_t registerStatus = NRF24L01_STATUS_R_RX_DR;
    writeRegister(NRF24L01_REG_STATUS, &registerStatus, 1);
 
    chipEnable();
 
    return rxBuffer;
}

int nrf24l01::getStatus() {
    char command = NRF24L01_CMD_NOP;
    char status;
    int numBytesTransferred = lgSpiXfer(spiHandle, &command, &status, 1);
 
    if (numBytesTransferred >= 0) {
        return (int)status;
    }
 
    return numBytesTransferred;
}

void nrf24l01::configureSPI(uint32_t spiFrequency, int CEPin) {
    cePinID = CEPin;

    ceGPIOChipID = lgGpiochipOpen(0);

    lgGpioClaimOutput(ceGPIOChipID, 0, cePinID, 0);

    chipDisable();

    spiChannel = NRF_SPI_CHANNEL;
    spiDevice = NRF_SPI_DEVICE;
    
    spiHandle = lgSpiOpen(
                        spiChannel, 
                        spiDevice, 
                        spiFrequency, 
                        0);
}

void nrf24l01::open(nrfcfg & cfg) {
    logger & log = logger::getInstance();

    if (!cfg.isValidated) {
        throw nrf24_error("Error, radio has not been configured or the configuartion is invalid");
    }

    log.logInfo("Got RF channel: %d", cfg.channel);
    log.logInfo("Got local address '%s'", cfg.localAddress);
    log.logInfo("Got remote address '%s'", cfg.remoteAddress);

    setRFChannel(cfg.channel);
    setRFPayloadLength(cfg.payloadLength, false);
    setRFAddressLength(cfg.addressLength);

    rfPowerDown();

    uint8_t registerSetupRetr = 0x00;
    writeRegister(NRF24L01_REG_SETUP_RETR, &registerSetupRetr, 1);

    rfSetup(cfg.airDataRate, cfg.rfPower, cfg.lnaGainOn);

    uint8_t registerEnableAA = 0x00;
    writeRegister(NRF24L01_REG_EN_AA, &registerEnableAA, 1);

    uint8_t registerFeature = 
                NRF24L01_FEATURE_EN_DYN_PAYLOAD_LEN | 
                NRF24L01_FEATURE_EN_PAYLOAD_WITH_ACK;

    writeRegister(NRF24L01_REG_FEATURE, &registerFeature, 1);
    
    setLocalAddress(cfg.localAddress);
    setRemoteAddress(cfg.remoteAddress);

    rfFlushRx();
    rfFlushTx();

    if (cfg.primaryMode == nrfcfg::mode_rx) {
        rfPowerUpRx();
    }
    else {
        rfPowerUpTx();
    }

    uint8_t configRegister;
    int status = readRegister(NRF24L01_REG_CONFIG, &configRegister, 1);

    if (status < 0) {
        throw nrf24_error(nrf24_error::buildMsg("Failed to transfer SPI data with radio: %s", lguErrorText(status)));
    }

    log.logInfo("Read back CONFIG reg: 0x%02X\n", configRegister);

    if (configRegister == 0x00) {
        throw nrf24_error("Config read back as 0x00, device is probably not plugged in?");
    }
}

void nrf24l01::close() {
    rfPowerDown();

    lgSpiClose(spiHandle);
    lgGpioFree(ceGPIOChipID, cePinID);
    lgGpiochipClose(ceGPIOChipID);
}
