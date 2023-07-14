#include <internal_com.h>
#include <Arduino.h>
#include <registers.h>
#include <log.h>

#define INTC_SERIAL Serial2
#define INTC_FRAMESIZE      (7)
#define INTC_FRAME_HEADER   (0x55)
#define INTC_TIMEOUT_US     (200000)

uint8_t intcRegTxCounter;
uint8_t intcRxBuffer[INTC_FRAMESIZE];
uint32_t intcRxBuffWriteIdx, intcRxBuffStartIdx;
uint32_t intcLastReceivedUs = 0;
bool intcTimeoutFlg = false;

bool intcInit() {
    INTC_SERIAL.begin(115200);
    INTC_SERIAL.setRxBufferSize(256);
    intcRegTxCounter = 0;
    intcRxBuffWriteIdx = 0;
    intcRxBuffStartIdx = 0;
    return true;
}

void intcSendData(uint8_t idx) { // TODO: pack multiple frames to reduce bus load
    uint8_t buffer[INTC_FRAMESIZE];
    buffer[0] = INTC_FRAME_HEADER;
    buffer[1] = idx;
    *(uint32_t*)(&buffer[2]) = *regsRegisters[idx].data.pi;
    buffer[INTC_FRAMESIZE-1] = buffer[1];
    for(int i=2;i<(INTC_FRAMESIZE-1);i++) buffer[INTC_FRAMESIZE-1] += buffer[i];
    INTC_SERIAL.write(buffer, sizeof(buffer)); // TODO: check if really async
}

void intcSendNext() {
    uint8_t nextReg = intcRegTxCounter;
    do {
        nextReg++;
        if(nextReg >= REGS_REG_CNT) {
            nextReg = 0;
        }
    } while(!regsRegisters[nextReg].isTx && nextReg != intcRegTxCounter);
    if(regsRegisters[nextReg].isTx) {
        intcRegTxCounter = nextReg;
        intcSendData(nextReg);
    }
}

bool intcCheckIdTx(int idx) {
    if(idx >= REGS_REG_CNT) return false;
    return regsRegisters[idx].isTx;
}

bool intcCheckRxFrame() {
    uint8_t checksum = intcRxBuffer[1];
    for(int i=2;i<(INTC_FRAMESIZE-1);i++) checksum += intcRxBuffer[i];
    if(checksum == intcRxBuffer[INTC_FRAMESIZE-1]) {
        uint8_t idx = intcRxBuffer[1];
        if(!intcCheckIdTx(idx)) {
            uint32_t* reg = regsRegisters[idx].data.pi;
            if(reg != nullptr) {
                *reg = *(uint32_t*)&intcRxBuffer[2];
            }
        }
        return true;
    }
    return false;
}

void intcReceiveAll() {
    int data;
    while((data = INTC_SERIAL.read()) >= 0) {
        if(intcRxBuffWriteIdx == 0) {
            if(data == INTC_FRAME_HEADER) {
                intcRxBuffer[0] = INTC_FRAME_HEADER;
                intcRxBuffWriteIdx++;
            }
        } else {
            intcRxBuffer[intcRxBuffWriteIdx++] = data;
            if(intcRxBuffWriteIdx == INTC_FRAMESIZE) {
                if(intcCheckRxFrame()) {
                    intcLastReceivedUs = micros();
                    intcTimeoutFlg = false;
                } else {
                    LOG_F("INTC Invalid frame with ID: %i\n", intcRxBuffer[1]);
                }
                intcRxBuffWriteIdx = 0;
            }
        }
    }
}

void intcHandle() {
    intcSendNext();
    intcReceiveAll();
    if((micros() - intcLastReceivedUs) > INTC_TIMEOUT_US) {
        intcTimeoutFlg = true;
    }
}
