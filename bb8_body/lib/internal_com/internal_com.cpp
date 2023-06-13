#include <internal_com.h>
#include <SPI.h>
#include <Arduino.h>

#define INTC_SPI_CLK 1000000 // 1 MHz

bool intcInit() {
    SPI.begin(SCK, MISO, MOSI, SS);
    pinMode(SS, OUTPUT);
    SPI.beginTransaction(SPISettings(INTC_SPI_CLK, MSBFIRST, SPI_MODE0));
    SPI.setHwCs(true); // TODO: check this
    return true;
}

bool intcSendData(int idx) {
    
}

void intcSendSPI(uint8_t* dataIn, uint8_t* dataOut, uint32_t len) {
//   digitalWrite(spi->pinSS(), HIGH); //pull ss low to signify start of data transfer
    SPI.transferBytes(dataIn, dataOut, len);
//   digitalWrite(spi->pinSS(), HIGH); //pull ss high to signify end of data transfer
}
