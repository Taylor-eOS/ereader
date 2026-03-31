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

static void filenameTotitle(const char* filename, char* out, int outSize) {
    strncpy(out, filename, outSize - 1);
    out[outSize - 1] = '\0';
    char* dot = strrchr(out, '.');
    if (dot != nullptr) *dot = '\0';
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
        filenameTotitle(f.name(), titleBufs[count], sizeof(titleBufs[count]));
        f.close();
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
    f.seek(offset);
    int bytesRead = f.read((uint8_t*)buf, bufSize - 1);
    f.close();
    if (bytesRead < 0) bytesRead = 0;
    buf[bytesRead] = '\0';
    return bytesRead;
}
