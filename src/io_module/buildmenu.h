#include "menuitem.h"

/* 
  To build a menu-tree, follow these steps:

  STEP 1: Define all the leaves (endpoints) and submenu's:

    MenuLeaf(NAME, LABEL, RETURN, SELECT_CODE);
  
    * NAME:        The MenuLeaf macro defines a new type with a name equal to NAME. 
    * LABEL:       The label that will be displayed on-screen (string).
    * RETURN:      A pointer to the submenu that will be returned to after select.
                   This can be one of:
                     * self->home() to return to the root menu
                     * self->stay() to stay within the same menu
                     * self->back() to go to the previous menu
                     * self->exit() to exit from the menu
    * SELECT_CODE: A code-block acting on "buffer" (type LCDBuffer). In this block
                   the corresponding settings are to be set to the LCDBuffer object.


    SubMenu(NAME, LABEL, NUM_OPTS, IS_ROOT, SELECT_CODE);

      * NAME:        The SubMenu macro defines a new type with a name equal to NAME.
      * LABEL:       The label that will be displayed on-screen.
      * NUM_OPTS:    The number of options of the submenu (<= 9).
      * IS_ROOT:     true if this is the root-menu, false otherwise.
      * SELECT_CODE: A code-block that is executed when entering this menu. Like 
                     with the MenuLeaf code-block, this code should act on the 
                     "buffer" object.

    Example:

    SubMenu(Main, "Main Menu", 2, true,  {}); // No action on select.
    SubMenu(Echo, "Echo",      2, false, {}); // No action on select.

    MenuLeaf(EchoOn, "On",   self->home(), { buffer.setEchoEnabled(true); })
    MenuLeaf(EchoOn, "Off",  self->home(), { buffer.setEchoEnabled(true); })
    MenuLeaf(Exit,   "Exit", self->exit(), {}); // no code executed  

  STEP 2: Build the tree by creating a typedef with the "using" directive.
          Make sure that the root-menu is actually at the root of the definition.

    Example:

    using Menu = 
      Main<
        Echo <
          EchoOn,
          EchoOff
        >,
        Exit
      >;  
*/

#define TYPENAME_LIST_1  typename Opt1
#define TYPENAME_LIST_2  typename Opt1, typename Opt2
#define TYPENAME_LIST_3  typename Opt1, typename Opt2, typename Opt3
#define TYPENAME_LIST_4  typename Opt1, typename Opt2, typename Opt3, typename Opt4
#define TYPENAME_LIST_5  typename Opt1, typename Opt2, typename Opt3, typename Opt4, typename Opt5
#define TYPENAME_LIST_6  typename Opt1, typename Opt2, typename Opt3, typename Opt4, typename Opt5, typename Opt6
#define TYPENAME_LIST_7  typename Opt1, typename Opt2, typename Opt3, typename Opt4, typename Opt5, typename Opt6, typename Opt7
#define TYPENAME_LIST_8  typename Opt1, typename Opt2, typename Opt3, typename Opt4, typename Opt5, typename Opt6, typename Opt7, typename Opt8
#define TYPENAME_LIST_9  typename Opt1, typename Opt2, typename Opt3, typename Opt4, typename Opt5, typename Opt6, typename Opt7, typename Opt8, typename Opt9

#define OPT_LIST_1  Opt1
#define OPT_LIST_2  Opt1, Opt2
#define OPT_LIST_3  Opt1, Opt2, Opt3
#define OPT_LIST_4  Opt1, Opt2, Opt3, Opt4
#define OPT_LIST_5  Opt1, Opt2, Opt3, Opt4, Opt5
#define OPT_LIST_6  Opt1, Opt2, Opt3, Opt4, Opt5, Opt6
#define OPT_LIST_7  Opt1, Opt2, Opt3, Opt4, Opt5, Opt6, Opt7
#define OPT_LIST_8  Opt1, Opt2, Opt3, Opt4, Opt5, Opt6, Opt7, Opt8
#define OPT_LIST_9  Opt1, Opt2, Opt3, Opt4, Opt5, Opt6, Opt7, Opt8, Opt9

#define GET_TYPENAME_LIST(N) TYPENAME_LIST_##N
#define GET_OPT_LIST(N) OPT_LIST_##N

#define MenuLeaf(NAME, LABEL, RETURN, SELECT_CODE)                          \
struct NAME##_ {                                                            \
  static constexpr char const* getLabel() { return LABEL; }                 \
  static MenuItem* select(MenuItem*self, LCDBuffer &buffer) {               \
      (void)self; (void)buffer;                                             \
      SELECT_CODE                                                           \
      return RETURN;                                                        \
  }                                                                         \
};                                                                          \
using NAME = Helpers::MenuItemImpl<NAME##_>;

#define SubMenu(NAME, LABEL, NUM_OPTS, IS_ROOT, SELECT_CODE)                \
struct NAME##_ : Helpers::SubMenu_<IS_ROOT> {                               \
  static constexpr char const* getLabel() { return LABEL; }                 \
  static MenuItem* select(MenuItem*self, LCDBuffer &buffer) {               \
      (void)self; (void)buffer;                                             \
      SELECT_CODE                                                           \
      return self;                                                          \
  }                                                                         \
};                                                                          \
template <GET_TYPENAME_LIST(NUM_OPTS)>                                      \
using NAME = Helpers::MenuItemImpl<NAME##_, GET_OPT_LIST(NUM_OPTS)>;
