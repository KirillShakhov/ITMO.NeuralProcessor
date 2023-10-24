#include <systemc.h>

template<int N_PORTS>
SC_MODULE(AdderMMM) {
    sc_vector<sc_in<float>>  inputs{"input1", N_PORTS};
    sc_out<float>            output;

    void adder() {
        float result_value = 0;
        for (int i = 0; i < N_PORTS; ++i) {
            result_value += inputs[i].read();
        }
        output.write(result_value);
    }

    SC_CTOR(AdderMMM) {
        SC_METHOD(adder);
        for (int i = 0; i < N_PORTS; ++i) {
            sensitive << inputs[i];
        }
    }
};