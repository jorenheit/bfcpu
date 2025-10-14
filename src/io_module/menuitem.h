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
  virtual MenuItem *highlighted(int8_t const = 0) { return this; }
  virtual uint8_t count() const { return 0; }
  virtual bool up()   { return false; }
  virtual bool down() { return false; }
  virtual bool isValueSelect() const { return false; }


  virtual MenuItem *back() { return parent ? parent : this; } // should be overridden by leaf-types
  MenuItem *stay() { return parent; };
  MenuItem *home() { return root; };
  MenuItem *exit() { return nullptr; };

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

    virtual char const *getLabel() const override {
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

    virtual Pointer highlighted(int8_t const offset) override final {
      int8_t const index = highlightedIndex + offset;
      return (index < 0 || index >= numChildren) ? nullptr : childPointers[index]; 
    }

    virtual bool up() override final {
      highlightedIndex = (highlightedIndex + numChildren - 1) % numChildren;
      return highlightedIndex == (numChildren - 1);
    }

    virtual bool down() override final {
      highlightedIndex = (highlightedIndex + 1) % numChildren;
      return (highlightedIndex == 0);
    }

    virtual uint8_t count() const override final {
      return numChildren;
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


  template <typename Actions, typename Impl, int Min, int Max> 
  class MenuItemImplValueSelect: public MenuItemImplBase<Actions, Impl> {
    static_assert(Min >= 0, "Negative values are not supported.");
    static_assert(Max > Min, "Max must be greater than Min.");

  public:
    virtual bool isValueSelect() const override final {
      return true;
    }

    virtual bool up() override final {
      int &value = *Impl::ptr();
      if (++value > Max) value = Min;
      return value == Min;			
    }

    virtual bool down() override final {
      int &value = *Impl::ptr();
      if (--value < Min) value = Max;
      return value == Max;
    }

    virtual char const *getLabel() const override final {
      static constexpr uint8_t numChars = countDigits(Max);
      static constexpr uint8_t bufSize = LINE_SIZE - MENU_LABEL_OFFSET + 1;
      static char buffer[bufSize];

      buffer[bufSize - 1] = '\0';
      int firstIdx = (bufSize - numChars - 1) / 2;
      int lastIdx = firstIdx + numChars - 1;

      // Leading spaces
      for (int idx = 0; idx != firstIdx; ++idx) {
        buffer[idx] = ' ';
      }

      // Number
      int value = *Impl::ptr();
      for (int idx = lastIdx; idx >= firstIdx; --idx) {
        buffer[idx] = '0' + (value % 10);
        value /= 10;
      }

      // Trailing spaces
      for (int idx = lastIdx + 1; idx != bufSize - 1; ++idx) {
        buffer[idx] = ' ';
      }

      return buffer;
    }
    
    private:
      static constexpr uint8_t countDigits(uint8_t value) {
        return (value == 0) ? 1 : (value / 10 > 0) + countDigits(value / 10);
      }

  };
}

}