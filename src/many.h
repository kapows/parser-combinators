#pragma once

#include "parser.h"

#include <optional>
#include <utility>
#include <vector>

template <Parser T, typename Container = std::vector<parser_result_t<T>>>
constexpr auto operator*(T &&x) {
  return [x = std::forward<T>(x)](
             parser_input_t input) -> parser_output_t<Container> {
    Container os;
    auto rest = input;
    while (auto out = x(rest)) {
      os.push_back(out->first);
      rest = out->second;
    }
    return std::make_pair(std::move(os), rest);
  };
}

template <Parser T> constexpr auto operator+(T &&x) {
  return [x = std::forward<T>(x)](parser_input_t input)
             -> parser_output_t<std::vector<parser_result_t<T>>> {
    if (x(input)) {
      return (*x)(input);
    } else {
      return std::nullopt;
    }
  };
}