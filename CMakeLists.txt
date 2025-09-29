cmake_minimum_required(VERSION 3.10)

project(hashmap VERSION 1.0
  DESCRIPTION "hashmap"
  LANGUAGES C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Werror=vla -Werror=implicit-function-declaration -Wno-psabi")

include(CheckCSourceCompiles)

set(CMAKE_REQUIRED_LIBRARIES asan)
check_c_source_compiles("int main() { return 0; }" HAVE_LIBASAN)
set(CMAKE_REQUIRED_LIBRARIES ubsan)
check_c_source_compiles("int main() { return 0; }" HAVE_LIBUBSAN)
set(CMAKE_REQUIRED_LIBRARIES "")

if(HAVE_LIBASAN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
else()
  message(WARNING "AddressSanitizer not available.")
endif()

if(HAVE_LIBUBSAN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined")
else()
  message(WARNING "UndefinedBehaviorSanitizer not available.")
endif()

add_executable(hashmap hashmap.c)
