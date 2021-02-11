#include "alt.h"
#include "many.h"
#include "map.h"
#include "parser.h"
#include "ref.h"
#include "seq.h"
#include "skip.h"

#include <functional>
#include <iostream>
#include <numeric>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

constexpr auto char_parser(char c) {
  return [c](parser_input_t input) -> parser_output_t<char> {
    if (input.starts_with(c)) {
      return std::make_pair(c, input.substr(1));
    } else {
      return std::nullopt;
    }
  };
}
constexpr auto whitespace_parser = char_parser(' ') | char_parser('\t') |
                                   char_parser('\r') | char_parser('\n');
constexpr auto digit_parser =
    (char_parser('0') | char_parser('1') | char_parser('2') | char_parser('3') |
     char_parser('4') | char_parser('5') | char_parser('6') | char_parser('7') |
     char_parser('8') | char_parser('9')) >= [](char x) { return x - '0'; };
constexpr auto number_parser = digit_parser >> *digit_parser >=
                               [](std::tuple<int, std::vector<int>> const &x) {
                                 auto &&[first, rest] = x;
                                 return std::accumulate(rest.begin(),
                                                        rest.end(), first,
                                                        [](int acc, int x) {
                                                          return acc * 10 + x;
                                                        });
                               };

int main() {

  std::function<parser_output_t<int>(parser_input_t)> expr_parser;
  std::function<parser_output_t<int>(parser_input_t)> term_parser;
  std::function<parser_output_t<int>(parser_input_t)> factor_parser;

  auto sw = skip(whitespace_parser);
  expr_parser =
      sw(ref(term_parser)) >>
      *(sw(char_parser('+') | char_parser('-')) >> sw(ref(term_parser))) >=
      [](std::tuple<int, std::vector<std::tuple<char, int>>> const &x) {
        auto value = std::get<0>(x);
        for (auto &&rhs : std::get<1>(x)) {
          switch (std::get<0>(rhs)) {
          case '+':
            value = value + std::get<1>(rhs);
            break;
          case '-':
            value = value - std::get<1>(rhs);
            break;
          }
        }
        return value;
      };
  term_parser =
      sw(ref(factor_parser)) >>
      *(sw(char_parser('*') | char_parser('/')) >> sw(ref(factor_parser))) >=
      [](std::tuple<int, std::vector<std::tuple<char, int>>> const &x) {
        auto value = std::get<0>(x);
        for (auto &&rhs : std::get<1>(x)) {
          switch (std::get<0>(rhs)) {
          case '*':
            value = value * std::get<1>(rhs);
            break;
          case '/':
            value = value / std::get<1>(rhs);
            break;
          }
        }
        return value;
      };
  factor_parser =
      sw(number_parser) |
      (sw(char_parser('(')) >> sw(ref(expr_parser)) >> sw(char_parser(')'))) >=
          [](std::variant<int, std::tuple<char, int, char>> const &x) {
            if (std::holds_alternative<int>(x)) {
              return std::get<0>(x);
            } else {
              auto [lbracket, value, rbracket] = std::get<1>(x);
              return value;
            }
          };

  auto i = expr_parser("5*(3   +  \n5)");
  if (i) {
    std::cout << i->first << std::endl;
  }
  return 0;
}