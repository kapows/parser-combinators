cmake_minimum_required(VERSION 3.19)

project(parser-combinators
  LANGUAGES CXX
  VERSION 0.1.0
)

add_executable(parser-combinators src/main.cpp)
target_compile_features(parser-combinators PUBLIC cxx_std_20)