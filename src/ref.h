#pragma once

#include "parser.h"

template <Parser T> auto ref(T &x) {
  return [&](parser_input_t input) { return x(input); };
}