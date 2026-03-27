#include <Arduino.h>
#include <SPI.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <GxEPD2_BW.h>

#define EPD_PIN_CS   41
#define EPD_PIN_DC   42
#define EPD_PIN_RST  2
#define EPD_PIN_BUSY 1
#define SPI_SCK      39
#define SPI_MOSI     40
#define BTN_UP       5
#define BTN_DOWN     6
#define BTN_SELECT   7
#define EPD_W        400
#define EPD_H        300
#define MARGIN_X     12
#define MARGIN_Y     12

static GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> epd(
    GxEPD2_420_GDEY042T81(EPD_PIN_CS, EPD_PIN_DC, EPD_PIN_RST, EPD_PIN_BUSY)
);

static const char* menuItems[] = {
    "Continue Reading",
    "Library",
    "Settings",
    "About"
};

static const int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);

static int selectedIndex = 0;
static int16_t g_line_h = 24;
static bool g_needsRedraw = true;

bool buttonPressed(uint8_t pin);
void drawMenu();
void updateInput();

void setup() {
    Serial.begin(115200);

    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);

    SPI.begin(SPI_SCK, -1, SPI_MOSI, EPD_PIN_CS);

    epd.init(115200, true, 50, false);
    epd.setRotation(0);
    epd.setFullWindow();
    epd.setFont(&FreeMonoBold12pt7b);
    epd.setTextColor(GxEPD_BLACK);

    int16_t tbx, tby;
    uint16_t tbw, tbh;
    epd.getTextBounds("Ag", 0, 0, &tbx, &tby, &tbw, &tbh);
    g_line_h = (int16_t)tbh + 12;

    drawMenu();
}

void loop() {
    updateInput();

    if (g_needsRedraw) {
        drawMenu();
        g_needsRedraw = false;
    }

    delay(20);
}

bool buttonPressed(uint8_t pin) {
    static uint32_t lastPressTime[40] = {0};

    if (digitalRead(pin) == LOW) {
        uint32_t now = millis();
        if (now - lastPressTime[pin] > 220) {
            lastPressTime[pin] = now;
            return true;
        }
    }

    return false;
}

void updateInput() {
    if (buttonPressed(BTN_UP)) {
        selectedIndex--;
        if (selectedIndex < 0) {
            selectedIndex = menuCount - 1;
        }
        g_needsRedraw = true;
        Serial.printf("UP -> %d\n", selectedIndex);
    }

    if (buttonPressed(BTN_DOWN)) {
        selectedIndex++;
        if (selectedIndex >= menuCount) {
            selectedIndex = 0;
        }
        g_needsRedraw = true;
        Serial.printf("DOWN -> %d\n", selectedIndex);
    }

    if (buttonPressed(BTN_SELECT)) {
        Serial.printf("SELECT -> %s\n", menuItems[selectedIndex]);
    }
}

void drawMenu() {
    epd.fillScreen(GxEPD_WHITE);

    epd.setTextColor(GxEPD_BLACK);
    epd.setCursor(MARGIN_X, MARGIN_Y + g_line_h - 4);
    epd.print("E-Reader");

    int startY = MARGIN_Y + g_line_h + 18;

    for (int i = 0; i < menuCount; i++) {
        int y = startY + i * (g_line_h + 10);
        int boxY = y - g_line_h + 6;
        int boxH = g_line_h + 6;

        if (i == selectedIndex) {
            epd.fillRect(MARGIN_X - 4, boxY, EPD_W - (MARGIN_X * 2), boxH, GxEPD_BLACK);
            epd.setTextColor(GxEPD_WHITE);
        } else {
            epd.setTextColor(GxEPD_BLACK);
        }

        epd.setCursor(MARGIN_X + 4, y);
        epd.print(menuItems[i]);
    }

    epd.display(false);
}
