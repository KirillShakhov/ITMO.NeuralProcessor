#include <systemc.h>
#include <cmath>

template<int ADDR_BITS, int DATA_BITS, int PE_CORES>
SC_MODULE(NeuralProccesor) {
    sc_in<bool> clk_i;
    sc_in<sc_uint<ADDR_BITS>> addr_bo;
    sc_in<sc_uint<DATA_BITS>> data_bo;
    sc_out<sc_uint<DATA_BITS>> data_bi;
    sc_in<bool> wr_o;
    sc_in<bool> rd_o;

//    void sigmoid_process() {
//        float x = input.read();
//        float result = 1.0f / (1.0f + exp(-x));
//        output.write(result);
//    }

    SC_CTOR(NeuralProccesor) {


//        SC_METHOD(sigmoid_process);
//        sensitive << input;
    }
};
