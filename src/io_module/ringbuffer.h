#pragma once

namespace Helper {

  template <bool Condition, typename IfTrue, typename IfFalse>
  struct PickType;

  template <typename IfTrue, typename IfFalse>
  struct PickType<true, IfTrue, IfFalse> {
    using Type = IfTrue;
  };

  template <typename IfTrue, typename IfFalse>
  struct PickType<false, IfTrue, IfFalse> {
    using Type = IfFalse;
  };

  template <uint32_t N> 
  struct GetIndexType {
    using Type = typename PickType<(N <= (1UL << 8)), uint8_t, typename PickType<(N < (1UL << 16)), uint16_t, uint32_t>::Type>::Type;
  };
}

template <typename T>
struct RingBufferResult {
  T value;
  bool good;
};

template <typename, uint32_t>
class RingBuffer;

template <typename ValueType, uint32_t N>
class RingBufferBase {
protected:
  using IndexType = typename Helper::GetIndexType<N>::Type;
private:
  static_assert(N <= (1 << 8 * sizeof(IndexType)), "IndexType not big enough to index RingBuffer of N elements"); 
  using Derived = RingBuffer<ValueType, N>;
  using Result = RingBufferResult<ValueType>;

  volatile ValueType buffer[N]{};
  volatile IndexType head = 0;
  volatile IndexType tail = 0;

public:
  bool put(ValueType const &val) {
    IndexType const nextHead = Derived::next(head);
    if (nextHead != tail) {
      buffer[head] = val;
      // Memory barrier agains race conditions with get().
      // This makes sure that buffer[head] is updated BEFORE head 
      // is incremented. If not, get() could retrieve the old value
      // still stored at this location.
      __asm__ __volatile__ ("" ::: "memory"); 
      head = nextHead;
      return true;
    }
    return false;
  }

  Result get() {
    if (head == tail) return Result{ValueType{}, false};
    Result result{buffer[tail], true};
    tail = Derived::next(tail);
    return result;
  }

  Result peek() const {
    return (head == tail) ? Result{ValueType{}, false} 
                          : Result{buffer[tail], true};
  }

  void clear() {
    head = 0;
    tail = 0;
  }

  inline bool available() const {
    return (head != tail);
  }
};

template <typename ValueType, uint32_t N>
class RingBuffer: public RingBufferBase<ValueType, N> {
  using IndexType = typename RingBufferBase<ValueType, N>::IndexType;
public:
  inline static IndexType next(IndexType const index) {
    return (index + 1) % static_cast<IndexType>(N);
  }  
};

template <typename ValueType>
class RingBuffer<ValueType, (1UL << 8)>: public RingBufferBase<ValueType, (1UL << 8)> {
  using IndexType = typename RingBufferBase<ValueType, (1UL << 8)>::IndexType;
public:
  inline static IndexType next(IndexType const index) {
    return index + 1;
  }  
};

template <typename ValueType>
class RingBuffer<ValueType, (1UL << 16)>: public RingBufferBase<ValueType, (1UL << 16)> {
  using IndexType = typename RingBufferBase<ValueType, (1UL << 16)>::IndexType;
public:
  inline static IndexType next(IndexType const index) {
    return index + 1;
  }  
};

template <typename ValueType>
class RingBuffer<ValueType, -1UL>: public RingBufferBase<ValueType, -1UL> {
  using IndexType = typename RingBufferBase<ValueType, -1UL>::IndexType;
public:
  inline static IndexType next(IndexType const index) {
    return index + 1;
  }  
};