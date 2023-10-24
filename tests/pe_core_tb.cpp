#include <systemc.h>
#include "../src/local_memory.h"
#include "../src/pe_core.h"
#include "../src/clk.h"

int sc_main(int argc, char* argv[]) {
    static const int ADDR_BITS = 16;
    static const int DATA_BITS = 32;
    static const int POCKET_SIZE = 8;

    sc_signal<bool>   clk;

    // Local Memory Signals
    sc_signal<bool>   local_memory_enable;
    sc_signal<bool>   local_memory_rd;
    sc_signal<bool>   local_memory_wr;
    sc_signal<sc_uint<ADDR_BITS>>  local_memory_addr;
    sc_vector<sc_signal<sc_uint<DATA_BITS>>>  local_memory_data_in{"data_in", POCKET_SIZE};
    sc_vector<sc_signal<sc_uint<DATA_BITS>>>  local_memory_data_data_out{"data_out", POCKET_SIZE};
    // Pe Core Signals
    sc_signal<bool>   pe_core_rst;
    sc_signal<bool>   pe_core_enable;
    sc_signal<bool>   pe_core_busy;

    ClockGenerator clock_gen("clock_gen");
    clock_gen.clk(clk);


    LocalMemory<ADDR_BITS, DATA_BITS, POCKET_SIZE> memory("LocalMemory");
    memory.clk(clk);
    memory.enable(local_memory_enable);
    memory.rd(local_memory_rd);
    memory.wr(local_memory_wr);
    memory.address(local_memory_addr);
    memory.data_in(local_memory_data_in);
    memory.data_out(local_memory_data_data_out);

    PeCore<ADDR_BITS,DATA_BITS,POCKET_SIZE> pe_core("PeCore");
    pe_core.clk_i(clk);
    pe_core.rst_i(pe_core_rst);
    pe_core.enable_i(pe_core_enable);
    pe_core.busy_o(pe_core_busy);
    pe_core.local_memory_addr(local_memory_addr);
    pe_core.local_memory_wr(local_memory_wr);
    pe_core.local_memory_rd(local_memory_rd);
    pe_core.local_memory_enable(local_memory_enable);
    pe_core.local_memory_data_bi(local_memory_data_data_out);
    pe_core.local_memory_data_bo(local_memory_data_in);

    sc_trace_file* tf = sc_create_vcd_trace_file("pe_core_tb");
    tf->set_time_unit(1, SC_NS);
    sc_trace(tf, clk, "clk");
    sc_trace(tf, pe_core_enable, "pe_core_enable");
    sc_trace(tf, pe_core_busy, "pe_core_busy");
    sc_trace(tf, pe_core_rst, "pe_core_rst");

    local_memory_enable = true;
    local_memory_wr = true;
    local_memory_rd = false;
    local_memory_addr = 0;
    local_memory_data_in[0] = 200;
    local_memory_data_in[1] = 8;
    sc_start(10, SC_NS);
    local_memory_addr = 2;
    local_memory_data_in[0] = 0.9477695660751321;
    local_memory_data_in[1] = -4.0961078688499155;
    local_memory_data_in[2] = 0.9937557133609726;
    local_memory_data_in[3] = -14.892351723504802;
    local_memory_data_in[4] = 0.999224345493303;
    local_memory_data_in[5] = -8.589540987991482;
    local_memory_data_in[6] = 0.9992826834871;
    local_memory_data_in[7] = -9.79067155131993;
    sc_start(10, SC_NS);
    local_memory_addr = 9;
    local_memory_data_in[0] = 0.9991566901389833;
    local_memory_data_in[1] = 6.262679709682974;
    local_memory_data_in[2] = 0.9999998737712528;
    local_memory_data_in[3] = 45.32539856128967;
    local_memory_data_in[4] = 0.9259701252780933;
    local_memory_data_in[5] = 6.072379717196129;
    local_memory_data_in[6] = 0.9993096918528903;
    local_memory_data_in[7] = -17.452001343590457;
    sc_start(10, SC_NS);
    local_memory_enable = false;
    sc_start(10, SC_NS);
    pe_core_rst = true;
    pe_core_enable = true;
    sc_start(10, SC_NS);
    pe_core_rst = false;
    sc_start(10, SC_NS);

    sc_start(100, SC_NS);
    sc_close_vcd_trace_file(tf);
    return 0;
}