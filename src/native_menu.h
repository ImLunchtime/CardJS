#pragma once

#include <Arduino.h>
#include <vector>

#include "duktape_port.h"

enum MenuItemType {
  MENU_ITEM_BUTTON,
  MENU_ITEM_NUMBER,
  MENU_ITEM_STRING,
  MENU_ITEM_SELECTION,
  MENU_ITEM_BOOL
};

struct NativeMenuItem {
  MenuItemType type;
  String label;
  int numberValue;
  int numberMin;
  int numberMax;
  String stringValue;
  int maxLength;
  std::vector<String> options;
  int selectionIndex;
  bool boolValue;
  String trueText;
  String falseText;
  void *callbackFunc;
};

 class NativeMenu {
 public:
  NativeMenu();
  void addButton(const char *label, void *callbackFunc);
  void addNumber(const char *label, int defaultValue, int minVal, int maxVal);
  void addString(const char *label, const char *defaultValue, int maxLen);
  void addSelection(const char *label, const std::vector<String> &options, int defaultIndex);
  void addBool(const char *label, bool defaultValue, const char *trueText, const char *falseText);
  void refresh();
  void update(duk_context *ctx);

 private:
  std::vector<NativeMenuItem> items;
  int selectedIndex;
  int scrollOffset;
  bool needsInitialDraw;
  bool editing;
  int editingIndex;
  bool editingIsNumber;
  String editBuffer;
  uint16_t bgColor;
  uint16_t itemColor;
  uint16_t itemHighlightColor;
  uint16_t textColor;
  uint16_t textSelectedColor;
  uint16_t lineLightColor;
  uint16_t lineDarkColor;

  void ensureSelectionVisible();
  void draw();
  void drawItem(const NativeMenuItem &item, int index, int y, int itemHeight, bool selected);
};
