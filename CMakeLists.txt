cmake_minimum_required(VERSION 3.0)
project(ITMO.NeuralProcessor)

find_package(SystemCLanguage CONFIG REQUIRED)
set (CMAKE_CXX_STANDARD ${SystemC_CXX_STANDARD})

link_libraries(SystemC::systemc)

add_executable(test_counter main.cpp)

add_executable(test_systemc
        src/counter.cpp
        tests/counter_tb.cpp)

add_executable(test_multiplier
        src/floating_point_multiplier.cpp
        tests/multiplier_tb.cpp)

add_executable(test_adder
        src/floating_point_adder.cpp
        tests/adder_tb.cpp)

add_executable(test_summator
        src/summator.cpp
        tests/summator_tb.cpp)
