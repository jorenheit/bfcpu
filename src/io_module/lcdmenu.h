#pragma once

#include "settings.h"
#include "button.h"
#include "lcdbuffer.h"
#include "lcdscreen.h"
#include "buildmenu.h"

class LCDMenu {

  // Actions contains the functions that are available from within the menu-nodes.
  class Actions {
    LCDBuffer &_buffer;
  public:
    Actions(LCDBuffer &buffer):
      _buffer(buffer)
    {}

    inline void clear()                              { _buffer.clear();}
    inline void setEchoEnabled(bool const val)       { _buffer.setEchoEnabled(val); }
    inline void setAutoscrollEnabled(bool const val) { _buffer.setAutoscrollEnabled(val); }
    inline void setMode(::DisplayMode const mode)    { _buffer.setMode(mode); }
    inline void setDelimiter(char const delim)       { _buffer.setDelimiter(delim); }
    inline void restoreDefaults()                    { _buffer.setSettings({}); }
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
  MenuLeaf(BarDelim,      "|",           item.home(),   { actions.setDelimiter('|');           });
  MenuLeaf(CommaDelim,    ",",           item.home(),   { actions.setDelimiter(',');           });
  MenuLeaf(SemiDelim,     ";",           item.home(),   { actions.setDelimiter(';');           });
  MenuLeaf(Defaults,      "Defaults",    item.exit(),   { actions.restoreDefaults();           })
  MenuLeaf(Exit,          "Exit",        item.exit(),   { /* No action on select */            });

  // Build the final menu-type:
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
    Defaults,
    Exit
  >;

  Menu _menu;
  Menu::BasePtr _current = nullptr;

  LCDBuffer &_buffer;
  LCDScreen &_screen;
  Actions _actions;
  unsigned long _lastActiveTime = 0;

public:
  LCDMenu(LCDBuffer &buf, LCDScreen &scr);
  void enter();
  bool active() const;
  void handleButtons(ButtonState const up, ButtonState const down, ButtonState const both);
  void display();
  void saveSettings();
  void loadSettings();
  void exit();
};