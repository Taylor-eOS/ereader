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

static void draw_screen(int index) {
    if (index == 0) {
        epd.fillScreen(GxEPD_WHITE);
        epd.setTextColor(GxEPD_BLACK);
        epd.setCursor(MARGIN_X, MARGIN_Y + g_line_h);
        epd.print("White background");
        epd.setCursor(MARGIN_X, MARGIN_Y + g_line_h * 2);
        epd.print("Black text");
    } else if (index == 1) {
        epd.fillScreen(GxEPD_BLACK);
        epd.setTextColor(GxEPD_WHITE);
        epd.setCursor(MARGIN_X, MARGIN_Y + g_line_h);
        epd.print("Black background");
        epd.setCursor(MARGIN_X, MARGIN_Y + g_line_h * 2);
        epd.print("White text");
    } else if (index == 2) {
        epd.fillScreen(GxEPD_WHITE);
        for (int y = 0; y < EPD_H; y += 20)
            for (int x = 0; x < EPD_W; x += 20)
                if (((x / 20) + (y / 20)) % 2 == 0)
                    epd.fillRect(x, y, 20, 20, GxEPD_BLACK);
        epd.setTextColor(GxEPD_WHITE);
        epd.setCursor(MARGIN_X, MARGIN_Y + g_line_h);
        epd.print("Checkerboard");
    } else {
        epd.fillScreen(GxEPD_WHITE);
        for (int i = 0; i < 5; i++)
            epd.drawRect(i * 12, i * 12, EPD_W - i * 24, EPD_H - i * 24, GxEPD_BLACK);
        epd.setTextColor(GxEPD_BLACK);
        epd.setCursor(MARGIN_X + 20, MARGIN_Y + g_line_h);
        epd.print("Border test");
    }
    epd.display(false);
}

void setup() {
    randomSeed(esp_random());
    SPI.begin(39, -1, 40, 41);
    epd.init(115200, true, 50, false);
    epd.setRotation(0);
    epd.setFullWindow();
    epd.setFont(&FreeMonoBold12pt7b);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    epd.getTextBounds("A", 0, 0, &tbx, &tby, &tbw, &tbh);
    g_line_h = (int16_t)tbh + 6;
    draw_screen(random(4));
}

void loop() {
    delay(10000);
}
