#pragma once 

namespace Helpers {
  template <typename, typename ...>
  struct MenuItemImpl;
}

class MenuItem {
  template <typename Impl, typename ... Children>
  friend struct Helpers::MenuItemImpl;

  MenuItem *root = 0;
  MenuItem *parent = 0;
  uint8_t parentPos = 0;

public:
  virtual char const *getLabel() = 0;
  virtual MenuItem *select(LCDBuffer &) = 0;
  virtual MenuItem *highlighted() { return this; }
  virtual void up() {}
  virtual void down() {}
  
  virtual MenuItem *back() { return parent ? parent : this; } // should be overridden by leaf-types
  MenuItem *stay() { return parent; };
  MenuItem *home() { return root; };
  MenuItem *exit() { return nullptr; };
  
  char const *getNumberedLabel() {
    static char buffer[LINE_SIZE + 1]; // shared buffer between all items, filled on demand
    buffer[0] = '0' + parentPos;
    buffer[1] = '.';
    buffer[2] = ' ';

    for (uint8_t idx = 3; idx != LINE_SIZE + 1; ++idx) {
      char const c = getLabel()[idx - 3];
      if (c) buffer[idx] = c;
      else {
        buffer[idx] = 0;
        break;
      };
    }
    //snprintf(buffer, LINE_SIZE + 1, "%d. %s", parentPos, getLabel()); 
    return buffer;
  }

  MenuItem *getRoot() {
    return root;
  }

  MenuItem *getParent() {
    return parent;
  }
  
private:
  void setParent(MenuItem *par, uint8_t const pos) {
    parent = par;
    parentPos = pos;
  }

  void setRoot(MenuItem *root_) {
    root = root_;
    this->setRootForChildren(root);
  }

  virtual void setRootForChildren(MenuItem*) {};
};


namespace Helpers {
  template <typename... Args>
  struct ChildTuple;

  template <>
  struct ChildTuple<> {
    void storePointers(MenuItem **) {}
  };

  template <typename First, typename... Rest>
  class ChildTuple<First, Rest...>: public ChildTuple<Rest...> {
    First value;

  public:
    void storePointers(MenuItem *dest[]) {
      dest[0] = value.getPointer();
      ChildTuple<Rest ...>::storePointers(dest + 1);
    }
  };

  template <typename T, typename = void>
  struct IsRoot {
    static constexpr bool value = false;
  };

  template <typename T>
  struct IsRoot<T, decltype((void)T::is_root, void())> {
    static constexpr bool value = true;
  };

  template <bool Root = false>
  struct SubMenu_ {};

  template <>
  struct SubMenu_<true>: SubMenu_<false> {
    static constexpr bool is_root = true; 
  };


  template <typename Impl>
  struct MenuItemImplBase: public MenuItem
  {
    MenuItem *getPointer() {
      return this;
    }

    virtual char const *getLabel() override final {
      return Impl::getLabel();
    };

    virtual MenuItem *select(LCDBuffer &buffer) override final {
      return Impl::select(this, buffer);
    };
  };

  template <typename Impl, typename ... Children> 
  class MenuItemImpl: public MenuItemImplBase<Impl> {
    static constexpr uint8_t numChildren = (sizeof ... (Children));
    ChildTuple<Children ...> children;
    MenuItem *childPointers[numChildren];
    uint8_t highlightedIndex = 0;

  public:
    MenuItemImpl() {
      children.storePointers(childPointers);
      for (uint8_t idx = 0; idx != numChildren; ++idx) {
        childPointers[idx]->setParent(this, idx + 1);
      }
      if (IsRoot<Impl>::value) this->setRoot(this);
    }

    virtual MenuItem *highlighted() override final {
      return childPointers[highlightedIndex];
    }

    virtual void up() override final {
      highlightedIndex = (highlightedIndex + numChildren - 1) % numChildren;
    }

    virtual void down() override final {
      highlightedIndex = (highlightedIndex + 1) % numChildren;
    }

  private:
    virtual void setRootForChildren(MenuItem *root) {
      for (uint8_t idx = 0; idx != numChildren; ++idx) {
        childPointers[idx]->setRoot(root);
      }
    }
  };

  template <typename Impl> 
  struct MenuItemImpl<Impl>: public MenuItemImplBase<Impl> {
    static constexpr uint8_t numChildren = 0;

  public:
    MenuItemImpl() {
      if (IsRoot<Impl>::value) 
        MenuItemImplBase<Impl>::setRoot(this);
    }

    virtual MenuItem *back() override final {
      return this->getParent()->getParent();
    }
  };
}
