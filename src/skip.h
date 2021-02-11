#pragma once

#include "parser.h"

#include <utility>

template <Parser T> auto skip(T &&x) {
  return [x = std::forward<T>(x)]<Parser U>(U &&y) {
    return [x = std::move(x), y = std::forward<U>(y)](
               parser_input_t input) -> decltype(y(input)) {
      auto rest = input;
      while (auto out = x(rest)) {
        rest = out->second;
      }
      return y(rest);
    };
  };
}