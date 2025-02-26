#pragma once

template <typename T>
struct RingBufferResult {
  T value;
  bool ok;
  operator bool() const {
    return ok; 
  }
};

template <typename, size_t>
class RingBuffer;

template <typename ValueType, size_t N, typename IndexType = size_t>
class RingBufferBase {
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

template <typename ValueType, size_t N>
class RingBuffer: public RingBufferBase<ValueType, N> {
public:
  inline static size_t next(size_t const index) {
    return (index + 1) % N;
  }  
};

template <typename ValueType>
class RingBuffer<ValueType, 256>: public RingBufferBase<ValueType, 256, uint8_t> {
public:
  inline static size_t next(size_t const index) {
    return index + 1;
  }  
};

template <typename ValueType>
class RingBuffer<ValueType, 65535>: public RingBufferBase<ValueType, 65535, uint16_t> {
public:
  inline static size_t next(size_t const index) {
    return index + 1;
  }  
};
