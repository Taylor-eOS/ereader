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
static int openBookIndex = 0;
static bool needsRedraw = true;
static int textOffset = 0;

void setup() {
    Serial.begin(115200);
    inputInit();
    bool fsOk = storageInit();
    Serial.print("LittleFS init: ");
    Serial.println(fsOk ? "OK" : "FAILED");
    bookCount = storageGetBooks(books, MAX_BOOKS);
    Serial.print("Books found: ");
    Serial.println(bookCount);
    for (int i = 0; i < bookCount; i++) {
        Serial.print("  ");
        Serial.print(books[i].title);
        Serial.print(" -> ");
        Serial.println(books[i].path);
    }
    menuCount = bookCount + 1;
    displayInit();
    redrawScreen(currentScreen, books, bookCount, selectedIndex, openBookIndex, textOffset);
}

void loop() {
    updateInput(currentScreen, selectedIndex, openBookIndex, textOffset, needsRedraw, menuCount);
    if (needsRedraw) {
        redrawScreen(currentScreen, books, bookCount, selectedIndex, openBookIndex, textOffset);
        needsRedraw = false;
    }
    delay(20);
}
