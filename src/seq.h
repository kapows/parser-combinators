#pragma once

#include "parser.h"

#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace seq_infrastructure {
template <typename... Ts> struct set {};
template <typename Set, typename... Ts> struct flat_tuple_helper {};
template <typename... Ts> struct flat_tuple_helper<set<Ts...>> {
  using type = std::tuple<Ts...>;
};
template <typename... Ts, typename U, typename... Us>
struct flat_tuple_helper<set<Ts...>, U, Us...>
    : flat_tuple_helper<set<Ts..., U>, Us...> {};
template <typename... Ts, typename... Us, typename... Vs>
struct flat_tuple_helper<set<Ts...>, std::tuple<Us...>, Vs...>
    : flat_tuple_helper<set<Ts..., Us...>, Vs...> {};
template <typename... Ts>
using flat_tuple = typename flat_tuple_helper<set<>, Ts...>::type;

template <typename T> struct is_tuple : std::false_type {};
template <typename... Ts>
struct is_tuple<std::tuple<Ts...>> : std::true_type {};
template <typename T> inline constexpr auto is_tuple_v = is_tuple<T>::value;
template <typename T> constexpr auto make_tuple(T &&x) {
  if constexpr (is_tuple_v<std::remove_reference_t<T>>) {
    return std::forward<T>(x);
  } else {
    return std::make_tuple(std::forward<T>(x));
  }
}
} // namespace seq_infrastructure
template <Parser T, Parser U> constexpr auto operator>>(T &&lhs, U &&rhs) {
  using namespace seq_infrastructure;
  using result_t = flat_tuple<parser_result_t<T>, parser_result_t<U>>;
  return [lhs = std::forward<T>(lhs), rhs = std::forward<U>(rhs)](
             parser_input_t input) -> parser_output_t<result_t> {
    if (auto lhs_out = lhs(input)) {
      if (auto rhs_out = rhs(lhs_out->second)) {
        return std::make_pair(std::tuple_cat(make_tuple(lhs_out->first),
                                             make_tuple(rhs_out->first)),
                              rhs_out->second);
      }
    }
    return std::nullopt;
  };
}