#include <systemc.h>
#include "../src/adder_buffer.h"
#include "../src/clk.h"

int sc_main (int argc, char* argv[]) {
    sc_signal<bool>   clock;
    sc_signal<bool>   reset;
    sc_signal<float> in;
    sc_signal<float> out1;

    ClockGenerator clock_gen("clock_gen");
    clock_gen.clk(clock);

    AdderBuffer adderBuffer("AdderBuffer");
    adderBuffer.clock(clock);
    adderBuffer.reset(reset);
    adderBuffer.input(in);
    adderBuffer.output(out1);


    sc_trace_file *wf = sc_create_vcd_trace_file("summator");
    sc_trace(wf, clock, "clock");
    sc_trace(wf, reset, "reset");
    sc_trace(wf, in, "in");
    sc_trace(wf, out1, "out1");

    reset = 1;
    in   = 0;
    out1  = 0;
    sc_start(10, SC_NS);

    reset = 0;
    in   = 1;
    sc_start(10, SC_NS);
    in   = 7;
    sc_start(10, SC_NS);
    in   = 3;
    sc_start(10, SC_NS);
    in   = 5;
    sc_start(100, SC_NS);

    sc_close_vcd_trace_file(wf);
    return 0;// Terminate simulation

}
