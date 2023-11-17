cmake_minimum_required(VERSION 3.10)
find_library(buddyLib buddy HINTS ./lib REQUIRED)
project(bdd_example LANGUAGES CXX C)
set(target bdd_example)
set(CMAKE_CXX_STANDARD 23)
add_executable(${target}
  src/main.cpp
  include/bdd.h
  src/BDDHelper.hpp
  src/BDDHelper.cpp
)
set_target_properties(${target} PROPERTIES CXX_STANDARD 23)
target_link_libraries(${target} PUBLIC ${buddyLib})
target_include_directories(${target} PUBLIC include)
