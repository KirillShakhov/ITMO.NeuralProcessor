#include <systemc.h>
#include "../src/memory.h"
#include "../src/clk.h"

int sc_main(int argc, char* argv[]) {
    static const int ADDR_BITS = 16;
    static const int DATA_BITS = 32;

    sc_signal<bool>   clock;
    sc_signal<bool>   enable;
    sc_signal<bool>   rd;
    sc_signal<bool>   wr;
    sc_signal<sc_uint<ADDR_BITS>> addr;
    sc_signal<sc_uint<DATA_BITS>> data_in;
    sc_signal<sc_uint<DATA_BITS>> data_out;

    ClockGenerator clock_gen("clock_gen");
    clock_gen.clk(clock);

    Memory<ADDR_BITS, DATA_BITS> memory("Memory");
    memory.clk(clock);
    memory.enable(enable);
    memory.address(addr);
    memory.data_in(data_in);
    memory.data_out(data_out);
    memory.rd(rd);
    memory.wr(wr);

    sc_trace_file* tf = sc_create_vcd_trace_file("memory_tb");
    tf->set_time_unit(1, SC_NS);
    sc_trace(tf, clock, "clock");
    sc_trace(tf, enable, "enable");
    sc_trace(tf, rd, "rd");
    sc_trace(tf, wr, "wr");
    sc_trace(tf, addr, "addr");
    sc_trace(tf, data_in, "data_in");
    sc_trace(tf, data_out, "data_out");

    enable = true;
    addr = 256;
    data_in = 432;
    wr = true;
    rd = false;
    sc_start(10, SC_NS);
    enable = true;
    addr = 256;
    data_in = 123;
    wr = true;
    rd = true;
    sc_start(10, SC_NS);
    enable = true;
    addr = 123;
    data_in = 0;
    wr = false;
    rd = true;
    sc_start(10, SC_NS);
    enable = true;
    addr = 256;
    data_in = 0;
    wr = false;
    rd = true;
    sc_start(10, SC_NS);
    enable = false;

    sc_close_vcd_trace_file(tf);
    return 0;
}