#include <systemc.h>
#include <cmath>

SC_MODULE(Sigmoid) {
    sc_in<float> input;
    sc_out<float> output;

    void sigmoid_process() {
        float x = input.read();
        float result = 1.0f / (1.0f + exp(-x));
        output.write(result);
    }

    SC_CTOR(Sigmoid) {
        SC_METHOD(sigmoid_process);
        sensitive << input;
    }
};
