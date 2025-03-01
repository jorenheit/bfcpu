#pragma once

namespace Helper {
  template <int Bytes>
  struct GetIndexTypeFromBytes;

  template <>
  struct GetIndexTypeFromBytes<1> {
    using Type = uint8_t;
  };

  template <>
  struct GetIndexTypeFromBytes<2> {
    using Type = uint16_t;
  };

  template <>
  struct GetIndexTypeFromBytes<4> {
    using Type = uint32_t;
  };
  
  template <typename ValueType> 
  struct GetIndexType {
    using Type = typename GetIndexTypeFromBytes<sizeof(ValueType)>::Type;
  };
}

template <typename T>
struct RingBufferResult {
  T value;
  bool ok;
  operator bool() const {
    return ok; 
  }
};

template <typename, uint32_t>
class RingBuffer;

template <typename ValueType, uint32_t N, typename IndexType = typename Helper::GetIndexType<ValueType>::Type>
class RingBufferBase {
  static_assert(N <= (1 << 8 * sizeof(IndexType)), "IndexType not big enough to index RingBuffer of N elements"); 
  using Derived = RingBuffer<ValueType, N>;
  using Result = RingBufferResult<ValueType>;

  volatile ValueType buffer[N]{};
  volatile IndexType head;
  volatile IndexType tail;

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

  inline bool available() const {
    return (head != tail);
  }
};

template <typename ValueType, uint32_t N>
class RingBuffer: public RingBufferBase<ValueType, N> {
public:
  inline static size_t next(size_t const index) {
    return (index + 1) % N;
  }  
};

template <typename ValueType>
class RingBuffer<ValueType, (1UL << 8)>: public RingBufferBase<ValueType, (1UL << 8)> {
public:
  inline static size_t next(size_t const index) {
    return index + 1;
  }  
};

template <typename ValueType>
class RingBuffer<ValueType, (1UL << 16)>: public RingBufferBase<ValueType, (1UL << 16)> {
public:
  inline static size_t next(size_t const index) {
    return index + 1;
  }  
};

template <typename ValueType>
class RingBuffer<ValueType, -1UL>: public RingBufferBase<ValueType, -1UL> {
public:
  inline static size_t next(size_t const index) {
    return index + 1;
  }  
};