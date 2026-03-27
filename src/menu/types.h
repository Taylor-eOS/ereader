#pragma once

enum Screen {
    SCREEN_MENU,
    SCREEN_READING,
    SCREEN_SETTINGS
};

struct Book {
    const char* title;
    const char* text;
};
