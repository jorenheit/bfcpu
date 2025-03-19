#ifndef PS2Keyboard_h
#define PS2Keyboard_h

// Every call to read() returns a single byte for each
// keystroke.  These configure what byte will be returned
// for each "special" key.  To ignore a key, use zero.

#define PS2_TAB          '\t'
#define PS2_ENTER        '\n'
#define PS2_BACKSPACE    0
#define PS2_ESC          0
#define PS2_INSERT       0
#define PS2_DELETE       0
#define PS2_HOME         0
#define PS2_END          0
#define PS2_PAGEUP       0
#define PS2_PAGEDOWN     0
#define PS2_UPARROW      0
#define PS2_LEFTARROW    0
#define PS2_DOWNARROW    0
#define PS2_RIGHTARROW   0
#define PS2_F1           0
#define PS2_F2           0
#define PS2_F3           0
#define PS2_F4           0
#define PS2_F5           0
#define PS2_F6           0
#define PS2_F7           0
#define PS2_F8           0
#define PS2_F9           0
#define PS2_F10          0
#define PS2_F11          0
#define PS2_F12          0
#define PS2_SCROLL       0
#define PS2_EURO_SIGN    0

class PS2Keyboard {
public:
  static void begin();
  static bool available();
  static void clear();
  static uint8_t read();
};

#endif