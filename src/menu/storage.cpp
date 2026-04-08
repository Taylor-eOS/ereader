#include "storage.h"
#include <SD_MMC.h>

static char titleBufs[MAX_BOOKS][64];
static char pathBufs[MAX_BOOKS][128];

bool storageInit() {
    esp_err_t err = SD_MMC.setPins(12, 11, 13);
    Serial.printf("setPins: %d (%s)\n", err, esp_err_to_name(err));
    bool success = SD_MMC.begin("/sdmc", true, false, SDMMC_FREQ_PROBING);
    Serial.printf(success ? "SD card initialized\n" : "SD card initialization failed\n");
    Serial.printf("Card type: %d\n", SD_MMC.cardType());
    return success;
}

static void filenameToTitle(const char* filename, char* out, int outSize) {
    strncpy(out, filename, outSize - 1);
    out[outSize - 1] = '\0';
    char* dot = strrchr(out, '.');
    if (dot != nullptr) *dot = '\0';
}

int storageGetBooks(Book* out, int maxCount) {
    File root = SD_MMC.open("/");
    if (!root || !root.isDirectory()) {
        if (root) root.close();
        return 0;
    }
    root.rewindDirectory();
    int count = 0;
    while (count < maxCount) {
        File f = root.openNextFile();
        if (!f) break;
        if (f.isDirectory() || f.size() == 0) {
            f.close();
            continue;
        }
        snprintf(pathBufs[count], sizeof(pathBufs[count]), "/%s", f.name());
        filenameToTitle(f.name(), titleBufs[count], sizeof(titleBufs[count]));
        out[count].title = titleBufs[count];
        out[count].path = pathBufs[count];
        out[count].size = (int)f.size();
        f.close();
        count++;
    }
    root.close();
    Serial.printf("Books loaded: %d\n", count);
    return count;
}

int storageReadBookChunk(const Book* book, int offset, char* buf, int bufSize) {
    if (offset >= book->size) { buf[0] = '\0'; return 0; }
    File f = SD_MMC.open(book->path);
    if (!f) { buf[0] = '\0'; return 0; }
    f.seek(offset);
    int bytesRead = f.read((uint8_t*)buf, bufSize - 1);
    f.close();
    if (bytesRead < 0) bytesRead = 0;
    buf[bytesRead] = '\0';
    return bytesRead;
}

