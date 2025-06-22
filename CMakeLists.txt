cmake_minimum_required(VERSION 3.11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(_util)

set(CMAKE_CXX_STANDARD 17)
set(SPDLOG_BUILD_TESTING ON CACHE BOOL "Enable spdlog tests")

add_subdirectory(external/googletest)
add_subdirectory(external/spdlog)

include(GoogleTest)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

foreach(file ${SRC_FILES})
    message(STATUS "Source: ${file}")
endforeach()

add_library(_util STATIC ${SRC_FILES})
target_include_directories(_util PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Link against spdlog with async support
target_link_libraries(_util PUBLIC spdlog::spdlog)

# Enable async logger (define macro to build spdlog with threading support)
target_compile_definitions(_util PRIVATE SPDLOG_COMPILED_LIB)

# Unit-Test
enable_testing()

# ADD_EXECUTE
add_executable(memory_pool_test ${CMAKE_SOURCE_DIR}/gtest/memory_pool_gtest.cpp)
#add_executable(thread_pool_test ${CMAKE_SOURCE_DIR}/gtest/thread_pool_gtest.cpp)
add_executable(singleton_test ${CMAKE_SOURCE_DIR}/gtest/singleton_gtest.cpp)

#
target_compile_definitions(singleton_test PRIVATE UNIT_TEST)

# TARGET_LINK_LIBRARY
target_link_libraries(memory_pool_test PRIVATE _util gtest)
#target_link_libraries(thread_pool_test PRIVATE _util gtest)
target_link_libraries(singleton_test PRIVATE _util gtest)

# SET_TARGET_PROPERTIY
set_target_properties(memory_pool_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)
#set_target_properties(thread_pool_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)
set_target_properties(singleton_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)

# ADD_TEST
add_test(NAME memory_pool COMMAND memory_pool_test)
#add_test(NAME thread_pool COMMAND thread_pool_test)
add_test(NAME singleton COMMAND singleton_test)
