cmake_minimum_required(VERSION 3.10)
project(MemoryPoolUnitTest)

set(CMAKE_CXX_STANDARD 17)

set(GTEST_ROOT "/home/olbbemi/ABYSS/LIB/GTEST")

include_directories(${GTEST_ROOT}/include)
link_directories(${GTEST_ROOT}/lib)

add_executable(memory_pool_unit_test ./gtest/memory_pool_gtest.cpp util_memory_pool.cpp)
target_link_libraries(memory_pool_unit_test gtest gtest_main)
