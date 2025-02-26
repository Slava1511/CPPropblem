cmake_minimum_required(VERSION 3.11)
project(cpproblem CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(cpproblem 
    "src/main.cpp"
    "src/consumer_producer.h"
)

if(MSVC)
  target_compile_options(cpproblem PRIVATE /W4 /WX)
else()
  target_compile_options(cpproblem PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()