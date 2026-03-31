#include <SD.h>
#include <SPI.h>
SPIClass spi(1);
void setup() {
    Serial.begin(115200);
    spi.begin(12, 13, 11, 10);
    SD.begin(10, spi);
    File root = SD.open("/");
    root.rewindDirectory();
    File f = root.openNextFile();
    Serial.println(f ? f.name() : "no files");
}
void loop() {}
