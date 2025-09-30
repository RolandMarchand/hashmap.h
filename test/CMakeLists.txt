add_subdirectory(unity)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
enable_testing()

add_subdirectory(out_of_mem)
add_subdirectory(pass_null_abort)
add_subdirectory(pass_null_ignore)
add_subdirectory(usual_behavior)

add_custom_target(test
  DEPENDS test_hashmap_out_of_mem test_hashmap_pass_null_abort test_hashmap_pass_null_ignore test_hashmap_usual_behavior
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test
  COMMAND ctest
)
