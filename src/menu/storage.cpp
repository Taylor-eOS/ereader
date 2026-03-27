#include "storage.h"

static const char demoText[] =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
    "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
    "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
    "nisi ut aliquip ex ea commodo consequat.";

bool storageInit() {
    // TODO: replace with SD.begin(SD_PIN_CS)
    // #include <SD.h>
    // return SD.begin(SD_PIN_CS);
    return true;
}

int storageGetBooks(Book* out, int maxCount) {
    // TODO: replace with SD directory scan
    // Open SD root, iterate .txt files, populate out[] with title from filename
    // and text as a null-terminated buffer read from the file.
    if (maxCount < 1) return 0;
    out[0].title = "Lorem Ipsum";
    out[0].text = demoText;
    return 1;
}
