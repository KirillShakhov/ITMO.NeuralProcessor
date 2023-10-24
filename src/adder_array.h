#include <systemc.h>

SC_MODULE(AdderArray) {
    static const int N_PORTS = 2;
    sc_vector<sc_in<float>>  input1{"input1", N_PORTS};
    sc_vector<sc_in<float>>  input2{"input2", N_PORTS};
    sc_vector<sc_out<float>>  output{"output", N_PORTS};

    void adder() {
        for (int i = 0; i < N_PORTS; ++i) {
            float result_value = input1[i].read() + input2[i].read();
            output[i].write(result_value);
        }
    }

    SC_CTOR(AdderArray) {
        SC_METHOD(adder);
        for (int i = 0; i < N_PORTS; ++i) {
            sensitive << input1[i] << input2[i];
        }
    }
};