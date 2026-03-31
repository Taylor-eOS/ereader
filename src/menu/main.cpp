#include <Arduino.h>
#include "types.h"
#include "storage.h"
#include "display.h"
#include "input.h"

static Book books[MAX_BOOKS];
static int bookCount = 0;
static int menuCount = 0;
static Screen currentScreen = SCREEN_MENU;
static int selectedIndex = 0;
static int openBookIndex = -1;
static bool needsRedraw = true;
static int textOffset = 0;

void setup() {
    Serial.begin(115200);
    inputInit();
    storageInit();
    bookCount = storageGetBooks(books, MAX_BOOKS);
    menuCount = bookCount + 1;
    displayInit();
}

void loop() {
    updateInput(currentScreen, selectedIndex, openBookIndex, textOffset, needsRedraw, menuCount, books);
    if (needsRedraw) {
        redrawScreen(currentScreen, books, bookCount, selectedIndex, openBookIndex, textOffset);
        needsRedraw = false;
    }
    delay(20);
}
