cmake_minimum_required(VERSION 3.0)
project(test_systemc)

find_package(SystemCLanguage CONFIG REQUIRED)
set (CMAKE_CXX_STANDARD ${SystemC_CXX_STANDARD})

add_executable(test_counter main.cpp)
target_link_libraries(test_counter SystemC::systemc)

add_executable(test_systemc
        src/counter.cpp
        tests/counter_tb.cpp)
target_link_libraries(test_systemc SystemC::systemc)

add_executable(test_multiplier
        src/multiplier.cpp
        tests/multiplier_tb.cpp)
target_link_libraries(test_multiplier SystemC::systemc)