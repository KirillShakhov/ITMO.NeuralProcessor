#include <systemc.h>
#include "../src/local_memory.h"
#include "../src/pe_core.h"
#include "../src/clk.h"

int sc_main(int argc, char* argv[]) {
    static const int ADDR_BITS = 16;
    static const int DATA_BITS = 32;
    static const int POCKET_SIZE = 8;

    // Local Memory Signals
    sc_signal<bool>   local_memory_enable;
    sc_signal<bool>   local_memory_rd;
    sc_signal<bool>   local_memory_wr;
    sc_signal<sc_uint<ADDR_BITS>>  local_memory_addr;
    sc_vector<sc_signal<float>>  local_memory_data_in{"local_memory_data_in", POCKET_SIZE};
    sc_vector<sc_signal<float>>  local_memory_data_out{"local_memory_data_out", POCKET_SIZE};
    // Pe Core Signals
    sc_signal<bool>   pe_core_rst;
    sc_signal<bool>   pe_core_enable;
    sc_signal<bool>   pe_core_busy;

    sc_clock clk("ck1", 10, SC_NS);

    LocalMemory<ADDR_BITS, DATA_BITS, POCKET_SIZE> memory("LocalMemory");
    memory.clk(clk);
    memory.enable(local_memory_enable);
    memory.rd(local_memory_rd);
    memory.wr(local_memory_wr);
    memory.address(local_memory_addr);
    memory.data_in(local_memory_data_in);
    memory.data_out(local_memory_data_out);

    PeCore<ADDR_BITS,DATA_BITS,POCKET_SIZE> pe_core("PeCore");
    pe_core.clk_i(clk);
    pe_core.rst_i(pe_core_rst);
    pe_core.enable_i(pe_core_enable);
    pe_core.busy_o(pe_core_busy);
    pe_core.local_memory_addr(local_memory_addr);
    pe_core.local_memory_wr(local_memory_wr);
    pe_core.local_memory_rd(local_memory_rd);
    pe_core.local_memory_enable(local_memory_enable);
    pe_core.local_memory_data_bi(local_memory_data_out);
    pe_core.local_memory_data_bo(local_memory_data_in);

    sc_trace_file* tf = sc_create_vcd_trace_file("pe_core_tb");
    tf->set_time_unit(1, SC_NS);
    sc_trace(tf, clk, "clk");
    sc_trace(tf, pe_core_enable, "pe_core_enable");
    sc_trace(tf, pe_core_busy, "pe_core_busy");
    sc_trace(tf, pe_core_rst, "pe_core_rst");
    sc_trace(tf, local_memory_enable, "local_memory_enable");
    sc_trace(tf, local_memory_wr, "local_memory_wr");
    sc_trace(tf, local_memory_rd, "local_memory_rd");
    sc_trace(tf, local_memory_addr, "local_memory_addr");
    sc_trace(tf, local_memory_data_out[0], "local_memory_data_data_out0");

    local_memory_enable = true;
    local_memory_wr = true;
    local_memory_rd = false;
    local_memory_addr = 0;
    local_memory_data_in[0] = 200;
    local_memory_data_in[1] = 16;
    sc_start(10, SC_NS);
    local_memory_addr = 2;
    local_memory_data_in[0] = 0.9990631251789995;
    local_memory_data_in[1] = 0.7550705154311167;
    local_memory_data_in[2] = 0.9950311958727136;
    local_memory_data_in[3] = 0.6481739272549456;
    local_memory_data_in[4] = 0.9965130706156015;
    local_memory_data_in[5] = -0.03327157936852162;
    local_memory_data_in[6] = 0.9987707591591141;
    local_memory_data_in[7] = -0.42765877612887726;
    sc_start(10, SC_NS);
    local_memory_addr = 10;
    local_memory_data_in[0] = 0.999039373995782;
    local_memory_data_in[1] = 0.2268078469768019;
    local_memory_data_in[2] = 0.9572634478083041;
    local_memory_data_in[3] = 0.5068606953692869;
    local_memory_data_in[4] = 0.007089524427052066;
    local_memory_data_in[5] = 0.7370486058746694;
    local_memory_data_in[6] = 0.9973003050392526;
    local_memory_data_in[7] = 0.5651521802118312;
    sc_start(10, SC_NS);
    local_memory_addr = 18;
    local_memory_data_in[0] = 0.9882247596998172;
    local_memory_data_in[1] = 0.11509488104106606;
    local_memory_data_in[2] = 0.9012400716710605;
    local_memory_data_in[3] = 1.1424770795739387;
    local_memory_data_in[4] = 0.9683287440476079;
    local_memory_data_in[5] = 3.1055546438514123;
    local_memory_data_in[6] = 0.9964592661223179;
    local_memory_data_in[7] = 0.7880725968841713;
    sc_start(10, SC_NS);
    local_memory_addr = 26;
    local_memory_data_in[0] = 0.2479846432195069;
    local_memory_data_in[1] = 0.3983616222528344;
    local_memory_data_in[2] = 0.003428754170688519;
    local_memory_data_in[3] = 0.8743129908062628;
    local_memory_data_in[4] = 0.9931021038679053;
    local_memory_data_in[5] = 0.269989261270187;
    local_memory_data_in[6] = 0.9993619882449931;
    local_memory_data_in[7] = -0.24792302708954958;

    // 0.9993096918528903

    sc_start(10, SC_NS);
    local_memory_enable = false;
    local_memory_addr = 0;
    sc_start(10, SC_NS);
    pe_core_rst = true;
    pe_core_enable = true;
    sc_start(10, SC_NS);
    pe_core_rst = false;
    sc_start(400, SC_NS);

    // Read Results
    local_memory_enable = true;
    local_memory_wr = false;
    local_memory_rd = true;
    local_memory_addr = 200;
    sc_start(10, SC_NS);
    cout << "Result: " << local_memory_data_out[0] << endl;

    sc_close_vcd_trace_file(tf);
    return 0;
}