#pragma once 

namespace Helpers {
  template <typename, typename, typename ...>
  struct MenuItemImpl;
}

template <typename Actions>
class MenuItem {
  template <typename, typename, typename ...>
  friend struct Helpers::MenuItemImpl;

  MenuItem *root = 0;
  MenuItem *parent = 0;
  uint8_t parentPos = 0;

public:
  using BasePtr = MenuItem<Actions>*;

  virtual char const *getLabel() = 0;
  virtual MenuItem *select(Actions &) = 0;
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
    template <typename T>
    void storePointers(T **) {}
  };

  template <typename First, typename... Rest>
  class ChildTuple<First, Rest...>: public ChildTuple<Rest...> {
    using BasePtr = typename First::BasePtr;
    First value;

  public:
    void storePointers(BasePtr dest[]) {
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


  template <typename Actions, typename Impl>
  struct MenuItemImplBase: public MenuItem<Actions>
  {
    using BasePtr = typename MenuItem<Actions>::BasePtr;

    BasePtr getPointer() {
      return this;
    }

    virtual char const *getLabel() override final {
      return Impl::getLabel();
    };

    virtual BasePtr select(Actions &actions) override final {
      return Impl::select(*this, actions);
    };
  };

  template <typename Actions, typename Impl, typename ... Children> 
  class MenuItemImpl: public MenuItemImplBase<Actions, Impl> {
  public:
    using BasePtr = typename MenuItemImplBase<Actions, Impl>::BasePtr;
  
  private:
    static constexpr uint8_t numChildren = (sizeof ... (Children));
    ChildTuple<Children ...> children;
    BasePtr childPointers[numChildren];
    uint8_t highlightedIndex = 0;

  public:
    MenuItemImpl() {
      children.storePointers(childPointers);
      for (uint8_t idx = 0; idx != numChildren; ++idx) {
        childPointers[idx]->setParent(this, idx + 1);
      }
      if (IsRoot<Impl>::value) this->setRoot(this);
    }

    virtual BasePtr highlighted() override final {
      return childPointers[highlightedIndex];
    }

    virtual void up() override final {
      highlightedIndex = (highlightedIndex + numChildren - 1) % numChildren;
    }

    virtual void down() override final {
      highlightedIndex = (highlightedIndex + 1) % numChildren;
    }

  private:
    virtual void setRootForChildren(BasePtr root) {
      for (uint8_t idx = 0; idx != numChildren; ++idx) {
        childPointers[idx]->setRoot(root);
      }
    }
  };

  template <typename Actions, typename Impl> 
  class MenuItemImpl<Actions, Impl>: public MenuItemImplBase<Actions, Impl> {
    static constexpr uint8_t numChildren = 0;

  public:
    using BasePtr = typename MenuItemImplBase<Actions, Impl>::BasePtr;
    MenuItemImpl() {
      if (IsRoot<Impl>::value) this->setRoot(this);
    }

    virtual BasePtr back() override final {
      return this->getParent()->getParent();
    }
  };
}
