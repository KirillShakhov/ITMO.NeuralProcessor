#include <systemc.h>
#include <cmath>

template<int ADDR_BITS>
SC_MODULE(ControlUnit) {
    sc_in_clk clk_i;
    sc_in<bool> enable;
    sc_out<bool> bus_rd;
    sc_out<bool> bus_wr;
    sc_out<sc_uint<ADDR_BITS>> bus_addr;
    sc_out<float> bus_data_in;
    sc_in<float> bus_data_out;

    void process() {
        if (!enable) return;
        start();
    }

    void start(){
        cout << "Control Unit Started" << endl;
        stop();
    }

    void stop(){

    }

    SC_CTOR(ControlUnit) {
        SC_METHOD(process);
        sensitive << clk_i.pos();
    }
};
