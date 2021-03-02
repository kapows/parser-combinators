#pragma once

#include "parser.h"
#include "primitives.h"

#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

// Bind :: Parser a -> (a -> Parser b) -> Parser b
template <Parser P, typename F> constexpr auto bind(P p, F f) {
  using result_t = parser_result_t<std::invoke_result_t<F, parser_result_t<P>>>;
  return [=](parser_input_t input) -> parser_output_t<result_t> {
    if (auto out = p(input)) {
      return f(out->first)(out->second);
    } else {
      return zero<result_t>(input);
    }
  };
}

// Map :: (a -> b) -> Parser a -> Parser b
template <typename F, Parser P> constexpr auto map(F f, P p) {
  return bind(p, [f](auto x) { return result(f(x)); });
}

// Sat :: Parser a -> (a -> bool) -> Parser a
template <Parser P, typename F> constexpr auto sat(P p, F f) {
  return bind(p, [f]<typename T>(T x) {
    return [x, b = f(x)](parser_input_t input) -> parser_output_t<T> {
      if (b) {
        return result(x)(input);
      } else {
        return zero<T>(input);
      }
    };
  });
}

namespace seq_infrastructure {
template <typename T> struct is_tuple : std::false_type {};
template <typename... Ts>
struct is_tuple<std::tuple<Ts...>> : std::true_type {};
template <typename T> inline constexpr auto is_tuple_v = is_tuple<T>::value;

template <typename T> constexpr auto make_tuple(T &&x) {
  if constexpr (is_tuple_v<std::decay_t<T>>) {
    return x;
  } else {
    return std::make_tuple(x);
  }
}
template <typename... Ts> constexpr auto tuple_cat(Ts &&...xs) {
  return std::tuple_cat(make_tuple(std::forward<Ts>(xs))...);
}
} // namespace seq_infrastructure

// Seq :: Parser a -> Parser b -> Parser (a,b)
template <Parser P, Parser Q> constexpr auto seq(P p, Q q) {
  namespace si = seq_infrastructure;
  return bind(p, [=](auto x) {
    return bind(q, [x](auto y) { return result(si::tuple_cat(x, y)); });
  });
}
template <Parser P, Parser Q> constexpr auto operator>>(P p, Q q) {
  return seq(p, q);
}

namespace either_infrastructure {
template <typename... Ts> struct set {};

template <typename... Ts>
struct unique_variant : unique_variant<set<>, Ts...> {};
template <typename... Ts> struct unique_variant<set<Ts...>> {
  using type = std::variant<Ts...>;
};
template <typename... Ts, typename U, typename... Us>
struct unique_variant<set<Ts...>, U, Us...>
    : std::conditional_t<(... || std::is_same_v<Ts, U>),
                         unique_variant<set<Ts...>, Us...>,
                         unique_variant<set<Ts..., U>, Us...>> {};

template <typename... Ts, typename... Us, typename... Vs>
struct unique_variant<set<Ts...>, std::variant<Us...>, Vs...>
    : unique_variant<set<Ts...>, Us..., Vs...> {};

template <typename... Ts>
using unique_variant_t = typename unique_variant<Ts...>::type;

template <typename R, typename T> constexpr auto make_result_helper(T x) -> R {
  return x;
}
template <typename R, typename... Ts>
constexpr auto make_result_helper(std::variant<Ts...> x) -> R {
  return std::visit([](auto y) -> R { return y; }, x);
}
template <typename R, typename T> constexpr auto make_result(T x) -> R {
  return make_result_helper<R>(x);
}
} // namespace either_infrastructure

// Either :: Parser a -> Parser a -> Parser a
// Either :: Parser a -> Parser b -> Parser (a,b)
template <Parser P, Parser Q> constexpr auto either(P p, Q q) {
  using namespace either_infrastructure;
  using result_t = std::conditional_t<
      std::is_same_v<parser_result_t<P>, parser_result_t<Q>>,
      parser_result_t<P>,
      unique_variant_t<parser_result_t<P>, parser_result_t<Q>>>;
  return [=](parser_input_t input) -> parser_output_t<result_t> {
    if (auto out = p(input)) {
      return result(make_result<result_t>(out->first))(out->second);
    } else if (auto out = q(input)) {
      return result(make_result<result_t>(out->first))(out->second);
    } else {
      return zero<result_t>(input);
    }
  };
}
template <Parser P, Parser Q> constexpr auto operator||(P p, Q q) {
  return either(p, q);
}

// Many :: Parser a -> Parser [a]
template <Parser P, typename R = parser_result_t<P>>
constexpr auto many(P p) -> parser_t<std::vector<R>> {
  return either(bind(p,
                     [p](auto x) {
                       return bind(many(p), [x](auto xs) {
                         xs.insert(xs.begin(), x);
                         return result(std::move(xs));
                       });
                     }),
                result(std::vector<R>{}));
}
template <Parser P> constexpr auto operator*(P p) { return many(p); }

// OneOrMore :: Parser a -> Parser [a]
template <Parser P, typename R = parser_result_t<P>>
constexpr auto one_or_more(P p) {
  return either(bind(seq(p, many(p)),
                     [](auto t) {
                       auto &[x, xs] = t;
                       xs.insert(xs.begin(), x);
                       return result(std::move(xs));
                     }),
                zero<std::vector<R>>);
}
template <Parser P> constexpr auto operator+(P p) { return one_or_more(p); }

// Optional :: Parser a -> Parser (Optional a)
template <Parser P, typename R = parser_result_t<P>>
constexpr auto optional(P p) {
  return either(bind(p, [](auto x) { return result<std::optional<R>>(x); }),
                result<std::optional<R>>(std::nullopt));
}
template <Parser P> constexpr auto operator-(P p) { return optional(p); }

// MakeParse :: Parser a -> (Parser a -> Parser a)
template <Parser P> constexpr auto make_parse(P p) {
  return [=]<Parser Q>(Q q) {
    return map([](auto x) { return std::get<1>(x); }, seq(p, q));
  };
}

// MakeToken :: Parser a -> (Parser a -> Parser a)
template <Parser P> constexpr auto make_token(P p) {
  return [=]<Parser Q>(Q q) {
    return map([](auto x) { return std::get<0>(x); }, seq(q, p));
  };
}