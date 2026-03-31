#pragma once
#include "types.h"
#include "storage.h"
#include <Arduino.h>

#define BTN_UP     5
#define BTN_DOWN   6
#define BTN_SELECT 7
#define PAGE_STEP  CHUNK_SIZE

void inputInit();
bool buttonPressed(uint8_t pin);
void updateInput(Screen& currentScreen, int& selectedIndex, int& openBookIndex, int& textOffset, bool& needsRedraw, int menuCount, const Book* books);
