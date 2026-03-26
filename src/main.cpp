#include <Arduino.h>
#include <SPI.h>

#include "ui.h"

#define BTN_UP 33
#define BTN_DOWN 32
#define BTN_ENTER 27
#define BTN_BACK 26

static const unsigned long DEBOUNCE_MS = 180;
static unsigned long last_up = 0;
static unsigned long last_down = 0;
static unsigned long last_enter = 0;
static unsigned long last_back = 0;

void read_buttons(bool& up, bool& down, bool& enter, bool& back) {
  unsigned long now = millis();
  up = digitalRead(BTN_UP) == LOW && now - last_up > DEBOUNCE_MS;
  down = digitalRead(BTN_DOWN) == LOW && now - last_down > DEBOUNCE_MS;
  enter = digitalRead(BTN_ENTER) == LOW && now - last_enter > DEBOUNCE_MS;
  back = digitalRead(BTN_BACK) == LOW && now - last_back > DEBOUNCE_MS;
  if (up) last_up = now;
  if (down) last_down = now;
  if (enter) last_enter = now;
  if (back) last_back = now;
}

void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  SPI.begin(39, -1, 40, 41);
  ui_init();
  setup_menus();
  render_current_menu();
}

void loop() {
  if (ui_halted()) {
    delay(1000);
    return;
  }
  bool up, down, enter, back;
  read_buttons(up, down, enter, back);
  if (up) {
    menu_move_up();
    render_current_menu();
  }
  if (down) {
    menu_move_down();
    render_current_menu();
  }
  if (enter) {
    menu_enter();
    render_current_menu();
  }
  if (back) {
    menu_back();
    render_current_menu();
  }
  delay(60);
}
