#include "display.h"
#include <SPI.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <GxEPD2_BW.h>
#include <LittleFS.h>

static GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> epd(GxEPD2_420_GDEY042T81(EPD_PIN_CS, EPD_PIN_DC, EPD_PIN_RST, EPD_PIN_BUSY));

static int16_t lineHeight = 18;

void displayInit() {
    SPI.begin(SPI_SCK, -1, SPI_MOSI, EPD_PIN_CS);
    epd.init(115200, true, 50, false);
    epd.setRotation(0);
    epd.setFullWindow();
    epd.setFont(&FreeSerif9pt7b);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    epd.getTextBounds("Ag", 0, 0, &tbx, &tby, &tbw, &tbh);
    lineHeight = tbh + 6;
}

static void drawMenu(const Book* books, int bookCount, int selectedIndex) {
    int menuCount = bookCount + 1;
    int y = MARGIN_Y + lineHeight;
    for (int i = 0; i < bookCount; i++) {
        int boxY = y - lineHeight + 4;
        int boxH = lineHeight + 6;
        if (i == selectedIndex) {
            epd.fillRect(MARGIN_X - 4, boxY, EPD_W - (MARGIN_X * 2), boxH, GxEPD_BLACK);
            epd.setTextColor(GxEPD_WHITE);
        } else {
            epd.setTextColor(GxEPD_BLACK);
        }
        epd.setCursor(MARGIN_X, y);
        epd.print(books[i].title);
        y += lineHeight + 10;
    }
    int settingsIndex = menuCount - 1;
    int boxY = y - lineHeight + 4;
    int boxH = lineHeight + 6;
    if (selectedIndex == settingsIndex) {
        epd.fillRect(MARGIN_X - 4, boxY, EPD_W - (MARGIN_X * 2), boxH, GxEPD_BLACK);
        epd.setTextColor(GxEPD_WHITE);
    } else {
        epd.setTextColor(GxEPD_BLACK);
    }
    epd.setCursor(MARGIN_X, y);
    epd.print("Settings");
}

static void drawReading(const Book* books, int openBookIndex, int textOffset) {
    epd.setTextColor(GxEPD_BLACK);
    epd.setTextWrap(true);
    epd.setCursor(MARGIN_X, MARGIN_Y + lineHeight);
    File f = LittleFS.open(books[openBookIndex].path, "r");
    if (!f) return;
    if (textOffset > 0) f.seek(textOffset);
    char buf[800];
    int bytesRead = f.read((uint8_t*)buf, sizeof(buf) - 1);
    f.close();
    if (bytesRead <= 0) return;
    buf[bytesRead] = '\0';
    epd.print(buf);
}

static void drawSettings() {
    epd.setTextColor(GxEPD_BLACK);
    epd.setCursor(MARGIN_X, MARGIN_Y + lineHeight);
    epd.print("Settings");
    epd.setCursor(MARGIN_X, MARGIN_Y + lineHeight * 2 + 10);
    epd.print("Press select to go back.");
}

void redrawScreen(Screen currentScreen, const Book* books, int bookCount, int selectedIndex, int openBookIndex, int textOffset) {
    epd.fillScreen(GxEPD_WHITE);
    if (currentScreen == SCREEN_MENU) {
        drawMenu(books, bookCount, selectedIndex);
    } else if (currentScreen == SCREEN_READING) {
        drawReading(books, openBookIndex, textOffset);
    } else if (currentScreen == SCREEN_SETTINGS) {
        drawSettings();
    }
    epd.display(false);
}
