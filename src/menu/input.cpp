#include "input.h"

void inputInit() {
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
}

bool buttonPressed(uint8_t pin) {
    static uint32_t lastPress[40] = {0};
    if (digitalRead(pin) == LOW) {
        uint32_t now = millis();
        if (now - lastPress[pin] > 200) {
            lastPress[pin] = now;
            return true;
        }
    }
    return false;
}

void updateInput(Screen& currentScreen, int& selectedIndex, int& openBookIndex, int& textOffset, bool& needsRedraw, int menuCount, const Book* books) {
    if (currentScreen == SCREEN_MENU) {
        if (buttonPressed(BTN_UP)) {
            selectedIndex = (selectedIndex - 1 + menuCount) % menuCount;
            needsRedraw = true;
        }
        if (buttonPressed(BTN_DOWN)) {
            selectedIndex = (selectedIndex + 1) % menuCount;
            needsRedraw = true;
        }
        if (buttonPressed(BTN_SELECT)) {
            if (selectedIndex == menuCount - 1) {
                currentScreen = SCREEN_SETTINGS;
            } else {
                openBookIndex = selectedIndex;
                textOffset = 0;
                currentScreen = SCREEN_READING;
            }
            needsRedraw = true;
        }
    } else if (currentScreen == SCREEN_READING) {
        if (buttonPressed(BTN_DOWN)) {
            int next = textOffset + PAGE_STEP;
            if (next < books[openBookIndex].size) {
                textOffset = next;
                needsRedraw = true;
            }
        }
        if (buttonPressed(BTN_UP)) {
            textOffset -= PAGE_STEP;
            if (textOffset < 0) textOffset = 0;
            needsRedraw = true;
        }
        if (buttonPressed(BTN_SELECT)) {
            currentScreen = SCREEN_MENU;
            needsRedraw = true;
        }
    } else if (currentScreen == SCREEN_SETTINGS) {
        if (buttonPressed(BTN_SELECT)) {
            currentScreen = SCREEN_MENU;
            needsRedraw = true;
        }
    }
}
