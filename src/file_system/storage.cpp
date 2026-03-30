#include "storage.h"
#include <LittleFS.h>

bool storageInit() {
    return LittleFS.begin(false, "/littlefs", 10, "spiffs");
}

int storageGetBooks(Book* out, int maxCount) {
    File root = LittleFS.open("/");
    if (!root || !root.isDirectory()) return 0;
    int count = 0;
    while (count < maxCount) {
        File entry = root.openNextFile();
        if (!entry) break;
        if (entry.isDirectory()) {
            entry.close();
            continue;
        }
        const char* name = entry.name();
        int nameLen = strlen(name);
        if (nameLen < 5) {
            entry.close();
            continue;
        }
        const char* ext = name + nameLen - 4;
        if (!(ext[0] == '.' &&
              (ext[1] == 't' || ext[1] == 'T') &&
              (ext[2] == 'x' || ext[2] == 'X') &&
              (ext[3] == 't' || ext[3] == 'T'))) {
            entry.close();
            continue;
        }
        int copyLen = nameLen - 4 < 63 ? nameLen - 4 : 63;
        memcpy(out[count].title, name, copyLen);
        out[count].title[copyLen] = '\0';
        out[count].path[0] = '/';
        strncpy(out[count].path + 1, name, 62);
        out[count].path[63] = '\0';
        out[count].fileSize = entry.size();
        entry.close();
        count++;
    }
    root.close();
    return count;
}
