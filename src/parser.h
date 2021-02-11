#pragma once

#include <concepts>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

using parser_input_t = std::string_view;
template <typename T>
using parser_output_t = std::optional<std::pair<T, parser_input_t>>;

template <typename T> concept Parser = requires(T x, parser_input_t input) {
  typename std::invoke_result_t<T, parser_input_t>::value_type::first_type;
  {static_cast<bool>(x(input))};
  {x(input)->first};
  { x(input)->second }
  ->std::convertible_to<parser_input_t>;
};
template <Parser T> struct parser_traits {
  using result_type =
      typename std::invoke_result_t<T, parser_input_t>::value_type::first_type;
};
template <Parser T>
using parser_result_t = typename parser_traits<T>::result_type;