#include <systemc.h>

SC_MODULE(MemoryModule) {
    // Ports
    sc_in<bool> clk_i;
    sc_in<bool> wr_o;         // write enable
    sc_in<bool> rd_o;         // read enable
    sc_in<sc_uint<32>> addr_bo;  // Assuming 32-bit address bus for simplicity
    sc_in<sc_uint<32>> data_bo;  // Write data bus, assuming 32-bit data
    sc_out<sc_uint<32>> data_bi; // Read data bus

    // Internal memory array (using a simple array for simplicity)
    sc_uint<32> mem[1024];  // 1024 memory locations for example

    void write_operation() {
        if (wr_o.read() == 1) {
            mem[addr_bo.read()] = data_bo.read();
        }
    }

    void read_operation() {
        if (rd_o.read() == 1) {
            data_bi.write(mem[addr_bo.read()]);
        }
    }

    SC_CTOR(MemoryModule) {
        // Registering methods with clock
        SC_METHOD(write_operation);
        sensitive << clk_i.pos();

        SC_METHOD(read_operation);
        sensitive << clk_i.pos();
    }
};

int sc_main(int argc, char* argv[]) {
    // Signals
    sc_signal<bool> clk_i, wr_o, rd_o;
    sc_signal<sc_uint<32>> addr_bo, data_bo, data_bi;

    // Instantiate the module
    MemoryModule memory("MemoryModule");
    memory.clk_i(clk_i);
    memory.wr_o(wr_o);
    memory.rd_o(rd_o);
    memory.addr_bo(addr_bo);
    memory.data_bo(data_bo);
    memory.data_bi(data_bi);

    sc_start(10, SC_NS); // Simulate for 10 ns
    addr_bo.write(10);
    data_bo.write(10);
    sc_start(10, SC_NS); // Simulate for 10 ns

    sc_start(10, SC_NS); // Simulate for 10 ns

    return 0;
}