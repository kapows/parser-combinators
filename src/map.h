#pragma once

#include "parser.h"

#include <optional>
#include <type_traits>
#include <utility>

template <Parser T, typename F> constexpr auto operator>=(T &&x, F &&f) {
  using result_t = std::invoke_result_t<F, parser_result_t<T>>;
  return [x = std::forward<T>(x), f = std::forward<F>(f)](
             parser_input_t input) -> parser_output_t<result_t> {
    if (auto out = x(input)) {
      return std::make_pair(f(out->first), out->second);
    } else {
      return std::nullopt;
    }
  };
}