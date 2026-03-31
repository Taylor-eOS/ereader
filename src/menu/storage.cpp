#include "storage.h"
#include <SD_MMC.h>

static char titleBufs[MAX_BOOKS][64];
static char pathBufs[MAX_BOOKS][128];

bool storageInit() {
    esp_err_t err = SD_MMC.setPins(12, 11, 13);
    Serial.printf("setPins: %d (%s)\n", err, esp_err_to_name(err));
    bool success = SD_MMC.begin("/sdmc", true, false, SDMMC_FREQ_PROBING);
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
        if (f.isDirectory() || f.size() == 0) { f.close(); continue; }
        snprintf(pathBufs[count], sizeof(pathBufs[count]), "/%s", f.name());
        char headBuf[128];
        size_t headRead = f.read((uint8_t*)headBuf, sizeof(headBuf) - 1);
        f.close();
        headBuf[headRead] = '\0';
        char* newline = strchr(headBuf, '\n');
        if (newline != nullptr) {
            int titleLen = newline - headBuf;
            if (titleLen > 63) titleLen = 63;
            memcpy(titleBufs[count], headBuf, titleLen);
            titleBufs[count][titleLen] = '\0';
        } else {
            strncpy(titleBufs[count], f.name(), 63);
            titleBufs[count][63] = '\0';
        }
        out[count].title = titleBufs[count];
        out[count].path = pathBufs[count];
        Serial.printf("  Loaded: %s -> %s\n", titleBufs[count], pathBufs[count]);
        count++;
    }
    root.close();
    Serial.printf("Books loaded: %d\n", count);
    return count;
}

int storageReadBookChunk(const Book* book, int offset, char* buf, int bufSize) {
    File f = SD_MMC.open(book->path);
    if (!f) { buf[0] = '\0'; return 0; }
    char headBuf[128];
    size_t headRead = f.read((uint8_t*)headBuf, sizeof(headBuf) - 1);
    headBuf[headRead] = '\0';
    char* newline = strchr(headBuf, '\n');
    int bodyStart = newline != nullptr ? (newline - headBuf) + 1 : 0;
    f.seek(bodyStart + offset);
    int bytesRead = f.read((uint8_t*)buf, bufSize - 1);
    f.close();
    if (bytesRead < 0) bytesRead = 0;
    buf[bytesRead] = '\0';
    return bytesRead;
}
