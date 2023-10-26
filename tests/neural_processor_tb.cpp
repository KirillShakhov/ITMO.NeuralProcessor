#include <systemc.h>
#include "../src/neural_processor.h"

int sc_main(int argc, char* argv[]) {
    sc_clock clk("clk", 10, SC_NS, 0.5, 5, SC_NS, false);
    NeuralProcessor<16,32,4, 8> neuralProcessor("NeuralProcessor");
    neuralProcessor.clk_i(clk);
    sc_trace_file* tf = sc_create_vcd_trace_file("sigmoid");
    tf->set_time_unit(1, SC_NS);
    sc_start(100000, SC_NS);
    sc_close_vcd_trace_file(tf);
    return 0;
}