#include "systemc.h"

template<int ADDR_BITS, int POCKET_SIZE>
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
//            cout << "////////////////////////////" << endl;
//            for (int i = 400; i < 402+64; ++i) {
//                cout << "local_memory[" << i << "] = " << mem[i] << endl;
//            }
            for (int i = 0; i < POCKET_SIZE; ++i) {
                data_out[i].write(mem[address.read()+i]);
            }
        }
        if (wr) {
//            cout << "////////////////////////////" << endl;
//            for (int i = 400; i < 402+64; ++i) {
//                cout << "local_memory[" << i << "] = " << mem[i] << endl;
//            }
            if (single_channel){
                mem[address.read()] = data_in[0].read();
                return;
            }
            for (int i = 0; i < POCKET_SIZE; ++i) {
                mem[address.read()+i] = data_in[i].read();
            }
        }
    }

    SC_CTOR(LocalMemory) {
        SC_METHOD(proc)
        sensitive << clk;
    }
};
