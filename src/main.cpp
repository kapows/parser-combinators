#include "combinators.h"
#include "parser.h"
#include "primitives.h"

#include <iostream>
#include <numeric>
#include <string>
#include <tuple>

// char_ :: char -> Parser char
constexpr auto char_(char c) {
  return sat(item, [c](char d) { return c == d; });
}

// digit_ :: Parser char
constexpr auto digit_ = sat(item, [](char c) { return '0' <= c && c <= '9'; });

// to_int :: char -> int
constexpr auto to_int(char c) -> int { return c - '0'; }

// number_ :: Parser int
inline auto const number_ = map(
    [](auto xs) -> int {
      return std::transform_reduce(
          xs.begin(), xs.end(), 0,
          [](auto acc, auto x) -> int { return acc * 10 + x; }, to_int);
    },
    +digit_);

// whitespace_ :: Parser [char]
inline auto const whitespace_ = many(sat(item, std::isspace));

// parse :: Parser a -> Parser a
inline auto const parse = make_parse(whitespace_);

// token :: Parser a -> Parser a
inline auto const token = make_token(whitespace_);

extern parser_t<int> expr;
extern parser_t<int> factor;
extern parser_t<int> term;

inline parser_t<int> expr = token(map(
    [](auto x) {
      auto [y, ys] = x;
      return std::reduce(ys.begin(), ys.end(), y, [](auto acc, auto x) {
        auto [op, y] = x;
        if (op == '+') {
          return acc + y;
        } else {
          return acc - y;
        }
      });
    },
    [](parser_input_t input) {
      static auto p = factor >> *(token(char_('+') || char_('-')) >> factor);
      return p(input);
    }));
inline parser_t<int> factor = token(map(
    [](auto x) {
      auto [y, ys] = x;
      return std::reduce(ys.begin(), ys.end(), y, [](auto acc, auto x) {
        auto [op, y] = x;
        if (op == '*') {
          return acc * y;
        } else {
          return acc / y;
        }
      });
    },
    [](parser_input_t input) {
      static auto p = term >> *(token(char_('*') || char_('/')) >> term);
      return p(input);
    }));
inline parser_t<int> term = token(map(
    [](auto x) {
      if (x.index() == 0) {
        return std::get<0>(x);
      } else if (x.index() == 1) {
        auto [i, y, j] = std::get<1>(x);
        return y;
      } else {
        auto [i, y] = std::get<2>(x);
        return -y;
      }
    },
    [](parser_input_t input) {
      static auto p = number_ ||
                      (token(char_('(')) >> expr >> token(char_(')'))) ||
                      (token(char_('-')) >> term);
      return p(input);
    }));

int main() {
  auto i = parse(expr)("  -( -5 + --10  ) * -3 ");
  if (i) {
    std::cout << i->first << std::endl;
  }
  return 0;
}