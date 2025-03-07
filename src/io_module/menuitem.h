#pragma once 

namespace Menu {

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
  using Pointer = MenuItem<Actions>*;

  MenuItem *begin() {
    setRoot(this);
    return this;
  }

  MenuItem *getRoot() {
    return root;
  }

  MenuItem *getParent() {
    return parent;
  }
  
  virtual char const *getLabel() const = 0;
  virtual MenuItem *select(Actions &) = 0;
  virtual MenuItem *highlighted() { return this; }
  virtual void up() {}
  virtual void down() {}

  virtual MenuItem *back() { return parent ? parent : this; } // should be overridden by leaf-types
  MenuItem *stay() { return parent; };
  MenuItem *home() { return root; };
  MenuItem *exit() { return nullptr; };
  
  char const *getNumberedLabel() const {
    static char buffer[LINE_SIZE + 1]; // shared buffer between all items, filled on demand
    buffer[0] = '0' + parentPos;
    buffer[1] = '.';
    buffer[2] = ' ';
    buffer[LINE_SIZE] = '\0';
    
    for (uint8_t idx = 3; idx != LINE_SIZE; ++idx) {
      char const c = getLabel()[idx - 3];
      buffer[idx] = c;
      if (!c) break;
    }
    return buffer;
  }

protected:
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
    template <int Index, typename T, int N>
    void storePointers(T (&)[N]) {
      static_assert(Index == N, "Destination array size does not match the number of children");
    }
  };

  template <typename First, typename... Rest>
  class ChildTuple<First, Rest...>: public ChildTuple<Rest...> {
    template <typename ...> friend struct ChildTuple;

    using Pointer = typename First::Pointer;
    First child;

    template <int Index, int N>
    void storePointers(Pointer (&dest)[N]) {
      static_assert(Index + sizeof...(Rest) < N, "Destination array not large enough for all children");

      dest[Index] = &child;
      ChildTuple<Rest...>::template storePointers<Index + 1>(dest);
    }

  public:
    template <int N>
    void storePointers(Pointer (&dest)[N]) {
      storePointers<0>(dest);
    }
  };

  template <typename Actions, typename Impl>
  struct MenuItemImplBase: public MenuItem<Actions>
  {
    using Pointer = typename MenuItem<Actions>::Pointer;

    virtual char const *getLabel() const override final {
      return Impl::getLabel();
    };

    virtual Pointer select(Actions &actions) override final {
      return Impl::select(*this, actions);
    };
  };

  template <typename Actions, typename Impl, typename ... Children> 
  class MenuItemImpl: public MenuItemImplBase<Actions, Impl> {
  public:
    using Pointer = typename MenuItemImplBase<Actions, Impl>::Pointer;
  
  private:
    static constexpr uint8_t numChildren = (sizeof ... (Children));
    ChildTuple<Children ...> children;
    Pointer childPointers[numChildren];
    uint8_t highlightedIndex = 0;

  public:
    MenuItemImpl() {
      children.storePointers(childPointers);
      for (uint8_t idx = 0; idx != numChildren; ++idx) {
        childPointers[idx]->setParent(this, idx + 1);
      }
    }

    virtual Pointer highlighted() override final {
      return childPointers[highlightedIndex];
    }

    virtual void up() override final {
      highlightedIndex = (highlightedIndex + numChildren - 1) % numChildren;
    }

    virtual void down() override final {
      highlightedIndex = (highlightedIndex + 1) % numChildren;
    }

  private:
    virtual void setRootForChildren(Pointer root) override final {
      for (uint8_t idx = 0; idx != numChildren; ++idx) {
        childPointers[idx]->setRoot(root);
      }
    }
  };

  template <typename Actions, typename Impl> 
  class MenuItemImpl<Actions, Impl>: public MenuItemImplBase<Actions, Impl> {
    static constexpr uint8_t numChildren = 0;

  public:
    using Pointer = typename MenuItemImplBase<Actions, Impl>::Pointer;

    virtual Pointer back() override final {
      return this->getParent()->getParent();
    }
  };
}

}