cmake_minimum_required(VERSION 3.11)
project(cpproblem CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(cpproblem 
  "src/common.h"
  "src/consumer.h"
  "src/logger.h"
  "src/main.cpp"
  "src/manager.h"
  "src/my_manager.h"
  "src/producer.h"
  "src/task.h"
  "src/thread_safe_map.h"
  "src/thread_safe_queue.h"
)

if(MSVC)
  target_compile_options(cpproblem PRIVATE /W4 /WX)
else()
  target_compile_options(cpproblem PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()