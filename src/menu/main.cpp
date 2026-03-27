#include <Arduino.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <GxEPD2_BW.h>
#include <SPI.h>

#define EPD_PIN_CS 41
#define EPD_PIN_DC 42
#define EPD_PIN_RST 2
#define EPD_PIN_BUSY 1
#define SPI_SCK 39
#define SPI_MOSI 40
#define BTN_UP 5
#define BTN_DOWN 6
#define BTN_SELECT 7
#define EPD_W 400
#define EPD_H 300
#define MARGIN_X 12
#define MARGIN_Y 12

static GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> epd(GxEPD2_420_GDEY042T81(EPD_PIN_CS, EPD_PIN_DC, EPD_PIN_RST, EPD_PIN_BUSY));

enum Screen { SCREEN_MENU, SCREEN_READING };

static Screen currentScreen = SCREEN_MENU;
static const char* menuItems[] = {"Read Demo Book", "Settings"};

static const int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);
static int selectedIndex = 0;
static bool needsRedraw = true;
static int16_t lineHeight = 18;
static const char* demoText =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
    "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
    "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
    "nisi ut aliquip ex ea commodo consequat.";
static int textOffset = 0;
bool buttonPressed(uint8_t pin);
void updateInput();
void redrawScreen();
void drawMenu();
void drawReading();
int measureWordWidth(const char* word);
int drawWrappedText(const char* text, int startIndex);

void setup() {
  Serial.begin(115200);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  SPI.begin(SPI_SCK, -1, SPI_MOSI, EPD_PIN_CS);
  epd.init(115200, true, 50, false);
  epd.setRotation(0);
  epd.setFullWindow();
  epd.setFont(&FreeSerif9pt7b);
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  epd.getTextBounds("Ag", 0, 0, &tbx, &tby, &tbw, &tbh);
  lineHeight = tbh + 6;
  redrawScreen();
}

void loop() {
  updateInput();
  if (needsRedraw) {
    redrawScreen();
    needsRedraw = false;
  }
  delay(20);
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

void updateInput() {
  if (currentScreen == SCREEN_MENU) {
    if (buttonPressed(BTN_UP)) {
      selectedIndex--;
      if (selectedIndex < 0) selectedIndex = menuCount - 1;
      needsRedraw = true;
    }
    if (buttonPressed(BTN_DOWN)) {
      selectedIndex++;
      if (selectedIndex >= menuCount) selectedIndex = 0;
      needsRedraw = true;
    }
    if (buttonPressed(BTN_SELECT)) {
      if (selectedIndex == 0) {
        currentScreen = SCREEN_READING;
        textOffset = 0;
        needsRedraw = true;
      }
    }
  } else if (currentScreen == SCREEN_READING) {
    if (buttonPressed(BTN_DOWN)) {
      textOffset += 100;
      needsRedraw = true;
    }
    if (buttonPressed(BTN_UP)) {
      textOffset -= 100;
      if (textOffset < 0) textOffset = 0;
      needsRedraw = true;
    }
    if (buttonPressed(BTN_SELECT)) {
      currentScreen = SCREEN_MENU;
      needsRedraw = true;
    }
  }
}

void redrawScreen() {
  epd.fillScreen(GxEPD_WHITE);
  if (currentScreen == SCREEN_MENU) {
    drawMenu();
  } else if (currentScreen == SCREEN_READING) {
    drawReading();
  }
  epd.display(false);
}

void drawMenu() {
  int y = MARGIN_Y + lineHeight;
  for (int i = 0; i < menuCount; i++) {
    int boxY = y - lineHeight + 4;
    int boxH = lineHeight + 6;
    if (i == selectedIndex) {
      epd.fillRect(MARGIN_X - 4, boxY, EPD_W - (MARGIN_X * 2), boxH, GxEPD_BLACK);
      epd.setTextColor(GxEPD_WHITE);
    } else {
      epd.setTextColor(GxEPD_BLACK);
    }
    epd.setCursor(MARGIN_X, y);
    epd.print(menuItems[i]);
    y += lineHeight + 10;
  }
}

void drawReading() {
  epd.setTextColor(GxEPD_BLACK);
  drawWrappedText(demoText, textOffset);
}

int measureWordWidth(const char* word) {
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  epd.getTextBounds(word, 0, 0, &tbx, &tby, &tbw, &tbh);
  return tbw;
}

int drawWrappedText(const char* text, int startIndex) {
  int x = MARGIN_X;
  int y = MARGIN_Y + lineHeight;
  int spaceWidth = measureWordWidth(" ");
  int i = startIndex;
  int len = strlen(text);
  char word[64];
  int wlen = 0;
  while (i <= len) {
    if (text[i] == ' ' || text[i] == '\0') {
      word[wlen] = '\0';
      int w = measureWordWidth(word);
      if (x + w > EPD_W - MARGIN_X) {
        x = MARGIN_X;
        y += lineHeight;
      }
      if (y > EPD_H - MARGIN_Y) {
        return i;
      }
      epd.setCursor(x, y);
      epd.print(word);
      x += w + spaceWidth;
      wlen = 0;
      i++;
      continue;
    }
    if (wlen < 63) {
      word[wlen++] = text[i];
    }
    i++;
  }
  return i;
}
