#pragma once

#include <concepts>
#include <functional>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

using parser_input_t = std::string_view;
template <typename T>
using parser_output_t = std::optional<std::pair<T, parser_input_t>>;

template <typename T> concept Parser = requires(T x, parser_input_t input) {
  {static_cast<bool>(x(input))};
  {x(input)->first};
  { x(input)->second }
  ->std::convertible_to<parser_input_t>;
};

template <Parser P>
using parser_result_t =
    decltype(std::declval<P>()(std::declval<parser_input_t>())->first);

template <typename T>
using parser_t = std::function<parser_output_t<T>(parser_input_t)>;