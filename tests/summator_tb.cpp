#include <systemc.h>
#include "../src/summator.cpp"
#include "../src/clk.cpp"

int sc_main (int argc, char* argv[]) {
    sc_signal<bool>   clock;
    sc_signal<bool>   reset;
    sc_signal<float> in1;
    sc_signal<float> in2;
    sc_signal<float> out1;

    ClockGenerator clock_gen("clock_gen");
    clock_gen.clk(clock);

    SumModule sumModule("SumModule");
    sumModule.clock(clock);
    sumModule.reset(reset);
    sumModule.input1(in1);
    sumModule.input2(in2);
    sumModule.output(out1);


    sc_trace_file *wf = sc_create_vcd_trace_file("summator");
    sc_trace(wf, clock, "clock");
    sc_trace(wf, reset, "reset");
    sc_trace(wf, in1, "in1");
    sc_trace(wf, in2, "in2");
    sc_trace(wf, out1, "out1");

    reset = 1;
    in1   = 0;
    in2   = 0;
    out1  = 0;
    sc_start(10, SC_NS);

    reset = 0;
    in1   = 1;
    in2   = 2;
    sc_start(10, SC_NS);
    in1   = 7;
    in2   = 9;
    sc_start(10, SC_NS);
    in1   = 3;
    in2   = 6;
    sc_start(10, SC_NS);
    in1   = 5;
    in2   = 2;
    sc_start(100, SC_NS);

    sc_close_vcd_trace_file(wf);
    return 0;// Terminate simulation

}
