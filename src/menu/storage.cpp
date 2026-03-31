#include "storage.h"
#include <SD_MMC.h>

static char titleBufs[MAX_BOOKS][64];
static char* textBufs[MAX_BOOKS] = {nullptr};

bool storageInit() {
    esp_err_t err = SD_MMC.setPins(12, 11, 13);
    Serial.printf("setPins: %d (%s)\n", err, esp_err_to_name(err));
    bool success = SD_MMC.begin("/sdmc", true, false, SDMMC_FREQ_DEFAULT);
    Serial.println(success ? "SD card initialized" : "SD card initialization failed");
    Serial.printf("Card type: %d\n", SD_MMC.cardType());
    return success;
}

int storageGetBooks(Book* out, int maxCount) {
    Serial.printf("Used bytes: %llu\n", SD_MMC.usedBytes());
    File root = SD_MMC.open("/");
    Serial.printf("root: %s isDir: %d\n", root ? "OK" : "FAIL", root ? (int)root.isDirectory() : -1);
    if (!root || !root.isDirectory()) { root.close(); return 0; }
    root.rewindDirectory();
    int count = 0;
    while (count < maxCount) {
        File f = root.openNextFile();
        if (!f) break;
        Serial.printf("Found: %s (%d bytes)\n", f.name(), (int)f.size());
        size_t fileSize = f.size();
        if (f.isDirectory() || fileSize == 0) { f.close(); continue; }
        if (textBufs[count] != nullptr) { free(textBufs[count]); textBufs[count] = nullptr; }
        textBufs[count] = (char*)malloc(fileSize + 1);
        if (textBufs[count] == nullptr) { Serial.println("  malloc failed"); f.close(); continue; }
        size_t bytesRead = f.read((uint8_t*)textBufs[count], fileSize);
        f.close();
        if (bytesRead != fileSize) {
            Serial.printf("  read error: %d/%d\n", (int)bytesRead, (int)fileSize);
            free(textBufs[count]); textBufs[count] = nullptr; continue;
        }
        textBufs[count][fileSize] = '\0';
        char* newline = strchr(textBufs[count], '\n');
        if (newline == nullptr) { Serial.println("  no newline"); free(textBufs[count]); textBufs[count] = nullptr; continue; }
        int titleLen = newline - textBufs[count];
        if (titleLen > 63) titleLen = 63;
        memcpy(titleBufs[count], textBufs[count], titleLen);
        titleBufs[count][titleLen] = '\0';
        out[count].title = titleBufs[count];
        out[count].text = newline + 1;
        Serial.printf("  Loaded: %s\n", titleBufs[count]);
        count++;
    }
    root.close();
    Serial.printf("Books loaded: %d\n", count);
    return count;
}
