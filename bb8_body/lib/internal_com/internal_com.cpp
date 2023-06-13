#include <internal_com.h>
#include <Arduino.h>
#include <registers.h>

#define INTC_SERIAL Serial2
const uint8_t IntcRegTxIndexes[] = {1,5,9}; // TODO: set real indexes
#define INTC_REGTX_COUNT (sizeof(IntcRegTxIndexes))
#define INTC_RXBUFF_SIZE (256) // TODO: check
#define INTC_FRAMESIZE  (6)

uint8_t intcRegTxCounter;
uint8_t intcRxBuffer[INTC_RXBUFF_SIZE];
uint32_t intcRxBuffWriteIdx, intcRxBuffStartIdx;

bool intcInit() {
    INTC_SERIAL.begin(115200);
    intcRegTxCounter = 0;
    intcRxBuffWriteIdx = 0;
    intcRxBuffStartIdx = 0;
    return true;
}

void intcSendData(uint8_t idx) { // TODO: pack multiple frames to reduce bus load
    uint8_t buffer[INTC_FRAMESIZE];
    buffer[0] = idx;
    *(uint32_t*)(&buffer[1]) = *regsRegisters[idx].pi;
    buffer[5] = buffer[0] + buffer[1] + buffer[2] + buffer[3] + buffer[4];
    INTC_SERIAL.write(buffer, sizeof(buffer)); // TODO: check if really async
}

void intcSendNext() {
    if(intcRegTxCounter >= INTC_REGTX_COUNT) {
        intcRegTxCounter = 0;
    }
    intcSendData(IntcRegTxIndexes[intcRegTxCounter++]);
}

void intcReceiveAll() {
    size_t count = INTC_SERIAL.available();
    if(count > (INTC_RXBUFF_SIZE - intcRxBuffWriteIdx)) count = INTC_RXBUFF_SIZE - intcRxBuffWriteIdx;
    count = INTC_SERIAL.readBytes(&intcRxBuffer[intcRxBuffWriteIdx], count);
    intcRxBuffWriteIdx += count;
}

bool intcCheckReceivedFrame() {
    if(intcRxBuffer[intcRxBuffStartIdx + 5] == 
        intcRxBuffer[intcRxBuffStartIdx + 0] +
        intcRxBuffer[intcRxBuffStartIdx + 1] +
        intcRxBuffer[intcRxBuffStartIdx + 2] +
        intcRxBuffer[intcRxBuffStartIdx + 3] +
        intcRxBuffer[intcRxBuffStartIdx + 4]) {
            uint32_t* reg = regsRegisters[intcRxBuffer[intcRxBuffStartIdx + 0]].pi;
            if(reg != nullptr) {
                *reg = *(uint32_t*)&intcRxBuffer[intcRxBuffStartIdx + 1];
            }
            return true;
    }
    return false;
}

void intcDecodeFrames() {
    while((intcRxBuffStartIdx + 6) > intcRxBuffWriteIdx) {
        if(intcCheckReceivedFrame()) {
            intcRxBuffStartIdx += 6;
        } else {
            intcRxBuffStartIdx++;
        }
    }
    // clean up processed data
    if(intcRxBuffWriteIdx > (INTC_RXBUFF_SIZE - INTC_FRAMESIZE)) {
        if(intcRxBuffStartIdx == intcRxBuffWriteIdx) {
            intcRxBuffStartIdx = 0;
            intcRxBuffWriteIdx = 0;
        } else {
            uint32_t tmpIdx = 0;
            while(intcRxBuffStartIdx < intcRxBuffWriteIdx) {
                intcRxBuffer[tmpIdx++] = intcRxBuffer[intcRxBuffStartIdx++];
            }
            intcRxBuffStartIdx = 0;
            intcRxBuffWriteIdx = tmpIdx;
        }
    }
}

void intcHandle() {
    intcSendNext();
    intcReceiveAll();
    intcDecodeFrames();
}
