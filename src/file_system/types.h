#pragma once
#include <cstddef>

enum Screen {
    SCREEN_MENU,
    SCREEN_READING,
    SCREEN_SETTINGS
};

struct Book {
    char title[64];
    char path[64];
    size_t fileSize;
};
