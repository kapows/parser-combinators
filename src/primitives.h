#pragma once

#include "parser.h"

#include <optional>
#include <utility>

// Zero :: Parser a
template <typename T>
constexpr auto zero(parser_input_t) -> parser_output_t<T> {
  return std::nullopt;
};

// Result :: a -> Parser a
template <typename T> constexpr auto result(T x) {
  return [x](parser_input_t input) -> parser_output_t<T> {
    return std::make_pair(x, input);
  };
}

// Item :: Parser a
constexpr auto item(parser_input_t input) -> parser_output_t<char> {
  if (!input.empty()) {
    return result(input.front())(input.substr(1));
  } else {
    return zero<char>(input);
  }
};