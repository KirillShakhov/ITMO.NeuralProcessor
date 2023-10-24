#include <systemc.h>
#include "../src/sigmoid.h"

int sc_main(int argc, char* argv[]) {
    sc_signal<float> input;
    sc_signal<float> output;

    Sigmoid sigmoid("Adder");
    sigmoid.input(input);
    sigmoid.output(output);

    sc_trace_file* tf = sc_create_vcd_trace_file("sigmoid");
    tf->set_time_unit(1, SC_NS);
    sc_trace(tf, input, "input");
    sc_trace(tf, output, "output");

    input = 0.0;
    sc_start(10, SC_NS);
    input = 1.0;
    sc_start(10, SC_NS);
    input = 2.0;
    sc_start(10, SC_NS);
    input = 3.0;
    sc_start(10, SC_NS);
    input = -1.0;
    sc_start(10, SC_NS);

    sc_close_vcd_trace_file(tf);

    return 0;
}