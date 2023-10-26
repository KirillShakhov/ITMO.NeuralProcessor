#include "systemc.h"

template<int ADDR_BITS, int DATA_BITS, int POCKET_SIZE>
SC_MODULE(LocalMemory) {
    sc_in<bool> clk;
    sc_in<bool> enable;
    sc_in<bool> wr;
    sc_in<bool> rd;
    sc_in<bool> single_channel;
    sc_in<sc_uint<ADDR_BITS>> address;
    sc_vector<sc_in<float>> data_in{"data_in", POCKET_SIZE};
    sc_vector<sc_out<float>> data_out{"data_out", POCKET_SIZE};

    // Локальная память
    float mem[(1 << ADDR_BITS)];

    void proc(){
        if (!enable) return;
        if (rd) {
//            for (int i = 0; i < 50; ++i) {
//                cout << "local_memory[" << i << "] = " << mem[i] << endl;
//            }

            for (int i = 0; i < POCKET_SIZE; ++i) {
                data_out[i].write(mem[address.read()+i]);
            }
        }
        if (wr) {
            cout << "Local Memory Write Addr: " << address.read() << " Data: " << data_in[0].read() << endl;
            if (single_channel){
                mem[address.read()] = data_in[0].read();
                return;
            }
            for (int i = 0; i < POCKET_SIZE; ++i) {
                mem[address.read()+i] = data_in[i].read();
            }
        }
    }

    // Конструктор модуля
    SC_CTOR(LocalMemory) {
        SC_METHOD(proc)
//        for (int i = 0; i < (2^ADDR_BITS); ++i) {
//            mem[i] = 0;
//        }
        sensitive << clk;
    }
};
