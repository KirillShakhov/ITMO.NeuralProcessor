#include <systemc.h>
#include "../src/io_module.h"

int sc_main(int argc, char* argv[]) {
    const int ADDR_BITS = 32;
    sc_signal<bool> bus_rd{"bus_rd"};
    sc_signal<bool, SC_MANY_WRITERS> bus_wr{"bus_wr"};
    sc_signal<sc_uint<ADDR_BITS>, SC_MANY_WRITERS> bus_addr{"bus_addr"};
    sc_signal<float, SC_MANY_WRITERS> bus_data_in{"bus_data_in"};
    sc_signal<float, SC_MANY_WRITERS> bus_data_out{"bus_data_out"};

    sc_clock clk("ck1", 10, SC_NS);
    IoModule<ADDR_BITS> ioModule("IoModule");
    ioModule.clk_i(clk);
    ioModule.bus_addr(bus_addr);
    ioModule.bus_wr(bus_wr);
    ioModule.bus_rd(bus_rd);
    ioModule.bus_data_in(bus_data_in);
    ioModule.bus_data_out(bus_data_out);

    sc_trace_file* tf = sc_create_vcd_trace_file("io_module_tb");
    tf->set_time_unit(1, SC_NS);
    sc_trace(tf, clk, "clk");
    sc_trace(tf, bus_addr, "bus_addr");
    sc_trace(tf, bus_wr, "bus_wr");
    sc_trace(tf, bus_rd, "bus_rd");
    sc_trace(tf, bus_data_in, "bus_data_in");
    sc_trace(tf, bus_data_out, "bus_data_out");

    sc_start(100, SC_NS);
    sc_close_vcd_trace_file(tf);
    return 0;
}