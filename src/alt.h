#pragma once

#include "parser.h"

#include <type_traits>
#include <utility>
#include <variant>

namespace alt_infrastructure {
template <typename... Ts> struct set {};
template <typename... Ts> struct unique_variant_helper {};
template <typename T> struct unique_variant_helper<set<T>> { using type = T; };
template <typename T, typename... Ts>
struct unique_variant_helper<set<T, Ts...>> {
  using type = std::variant<T, Ts...>;
};
template <typename... Ts, typename U, typename... Us>
struct unique_variant_helper<set<Ts...>, U, Us...>
    : std::conditional_t<(... || std::is_same_v<Ts, U>),
                         unique_variant_helper<set<Ts...>, Us...>,
                         unique_variant_helper<set<Ts..., U>, Us...>> {};
template <typename... Ts, typename... Us, typename... Vs>
struct unique_variant_helper<set<Ts...>, std::variant<Us...>, Vs...>
    : unique_variant_helper<set<Ts...>, Us..., Vs...> {};
template <typename... Ts>
using unique_variant = typename unique_variant_helper<set<>, Ts...>::type;
} // namespace alt_infrastructure
template <Parser T, Parser R> constexpr auto operator|(T &&lhs, R &&rhs) {
  using namespace alt_infrastructure;
  using result_t = unique_variant<parser_result_t<T>, parser_result_t<R>>;
  return [lhs = std::forward<T>(lhs), rhs = std::forward<R>(rhs)](
             parser_input_t input) -> parser_output_t<result_t> {
    if (auto out = lhs(input)) {
      return std::make_pair(result_t{out->first}, out->second);
    } else if (auto out = rhs(input)) {
      return std::make_pair(result_t{out->first}, out->second);
    } else {
      return std::nullopt;
    }
  };
}