#include "menuitem.h"

/* 
  To build a menu-tree, follow these steps:

  STEP 1: Define a type with the functions that perform actions you would like
          to be available from the menu:

    Example:

    struct Actions {
      void setEchoEnabled(bool val);
      ...
    };

  STEP 2: Set this type as the MenuAction type using the MenuAction macro

    Example:
    
    MenuAction(Actions);

  STEP 3: Define all the leaves (endpoints) and submenu's:

    MenuLeaf(NAME, LABEL, RETURN, SELECT_CODE);
  
    * NAME:        The MenuLeaf macro defines a new type with a name equal to NAME. 
    * LABEL:       The label that will be displayed on-screen (string).
    * RETURN:      A pointer to the submenu that will be returned to after select.
                   This can be one of:
                     * item.home() to return to the root menu
                     * item.stay() to stay within the same menu
                     * item.back() to go to the previous menu
                     * item.exit() to exit from the menu
    * SELECT_CODE: A code-block acting on "actions". In this block the actions as
                   defined in the MenuAction type are available from the actions-object.

    SubMenu(NAME, LABEL, IS_ROOT, SELECT_CODE);

      * NAME:        The SubMenu macro defines a new type with a name equal to NAME.
      * LABEL:       The label that will be displayed on-screen.
      * IS_ROOT:     true if this is the root-menu, false otherwise.
      * SELECT_CODE: A code-block that is executed when entering this menu. Like 
                     with the MenuLeaf code-block, this code acts on the action-object.

    Example:

    SubMenu(Main, "Main Menu", true,  {}); // No action on select.
    SubMenu(Echo, "Echo",      false, {}); // No action on select.

    MenuLeaf(EchoOn, "On",   item.home(), { action.setEchoEnabled(true); })
    MenuLeaf(EchoOn, "Off",  item.home(), { action.setEchoEnabled(true); })
    MenuLeaf(Exit,   "Exit", item.exit(), {}); // no code executed  

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

  STEP 3: Instantiate the menu and call begin().
          This will set the root node such that submenu's know where to go when home() is called.

    Example:

    Menu menu;
    Menu::BasePtr current = menu.begin();
*/

#define MenuItem_(NAME, LABEL, RETURN, SELECT_CODE)                               \
struct NAME##_ {                                                                  \
  static constexpr char const* getLabel() { return LABEL; }                       \
  template <typename Actions>                                                     \
  static MenuItem<Actions>*  select(MenuItem<Actions> &item, Actions &actions){   \
      (void)item; (void)actions;                                                  \
      SELECT_CODE                                                                 \
      return RETURN;                                                              \
  }                                                                               \
};                                                                          

#define MenuActions(ACTIONS) using Actions_ = ACTIONS;         

#define MenuLeaf(NAME, LABEL, RETURN, SELECT_CODE) MenuItem_(NAME, LABEL, RETURN, SELECT_CODE); \
using NAME = Helpers::MenuItemImpl<Actions_, NAME##_>;

#define SubMenu(NAME, LABEL, SELECT_CODE) MenuItem_(NAME, LABEL, &item, SELECT_CODE); \
template <typename First, typename ... Rest>                                          \
using NAME = Helpers::MenuItemImpl<Actions_, NAME##_, First, Rest...>;
