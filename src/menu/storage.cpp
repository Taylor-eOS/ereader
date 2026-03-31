#include "storage.h"
#include <SD.h>
#include <SPI.h>

#define SD_PIN_SCK  12
#define SD_PIN_MOSI 11
#define SD_PIN_MISO 13
#define SD_PIN_CS   10

static SPIClass sdSpi(HSPI);

static char titleBufs[MAX_BOOKS][64];
static char* textBufs[MAX_BOOKS] = {nullptr};

bool storageInit() {
    sdSpi.begin(SD_PIN_SCK, SD_PIN_MISO, SD_PIN_MOSI, SD_PIN_CS);
    return SD.begin(SD_PIN_CS, sdSpi);
}

int storageGetBooks(Book* out, int maxCount) {
    File root = SD.open("/");
    if (!root || !root.isDirectory()) return 0;
    int count = 0;
    while (count < maxCount) {
        File entry = root.openNextFile();
        if (!entry) break;
        if (entry.isDirectory()) { entry.close(); continue; }
        const char* name = entry.name();
        int nameLen = strlen(name);
        if (nameLen < 5 || strcmp(name + nameLen - 4, ".txt") != 0) { entry.close(); continue; }
        Serial.printf("Found book: %s (%u bytes)\n", name, (unsigned)entry.size());
        if (textBufs[count] != nullptr) {
            free(textBufs[count]);
            textBufs[count] = nullptr;
        }
        size_t fileSize = entry.size();
        textBufs[count] = (char*)malloc(fileSize + 1);
        if (textBufs[count] == nullptr) { Serial.printf("malloc failed for %s\n", name); entry.close(); continue; }
        entry.read((uint8_t*)textBufs[count], fileSize);
        textBufs[count][fileSize] = '\0';
        entry.close();
        int copyLen = nameLen - 4 < 63 ? nameLen - 4 : 63;
        memcpy(titleBufs[count], name, copyLen);
        titleBufs[count][copyLen] = '\0';
        out[count].title = titleBufs[count];
        out[count].text = textBufs[count];
        count++;
    }
    root.close();
    Serial.printf("Total books loaded: %d\n", count);
    return count;
}
