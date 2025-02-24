#pragma once

namespace StaticStringImpl_ {
  template <typename T, int N, int Index, T Value, T ... Pack>
  struct Build {
    static char const *get() {
      return Build<T, N, Index + 1, Value, Value, Pack ...>::get();
    }
  };

  template <typename T, int N, T Value, T ... All>
  struct Build<T, N, N, Value, All...>  {
    static char const *get() {
      static char const result[] = {All ..., '\0'};
      return static_cast<char const *>(result);
    }
  };
}


template <char C, int N>
char const *build_static_string() {
  return StaticStringImpl_::Build<char, N, 0, C>::get();
}