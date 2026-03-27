#include <Arduino.h>
#include <SPI.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <GxEPD2_BW.h>

#define EPD_PIN_CS   41
#define EPD_PIN_DC   42
#define EPD_PIN_RST  2
#define EPD_PIN_BUSY 1
#define EPD_W        400
#define EPD_H        300
#define MARGIN_X     8
#define MARGIN_Y     8

static GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> epd(
    GxEPD2_420_GDEY042T81(EPD_PIN_CS, EPD_PIN_DC, EPD_PIN_RST, EPD_PIN_BUSY));

static int16_t g_line_h = 20;

void setup() {
    SPI.begin(39, -1, 40, 41);
    epd.init(115200, true, 50, false);
    epd.setRotation(0);
    epd.setFullWindow();
    epd.setFont(&FreeMonoBold12pt7b);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    epd.getTextBounds("A", 0, 0, &tbx, &tby, &tbw, &tbh);
    g_line_h = (int16_t)tbh + 6;

    epd.fillScreen(GxEPD_WHITE);
    epd.fillRect(MARGIN_X - 2, MARGIN_Y - 2, EPD_W - MARGIN_X * 2 + 4, g_line_h + 4, GxEPD_BLACK);
    epd.setTextColor(GxEPD_WHITE);
    epd.setCursor(MARGIN_X + 2, MARGIN_Y + g_line_h - 2);
    epd.print("The Hobbit");
    epd.display(false);
}

void loop() {
    delay(10000);
}
