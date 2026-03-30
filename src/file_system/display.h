#pragma once
#include "types.h"
#include <Arduino.h>

#define EPD_PIN_CS   41
#define EPD_PIN_DC   42
#define EPD_PIN_RST  2
#define EPD_PIN_BUSY 1
#define SPI_SCK      39
#define SPI_MOSI     40
#define EPD_W        400
#define EPD_H        300
#define MARGIN_X     12
#define MARGIN_Y     12

void displayInit();
void redrawScreen(Screen currentScreen, const Book* books, int bookCount, int selectedIndex, int openBookIndex, int textOffset);
