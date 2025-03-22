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
    LCDBuffer &_buffer;
  public:
    Actions(Settings &s, LCDBuffer &buf):
      _settings(s),
      _buffer(buf)
    {}

    inline void clear()                              { _buffer.clear();}
    inline void setEchoEnabled(bool const val)       { _settings.echoEnabled = val; }
    inline void setAutoscrollEnabled(bool const val) { _settings.autoscrollEnabled = val; }
    inline void setMode(::DisplayMode const mode)    { _settings.mode = mode; }
    inline void setDelimiter(char const delim)       { _settings.delimiter = delim; }
    inline void restoreDefaults()                    { _settings = {}; }
  };

  MenuActions(Actions);

  // Define submenu nodes
  SubMenu(MainMenu,    "Main Menu",     { /* No action on select */    });
  SubMenu(Echo,        "Echo",          { /* No action on select */    });
  SubMenu(Autoscroll,  "Autoscroll",    { /* No action on select */    });
  SubMenu(DisplayMode, "Display Mode",  { /* No action on select */    });
  SubMenu(DecMode,     "Decimal",       { actions.setMode(DECIMAL);    });
  SubMenu(HexMode,     "Hexadecimal",   { actions.setMode(HEXADECIMAL);});

  // Define leaf nodes
  MenuLeaf(Clear,         "Clear",       item.exit(),   { actions.clear();                     });
  MenuLeaf(EchoOn,        "On",          item.home(),   { actions.setEchoEnabled(true);        });
  MenuLeaf(EchoOff,       "Off",         item.home(),   { actions.setEchoEnabled(false);       });
  MenuLeaf(AutoscrollOn,  "On",          item.home(),   { actions.setAutoscrollEnabled(true);  });
  MenuLeaf(AutoscrollOff, "Off",         item.home(),   { actions.setAutoscrollEnabled(false); });
  MenuLeaf(TextMode,      "Text",        item.home(),   { actions.setMode(ASCII);              });
  MenuLeaf(CommaDelim,    ",",           item.home(),   { actions.setDelimiter(',');           });
  MenuLeaf(SemiDelim,     ";",           item.home(),   { actions.setDelimiter(';');           });
  MenuLeaf(BarDelim,      "|",           item.home(),   { actions.setDelimiter('|');           });
  MenuLeaf(SpaceDelim,    "[SPACE]",     item.home(),   { actions.setDelimiter(' ');           });
  MenuLeaf(Defaults,      "Defaults",    item.exit(),   { actions.restoreDefaults();           });
  MenuLeaf(Exit,          "Exit",        item.exit(),   { /* No action on select */            });

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

  Settings &_settings;
  LCDBuffer &_buffer;
  LCDScreen &_screen;
  Actions _actions;
  unsigned long _lastActiveTime = 0;

public:
  LCDMenu(Settings &s, LCDBuffer &buf, LCDScreen &scr);
  void enter();
  bool active() const;
  void handleButtons(ButtonState const up, ButtonState const down, ButtonState const both);
  void display();
  void saveSettings();
  void loadSettings();
  void exit();
};