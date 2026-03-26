#include <Fonts/FreeMonoBold12pt7b.h>
#include <GxEPD2_BW.h>
#include "ui.h"

#define EPD_PIN_CS 41
#define EPD_PIN_DC 42
#define EPD_PIN_RST 2
#define EPD_PIN_BUSY 1
#define EPD_W 400
#define EPD_H 300
#define MARGIN_X 8
#define MARGIN_Y 8
static GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> epd(GxEPD2_420_GDEY042T81(EPD_PIN_CS, EPD_PIN_DC, EPD_PIN_RST, EPD_PIN_BUSY));
static bool g_halted = false;
static Menu* g_current = nullptr;
static int16_t g_line_h = 20;
static ScreenMode g_mode = ScreenMode::MENU;
static std::string g_page_title;
static std::string g_page_body;
Menu main_menu;
Menu books_menu;
Menu settings_menu;

static std::vector<std::string> wrap_text(const std::string& text, int16_t maxw) {
  std::vector<std::string> out;
  std::string cur;
  size_t i = 0;
  while (i < text.size()) {
    while (i < text.size() && text[i] == ' ') i++;
    if (i >= text.size()) break;
    size_t j = i;
    while (j < text.size() && text[j] != ' ') j++;
    std::string word = text.substr(i, j - i);
    std::string test = cur.empty() ? word : cur + " " + word;
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    epd.getTextBounds(test.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    if ((int16_t)tbw <= maxw) {
      cur = test;
    } else {
      if (!cur.empty()) out.push_back(cur);
      cur = word;
    }
    i = j;
  }
  if (!cur.empty()) out.push_back(cur);
  return out;
}

void ui_init() {
  epd.init(115200, true, 50, false);
  epd.setRotation(0);
  epd.setFullWindow();
  epd.setFont(&FreeMonoBold12pt7b);
  epd.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  epd.getTextBounds("A", 0, 0, &tbx, &tby, &tbw, &tbh);
  g_line_h = (int16_t)tbh + 4;
}

bool ui_halted() { return g_halted; }

void render_current_menu(bool full_refresh) {
  if (g_mode != ScreenMode::MENU || !g_current) return;
  epd.fillScreen(GxEPD_WHITE);
  int16_t y = MARGIN_Y;
  for (size_t i = 0; i < g_current->items.size(); i++) {
    std::vector<std::string> lines = wrap_text(g_current->items[i].text, EPD_W - MARGIN_X * 2);
    bool selected = (int)i == g_current->selected_index;
    if (selected) {
      int16_t block_h = (int16_t)lines.size() * g_line_h + 4;
      epd.fillRect(MARGIN_X - 2, y - 2, EPD_W - MARGIN_X * 2 + 4, block_h, GxEPD_BLACK);
    }
    for (const std::string& line : lines) {
      int16_t tbx, tby;
      uint16_t tbw, tbh;
      epd.getTextBounds(line.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
      epd.setTextColor(selected ? GxEPD_WHITE : GxEPD_BLACK);
      epd.setCursor(MARGIN_X + 2, y + (int16_t)tbh - 2);
      epd.print(line.c_str());
      y += g_line_h;
    }
    y += 4;
  }
  epd.display(!full_refresh);
}

void display_page(const std::string& title, const std::string& body) {
  g_page_title = title;
  g_page_body = body;
  g_mode = ScreenMode::PAGE;
  epd.fillScreen(GxEPD_WHITE);
  int16_t y = MARGIN_Y;
  std::vector<std::string> title_lines = wrap_text(title, EPD_W - MARGIN_X * 2);
  for (const std::string& line : title_lines) {
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    epd.getTextBounds(line.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    epd.setTextColor(GxEPD_BLACK);
    epd.setCursor(MARGIN_X + 2, y + (int16_t)tbh - 2);
    epd.print(line.c_str());
    y += g_line_h;
  }
  y += 6;
  std::vector<std::string> body_lines = wrap_text(body, EPD_W - MARGIN_X * 2);
  int max_lines = (EPD_H - y) / g_line_h;
  for (int i = 0; i < max_lines && i < (int)body_lines.size(); i++) {
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    epd.getTextBounds(body_lines[i].c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
    epd.setTextColor(GxEPD_BLACK);
    epd.setCursor(MARGIN_X + 2, y + (int16_t)tbh - 2);
    epd.print(body_lines[i].c_str());
    y += g_line_h;
  }
  epd.display(false);
}

void menu_move_up() {
  if (!g_current || g_current->items.empty()) return;
  if (g_current->selected_index > 0) g_current->selected_index--;
}

void menu_move_down() {
  if (!g_current || g_current->items.empty()) return;
  if (g_current->selected_index < (int)g_current->items.size() - 1) g_current->selected_index++;
}

void menu_enter() {
  if (!g_current || g_current->items.empty()) return;
  MenuItem& it = g_current->items[g_current->selected_index];
  if (it.submenu) {
    it.submenu->parent = g_current;
    g_current = it.submenu;
  } else if (it.action) {
    it.action();
  }
}

void menu_back() {
  if (g_mode == ScreenMode::PAGE) {
    g_mode = ScreenMode::MENU;
    return;
  }
  if (g_current && g_current->parent) g_current = g_current->parent;
}

static std::string repeat_text(const std::string& s, int n) {
  std::string out;
  for (int i = 0; i < n; i++) out += s;
  return out;
}

void setup_menus() {
  std::vector<MenuItem> books;
  books.push_back(MenuItem("The Hobbit", []() { display_page("The Hobbit", repeat_text("This is test content for The Hobbit. ", 8)); }));
  books.push_back(MenuItem("1984", []() { display_page("1984", repeat_text("This is test content for 1984. ", 8)); }));
  books.push_back(MenuItem("Brave New World", []() { display_page("Brave New World", repeat_text("This is test content for Brave New World. ", 8)); }));
  books_menu = Menu(books);

  std::vector<MenuItem> settings;
  settings.push_back(MenuItem("Font Size", []() { display_page("Settings", "Font size example"); }));
  settings.push_back(MenuItem("Theme", []() { display_page("Settings", "Theme example"); }));
  settings_menu = Menu(settings);

  std::vector<MenuItem> main;
  main.push_back(MenuItem("Open a Book", nullptr, &books_menu));
  main.push_back(MenuItem("Settings", nullptr, &settings_menu));
  main.push_back(MenuItem("About", []() { display_page("About", "E-ink reader test."); }));
  main.push_back(MenuItem("Exit", []() {
    display_page("Exit", "Goodbye.");
    g_halted = true;
  }));
  main_menu = Menu(main);

  g_current = &main_menu;
}
