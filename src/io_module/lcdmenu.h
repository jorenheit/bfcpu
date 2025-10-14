#pragma once

#include "settings.h"
#include "button.h"
#include "lcdbuffer.h"
#include "lcdscreen.h"
#include "menu.h"

class LCDMenu {

  // Actions contains the functions that are available from within the menu-nodes.
  class Actions {
    Settings &_settings;
    LCDBuffer &_lcdBuf;
    KeyboardBuffer &_kbBuf;
  public:
    Actions(Settings &s, LCDBuffer &buf1, KeyboardBuffer &buf2):
      _settings(s),
      _lcdBuf(buf1),
      _kbBuf(buf2)
    {}

    inline void clear()                              { _lcdBuf.clear(); _kbBuf.clear(); }
    inline void setEchoEnabled(bool const val)       { _settings.echoEnabled = val; }
    inline void setAutoscrollEnabled(bool const val) { _settings.autoscrollEnabled = val; }
    inline void setMode(::DisplayMode const mode)    { _settings.mode = mode; }
    inline void setDelimiter(char const delim)       { _settings.delimiter = delim; }
    inline void restoreDefaults()                    { _settings = {}; }
  };

  MenuActions(Actions);

  // Define submenu nodes
  SubMenu(MainMenu,    "Settings",      { /* No action on select */    });
  SubMenu(Echo,        "Echo",          { /* No action on select */    });
  SubMenu(Autoscroll,  "Autoscroll",    { /* No action on select */    });
  SubMenu(DisplayMode, "Display Mode",  { /* No action on select */    });
  SubMenu(DecMode,     "Decimal",       { actions.setMode(DECIMAL);    });
  SubMenu(HexMode,     "Hexadecimal",   { actions.setMode(HEXADECIMAL);});
  SubMenu(SetRNGSeed,  "Set RNG Seed",  { /* No action on select */    });

  // Define leaf nodes
  MenuLeaf(Clear,         "Clear Buffers", item.exit(),   { actions.clear();                     });
  MenuLeaf(EchoOn,        "On",            item.home(),   { actions.setEchoEnabled(true);        });
  MenuLeaf(EchoOff,       "Off",           item.home(),   { actions.setEchoEnabled(false);       });
  MenuLeaf(AutoscrollOn,  "On",            item.home(),   { actions.setAutoscrollEnabled(true);  });
  MenuLeaf(AutoscrollOff, "Off",           item.home(),   { actions.setAutoscrollEnabled(false); });
  MenuLeaf(TextMode,      "Text",          item.home(),   { actions.setMode(ASCII);              });
  MenuLeaf(CommaDelim,    ",",             item.home(),   { actions.setDelimiter(',');           });
  MenuLeaf(SemiDelim,     ";",             item.home(),   { actions.setDelimiter(';');           });
  MenuLeaf(BarDelim,      "|",             item.home(),   { actions.setDelimiter('|');           });
  MenuLeaf(SpaceDelim,    "[SPACE]",       item.home(),   { actions.setDelimiter(' ');           });
  MenuLeaf(Defaults,      "Defaults",      item.exit(),   { actions.restoreDefaults();           });
  MenuLeaf(Exit,          "Exit",          item.exit(),   { /* No action on select */            });
  
  // Value-select menu for the RNG seed
  ValueSelect(SeedSelecter, rngSeedPtr, 1, 100);  

  // Build the final menu-type:
  using Menu = MainMenu <
    Clear,
    Echo<
      EchoOn,
      EchoOff
    >,
    Autoscroll<
      AutoscrollOn,
      AutoscrollOff
    >,
    SetRNGSeed<
      SeedSelecter
    >,
    DisplayMode<
      TextMode,
      DecMode <
        CommaDelim,
        SemiDelim,
        BarDelim,
        SpaceDelim
      >,
      HexMode<
        CommaDelim,
        SemiDelim,
        BarDelim,
        SpaceDelim
      >
    >,
    Defaults,
    Exit
  >;

  Menu _menu;
  Menu::Pointer _current = nullptr;
  uint8_t _selectedLine = 1;

  Settings &_settings;
  LCDScreen &_screen;
  Actions _actions;
  unsigned long _lastActiveTime = 0;

public:
  LCDMenu(LCDScreen &scr, Settings &s, LCDBuffer &lcdBuf, KeyboardBuffer &kbBuf);
  void enter();
  bool active() const;
  void handleButtons(ButtonState const up, ButtonState const down, ButtonState const both);
  void display();
  void saveSettings();
  void loadSettings();
  void exit();
};