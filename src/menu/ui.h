#pragma once
#include <functional>
#include <string>
#include <vector>

using Action = std::function<void()>;

struct Menu;

struct MenuItem {
  std::string text;
  Action action;
  Menu* submenu;
  MenuItem(const std::string& t = "", Action a = nullptr, Menu* s = nullptr) : text(t), action(a), submenu(s) {}
};

struct Menu {
  std::vector<MenuItem> items;
  int selected_index;
  Menu* parent;
  Menu(const std::vector<MenuItem>& it = {}) : items(it), selected_index(0), parent(nullptr) {}
};

enum class ScreenMode { MENU, PAGE };

void ui_init();
bool ui_halted();
void setup_menus();
void render_current_menu(bool full_refresh = false);
void menu_move_up();
void menu_move_down();
void menu_enter();
void menu_back();
void display_page(const std::string& title, const std::string& body);
