#include "native_menu.h"

#include "M5Cardputer.h"
#include "M5GFX.h"

NativeMenu::NativeMenu()
    : selectedIndex(0),
      scrollOffset(0),
      active(true),
      needsInitialDraw(true),
      editing(false),
      editingIndex(-1),
      editingIsNumber(false),
      editBuffer("") {
  bgColor = M5Cardputer.Display.color565(16, 16, 16);
  itemColor = M5Cardputer.Display.color565(32, 32, 32);
  itemHighlightColor = M5Cardputer.Display.color565(64, 64, 64);
  textColor = M5Cardputer.Display.color565(200, 200, 200);
  textSelectedColor = M5Cardputer.Display.color565(255, 255, 255);
  lineLightColor = M5Cardputer.Display.color565(80, 80, 80);
  lineDarkColor = M5Cardputer.Display.color565(8, 8, 8);
}

void NativeMenu::addButton(const char *label, void *callbackFunc) {
  NativeMenuItem item;
  item.type = MENU_ITEM_BUTTON;
  item.label = String(label);
  item.numberValue = 0;
  item.numberMin = 0;
  item.numberMax = 0;
  item.stringValue = "";
  item.maxLength = 0;
  item.options.clear();
  item.selectionIndex = 0;
  item.boolValue = false;
  item.trueText = "";
  item.falseText = "";
  item.callbackFunc = callbackFunc;
  items.push_back(item);
}

void NativeMenu::addNumber(const char *label, int defaultValue, int minVal, int maxVal) {
  NativeMenuItem item;
  item.type = MENU_ITEM_NUMBER;
  item.label = String(label);
  item.numberValue = defaultValue;
  item.numberMin = minVal;
  item.numberMax = maxVal;
  item.stringValue = "";
  item.maxLength = 0;
  item.options.clear();
  item.selectionIndex = 0;
  item.boolValue = false;
  item.trueText = "";
  item.falseText = "";
  item.callbackFunc = nullptr;
  items.push_back(item);
}

void NativeMenu::addString(const char *label, const char *defaultValue, int maxLen) {
  NativeMenuItem item;
  item.type = MENU_ITEM_STRING;
  item.label = String(label);
  item.numberValue = 0;
  item.numberMin = 0;
  item.numberMax = 0;
  item.stringValue = String(defaultValue);
  item.maxLength = maxLen;
  item.options.clear();
  item.selectionIndex = 0;
  item.boolValue = false;
  item.trueText = "";
  item.falseText = "";
  item.callbackFunc = nullptr;
  items.push_back(item);
}

void NativeMenu::addSelection(const char *label, const std::vector<String> &options, int defaultIndex) {
  NativeMenuItem item;
  item.type = MENU_ITEM_SELECTION;
  item.label = String(label);
  item.numberValue = 0;
  item.numberMin = 0;
  item.numberMax = 0;
  item.stringValue = "";
  item.maxLength = 0;
  item.options = options;
  if (defaultIndex < 0) {
    defaultIndex = 0;
  }
  if (!item.options.empty() && defaultIndex >= (int)item.options.size()) {
    defaultIndex = item.options.size() - 1;
  }
  item.selectionIndex = defaultIndex;
  item.boolValue = false;
  item.trueText = "";
  item.falseText = "";
  item.callbackFunc = nullptr;
  items.push_back(item);
}

void NativeMenu::addBool(const char *label, bool defaultValue, const char *trueTextValue, const char *falseTextValue) {
  NativeMenuItem item;
  item.type = MENU_ITEM_BOOL;
  item.label = String(label);
  item.numberValue = 0;
  item.numberMin = 0;
  item.numberMax = 0;
  item.stringValue = "";
  item.maxLength = 0;
  item.options.clear();
  item.selectionIndex = 0;
  item.boolValue = defaultValue;
  item.trueText = String(trueTextValue);
  item.falseText = String(falseTextValue);
  item.callbackFunc = nullptr;
  items.push_back(item);
}

void NativeMenu::ensureSelectionVisible() {
  if (items.empty()) {
    selectedIndex = 0;
    scrollOffset = 0;
    return;
  }
  if (selectedIndex < 0) {
    selectedIndex = 0;
  }
  if (selectedIndex >= (int)items.size()) {
    selectedIndex = items.size() - 1;
  }
  if (selectedIndex < scrollOffset) {
    scrollOffset = selectedIndex;
  }
  if (selectedIndex >= scrollOffset + 4) {
    scrollOffset = selectedIndex - 3;
  }
  if (scrollOffset < 0) {
    scrollOffset = 0;
  }
}

void NativeMenu::setItemCallback(int index, void *callbackFunc) {
  if (index < 0 || index >= (int)items.size()) {
    return;
  }
  items[index].callbackFunc = callbackFunc;
}

void NativeMenu::open() {
  if (active) {
    return;
  }
  active = true;
  needsInitialDraw = true;
}

void NativeMenu::close() {
  if (!active) {
    return;
  }
  active = false;
  int width = M5Cardputer.Display.width();
  int height = M5Cardputer.Display.height();
  M5Cardputer.Display.fillRect(0, 0, width, height, BLACK);
}

bool NativeMenu::isActive() const {
  return active;
}

void NativeMenu::drawItem(const NativeMenuItem &item, int index, int y, int itemHeight, bool selected) {
  int width = M5Cardputer.Display.width();
  uint16_t bg = selected ? itemHighlightColor : itemColor;
  M5Cardputer.Display.fillRect(0, y, width, itemHeight, bg);
  M5Cardputer.Display.drawLine(0, y, width - 1, y, lineLightColor);
  M5Cardputer.Display.drawLine(0, y + itemHeight - 1, width - 1, y + itemHeight - 1, lineDarkColor);

  uint16_t textCol = selected ? textSelectedColor : textColor;
  M5Cardputer.Display.setTextColor(textCol);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setFont(&efontCN_16);

  int labelX = 8;
  int textY = y + itemHeight / 2 - 8;
  M5Cardputer.Display.drawString(item.label.c_str(), labelX, textY);

  String value;
  if (item.type == MENU_ITEM_BUTTON) {
    value = ">";
  } else if (item.type == MENU_ITEM_NUMBER) {
    if (editing && editingIndex == index && editingIsNumber) {
      value = editBuffer + "_";
    } else {
      value = String(item.numberValue);
    }
  } else if (item.type == MENU_ITEM_STRING) {
    if (editing && editingIndex == index && !editingIsNumber) {
      value = editBuffer + "_";
    } else {
      value = item.stringValue;
    }
  } else if (item.type == MENU_ITEM_SELECTION) {
    if (!item.options.empty() && item.selectionIndex >= 0 && item.selectionIndex < (int)item.options.size()) {
      value = item.options[item.selectionIndex];
    } else {
      value = "";
    }
  } else if (item.type == MENU_ITEM_BOOL) {
    value = item.boolValue ? item.trueText : item.falseText;
  }

  if (value.length() > 0) {
    int valueWidth = M5Cardputer.Display.textWidth(value.c_str());
    int valueX = width - 8 - valueWidth;
    if (valueX < labelX + 8) {
      valueX = labelX + 8;
    }
    M5Cardputer.Display.drawString(value.c_str(), valueX, textY);
  }
}

void NativeMenu::draw() {
  int width = M5Cardputer.Display.width();
  int height = M5Cardputer.Display.height();
  M5Cardputer.Display.fillRect(0, 0, width, height, bgColor);

  int visibleCount = 4;
  int itemHeight = height / visibleCount;
  int y = 0;
  for (int i = 0; i < visibleCount; ++i) {
    int itemIndex = scrollOffset + i;
    if (itemIndex >= (int)items.size()) {
      M5Cardputer.Display.fillRect(0, y, width, itemHeight, bgColor);
      y += itemHeight;
      continue;
    }
    bool selected = (itemIndex == selectedIndex);
    drawItem(items[itemIndex], itemIndex, y, itemHeight, selected);
    y += itemHeight;
  }
}

void NativeMenu::refresh() {
  ensureSelectionVisible();
  draw();
  needsInitialDraw = false;
}

void NativeMenu::update(duk_context *ctx) {
  if (items.empty()) {
    return;
  }

  if (!active) {
    return;
  }

  if (needsInitialDraw) {
    refresh();
    return;
  }

  bool changed = M5Cardputer.Keyboard.isChange();
  bool pressed = M5Cardputer.Keyboard.isPressed();
  if (!(changed && pressed)) {
    return;
  }

  Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
  bool didChange = false;

  if (editing) {
    for (auto c : status.word) {
      if (!c) {
        continue;
      }
      if (editingIsNumber) {
        if ((c >= '0' && c <= '9') || c == '-' || c == '+') {
          editBuffer += c;
          didChange = true;
        }
      } else {
        if (c >= 32 && c <= 126 && editBuffer.length() < items[editingIndex].maxLength) {
          editBuffer += c;
          didChange = true;
        }
      }
    }
    if (status.del && editBuffer.length() > 0) {
      editBuffer.remove(editBuffer.length() - 1);
      didChange = true;
    }
    if (status.enter) {
      if (editingIndex >= 0 && editingIndex < (int)items.size()) {
        NativeMenuItem &item = items[editingIndex];
        if (editingIsNumber) {
          int v = item.numberValue;
          if (editBuffer.length() > 0) {
            v = editBuffer.toInt();
          }
          if (v < item.numberMin) {
            v = item.numberMin;
          }
          if (v > item.numberMax) {
            v = item.numberMax;
          }
          if (v != item.numberValue) {
            item.numberValue = v;
            if (item.callbackFunc) {
              duk_push_heapptr(ctx, item.callbackFunc);
              duk_push_int(ctx, item.numberValue);
              if (duk_pcall(ctx, 1) != 0) {
                duk_pop(ctx);
              } else {
                duk_pop(ctx);
              }
            }
          }
        } else {
          if (item.stringValue != editBuffer) {
            item.stringValue = editBuffer;
            if (item.callbackFunc) {
              duk_push_heapptr(ctx, item.callbackFunc);
              duk_push_string(ctx, item.stringValue.c_str());
              if (duk_pcall(ctx, 1) != 0) {
                duk_pop(ctx);
              } else {
                duk_pop(ctx);
              }
            }
          }
        }
      }
      editBuffer = "";
      editing = false;
      editingIndex = -1;
      editingIsNumber = false;
      didChange = true;
    }

    if (didChange) {
      ensureSelectionVisible();
      draw();
    }
    return;
  }

  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  for (auto c : status.word) {
    if (!c) {
      continue;
    }
    if (c == ';') {
      up = true;
    }
    if (c == '.') {
      down = true;
    }
    if (c == ',') {
      left = true;
    }
    if (c == '/') {
      right = true;
    }
  }
  bool enter = status.enter;

  if (up) {
    if (selectedIndex > 0) {
      selectedIndex--;
      didChange = true;
    }
  }
  if (down) {
    if (selectedIndex < (int)items.size() - 1) {
      selectedIndex++;
      didChange = true;
    }
  }

  ensureSelectionVisible();

  if (selectedIndex >= 0 && selectedIndex < (int)items.size()) {
    NativeMenuItem &item = items[selectedIndex];
    if (item.type == MENU_ITEM_BUTTON) {
      if (enter && item.callbackFunc) {
        duk_push_heapptr(ctx, item.callbackFunc);
        if (duk_pcall(ctx, 0) != 0) {
          duk_pop(ctx);
        } else {
          duk_pop(ctx);
        }
      }
    } else if (item.type == MENU_ITEM_NUMBER) {
      if (enter) {
        editing = true;
        editingIndex = selectedIndex;
        editingIsNumber = true;
        editBuffer = String(item.numberValue);
        didChange = true;
      } else if (left || right) {
        int delta = right ? 1 : -1;
        int v = item.numberValue + delta;
        if (v < item.numberMin) {
          v = item.numberMin;
        }
        if (v > item.numberMax) {
          v = item.numberMax;
        }
        if (v != item.numberValue) {
          item.numberValue = v;
          didChange = true;
          if (item.callbackFunc) {
            duk_push_heapptr(ctx, item.callbackFunc);
            duk_push_int(ctx, item.numberValue);
            if (duk_pcall(ctx, 1) != 0) {
              duk_pop(ctx);
            } else {
              duk_pop(ctx);
            }
          }
        }
      }
    } else if (item.type == MENU_ITEM_STRING) {
      if (enter) {
        editing = true;
        editingIndex = selectedIndex;
        editingIsNumber = false;
        editBuffer = item.stringValue;
        didChange = true;
      }
    } else if (item.type == MENU_ITEM_SELECTION) {
      if ((left || right) && !item.options.empty()) {
        int n = item.options.size();
        int delta = right ? 1 : -1;
        int idx = item.selectionIndex + delta;
        if (idx < 0) {
          idx = n - 1;
        }
        if (idx >= n) {
          idx = 0;
        }
        if (idx != item.selectionIndex) {
          item.selectionIndex = idx;
          didChange = true;
          if (item.callbackFunc && !item.options.empty() &&
              item.selectionIndex >= 0 &&
              item.selectionIndex < (int)item.options.size()) {
            duk_push_heapptr(ctx, item.callbackFunc);
            duk_push_int(ctx, item.selectionIndex);
            duk_push_string(ctx, item.options[item.selectionIndex].c_str());
            if (duk_pcall(ctx, 2) != 0) {
              duk_pop(ctx);
            } else {
              duk_pop(ctx);
            }
          }
        }
      }
    } else if (item.type == MENU_ITEM_BOOL) {
      if (left || right || enter) {
        item.boolValue = !item.boolValue;
        didChange = true;
        if (item.callbackFunc) {
          duk_push_heapptr(ctx, item.callbackFunc);
          duk_push_boolean(ctx, item.boolValue);
          if (duk_pcall(ctx, 1) != 0) {
            duk_pop(ctx);
          } else {
            duk_pop(ctx);
          }
        }
      }
    }
  }

  if (didChange) {
    ensureSelectionVisible();
    draw();
  }
}
