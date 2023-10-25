cmake_minimum_required(VERSION 3.0)
project(ITMO.NeuralProcessor)

find_package(SystemCLanguage CONFIG REQUIRED)
set (CMAKE_CXX_STANDARD ${SystemC_CXX_STANDARD})

link_libraries(SystemC::systemc)

add_executable(test_counter main.cpp)

add_executable(test_systemc
        src/counter.h
        tests/counter_tb.cpp)

add_executable(test_multiplier
        src/floating_point_multiplier.h
        tests/multiplier_tb.cpp)

add_executable(test_adder
        src/floating_point_adder.h
        tests/adder_tb.cpp)

add_executable(test_adder_buffer
        src/adder_buffer.h
        tests/adder_buffer_tb.cpp)

add_executable(test_adder_array
        src/adder_array.h
        tests/adder_array_tb.cpp)

add_executable(test_multiplier_array
        src/multiplier_array.h
        tests/multiplier_array_tb.cpp)

add_executable(test_adder_mmm
        src/adder_mmm.h
        tests/adder_mmm_tb.cpp)

add_executable(test_sigmoid
        src/sigmoid.h
        tests/sigmoid_tb.cpp)

add_executable(test_neural_math
        src/neural_math.h
        tests/neural_math_tb.cpp)

add_executable(test_memory
        src/memory.h
        tests/memory_tb.cpp)

add_executable(test_bus
        src/bus_channel.cpp
        tests/bus_tb.cpp)

add_executable(test_local_memory
        src/local_memory.h
        tests/local_memory_tb.cpp)

add_executable(test_pe_core
        src/pe_core.h
        src/local_memory.h
        tests/pe_core_tb.cpp)

add_executable(test_io_module
        src/io_module.h
        tests/io_module_tb.cpp)

add_executable(test_neural_processor
        src/neural_processor.h
        tests/neural_processor_tb.cpp)
