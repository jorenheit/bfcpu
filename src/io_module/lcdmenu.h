#pragma once

#include "button.h"
#include "lcdbuffer.h"
#include "lcdscreen.h"
#include "buildmenu.h"

class LCDMenu {
  // Define leaf nodes
  MenuLeaf(Clear,         "Clear",       self->exit(),   { buffer.clear(); });
  MenuLeaf(EchoOn,        "On",          self->home(),   { buffer.setEchoEnabled(true); });
  MenuLeaf(EchoOff,       "Off",         self->home(),   { buffer.setEchoEnabled(false); });
  MenuLeaf(AutoscrollOn,  "On",          self->home(),   { buffer.setAutoscrollEnabled(true); });
  MenuLeaf(AutoscrollOff, "Off",         self->home(),   { buffer.setAutoscrollEnabled(false); });
  MenuLeaf(TextMode,      "Text",        self->home(),   { buffer.setMode(ASCII); });
  MenuLeaf(BarDelim,      "|",           self->home(),   { buffer.setDelimiter('|'); });
  MenuLeaf(CommaDelim,    ",",           self->home(),   { buffer.setDelimiter(','); });
  MenuLeaf(SemiDelim,     ";",           self->home(),   { buffer.setDelimiter(';'); });
  MenuLeaf(Exit,          "Exit",        self->exit(),   { /* No action on select */ });

  // Define submenu nodes
  SubMenu(MainMenu,    "Main Menu",     5, true,  { /* No action on select */    });  // will be the root node
  SubMenu(Echo,        "Echo",          2, false, { /* No action on select */    });
  SubMenu(Autoscroll,  "Autoscroll",    2, false, { /* No action on select */    });
  SubMenu(DisplayMode, "Display Mode",  3, false, { /* No action on select */    });
  SubMenu(DecMode,     "Decimal",       3, false, { buffer.setMode(DECIMAL);     });
  SubMenu(HexMode,     "Hexadecimal",   3, false, { buffer.setMode(HEXADECIMAL); });

  // Build the final menu:
  using Menu = MainMenu <
      Clear,
      Echo<
        EchoOff,
        EchoOn
      >,
      Autoscroll<
        AutoscrollOn,
        AutoscrollOff
      >,
      DisplayMode<
        TextMode,
        DecMode <
          BarDelim,
          CommaDelim,
          SemiDelim
        >,
        HexMode<
          BarDelim,
          CommaDelim,
          SemiDelim
        >
      >,
      Exit
    >;

  Menu menu;
  LCDBuffer &_buffer;
  LCDScreen &_screen;
  MenuItem *_current;
  unsigned long _lastActiveTime = 0;

public:
  LCDMenu(LCDBuffer &buf, LCDScreen &scr);
  void enter();
  bool active() const;
  void handleButtons(ButtonState const up, ButtonState const down, ButtonState const both);
  void display();
};