#include <systemc.h>

template<int ADDR_BITS, int DATA_BITS, int OFFSET>
SC_MODULE(SharedMemory) {
    sc_in_clk clk_i;
    sc_in<bool> bus_rd;
    sc_in<bool> bus_wr;
    sc_in<sc_uint<ADDR_BITS>> bus_addr;
    sc_in<float> bus_data_in{"bus_data_in"};
    sc_out<float> bus_data_out{"bus_data_out"};

    float mem[(1 << ADDR_BITS)];
    void process() {
        if (bus_addr.read() >= OFFSET) {
            int address = bus_addr.read() - OFFSET;
            if (bus_rd.read() != 0) {
//                cout << "Shared Memory Read Addr: " << address << " Data: " << mem[address] << endl;
                bus_data_out.write(mem[address]);
            }
            if (bus_wr.read() != 0) {
                cout << "Shared Memory Save Addr: " << address << " Data: " << bus_data_in.read() << endl;
                mem[address] = bus_data_in.read();
            }
        }
    }

    SC_CTOR(SharedMemory) {
        SC_METHOD(process);
        sensitive << clk_i.pos();
    }
};
