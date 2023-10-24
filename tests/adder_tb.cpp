#include <systemc.h>
#include "../src/floating_point_adder.h"

int sc_main(int argc, char* argv[]) {
    sc_signal<float> input1;
    sc_signal<float> input2;
    sc_signal<float> result;

    FloatingPointAdder adder("Adder");
    adder.input1(input1);
    adder.input2(input2);
    adder.result(result);

    sc_trace_file* tf = sc_create_vcd_trace_file("waveform");
    tf->set_time_unit(1, SC_NS);

    sc_trace(tf, input1, "input1");
    sc_trace(tf, input2, "input2");
    sc_trace(tf, result, "output");

    input1 = 2.7; // Set input values
    input2 = 6.5;

    sc_start(10, SC_NS); // Simulate for 10 ns

    sc_close_vcd_trace_file(tf);

    return 0;
}