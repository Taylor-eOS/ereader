#include "display.h"
#include "storage.h"
#include <SPI.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <GxEPD2_BW.h>

static GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> epd(GxEPD2_420_GDEY042T81(EPD_PIN_CS, EPD_PIN_DC, EPD_PIN_RST, EPD_PIN_BUSY));
static int16_t lineHeight = 18;
static char chunkBuf[CHUNK_SIZE];
static int lastPageSize = 0;
static void drawSettings();
const int LINES_PER_PAGE = 10;

void displayInit() {
    SPI.begin(SPI_SCK, -1, SPI_MOSI, EPD_PIN_CS);
    epd.init(115200, true, 50, false);
    epd.setRotation(0);
    epd.setFullWindow();
    epd.setFont(&FreeSerif12pt7b);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    epd.getTextBounds("Ag", 0, 0, &tbx, &tby, &tbw, &tbh);
    lineHeight = tbh + 6;
}

static int getTextWidth(const char* str, int len) {
    char temp[256];
    if (len >= (int)sizeof(temp)) len = sizeof(temp) - 1;
    memcpy(temp, str, len);
    temp[len] = '\0';
    int16_t x1, y1;
    uint16_t w, h;
    epd.getTextBounds(temp, 0, 0, &x1, &y1, &w, &h);
    return w;
}

int displayGetLastPageSize() {
    return lastPageSize;
}

static void drawMenu(const Book* books, int bookCount, int selectedIndex) {
    int menuCount = bookCount + 1;
    int y = lineHeight;
    if (bookCount == 0) {
        epd.setTextColor(GxEPD_BLACK);
        epd.setCursor(0, y);
        epd.print("No books found.");
        y += lineHeight + 10;
    }
    for (int i = 0; i < bookCount; i++) {
        int boxY = y - lineHeight + 4;
        int boxH = lineHeight + 6;
        if (i == selectedIndex) {
            epd.fillRect(-4, boxY, EPD_W + 8, boxH, GxEPD_BLACK);
            epd.setTextColor(GxEPD_WHITE);
        } else {
            epd.setTextColor(GxEPD_BLACK);
        }
        epd.setCursor(0, y);
        epd.print(books[i].title);
        y += lineHeight + 10;
    }
    int settingsIndex = menuCount - 1;
    int boxY = y - lineHeight + 4;
    int boxH = lineHeight + 6;
    if (selectedIndex == settingsIndex) {
        epd.fillRect(-4, boxY, EPD_W + 8, boxH, GxEPD_BLACK);
        epd.setTextColor(GxEPD_WHITE);
    } else {
        epd.setTextColor(GxEPD_BLACK);
    }
    epd.setCursor(0, y);
    epd.print("Settings");
}

static int drawTextChunk(const char* text, int length, int maxLines) {
    int y = lineHeight;
    int pos = 0;
    int linesDrawn = 0;
    int totalConsumed = 0;
    const int maxWidth = EPD_W;

    while (linesDrawn < maxLines && pos < length) {
        int lineStart = pos;
        int lineEnd = pos;
        int lastSpace = -1;
        while (lineEnd < length && text[lineEnd] != '\n') {
            char c = text[lineEnd];
            if (c == ' ') lastSpace = lineEnd;
            int width = getTextWidth(text + lineStart, lineEnd - lineStart + 1);
            if (width > maxWidth) {
                if (lastSpace != -1) {
                    lineEnd = lastSpace;
                }
                break;
            }
            lineEnd++;
        }
        bool isNewline = (lineEnd < length && text[lineEnd] == '\n');
        if (isNewline) {
            lineEnd++;
        }
        epd.setCursor(0, y);
        for (int i = lineStart; i < lineEnd; i++) {
            if (text[i] != '\n') epd.print(text[i]);
        }
        y += lineHeight;
        linesDrawn++;
        totalConsumed += (lineEnd - lineStart);
        pos = lineEnd;
        if (pos < length && text[pos] == ' ' && !isNewline) {
            pos++;
            totalConsumed++;
        }
        if (pos < length && text[pos] == '\n') {
            pos++;
            totalConsumed++;
        }
    }
    return totalConsumed;
}

static void drawReading(const Book* books, int openBookIndex, int textOffset) {
    epd.setTextColor(GxEPD_BLACK);
    epd.setTextWrap(false);

    int bytesRead = storageReadBookChunk(&books[openBookIndex], textOffset, chunkBuf, sizeof(chunkBuf));
    if (bytesRead <= 0) {
        lastPageSize = 0;
        return;
    }
    int consumed = drawTextChunk(chunkBuf, bytesRead, LINES_PER_PAGE);
    lastPageSize = consumed;
}

static int measureFittedText(const Book* book, int textOffset) {
    if (textOffset >= book->size) return 0;
    int bytesRead = storageReadBookChunk(book, textOffset, chunkBuf, sizeof(chunkBuf));
    if (bytesRead <= 0) return 0;
    int pos = 0;
    int lines = 0;
    int totalConsumed = 0;
    const int maxWidth = EPD_W;
    while (lines < LINES_PER_PAGE && pos < bytesRead) {
        int lineStart = pos;
        int lineEnd = pos;
        int lastSpace = -1;
        while (lineEnd < bytesRead && chunkBuf[lineEnd] != '\n') {
            if (chunkBuf[lineEnd] == ' ') lastSpace = lineEnd;
            int width = getTextWidth(chunkBuf + lineStart, lineEnd - lineStart + 1);
            if (width > maxWidth) {
                if (lastSpace != -1) lineEnd = lastSpace;
                break;
            }
            lineEnd++;
        }
        if (lineEnd < bytesRead && chunkBuf[lineEnd] == '\n') lineEnd++;
        totalConsumed += (lineEnd - lineStart);
        pos = lineEnd;
        if (pos < bytesRead && chunkBuf[pos] == ' ') { pos++; totalConsumed++; }
        if (pos < bytesRead && chunkBuf[pos] == '\n') { pos++; totalConsumed++; }
        lines++;
    }
    return totalConsumed;
}

void redrawScreen(Screen currentScreen, const Book* books, int bookCount, int selectedIndex, int openBookIndex, int textOffset) {
    epd.fillScreen(GxEPD_WHITE);
    if (currentScreen == SCREEN_MENU) {
        drawMenu(books, bookCount, selectedIndex);
        epd.display(true);
    } else if (currentScreen == SCREEN_READING) {
        drawReading(books, openBookIndex, textOffset);
        epd.display(true);
    } else if (currentScreen == SCREEN_SETTINGS) {
        drawSettings();
        epd.display(true);
    }
}

static void drawSettings() {
    epd.setTextColor(GxEPD_BLACK);
    epd.setCursor(0, lineHeight);
    epd.print("Settings");
    epd.setCursor(0, lineHeight * 2 + 10);
    epd.print("Press select to go back.");
}
